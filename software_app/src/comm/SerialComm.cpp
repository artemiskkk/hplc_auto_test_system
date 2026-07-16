#include "comm/SerialComm.h"
#include "common/Logger.h"
#include <QDebug>
#include <QElapsedTimer>
#include <QThread>

namespace {
bool isCcoEndpoint(DvcType dvcType)
{
    return dvcType == CCO_GW || dvcType == CCO_NW;
}

int frame3762Length(const QByteArray &buf)
{
    if (buf.size() < 3)
        return 0;
    const int lo = static_cast<unsigned char>(buf.at(1));
    const int hi = static_cast<unsigned char>(buf.at(2));
    return lo | (hi << 8);
}

}

SerialComm::SerialComm(QObject *parent) : QObject(parent) {}
SerialComm::~SerialComm() { closePorts(); }

bool SerialComm::openPorts(const QList<SerialEndpoint> &eps, QString &err)
{
    closePorts();
    for (const SerialEndpoint &ep : eps) {
        QSerialPort *p = new QSerialPort(this);
        p->setPortName(ep.portName);
        p->setBaudRate(static_cast<qint32>(ep.baudRate));
        p->setDataBits(ep.dataBits);
        p->setStopBits(ep.stopBits);
        p->setParity(ep.parity);
        p->setFlowControl(QSerialPort::NoFlowControl);
        if (!p->open(QIODevice::ReadWrite)) {
            err = QString("打开串口失败 %1：%2").arg(ep.portName, p->errorString());
            p->deleteLater();
            closePorts();
            return false;
        }
        connect(p, &QSerialPort::readyRead, this, &SerialComm::onReadyRead);
        connect(p, &QSerialPort::errorOccurred, this, &SerialComm::onError);
        const QString key = endpointKey(ep.dvcType, ep.dvcId);
        if (m_ports.contains(key)) {
            err = QString("串口配置重复：type=%1,dvcId=%2").arg(int(ep.dvcType)).arg(ep.dvcId);
            p->close();
            p->deleteLater();
            closePorts();
            return false;
        }
        m_ports.insert(key, p);
        m_eps.insert(p, ep);
        m_rxBuffers.insert(p, QByteArray());
        Logger::instance().info(QString("串口已打开 %1 @%2bps (dvcId=%3,type=%4)")
                                .arg(ep.portName).arg(ep.baudRate).arg(ep.dvcId).arg(int(ep.dvcType)));
    }
    return true;
}

void SerialComm::closePorts()
{
    for (QSerialPort *p : m_ports.values()) {
        if (p->isOpen()) p->close();
        p->deleteLater();
    }
    m_ports.clear();
    m_eps.clear();
    m_rxBuffers.clear();
}

bool SerialComm::isOpen(int dvcId) const
{
    QSerialPort *p = firstPortByDvcId(dvcId);
    return p && p->isOpen();
}

bool SerialComm::isOpen(DvcType dvcType, int dvcId) const
{
    QSerialPort *p = portByEndpoint(dvcType, dvcId);
    return p && p->isOpen();
}

qint32 SerialComm::baudRate(DvcType dvcType, int dvcId) const
{
    QSerialPort *p = portByEndpoint(dvcType, dvcId);
    return p ? p->baudRate() : 0;
}

QString SerialComm::endpointKey(DvcType dvcType, int dvcId) const
{
    return QString("%1:%2").arg(int(dvcType)).arg(dvcId);
}

QSerialPort* SerialComm::portByEndpoint(DvcType dvcType, int dvcId) const
{
    return m_ports.value(endpointKey(dvcType, dvcId), nullptr);
}

QSerialPort* SerialComm::firstPortByDvcId(int dvcId) const
{
    for (QSerialPort *p : m_eps.keys()) {
        if (m_eps.value(p).dvcId == dvcId)
            return p;
    }
    return nullptr;
}

QSerialPort* SerialComm::firstCcoPort() const
{
    for (QSerialPort *p : m_eps.keys()) {
        if (isCcoEndpoint(m_eps.value(p).dvcType))
            return p;
    }
    return nullptr;
}

void SerialComm::send(DvcType dvcType, int id, const QByteArray &frame)
{
    QSerialPort *p = portByEndpoint(dvcType, id);
    if (!p && isCcoEndpoint(dvcType))
        p = firstCcoPort();
    if (!p || !p->isOpen()) {
        Logger::instance().error(QString("发送失败：type=%1,dvcId=%2 串口未打开").arg(int(dvcType)).arg(id));
        return;
    }
    p->write(frame);
    Logger::instance().msg(QString(">> type=%1,dvc%2 发送: %3")
                            .arg(int(dvcType)).arg(id)
                            .arg(QString(frame.toHex(' ').toUpper())));
}

bool SerialComm::setBaudRate(int dvcId, qint32 baud)
{
    QSerialPort *p = firstPortByDvcId(dvcId);
    if (!p) return false;
    const SerialEndpoint ep = m_eps.value(p);
    return setBaudRate(ep.dvcType, dvcId, baud);
}

bool SerialComm::setBaudRate(DvcType dvcType, int dvcId, qint32 baud)
{
    QSerialPort *p = portByEndpoint(dvcType, dvcId);
    if (!p && isCcoEndpoint(dvcType))
        p = firstCcoPort();
    if (!p) return false;
    if (p->baudRate() == baud) {   // 同波特率，无需动口
        Logger::instance().info(QString("设置本机串口波特率 type=%1,dvcId=%2 -> %3 (无变化)")
                                .arg(int(dvcType)).arg(dvcId).arg(baud));
        return true;
    }
    // 部分 USB 转串(CH340/PL2303 等)对"运行中热改波特率"支持差，会进异常态/重枚举，
    // 故采用 先 close → 设波特率 → 重开 的方式，让驱动以新波特率重新初始化（COM 号不变）。
    const bool wasOpen = p->isOpen();
    if (wasOpen) {
        const qint64 pendingBytes = p->bytesToWrite();
        const qint64 currentBaud = p->baudRate();
        // Use 12 bits/byte as a conservative upper bound (start + 8 data +
        // parity + up to 2 stop bits), then leave 20 ms for the USB/UART FIFO.
        const qint64 minWireMs = pendingBytes > 0 && currentBaud > 0
            ? ((pendingBytes * 12 * 1000 + currentBaud - 1) / currentBaud) + 20
            : 0;
        const qint64 drainTimeoutMs = qMax<qint64>(500, minWireMs + 300);
        QElapsedTimer drainTimer;
        drainTimer.start();
        while (p->bytesToWrite() > 0 && drainTimer.elapsed() < drainTimeoutMs) {
            const int waitMs = int(qMin<qint64>(100, drainTimeoutMs - drainTimer.elapsed()));
            if (waitMs <= 0)
                break;
            p->waitForBytesWritten(waitMs);
        }
        if (p->bytesToWrite() > 0) {
            Logger::instance().error(QString("切换波特率前发送队列未排空：type=%1,dvcId=%2,remaining=%3")
                                     .arg(int(dvcType)).arg(dvcId).arg(p->bytesToWrite()));
            return false;
        }
        if (drainTimer.elapsed() < minWireMs)
            QThread::msleep(static_cast<unsigned long>(minWireMs - drainTimer.elapsed()));
        if (pendingBytes > 0) {
            Logger::instance().info(QString("切换波特率前发送已排空：type=%1,dvcId=%2,bytes=%3,elapsed=%4ms")
                                    .arg(int(dvcType)).arg(dvcId).arg(pendingBytes).arg(drainTimer.elapsed()));
        }
        p->close();
    }
    p->setBaudRate(baud);
    bool ok = true;
    if (wasOpen) {
        ok = p->open(QIODevice::ReadWrite);
        if (ok) {
            p->clear();                            // 清 OS 收发缓冲里切换瞬间的乱字节
            if (m_rxBuffers.contains(p)) m_rxBuffers[p].clear();   // 清帧累积缓冲
        }
    }
    const SerialEndpoint ep = m_eps.value(p);
    Logger::instance().info(QString("设置本机串口波特率 type=%1,dvcId=%2,port=%3 -> %4 %5(先断开再重开)")
                            .arg(int(ep.dvcType)).arg(ep.dvcId).arg(ep.portName)
                            .arg(baud).arg(ok ? "成功" : "失败：" + p->errorString()));
    return ok;
}

void SerialComm::onReadyRead()
{
    QSerialPort *p = qobject_cast<QSerialPort*>(sender());
    if (!p) return;
    const SerialEndpoint ep = m_eps.value(p);
    QByteArray data = p->readAll();
    if (data.isEmpty()) return;

    if (!isCcoEndpoint(ep.dvcType)) {
        Logger::instance().msg(QString("<< type=%1,dvc%2 接收: %3")
                               .arg(int(ep.dvcType)).arg(ep.dvcId)
                               .arg(QString(data.toHex(' ').toUpper())));
        emit bytesReceived(ep.dvcType, ep.dvcId, data);
        return;
    }

    QByteArray &buf = m_rxBuffers[p];
    buf.append(data);

    while (!buf.isEmpty()) {
        const int start = buf.indexOf(char(0x68));
        if (start < 0) {
            // 无 0x68 起始的杂散字节（多为切波特率瞬间噪声）：静默丢弃，仅留调试输出，不刷 UI 日志
            qDebug() << "dvc" << ep.dvcId << "丢弃非376.2数据:" << buf.toHex(' ').toUpper();
            buf.clear();
            return;
        }
        if (start > 0) {
            qDebug() << "dvc" << ep.dvcId << "丢弃帧前杂散数据:" << buf.left(start).toHex(' ').toUpper();
            buf.remove(0, start);
        }

        if (buf.size() < 3)
            return;

        const int frameLen = frame3762Length(buf);
        if (frameLen < 6 || frameLen > 4096) {
            qDebug() << "dvc" << ep.dvcId << "376.2长度异常" << frameLen << "跳过起始字节";
            buf.remove(0, 1);
            continue;
        }

        if (buf.size() < frameLen)
            return;

        QByteArray frame = buf.left(frameLen);
        if (static_cast<unsigned char>(frame.at(frameLen - 1)) != 0x16) {
            const int previewLen = frame.size() < 32 ? frame.size() : 32;
            Logger::instance().msg(QString("<< dvc%1 376.2结束符异常，跳过起始字节: %2")
                                   .arg(ep.dvcId).arg(QString(frame.left(previewLen).toHex(' ').toUpper())));
            buf.remove(0, 1);
            continue;
        }

        Logger::instance().msg(QString("<< type=%1,dvc%2 接收完整帧: %3")
                               .arg(int(ep.dvcType)).arg(ep.dvcId)
                               .arg(QString(frame.toHex(' ').toUpper())));
        emit bytesReceived(ep.dvcType, ep.dvcId, frame);
        buf.remove(0, frameLen);
    }
}

void SerialComm::onError(QSerialPort::SerialPortError e)
{
    if (e == QSerialPort::NoError) return;
    QSerialPort *p = qobject_cast<QSerialPort*>(sender());
    if (!p) return;
    const SerialEndpoint ep = m_eps.value(p);
    emit portError(ep.dvcType, ep.dvcId, p->errorString());
}
