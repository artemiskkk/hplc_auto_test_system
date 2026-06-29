#include "device/VirtualMeter.h"
#include "comm/SerialComm.h"
#include "common/Logger.h"

#include <QDateTime>

namespace {

uchar u8(char c)
{
    return static_cast<uchar>(c);
}

quint16 u16le(const QByteArray &data, int pos)
{
    return quint16(u8(data.at(pos))) | (quint16(u8(data.at(pos + 1))) << 8);
}

void appendU16Le(QByteArray &out, quint16 value)
{
    out.append(char(value & 0xFF));
    out.append(char((value >> 8) & 0xFF));
}

QByteArray reversed(const QByteArray &in)
{
    QByteArray out;
    for (int i = in.size() - 1; i >= 0; --i)
        out.append(in.at(i));
    return out;
}

QByteArray encode645Data(const QByteArray &plain)
{
    QByteArray out;
    for (int i = 0; i < plain.size(); ++i)
        out.append(char(u8(plain.at(i)) + 0x33));
    return out;
}

QByteArray decode645Data(const QByteArray &encoded)
{
    QByteArray out;
    for (int i = 0; i < encoded.size(); ++i)
        out.append(char(u8(encoded.at(i)) - 0x33));
    return out;
}

QByteArray bcdLe(int value, int bytes)
{
    QByteArray out;
    int v = qMax(0, value);
    for (int i = 0; i < bytes; ++i) {
        const int lo = v % 10; v /= 10;
        const int hi = v % 10; v /= 10;
        out.append(char((hi << 4) | lo));
    }
    return out;
}

bool isSameDi(const QByteArray &di, const char *hex)
{
    return di == QByteArray::fromHex(hex);
}

bool is645BroadcastAddr(const QByteArray &addr)
{
    if (addr.size() != 6)
        return false;
    bool allAa = true;
    bool all99 = true;
    for (int i = 0; i < addr.size(); ++i) {
        allAa = allAa && (u8(addr.at(i)) == 0xAA);
        all99 = all99 && (u8(addr.at(i)) == 0x99);
    }
    return allAa || all99;
}

quint16 crc16X25(const QByteArray &data)
{
    quint16 crc = 0xFFFF;
    for (int i = 0; i < data.size(); ++i) {
        crc ^= u8(data.at(i));
        for (int b = 0; b < 8; ++b)
            crc = (crc & 0x0001) ? ((crc >> 1) ^ 0x8408) : (crc >> 1);
    }
    return quint16(~crc);
}

QByteArray uint16Be(quint16 value)
{
    QByteArray data;
    data.append(char((value >> 8) & 0xFF));
    data.append(char(value & 0xFF));
    return data;
}

QByteArray uint32Be(quint32 value)
{
    QByteArray data;
    data.append(char((value >> 24) & 0xFF));
    data.append(char((value >> 16) & 0xFF));
    data.append(char((value >> 8) & 0xFF));
    data.append(char(value & 0xFF));
    return data;
}

QByteArray arrayOf(uchar type, const QList<QByteArray> &items)
{
    QByteArray data;
    data.append(char(0x01)); // array
    data.append(char(items.size()));
    for (const QByteArray &item : items) {
        data.append(char(type));
        data.append(item);
    }
    return data;
}

QString oiText(quint16 oi)
{
    return QString("0x%1").arg(oi, 4, 16, QChar('0')).toUpper();
}

QString diKey(const QByteArray &di)
{
    return QString::fromLatin1(di.toHex().toUpper());
}

int baudFromCode(uchar code, int fallback)
{
    switch (code) {
    case 0x00: return 300;
    case 0x01: return 600;
    case 0x02: return 1200;
    case 0x03: return 2400;
    case 0x04: return 4800;
    case 0x05: return 7200;
    case 0x06: return 9600;
    case 0x07: return 19200;
    case 0x08: return 38400;
    case 0x09: return 57600;
    case 0x0A: return 115200;
    default: return fallback;
    }
}

} // namespace

VirtualMeter::VirtualMeter(QObject *parent)
    : QObject(parent),
      m_addrDisplay(QByteArray::fromHex("112233445566")),
      m_addrWire(reversed(m_addrDisplay))
{
    loadDefaultItems();
}

void VirtualMeter::setSerial(SerialComm *serial)
{
    m_serial = serial;
}

QString VirtualMeter::meterAddress() const
{
    return QString::fromLatin1(m_addrDisplay.toHex().toUpper());
}

bool VirtualMeter::setMeterAddress(const QString &hex12)
{
    QString value = hex12.trimmed().remove(' ').toUpper();
    if (value.size() != 12)
        return false;
    for (int i = 0; i < value.size(); ++i) {
        const QChar c = value.at(i);
        if (!((c >= QLatin1Char('0') && c <= QLatin1Char('9')) ||
              (c >= QLatin1Char('A') && c <= QLatin1Char('F')))) {
            return false;
        }
    }

    const QByteArray addr = QByteArray::fromHex(value.toLatin1());
    if (addr.size() != 6)
        return false;
    m_addrDisplay = addr;
    m_addrWire = reversed(m_addrDisplay);
    return true;
}

QSet<QString> VirtualMeter::enabled645Items() const
{
    return m_enabled645Items;
}

QSet<quint16> VirtualMeter::enabled698Items() const
{
    return m_enabled698Items;
}

void VirtualMeter::setEnabled645Items(const QSet<QString> &items)
{
    m_enabled645Items.clear();
    for (const QString &item : items)
        m_enabled645Items.insert(item.trimmed().toUpper());
}

void VirtualMeter::setEnabled698Items(const QSet<quint16> &items)
{
    m_enabled698Items = items;
}

VirtualMeter::BaudConsultMode VirtualMeter::baudConsultMode() const
{
    return m_baudConsultMode;
}

int VirtualMeter::baudConsultFallbackBaud() const
{
    return m_baudConsultFallbackBaud;
}

bool VirtualMeter::baudConsultSwitchSerial() const
{
    return m_baudConsultSwitchSerial;
}

void VirtualMeter::setBaudConsultMode(BaudConsultMode mode)
{
    m_baudConsultMode = mode;
}

void VirtualMeter::setBaudConsultFallbackBaud(int baud)
{
    m_baudConsultFallbackBaud = baud > 0 ? baud : 9600;
}

void VirtualMeter::setBaudConsultSwitchSerial(bool enabled)
{
    m_baudConsultSwitchSerial = enabled;
}

void VirtualMeter::loadDefaultItems()
{
    m_enabled645Items = QSet<QString>()
        << "READ_ADDR"
        << "01040004" << "01008104"
        << "00010102" << "00020102" << "00030102" << "00FF0102"
        << "00010202" << "00020202" << "00030202" << "00FF0202" << "01008002"
        << "00000302" << "00010302" << "00020302" << "00030302" << "00FF0302"
        << "00000602" << "00010602" << "00020602" << "00030602" << "00FF0602"
        << "00000000" << "00000100" << "00000200"
        << "00FF0000" << "00FF0100" << "00FF0200"
        << "00008002" << "02008002" << "04008002" << "05008002";

    m_enabled698Items = QSet<quint16>()
        << quint16(0x4001) << quint16(0x4000)
        << quint16(0x0000) << quint16(0x0010) << quint16(0x0020)
        << quint16(0x2000) << quint16(0x2001)
        << quint16(0x2004) << quint16(0x2005)
        << quint16(0x200A)
        << quint16(0x1010) << quint16(0x1030)
        << quint16(0x200C) << quint16(0x2010) << quint16(0x4300)
        << quint16(0xF209);
}

bool VirtualMeter::is645ItemEnabled(const QByteArray &di) const
{
    return m_enabled645Items.contains(diKey(di));
}

bool VirtualMeter::is645SearchEnabled() const
{
    return m_enabled645Items.contains("READ_ADDR");
}

bool VirtualMeter::is698ItemEnabled(quint16 oi) const
{
    return m_enabled698Items.contains(oi);
}

bool VirtualMeter::isStaEndpoint(DvcType dvcType) const
{
    return dvcType == SingleSTA || dvcType == ThreeSTA;
}

bool VirtualMeter::handleBytes(DvcType dvcType, int dvcId, const QByteArray &data)
{
    if (!isStaEndpoint(dvcType) || data.isEmpty())
        return false;

    m_rx.append(data);
    return processBuffer(dvcType, dvcId);
}

bool VirtualMeter::processBuffer(DvcType dvcType, int dvcId)
{
    bool handled = false;

    while (!m_rx.isEmpty()) {
        const int start = m_rx.indexOf(char(0x68));
        if (start < 0) {
            if (m_rx.size() > 32)
                m_rx.clear();
            return handled;
        }
        if (start > 0)
            m_rx.remove(0, start);

        if (m_rx.size() < 3)
            return true;

        if (m_rx.size() >= 8 && u8(m_rx.at(7)) == 0x68) {
            if (m_rx.size() < 12)
                return true;
            const int len = u8(m_rx.at(9));
            const int frameLen = 12 + len;
            if (frameLen < 12 || frameLen > 512) {
                m_rx.remove(0, 1);
                continue;
            }
            if (m_rx.size() < frameLen)
                return true;
            const QByteArray frame = m_rx.left(frameLen);
            m_rx.remove(0, frameLen);
            handled = process645Frame(dvcType, dvcId, frame) || handled;
            continue;
        }

        const int frameLen = int(u16le(m_rx, 1)) + 2;
        if (frameLen < 10 || frameLen > 4096) {
            m_rx.remove(0, 1);
            continue;
        }
        if (m_rx.size() < frameLen)
            return true;
        const QByteArray frame = m_rx.left(frameLen);
        m_rx.remove(0, frameLen);
        handled = process698Frame(dvcType, dvcId, frame) || handled;
    }

    return handled;
}

bool VirtualMeter::process645Frame(DvcType dvcType, int dvcId, const QByteArray &frame)
{
    if (frame.size() < 12 || u8(frame.at(0)) != 0x68 || u8(frame.at(7)) != 0x68
        || u8(frame.at(frame.size() - 1)) != 0x16) {
        return false;
    }

    uchar cs = 0;
    for (int i = 0; i < frame.size() - 2; ++i)
        cs += u8(frame.at(i));
    if (cs != u8(frame.at(frame.size() - 2))) {
        Logger::instance().error(QString("[虚拟表] 645 校验失败：%1").arg(QString(frame.toHex(' ').toUpper())));
        return true;
    }

    const uchar ctrl = u8(frame.at(8));
    const int len = u8(frame.at(9));
    const QByteArray addr = frame.mid(1, 6);
    const QByteArray plain = decode645Data(frame.mid(10, len));
    QByteArray response;
    QString desc;

    if (ctrl == 0x13 && is645SearchEnabled()) {
        response = build645Frame(0x93, m_addrWire, true);
        desc = "645 搜表/读通信地址";
    } else if (ctrl == 0x11 && plain.size() >= 4) {
        response = build645ReadDataResponse(plain.left(4));
        desc = QString("645 抄表 DI=%1").arg(QString(plain.left(4).toHex().toUpper()));
    } else if (ctrl == 0x15) {
        response = build645Frame(0x95, QByteArray(), false);
        desc = "645 写通信地址确认";
    } else if (ctrl == 0x08) {
        Logger::instance().info("[虚拟表] 收到 645 广播校时，按规约不回包");
        return true;
    } else if (is645BroadcastAddr(addr) && is645SearchEnabled()) {
        response = build645Frame(0x93, m_addrWire, true);
        desc = "645 广播搜表";
    }

    if (response.isEmpty()) {
        Logger::instance().info(QString("[虚拟表] 暂不支持的 645 控制码 0x%1：%2")
                                .arg(ctrl, 2, 16, QChar('0')).arg(QString(frame.toHex(' ').toUpper())));
        return true;
    }

    if (m_serial)
        m_serial->send(dvcType, dvcId, response);
    Logger::instance().info(QString("[虚拟表] %1，应答表地址 %2：%3")
                            .arg(desc, meterAddress(), QString(response.toHex(' ').toUpper())));
    return true;
}

QByteArray VirtualMeter::build645Frame(uchar ctrl, const QByteArray &plainData, bool encodeData) const
{
    const QByteArray data = encodeData ? encode645Data(plainData) : plainData;

    QByteArray frame;
    frame.append(char(0x68));
    frame.append(m_addrWire);
    frame.append(char(0x68));
    frame.append(char(ctrl));
    frame.append(char(data.size()));
    frame.append(data);

    uchar cs = 0;
    for (int i = 0; i < frame.size(); ++i)
        cs += u8(frame.at(i));
    frame.append(char(cs));
    frame.append(char(0x16));
    return frame;
}

QByteArray VirtualMeter::build645ReadDataResponse(const QByteArray &di) const
{
    QByteArray value = valueFor645Di(di);
    if (value.isEmpty())
        return QByteArray();
    return build645Frame(0x91, di + value, true);
}

QByteArray VirtualMeter::valueFor645Di(const QByteArray &di) const
{
    if (!is645ItemEnabled(di))
        return QByteArray();

    if (isSameDi(di, "00010102") || isSameDi(di, "00020102") || isSameDi(di, "00030102"))
        return bcdLe(2200, 2);           // 220.0 V
    if (isSameDi(di, "00ff0102"))
        return bcdLe(2200, 2) + bcdLe(2200, 2) + bcdLe(2200, 2);

    if (isSameDi(di, "00010202") || isSameDi(di, "00020202") || isSameDi(di, "00030202"))
        return bcdLe(5000, 3);           // 5.000 A
    if (isSameDi(di, "00ff0202"))
        return bcdLe(5000, 3) + bcdLe(5000, 3) + bcdLe(5000, 3);

    if (isSameDi(di, "00000302") || isSameDi(di, "00010302") || isSameDi(di, "00020302") || isSameDi(di, "00030302"))
        return bcdLe(1234, 3);           // 1.234 kW
    if (isSameDi(di, "00ff0302"))
        return bcdLe(1234, 3) + bcdLe(1234, 3) + bcdLe(1234, 3);

    if (isSameDi(di, "00000602") || isSameDi(di, "00010602") || isSameDi(di, "00020602") || isSameDi(di, "00030602"))
        return bcdLe(1000, 2);           // 1.000
    if (isSameDi(di, "00ff0602"))
        return bcdLe(1000, 2) + bcdLe(1000, 2) + bcdLe(1000, 2);

    if (isSameDi(di, "01008104"))
        return QByteArray::fromHex("01 02 03 04 05 06 07 08 09 10 11 12 13 14 15"); // 厂商信息，脚本期望 15 字节值

    if (isSameDi(di, "00000000") || isSameDi(di, "00000100") || isSameDi(di, "00000200"))
        return bcdLe(12345, 4);          // 123.45 kWh
    if (isSameDi(di, "00ff0000") || isSameDi(di, "00ff0100") || isSameDi(di, "00ff0200"))
        return bcdLe(12345, 4) + bcdLe(12345, 4) + bcdLe(12345, 4) + bcdLe(12345, 4) + bcdLe(12345, 4);

    if (isSameDi(di, "01040004"))
        return m_addrWire;               // 通信地址

    if (isSameDi(di, "00008002") || isSameDi(di, "01008002") || isSameDi(di, "02008002")
        || isSameDi(di, "04008002") || isSameDi(di, "05008002"))
        return bcdLe(250, 2);            // 25.0 C，兼容常见温度/模拟量 DI

    return QByteArray();
}

bool VirtualMeter::process698Frame(DvcType dvcType, int dvcId, const QByteArray &frame)
{
    if (frame.size() < 10 || u8(frame.at(0)) != 0x68 || u8(frame.at(frame.size() - 1)) != 0x16)
        return false;

    const int len = u16le(frame, 1);
    if (len + 2 != frame.size())
        return false;

    const int saLen = (u8(frame.at(4)) & 0x0F) + 1;
    const int addrFieldLen = 1 + saLen + 1;
    const int hcsPos = 4 + addrFieldLen;
    const int apduPos = hcsPos + 2;
    const int apduLen = frame.size() - apduPos - 3;
    if (apduLen <= 0)
        return true;

    const QByteArray apdu = frame.mid(apduPos, apduLen);
    QByteArray responseApdu;
    QString desc;
    int pendingBaudSwitch = 0;
    bool baudNoResponseHandled = false;

    if (u8(apdu.at(0)) == 0x05 && apdu.size() >= 2) {
        if (u8(apdu.at(1)) == 0x01) {
            responseApdu = build698GetNormalResponse(apdu);
            desc = "698 GetRequestNormal";
        } else if (u8(apdu.at(1)) == 0x02) {
            responseApdu = build698GetNormalListResponse(apdu);
            desc = "698 GetRequestNormalList";
        }
    } else if (u8(apdu.at(0)) == 0x07 && apdu.size() >= 7) {
        int targetBaud = 0;
        responseApdu = build698ActionResponse(apdu, &targetBaud);
        if (targetBaud > 0 && responseApdu.isEmpty() && m_baudConsultMode == BaudConsultNoResponse)
            baudNoResponseHandled = true;
        if (!responseApdu.isEmpty() && targetBaud > 0
            && m_baudConsultMode == BaudConsultConfirm && m_baudConsultSwitchSerial)
            pendingBaudSwitch = targetBaud;
        desc = QString("698 ActionRequest F209 波特率协商 %1bps").arg(targetBaud > 0 ? targetBaud : m_baudConsultFallbackBaud);
    }

    if (baudNoResponseHandled)
        return true;

    if (responseApdu.isEmpty()) {
        Logger::instance().info(QString("[虚拟表] 暂不支持的 698 APDU：%1").arg(QString(apdu.toHex(' ').toUpper())));
        return true;
    }

    const QByteArray response = build698Frame(frame, responseApdu);
    if (m_serial)
        m_serial->send(dvcType, dvcId, response);
    if (m_serial && pendingBaudSwitch > 0)
        m_serial->setBaudRate(dvcType, dvcId, pendingBaudSwitch);
    Logger::instance().info(QString("[虚拟表] %1，应答表地址 %2：%3")
                            .arg(desc, meterAddress(), QString(response.toHex(' ').toUpper())));
    return true;
}

QByteArray VirtualMeter::build698Frame(const QByteArray &request, const QByteArray &apdu) const
{
    const int reqSaLen = (u8(request.at(4)) & 0x0F) + 1;
    const int reqAddrFieldLen = 1 + reqSaLen + 1;
    QByteArray addrField = request.mid(4, reqAddrFieldLen);
    if (addrField.size() >= 8) {
        addrField[0] = char(0x05);
        for (int i = 0; i < 6; ++i)
            addrField[1 + i] = m_addrWire.at(i);
    }

    QByteArray frame;
    frame.append(char(0x68));
    frame.append(char(0x00));
    frame.append(char(0x00));
    frame.append(char(u8(request.at(3)) | 0x80));
    frame.append(addrField);

    const quint16 length = quint16(1 + addrField.size() + 2 + apdu.size() + 2);
    frame[1] = char(length & 0xFF);
    frame[2] = char((length >> 8) & 0xFF);

    const quint16 hcs = crc16X25(frame.mid(1, 2 + 1 + addrField.size()));
    appendU16Le(frame, hcs);
    frame.append(apdu);
    const quint16 fcs = crc16X25(frame.mid(1));
    appendU16Le(frame, fcs);
    frame.append(char(0x16));
    return frame;
}

QByteArray VirtualMeter::build698GetNormalResponse(const QByteArray &requestApdu) const
{
    if (requestApdu.size() < 7)
        return QByteArray();

    const uchar piid = u8(requestApdu.at(2));
    const quint16 oi = (quint16(u8(requestApdu.at(3))) << 8) | quint16(u8(requestApdu.at(4)));
    const QByteArray oad = requestApdu.mid(3, 4);
    QByteArray data = resultDataFor698Oi(oi);
    if (data.isEmpty())
        return QByteArray();

    QByteArray apdu;
    apdu.append(char(0x85));
    apdu.append(char(0x01));
    apdu.append(char(piid));
    apdu.append(oad);
    apdu.append(char(0x01)); // GetResultData
    apdu.append(data);

    Logger::instance().info(QString("[虚拟表] 698 OI=%1").arg(oiText(oi)));
    return apdu;
}

QByteArray VirtualMeter::build698GetNormalListResponse(const QByteArray &requestApdu) const
{
    if (requestApdu.size() < 4)
        return QByteArray();

    const uchar piid = u8(requestApdu.at(2));
    const int count = u8(requestApdu.at(3));
    int pos = 4;

    QByteArray apdu;
    apdu.append(char(0x85));
    apdu.append(char(0x02));
    apdu.append(char(piid));
    apdu.append(char(count));

    for (int i = 0; i < count && pos + 4 <= requestApdu.size(); ++i, pos += 4) {
        const QByteArray oad = requestApdu.mid(pos, 4);
        const quint16 oi = (quint16(u8(oad.at(0))) << 8) | quint16(u8(oad.at(1)));
        QByteArray data = resultDataFor698Oi(oi);
        if (data.isEmpty())
            data = QByteArray::fromHex("06 00 00 00 00");
        apdu.append(oad);
        apdu.append(char(0x01));
        apdu.append(data);
    }

    return apdu;
}

QByteArray VirtualMeter::build698ActionResponse(const QByteArray &requestApdu, int *targetBaud) const
{
    if (targetBaud)
        *targetBaud = 0;
    if (requestApdu.size() < 7 || u8(requestApdu.at(1)) != 0x01)
        return QByteArray();

    const quint16 oi = (quint16(u8(requestApdu.at(3))) << 8) | quint16(u8(requestApdu.at(4)));
    const QByteArray omd = requestApdu.mid(3, 4);
    if (oi != 0xF209 || !is698ItemEnabled(oi))
        return QByteArray();

    const int baud = baudRateFrom698Action(requestApdu);
    if (targetBaud)
        *targetBaud = baud;

    if (m_baudConsultMode == BaudConsultNoResponse) {
        Logger::instance().info(QString("[虚拟表] 698 F209 波特率协商按配置不响应，目标波特率 %1bps").arg(baud));
        return QByteArray();
    }

    uchar dar = 0x00; // success
    if (m_baudConsultMode == BaudConsultDeny)
        dar = 0x10;  // kCommRateCannotChange
    else if (m_baudConsultMode == BaudConsultWrongResponse)
        dar = 0x06;  // kObjctNotExist

    QByteArray apdu;
    apdu.append(char(0x87));             // ACTION-Response
    apdu.append(char(0x01));             // ActionResponseNormal
    apdu.append(requestApdu.at(2));       // PIID-ACD
    apdu.append(omd);                     // OMD
    apdu.append(char(dar));               // DAR
    apdu.append(char(0x00));              // no return data
    apdu.append(char(0x00));              // no follow report
    apdu.append(char(0x00));              // no time tag

    Logger::instance().info(QString("[虚拟表] 698 F209 波特率协商应答，DAR=%1，目标波特率 %2bps")
                            .arg(int(dar)).arg(baud));
    return apdu;
}

QByteArray VirtualMeter::resultDataFor698Oi(quint16 oi) const
{
    if (!is698ItemEnabled(oi))
        return QByteArray();

    switch (oi) {
    case 0x4001: { // 通信地址
        QByteArray data;
        data.append(char(0x09)); // octet-string
        data.append(char(m_addrDisplay.size()));
        data.append(m_addrDisplay);
        return data;
    }
    case 0x4000: { // 日期时间
        const QDateTime now = QDateTime::currentDateTime();
        QByteArray data;
        data.append(char(0x1C)); // date_time_s
        const int year = now.date().year();
        data.append(char((year >> 8) & 0xFF));
        data.append(char(year & 0xFF));
        data.append(char(now.date().month()));
        data.append(char(now.date().day()));
        data.append(char(now.time().hour()));
        data.append(char(now.time().minute()));
        data.append(char(now.time().second()));
        return data;
    }
    case 0x0000:
    case 0x0010:
    case 0x0020: { // 有功电能类
        QList<QByteArray> values;
        values << uint32Be(12345) << uint32Be(0) << uint32Be(0) << uint32Be(0) << uint32Be(0);
        return arrayOf(0x06, values);
    }
    case 0x2000: { // 电压
        QList<QByteArray> values;
        values << uint16Be(2200) << uint16Be(2200) << uint16Be(2200);
        return arrayOf(0x12, values);
    }
    case 0x2001: { // 电流
        QList<QByteArray> values;
        values << uint32Be(5000) << uint32Be(5000) << uint32Be(5000);
        return arrayOf(0x06, values);
    }
    case 0x2004:
    case 0x2005: { // 总功率/分相功率
        QList<QByteArray> values;
        values << uint32Be(1234) << uint32Be(1234) << uint32Be(0) << uint32Be(0);
        return arrayOf(0x06, values);
    }
    case 0x200A: { // 功率因数
        QList<QByteArray> values;
        values << uint16Be(1000) << uint16Be(1000) << uint16Be(1000) << uint16Be(1000);
        return arrayOf(0x12, values);
    }
    case 0x200C:
    case 0x2010:
    case 0x4300:
    case 0x1010:
    case 0x1030: { // 温度类 OI 兼容
        QByteArray data;
        data.append(char(0x10)); // long
        data.append(uint16Be(250));  // 25.0 C
        return data;
    }
    default:
        return QByteArray();
    }
}

int VirtualMeter::baudRateFrom698Action(const QByteArray &requestApdu) const
{
    if (requestApdu.size() < 8)
        return m_baudConsultFallbackBaud;

    int pos = 7; // after service/subtype/piid/OMD
    if (pos + 1 < requestApdu.size() && u8(requestApdu.at(pos)) == 0x02) {
        const int count = u8(requestApdu.at(pos + 1));
        pos += 2;
        for (int i = 0; i < count && pos < requestApdu.size(); ++i) {
            const uchar type = u8(requestApdu.at(pos++));
            if (type == 0x51 && pos + 4 <= requestApdu.size()) {          // OAD
                pos += 4;
            } else if (type == 0x5F && pos + 5 <= requestApdu.size()) {   // COMDCB
                return baudFromCode(u8(requestApdu.at(pos)), m_baudConsultFallbackBaud);
            } else {
                break;
            }
        }
    }

    const QByteArray marker = QByteArray::fromHex("F20902FD5F");
    const int idx = requestApdu.indexOf(marker);
    if (idx >= 0 && idx + marker.size() < requestApdu.size())
        return baudFromCode(u8(requestApdu.at(idx + marker.size())), m_baudConsultFallbackBaud);

    return m_baudConsultFallbackBaud;
}
