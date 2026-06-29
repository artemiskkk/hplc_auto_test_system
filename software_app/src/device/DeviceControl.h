#ifndef DEVICECONTROL_H
#define DEVICECONTROL_H
//
// controlDvc 适配（兼容层）。
// 详细设计 §4.2：SetBaudRate→设本机串口波特率；STA 上下电/复位→断路器控STA；
//   CCO 上电→空操作（已自组网）；CCO 自掉电/复位→断路器控不了，裁剪或协议复位；
//   其余空操作。所有命令均异步回 processCtrlDvcRes(isSucs=true)，避免同栈重入。
//
#include <QObject>
#include <QList>
#include <QByteArray>
#include "PublicDataStruct/commdatatype.h"

class SerialComm;

// 断路器控制子模块（命令帧格式待现场协议确认，此处预留接口）
class BreakerCtrl
{
public:
    void setChannel(SerialComm *serial, int ccoDvcId) { m_serial = serial; m_ccoDvcId = ccoDvcId; }
    // 通断指定 STA 的电源（经 CCO 通道下发断路器控电帧）
    bool powerSta(int staDvcId, bool on);

    // 构造 11F1「添加从节点」帧：把断路器/节点加入 CCO 路由（不入测试档案）。
    // addr6 为显示序的 6 字节地址，内部按小端反序填入；protocol 节点协议类型(HPLC=0x03)。
    static QByteArray buildAddNode11F1(const QByteArray &addr6, uchar seq, uchar protocol = 0x03);

    // 构造 02F1「转发」帧：3762 帧包裹内层 698 帧，转发到目的节点(断路器)。
    // src6=CCO 地址、dst6=断路器地址（均显示序，内部按小端反序填）；inner=完整内层 698 帧。
    static QByteArray build02F1(const QByteArray &src6, const QByteArray &dst6,
                                const QByteArray &inner, uchar seq, uchar protocol = 0x03);
private:
    SerialComm *m_serial = nullptr;
    int m_ccoDvcId = -1;
};

class DeviceControl : public QObject
{
    Q_OBJECT
public:
    explicit DeviceControl(QObject *parent = nullptr);

    void setSerial(SerialComm *serial)       { m_serial = serial; }
    void setCcoDvcId(int id)                  { m_ccoDvcId = id; m_breaker.setChannel(m_serial, id); }
    void setAckDelayMs(int ms)                { m_ackDelayMs = ms; }

    // 由宿主 controlDvc 转入；处理后异步发 ctrlResult。
    void handle(DvcType dvcType, QList<int> idList, CtrlCmdType cmd, QList<double> params);

signals:
    // 处理完成回执；宿主据此调用 plugin->processCtrlDvcRes(...)
    void ctrlResult(DvcType dvcType, QList<int> idList, CtrlCmdType cmd, bool isSucs, QList<double> params);

private:
    void emitResultAsync(DvcType t, QList<int> idList, CtrlCmdType cmd, bool ok, QList<double> params);
    static bool isCco(DvcType t) { return t == CCO_GW || t == CCO_NW; }
    static bool isSta(DvcType t) { return t == SingleSTA || t == ThreeSTA; }

    SerialComm *m_serial = nullptr;
    int m_ccoDvcId = -1;
    int m_ackDelayMs = 50;
    BreakerCtrl m_breaker;
};

#endif // DEVICECONTROL_H
