#include "device/DeviceControl.h"
#include "comm/SerialComm.h"
#include "common/Logger.h"
#include <QTimer>

bool BreakerCtrl::powerSta(int staDvcId, bool on)
{
    // TODO(现场协议待定)：构造断路器控电命令帧并经 CCO 通道下发。
    //   QByteArray frame = buildPowerFrame(staDvcId, on);
    //   if (m_serial && m_ccoDvcId >= 0) m_serial->send(CCO_GW, m_ccoDvcId, frame);
    Logger::instance().info(QString("[断路器] STA(dvcId=%1) %2电（经 CCO 发帧，命令帧待现场协议）")
                            .arg(staDvcId).arg(on ? "上" : "断"));
    return true;
}

QByteArray BreakerCtrl::buildAddNode11F1(const QByteArray &addr6, uchar seq, uchar protocol)
{
    QByteArray a = addr6.left(6);
    while (a.size() < 6) a.append('\0');      // 不足补零，保证 6 字节

    QByteArray body;                          // 控制域…协议类型（参与校验和）
    body.append(char(0x43));                  // 控制域
    body.append(QByteArray(5, '\0'));         // 地址区（下行给 CCO 全 0）
    body.append(char(seq));                   // 报文序号
    body.append(char(0x11));                  // AFN=11
    body.append(char(0x01));                  // Fn=F1 添加从节点
    body.append(char(0x00));                  // comu_rate / 保留
    body.append(char(0x01));                  // 节点数 = 1
    for (int i = 0; i < 6; ++i)               // 节点地址 6 字节，小端（显示序反转）
        body.append(a.at(5 - i));
    body.append(char(protocol));              // 协议类型（HPLC=0x03）

    uchar cs = 0;                             // 校验和 = sum(控制域…协议类型) mod 256
    for (int i = 0; i < body.size(); ++i) cs += uchar(body.at(i));

    const int total = 1 + 2 + body.size() + 1 + 1;   // 68 + 长度(2) + body + 校验 + 16
    QByteArray f;
    f.append(char(0x68));
    f.append(char(total & 0xFF));
    f.append(char((total >> 8) & 0xFF));
    f.append(body);
    f.append(char(cs));
    f.append(char(0x16));
    return f;
}

QByteArray BreakerCtrl::build02F1(const QByteArray &src6, const QByteArray &dst6,
                                  const QByteArray &inner, uchar seq, uchar protocol)
{
    QByteArray s = src6.left(6); while (s.size() < 6) s.append('\0');
    QByteArray d = dst6.left(6); while (d.size() < 6) d.append('\0');

    QByteArray body;                          // 控制域…内层帧末字节（参与校验和）
    body.append(char(0x43));                  // 控制域
    body.append(char(0x04));                  // 寻址转发标志（带源/目的地址）
    body.append(QByteArray(4, '\0'));         // 保留 00 00 00 00
    body.append(char(seq));                   // 报文序号
    for (int i = 0; i < 6; ++i) body.append(s.at(5 - i));   // 源地址(CCO)小端
    for (int i = 0; i < 6; ++i) body.append(d.at(5 - i));   // 目的地址(断路器)小端
    body.append(char(0x02));                  // AFN=02
    body.append(char(0x01));                  // Fn=F1（转发）
    body.append(char(0x00));                  // 保留
    body.append(char(protocol));              // 协议类型(HPLC=0x03)
    body.append(char(uchar(inner.size())));   // 内层帧长 L
    body.append(inner);                       // 内层 698 帧（原样）

    uchar cs = 0;                             // 校验和 = sum(控制域…内层698末字节) mod 256
    for (int i = 0; i < body.size(); ++i) cs += uchar(body.at(i));

    const int total = 1 + 2 + body.size() + 1 + 1;
    QByteArray f;
    f.append(char(0x68));
    f.append(char(total & 0xFF));
    f.append(char((total >> 8) & 0xFF));
    f.append(body);
    f.append(char(cs));
    f.append(char(0x16));
    return f;
}

DeviceControl::DeviceControl(QObject *parent) : QObject(parent) {}

void DeviceControl::emitResultAsync(DvcType t, QList<int> idList, CtrlCmdType cmd, bool ok, QList<double> params)
{
    // 异步回执：返回事件循环后再回灌，避免在脚本状态机处理栈内重入。
    QTimer::singleShot(m_ackDelayMs, this, [=]() {
        emit ctrlResult(t, idList, cmd, ok, params);
    });
}

void DeviceControl::handle(DvcType dvcType, QList<int> idList, CtrlCmdType cmd, QList<double> params)
{
    bool ok = true;
    QString desc;

    switch (cmd) {
    case CtrlCmd_SetBaudRate:
    case ReadCtrlDvc_SetBaudRate: {
        // 跟随脚本切本机串口波特率，使与 CCO 本地口保持同步。必须真改：
        //   - 升级遍历波特率(case 10)、20规范波特率协商：本就是要 CCO 真切、上位机跟切；
        //   - 组网/参数初始化兜底：超时后切 115200 探测 CCO，也要跟。
        // 目标为 STA/采集器槽位时 idList 不命中已开端口（无 STA 直连），自然空操作、无副作用。
        qint32 baud = params.isEmpty() ? 9600 : static_cast<qint32>(params.first());
        int targetId = (!idList.isEmpty()) ? idList.first() : m_ccoDvcId;
        if (m_serial) m_serial->setBaudRate(dvcType, targetId, baud);
        desc = QString("SetBaudRate(dvcId=%1) -> %2bps").arg(targetId).arg(baud);
        break;
    }
    case CtrlCmd_PowerOn_12V:
    case CtrlCmd_PowerOn_220V:
    case CtrlCmd_PowerOnAll:
        if (isCco(dvcType)) { desc = "CCO 上电：空操作（已自组网）"; }
        else if (isSta(dvcType)) {
            for (int id : idList) m_breaker.powerSta(id, true);
            desc = "STA 上电（断路器）";
        } else { desc = "上电：空操作"; }
        break;
    case CtrlCmd_PowerOff_12V:
    case CtrlCmd_PowerOff_220V:
    case CtrlCmd_PowerOffAll:
    case CtrlCmd_ModuleRST:
    case CtrlCmd_DeviceRST:
        if (isSta(dvcType)) {
            for (int id : idList) {           // 模块复位以断电再上电代替
                m_breaker.powerSta(id, false);
                m_breaker.powerSta(id, true);
            }
            desc = "STA 断电/复位（断路器）";
        } else if (isCco(dvcType)) {
            // 断路器控不了 CCO 电：裁剪该步 / 改协议复位（01F1/12F1）。此处按裁剪处理，回成功。
            desc = "CCO 自掉电/复位：断路器不可控，按裁剪处理（如需可改协议复位）";
        } else { desc = "断电/复位：空操作"; }
        break;
    case CtrlCmd_EventPinHigh:
    case CtrlCmd_EventPinLow:
        // 事件引脚：若为掉电事件，可由断 STA 电触发；其余事件待确认。此处仅记录。
        desc = QString("EventPin(%1)：建议以断 STA 电触发掉电事件（待确认）")
               .arg(cmd == CtrlCmd_EventPinHigh ? "High" : "Low");
        break;
    default:
        desc = "未知控制命令：空操作";
        break;
    }

    Logger::instance().info(QString("[controlDvc] type=%1 %2").arg(int(dvcType)).arg(desc));
    emitResultAsync(dvcType, idList, cmd, ok, params);
}
