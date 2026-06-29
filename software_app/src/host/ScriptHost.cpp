#include "host/ScriptHost.h"
#include "comm/SerialComm.h"
#include "device/DeviceControl.h"
#include "common/Logger.h"
#include "config/AppConfig.h"
#include <QException>
#include <QTimer>
#include <cstring>
#include <exception>

namespace {
bool isCcoTarget(DvcType dvcType)
{
    return dvcType == CCO_GW || dvcType == CCO_NW;
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
    if (AppConfig::instance().directCcoMode && !isCcoTarget(dvcType)) {
        Logger::instance().info(QString("Direct CCO mode ignored legacy local-device send: type=%1,dvcId=%2,len=%3")
                                .arg(int(dvcType)).arg(id).arg(datalen));
        delete[] data;
        return;
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
    if (cfg.directCcoMode) {
        // STA 上下电/复位/事件等无对应硬件 → 虚拟 ACK 即可。
        // 但"设波特率"针对 CCO 时必须真改本机串口：CCO 已通过 F0F3 切到新速率，
        // 主站不跟着切就会用旧速率发后续帧(如清除下装)，CCO 收不到 → 超时。
        if ((cmd == CtrlCmd_SetBaudRate || cmd == ReadCtrlDvc_SetBaudRate)
            && m_serial && !params.isEmpty()) {
            const qint32 baud = static_cast<qint32>(params.first());
            const int targetId = !idList.isEmpty() ? idList.first()
                                 : (!m_conc.isEmpty() ? m_conc.first().dvcId : 0);
            m_serial->setBaudRate(dvcType, targetId, baud);   // 真切本机串口（内部已"先断开再重开"）
        }
        Logger::instance().info(QString("[direct-cco] virtual controlDvc ACK type=%1 cmd=%2")
                                .arg(int(dvcType)).arg(int(cmd)));
        QTimer::singleShot(cfg.ctrlAckDelayMs, this, [=]() {
            if (!m_scriptFaulted)
                onCtrlResult(dvcType, idList, cmd, true, params);
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
