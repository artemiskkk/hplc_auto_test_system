#include "host/ScriptHost.h"
#include "comm/SerialComm.h"
#include "device/DeviceControl.h"
#include "device/VirtualMeter.h"
#include "common/Logger.h"
#include "config/AppConfig.h"
#include <QException>
#include <QStringList>
#include <QTimer>
#include <cstring>
#include <exception>

namespace {
bool isCcoTarget(DvcType dvcType)
{
    return dvcType == CCO_GW || dvcType == CCO_NW;
}

bool isBroadcastTime05F3(const uchar *data, int length)
{
    if (!data || length < 14)
        return false;

    int offset = 0;
    while (offset < length && data[offset] == 0xFE)
        ++offset;
    if (length - offset < 14 || data[offset] != 0x68)
        return false;

    const int frameLength = int(data[offset + 1]) | (int(data[offset + 2]) << 8);
    if (frameLength < 14 || frameLength > length - offset
            || data[offset + frameLength - 1] != 0x16)
        return false;

    return data[offset + 10] == 0x05
            && data[offset + 11] == 0x04
            && data[offset + 12] == 0x00;
}

bool isStaIdWrite645(const uchar *data, int length)
{
    if (!data || length < 16)
        return false;

    int offset = 0;
    while (offset < length && data[offset] == 0xFE)
        ++offset;
    if (length - offset < 16 || data[offset] != 0x68)
        return false;

    const uchar *frame = data + offset;
    if (frame[7] != 0x68 || frame[8] != 0x14)
        return false;

    const int dataLength = int(frame[9]);
    const int frameLength = dataLength + 12;
    if (frameLength > length - offset || frame[frameLength - 1] != 0x16)
        return false;

    const bool chipId = dataLength == 28
            && frame[10] == 0x33 && frame[11] == 0x33
            && frame[12] == 0x23 && frame[13] == 0x23;
    const bool moduleId = dataLength == 15
            && frame[10] == 0x34 && frame[11] == 0x33
            && frame[12] == 0x23 && frame[13] == 0x23;
    return chipId || moduleId;
}

bool isStaIdRead645(const uchar *data, int length)
{
    if (!data || length < 13)
        return false;

    int offset = 0;
    while (offset < length && data[offset] == 0xFE)
        ++offset;
    if (length - offset < 13)
        return false;

    const uchar *frame = data + offset;
    return frame[0] == 0x68 && frame[7] == 0x68
            && frame[8] == 0x1F && frame[9] == 0x01
            && (frame[10] == 0x01 || frame[10] == 0x02)
            && frame[12] == 0x16;
}

QString exceptionText(const char *where, const QString &detail)
{
    return QString("Script exception in %1: %2").arg(QString::fromLatin1(where), detail);
}

QString exceptionText(const char *where, const std::exception &e)
{
    return exceptionText(where, QString::fromLocal8Bit(e.what()));
}

void stopPluginQuietly(AbstractPluginScript *plugin)
{
    if (!plugin) return;
    try {
        plugin->stop();
    } catch (...) {
        Logger::instance().error("Script stop failed while handling exception");
    }
}

QString meterArchiveSummary(const QList<MeterInfo> &meters)
{
    QStringList parts;
    for (const MeterInfo &meter : meters) {
        const QString addr = QString(QByteArray(reinterpret_cast<const char*>(meter.mtrAddr), 6).toHex().toUpper());
        parts << QString("id=%1 addr=%2 type=%3 prtcl=%4 dvcId=%5")
                    .arg(meter.mtrID)
                    .arg(addr)
                    .arg(int(meter.slotPosition))
                    .arg(int(meter.prtcl))
                    .arg(meter.dvcId);
    }
    return parts.join("; ");
}
}

ScriptHost::ScriptHost(QObject *parent) : QObject(parent) {}

ScriptHost::~ScriptHost()
{
    if (m_loader.isLoaded()) m_loader.unload();
}

void ScriptHost::setDeviceControl(DeviceControl *d)
{
    m_devctrl = d;
    if (m_devctrl)
        connect(m_devctrl, &DeviceControl::ctrlResult, this, &ScriptHost::onCtrlResult);
}

bool ScriptHost::loadPlugin(const QString &path, QString &err)
{
    m_loader.setFileName(path);
    QObject *obj = m_loader.instance();
    if (!obj) {
        err = QString("加载插件失败：%1（%2）").arg(path, m_loader.errorString());
        return false;
    }
    m_plugin = qobject_cast<AbstractPluginScript*>(obj);
    if (!m_plugin) {
        err = QString("插件未实现 AbstractPluginScript 接口：%1").arg(path);
        return false;
    }
    Logger::instance().info(QString("脚本插件已加载：%1").arg(path));
    return true;
}

bool ScriptHost::runCase(const QString &className,
                         const QList<ConcentratorInfo> &conc,
                         const QList<MeterInfo> &meter,
                         const QList<SchemeCfgInfo> &scheme,
                         int freqEncoded,
                         const QMap<QString,QString> &para,
                         QString &err)
{
    if (!m_plugin) { err = "插件未加载"; return false; }
    if (conc.isEmpty()) { err = "方案未关联集中器档案"; return false; }

    m_conc = conc; m_meter = meter; m_scheme = scheme; m_para = para;
    m_scriptFaulted = false;
    const AppConfig &cfg = AppConfig::instance();
    if (cfg.directCcoMode) {
        if (cfg.skipScriptBuildNetwork)
            m_para.insert("needBuildNet", "false");
        if (cfg.skipScriptPowerOff)
            m_para.insert("needPowerOff", "false");
    }

    // 设备控制层需知道 CCO 的 dvcId（断路器经 CCO 通道下发）
    if (m_devctrl) m_devctrl->setCcoDvcId(m_conc.first().dvcId);

    // 注意：不要在升级用例开头强制把波特率压回 9600。
    // 仅“成功完整升级”后 CCO 才会重启回 9600；边界/错误注入类升级测试不会真正升级成功，
    // CCO 仍停在上一条用例切到的高速档。开头强制 9600 会与高速档的 CCO 失配，导致 10F21
    // 查拓扑超时、误入“重新组网”分支。正确的回 9600 时机是“升级包传输完成、CCO 即将重启”那一刻
    // （由脚本在 FileTransfer 完成处处理），故此处保持本机串口沿用上一条用例结束时的波特率。

    m_plugin->setHost(this);
    m_plugin->setScript(className);
    Logger::instance().info(QString("addAddrsInfo 表档案：count=%1 %2")
                            .arg(m_meter.size())
                            .arg(meterArchiveSummary(m_meter)));
    m_plugin->addAddrsInfo(&m_conc, &m_meter, &m_scheme, freqEncoded);
    try {
        m_plugin->config(&m_para);
    } catch (const std::exception &e) {
        err = exceptionText("config", e);
        Logger::instance().error(err);
        m_scriptFaulted = true;
        stopPluginQuietly(m_plugin);
        return false;
    } catch (const QException &) {
        err = exceptionText("config", "QException");
        Logger::instance().error(err);
        m_scriptFaulted = true;
        stopPluginQuietly(m_plugin);
        return false;
    } catch (...) {
        err = exceptionText("config", "unknown exception");
        Logger::instance().error(err);
        m_scriptFaulted = true;
        stopPluginQuietly(m_plugin);
        return false;
    }

    Logger::instance().info(QString("启动用例脚本：%1").arg(className));
    try {
        m_plugin->execute();
    } catch (const std::exception &e) {
        err = exceptionText("execute", e);
        Logger::instance().error(err);
        m_scriptFaulted = true;
        stopPluginQuietly(m_plugin);
        return false;
    } catch (const QException &) {
        err = exceptionText("execute", "QException");
        Logger::instance().error(err);
        m_scriptFaulted = true;
        stopPluginQuietly(m_plugin);
        return false;
    } catch (...) {
        err = exceptionText("execute", "unknown exception");
        Logger::instance().error(err);
        m_scriptFaulted = true;
        stopPluginQuietly(m_plugin);
        return false;
    }
    return true;
}

void ScriptHost::stopCase()
{
    m_scriptFaulted = true;
    if (m_plugin) m_plugin->stop();
}

// —— AbstractScriptHost 实现 ——

void ScriptHost::sendMsg2Dvc(DvcType dvcType, int id, uchar *data, int datalen)
{
    const AppConfig &cfg = AppConfig::instance();
    const bool directStaIdAccess = (cfg.directCcoMode || cfg.virtualMeterSerialEnabled)
            && (isStaIdWrite645(data, datalen) || isStaIdRead645(data, datalen));
    if ((cfg.directCcoMode || cfg.virtualMeterSerialEnabled) && !isCcoTarget(dvcType)
            && !directStaIdAccess) {
        Logger::instance().info(QString("Virtual meter/direct CCO mode ignored legacy local-device send: type=%1,dvcId=%2,len=%3")
                                .arg(int(dvcType)).arg(id).arg(datalen));
        delete[] data;
        return;
    }
    if (directStaIdAccess && m_serial) {
        const bool switched = m_serial->setBaudRate(dvcType, id, 2400);
        Logger::instance().info(QString("[direct-cco] direct STA ID 645 access type=%1,dvcId=%2 -> 2400: %3")
                                .arg(int(dvcType)).arg(id)
                                .arg(switched ? "success" : "failed"));
    }
    if (cfg.virtualMeterSerialEnabled && m_serial && isCcoTarget(dvcType)
            && isBroadcastTime05F3(data, datalen)) {
        bool mapped = false;
        for (const MeterInfo &meter : m_meter) {
            if (meter.slotPosition != SingleMeter)
                continue;
            mapped = m_serial->setBaudRate(meter.slotPosition, meter.dvcId, 2400);
            Logger::instance().info(QString("[direct-cco] 05F3 broadcast maps virtual meter type=%1,dvcId=%2 -> 2400: %3")
                                    .arg(int(meter.slotPosition)).arg(meter.dvcId)
                                    .arg(mapped ? "success" : "failed"));
            break;
        }
        if (!mapped)
            Logger::instance().error("[direct-cco] 05F3 broadcast cannot map a SingleMeter endpoint");
    }
    // 下行：data 由脚本 new uchar[] 分配，宿主发送后 delete[]（详见详细设计 §4.1）
    // 注意：不要在此对 01F1/01F2 复位帧自动切 9600。CCO 收到复位帧是先在“当前波特率”回确认、
    // 再重启回 9600；若发完立即切 9600，会在高速档下错过那条确认（路由复位等用例据此判超时）。
    // CCO 重启后的 9600 由脚本自身的 QueryVersion 波特率 toggle 兜底，宿主不应越权猜测。
    if (m_serial && data && datalen > 0)
        m_serial->send(dvcType, id, QByteArray(reinterpret_cast<const char*>(data), datalen));
    delete[] data;
}

void ScriptHost::controlDvc(DvcType dvcType, QList<int> idList, CtrlCmdType cmd, QList<double> params)
{
    const AppConfig &cfg = AppConfig::instance();
    if (cfg.directCcoMode || cfg.virtualMeterSerialEnabled) {
        // STA 上下电/复位/事件等无对应硬件 -> 虚拟 ACK 即可。
        // SingleSTA/ThreeSTA 的波特率表示物理 STA 与表之间的串口速率，因此需要
        // 明确映射到本次档案中的 SingleMeter 端点；不能仅凭相同 dvcId 猜端口。
        if ((cmd == CtrlCmd_SetBaudRate || cmd == ReadCtrlDvc_SetBaudRate)
            && m_serial && !params.isEmpty()
            && (dvcType == CCO_GW || dvcType == CCO_NW
                || dvcType == SingleMeter || dvcType == ThreeMeter)) {
            const qint32 baud = static_cast<qint32>(params.first());
            const int targetId = !idList.isEmpty() ? idList.first()
                                 : (!m_conc.isEmpty() ? m_conc.first().dvcId : 0);
            m_serial->setBaudRate(dvcType, targetId, baud);   // 真切本机串口（内部已"先断开再重开"）
        } else if ((cmd == CtrlCmd_SetBaudRate || cmd == ReadCtrlDvc_SetBaudRate)
                   && m_serial && !params.isEmpty()
                   && cfg.virtualMeterSerialEnabled
                   && (dvcType == SingleSTA || dvcType == ThreeSTA)) {
            const qint32 baud = static_cast<qint32>(params.first());
            if (m_virtualMeter && !m_virtualMeter->acceptsLegacyFixtureBaudControl()) {
                Logger::instance().info(QString("[direct-cco] TC_00117 ignore legacy %1 baud %2; virtual meter port keeps configured baud")
                                        .arg(dvcType == ThreeSTA ? "ThreeSTA" : "SingleSTA")
                                        .arg(baud));
            } else {
                bool mapped = false;
                for (const MeterInfo &meter : m_meter) {
                    if (meter.slotPosition != SingleMeter)
                        continue;
                    mapped = m_serial->setBaudRate(meter.slotPosition, meter.dvcId, baud);
                    Logger::instance().info(QString("[direct-cco] map %1 baud to virtual meter type=%2,dvcId=%3 -> %4: %5")
                                            .arg(dvcType == ThreeSTA ? "ThreeSTA" : "SingleSTA")
                                            .arg(int(meter.slotPosition)).arg(meter.dvcId).arg(baud)
                                            .arg(mapped ? "success" : "failed"));
                    break;
                }
                if (!mapped && m_meter.isEmpty()) {
                    Logger::instance().error(QString("[direct-cco] cannot map %1 baud %2: virtual meter archive is empty")
                                             .arg(dvcType == ThreeSTA ? "ThreeSTA" : "SingleSTA")
                                             .arg(baud));
                }
            }
        } else if ((cmd == CtrlCmd_SetBaudRate || cmd == ReadCtrlDvc_SetBaudRate)
                   && !params.isEmpty()) {
            Logger::instance().info(QString("[direct-cco] ignore legacy fixture baud change type=%1,dvcId=%2 -> %3; virtual meter port keeps its configured baud")
                                    .arg(int(dvcType))
                                    .arg(idList.isEmpty() ? 0 : idList.first())
                                    .arg(params.first()));
        }
        DvcType eventDvcType = SingleMeter;
        int eventDvcId = 0;
        bool triggerVirtualEvent = false;
        if (cmd == CtrlCmd_EventPinHigh && cfg.virtualMeterSerialEnabled && m_virtualMeter) {
            for (const MeterInfo &meter : m_meter) {
                if (meter.slotPosition != SingleMeter && meter.slotPosition != ThreeMeter)
                    continue;
                eventDvcType = meter.slotPosition;
                eventDvcId = meter.dvcId;
                triggerVirtualEvent = true;
                break;
            }
        }

        Logger::instance().info(QString("[direct-cco] virtual controlDvc ACK type=%1 cmd=%2")
                                .arg(int(dvcType)).arg(int(cmd)));
        QTimer::singleShot(cfg.ctrlAckDelayMs, this, [=]() {
            if (!m_scriptFaulted) {
                onCtrlResult(dvcType, idList, cmd, true, params);
                if (triggerVirtualEvent && !m_scriptFaulted
                    && !m_virtualMeter->triggerActiveEvent(eventDvcType, eventDvcId)) {
                    Logger::instance().error("[虚拟表] 事件触发失败：当前用例或虚拟表串口不可用");
                }
            }
        });
        return;
    }

    if (m_devctrl)
        m_devctrl->handle(dvcType, idList, cmd, params);
}

void ScriptHost::updateProgress(ProcessState state, QString desc)
{
    const bool verboseRawRecv =
        desc.startsWith(QStringLiteral("收到报文")) ||
        desc.startsWith(QStringLiteral("收到路由报文")) ||
        desc.startsWith(QStringLiteral("收到模块报文")) ||
        desc.contains(QStringLiteral(" 收到报文"));

    if (desc.startsWith(QStringLiteral("提取到完整3762帧")) || verboseRawRecv)
        return;

    const bool terminalState = state == ProcessState_Success
            || state == ProcessState_Failed
            || state == ProcessState_Error;
    if (terminalState)
        m_scriptFaulted = true;

    Logger::instance().info(QString("[进度] %1").arg(desc));
    emit progress(static_cast<int>(state), desc);
}

void ScriptHost::recordLog(LogItem *log)
{
    Logger::instance().record(log);
}

void ScriptHost::onBytesReceived(DvcType dvcType, int id, QByteArray data)
{
    if (!m_plugin || m_scriptFaulted || data.isEmpty()) return;
    // 上行：data 由宿主 new uchar[] 分配，脚本处理后 delete[]（详见详细设计 §4.1）
    uchar *buf = new uchar[static_cast<uint>(data.size())];
    std::memcpy(buf, data.constData(), static_cast<size_t>(data.size()));
    try {
        m_plugin->processMsg(dvcType, id, buf, data.size());
    } catch (const std::exception &e) {
        const QString msg = exceptionText("processMsg", e);
        Logger::instance().error(msg);
        m_scriptFaulted = true;
        stopPluginQuietly(m_plugin);
        emit progress(static_cast<int>(ProcessState_Error), msg);
    } catch (const QException &) {
        const QString msg = exceptionText("processMsg", "QException");
        Logger::instance().error(msg);
        m_scriptFaulted = true;
        stopPluginQuietly(m_plugin);
        emit progress(static_cast<int>(ProcessState_Error), msg);
    } catch (...) {
        const QString msg = exceptionText("processMsg", "unknown exception");
        Logger::instance().error(msg);
        m_scriptFaulted = true;
        stopPluginQuietly(m_plugin);
        emit progress(static_cast<int>(ProcessState_Error), msg);
    }
}

void ScriptHost::onCtrlResult(DvcType dvcType, QList<int> idList, CtrlCmdType cmd, bool isSucs, QList<double> params)
{
    if (!m_plugin || m_scriptFaulted) return;
    try {
        m_plugin->processCtrlDvcRes(dvcType, idList, cmd, isSucs, params);
    } catch (const std::exception &e) {
        const QString msg = exceptionText("processCtrlDvcRes", e);
        Logger::instance().error(msg);
        m_scriptFaulted = true;
        stopPluginQuietly(m_plugin);
        emit progress(static_cast<int>(ProcessState_Error), msg);
    } catch (const QException &) {
        const QString msg = exceptionText("processCtrlDvcRes", "QException");
        Logger::instance().error(msg);
        m_scriptFaulted = true;
        stopPluginQuietly(m_plugin);
        emit progress(static_cast<int>(ProcessState_Error), msg);
    } catch (...) {
        const QString msg = exceptionText("processCtrlDvcRes", "unknown exception");
        Logger::instance().error(msg);
        m_scriptFaulted = true;
        stopPluginQuietly(m_plugin);
        emit progress(static_cast<int>(ProcessState_Error), msg);
    }
}
