#ifndef SERIALCOMM_H
#define SERIALCOMM_H
//
// 串口通信模块。
// 设计要点：CCO 直连通道按 376.2 长度字段拼完整帧后上送，其他通道仍透传原始字节。
//
#include <QObject>
#include <QMap>
#include <QByteArray>
#include <QtSerialPort/QSerialPort>
#include "PublicDataStruct/commdatatype.h"

struct SerialEndpoint {
    int        dvcId   = 0;
    DvcType    dvcType = CCO_GW;   // 该端口归属的设备类型（回灌 processMsg 用）
    QString    portName;
    uint       baudRate = 115200;
    QSerialPort::DataBits dataBits = QSerialPort::Data8;
    QSerialPort::StopBits stopBits = QSerialPort::OneStop;
    QSerialPort::Parity   parity   = QSerialPort::NoParity;
};

class SerialComm : public QObject
{
    Q_OBJECT
public:
    explicit SerialComm(QObject *parent = nullptr);
    ~SerialComm();

    // 打开一组端点；返回是否全部成功，失败信息见 err。
    bool openPorts(const QList<SerialEndpoint> &eps, QString &err);
    void closePorts();
    bool isOpen(int dvcId) const;
    bool isOpen(DvcType dvcType, int dvcId) const;
    qint32 baudRate(DvcType dvcType, int dvcId) const;

    // 下行：按 (dvcType,id=dvcId) 选端口发送原始报文。
    void send(DvcType dvcType, int id, const QByteArray &frame);

    // 运行期改本机串口波特率（controlDvc 的 SetBaudRate 落地）。
    bool setBaudRate(int dvcId, qint32 baud);
    bool setBaudRate(DvcType dvcType, int dvcId, qint32 baud);

signals:
    // 上行：CCO 通道为完整 376.2 帧，其他通道为原始字节。
    void bytesReceived(DvcType dvcType, int id, QByteArray data);
    void portError(DvcType dvcType, int dvcId, const QString &msg);

private slots:
    void onReadyRead();
    void onError(QSerialPort::SerialPortError e);

private:
    QString endpointKey(DvcType dvcType, int dvcId) const;
    QSerialPort* portByEndpoint(DvcType dvcType, int dvcId) const;
    QSerialPort* firstPortByDvcId(int dvcId) const;
    QSerialPort* firstCcoPort() const;

    QMap<QString, QSerialPort*> m_ports; // "type:id" -> port，允许 CCO/STA 共用同一 dvcId
    QMap<QSerialPort*, SerialEndpoint> m_eps;
    QMap<QSerialPort*, QByteArray> m_rxBuffers;
};

#endif // SERIALCOMM_H
