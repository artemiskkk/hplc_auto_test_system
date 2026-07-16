#include "device/VirtualMeter.h"
#include "comm/SerialComm.h"
#include "common/Logger.h"

#include <QDateTime>
#include <QTimer>

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

uchar bcdByte(int value)
{
    const int v = qMax(0, qMin(99, value));
    return uchar(((v / 10) << 4) | (v % 10));
}

QByteArray dateAndWeek645()
{
    const QDateTime now = QDateTime::currentDateTime();
    const int weekday = (now.date().dayOfWeek() == 7) ? 0 : now.date().dayOfWeek();
    QByteArray out;
    out.append(char(bcdByte(weekday)));
    out.append(char(bcdByte(now.date().day())));
    out.append(char(bcdByte(now.date().month())));
    out.append(char(bcdByte(now.date().year() % 100)));
    return out;
}

QByteArray time645()
{
    const QDateTime now = QDateTime::currentDateTime();
    QByteArray out;
    out.append(char(bcdByte(now.time().second())));
    out.append(char(bcdByte(now.time().minute())));
    out.append(char(bcdByte(now.time().hour())));
    return out;
}

QByteArray lastDailyFreezeTime645()
{
    const QDateTime now = QDateTime::currentDateTime();
    QByteArray out;
    out.append(char(0x00));
    out.append(char(0x00));
    out.append(char(bcdByte(now.date().day())));
    out.append(char(bcdByte(now.date().month())));
    out.append(char(bcdByte(now.date().year() % 100)));
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

bool isBaudConsultCase(int tcID)
{
    switch (tcID) {
    case 183:
    case 185:
    case 186:
    case 187:
    case 188:
    case 189:
        return true;
    default:
        return false;
    }
}

bool isCommAddrRequestCase(int tcID)
{
    return tcID >= 155 && tcID <= 158;
}

bool commAddrReplies645(int tcID)
{
    return tcID == 156 || tcID == 158;
}

bool commAddrReplies698(int tcID)
{
    return tcID == 157 || tcID == 158;
}

bool isBaudConsultCoreDi(const QString &key)
{
    return key == "01040004"  // communication address
        || key == "0B040004"  // meter type
        || key == "01150004"  // active report status word
        || key == "0C010004"; // communication rate feature word
}

bool isBaudConsultCoreOi(quint16 oi)
{
    return oi == 0x4001  // communication address
        || oi == 0x4000  // date time
        || oi == 0x0010  // positive active energy
        || oi == 0xF209; // baud consult action
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
    if (mode < BaudConsultAuto || mode > BaudConsultConfirmDelay)
        mode = BaudConsultAuto;
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

void VirtualMeter::setCurrentCaseId(int tcID)
{
    m_currentCaseId = tcID;
    m_baudAutoSyncPending = isBaudConsultCase(tcID);
    m_commAddr158Saw645 = false;
}

bool VirtualMeter::acceptsLegacyFixtureBaudControl() const
{
    return m_currentCaseId != 117;
}

bool VirtualMeter::triggerActiveEvent(DvcType dvcType, int dvcId)
{
    if (m_currentCaseId != 117 || !m_serial || !m_serial->isOpen(dvcType, dvcId))
        return false;

    const QByteArray frame = build698ActiveEventFrame();
    if (frame.isEmpty())
        return false;

    m_serial->send(dvcType, dvcId, frame);
    Logger::instance().info(QString("[虚拟表] TC_00117 发送698主动事件，表地址=%1，串口波特率=%2：%3")
                            .arg(meterAddress())
                            .arg(m_serial->baudRate(dvcType, dvcId))
                            .arg(QString(frame.toHex(' ').toUpper())));
    return true;
}

QByteArray VirtualMeter::buildManual645Response(const QString &itemKey) const
{
    const QString key = itemKey.trimmed().toUpper();
    if (key == "READ_ADDR")
        return build645Frame(0x93, m_addrWire, true);

    const QByteArray di = QByteArray::fromHex(key.toLatin1());
    if (di.size() != 4)
        return QByteArray();
    const QByteArray value = valueFor645Di(di, false);
    if (value.isEmpty())
        return QByteArray();
    return build645Frame(0x91, di + value, true);
}

QByteArray VirtualMeter::buildManual698Response(quint16 oi) const
{
    if (oi == 0xF209) {
        QByteArray apdu;
        apdu.append(char(0x87)); // ActionResponse
        apdu.append(char(0x01)); // ActionResponseNormal
        apdu.append(char(0x01)); // PIID-ACD
        apdu.append(char(0xF2));
        apdu.append(char(0x09));
        apdu.append(char(0x80)); // baud-consult method
        apdu.append(char(0x00));
        apdu.append(char(0x00)); // DAR: success
        apdu.append(char(0x00)); // no return data
        apdu.append(char(0x00)); // no follow report
        apdu.append(char(0x00)); // no time tag

        QByteArray request(12, char(0x00));
        request[3] = char(0x43);
        request[4] = char(0x05);
        request[11] = char(0x10);
        return build698Frame(request, apdu);
    }

    const QByteArray data = resultDataFor698Oi(oi, false);
    if (data.isEmpty())
        return QByteArray();

    QByteArray apdu;
    apdu.append(char(0x85)); // GetResponse
    apdu.append(char(0x01)); // GetResponseNormal
    apdu.append(char(0x01)); // PIID-ACD
    apdu.append(char((oi >> 8) & 0xFF));
    apdu.append(char(oi & 0xFF));
    apdu.append(char(0x02)); // attribute 2
    apdu.append(char(0x00)); // element index 0
    apdu.append(char(0x01)); // GetResultData
    apdu.append(data);
    apdu.append(char(0x00)); // no follow report
    apdu.append(char(0x00)); // no time tag

    QByteArray request(12, char(0x00));
    request[3] = char(0x43);
    request[4] = char(0x05); // six-byte client address field
    request[11] = char(0x10);
    return build698Frame(request, apdu);
}

VirtualMeter::BaudConsultMode VirtualMeter::effectiveBaudConsultMode(int targetBaud) const
{
    if (m_baudConsultMode != BaudConsultAuto)
        return m_baudConsultMode;

    switch (m_currentCaseId) {
    case 183: // 20Standard BaudConsult Confirm CCO
    case 189: // 20Standard BaudConsult DefaultRate CCO
        return BaudConsultConfirm;
    case 185: // 20Standard BaudConsult Deny CCO
        return BaudConsultDeny;
    case 186: // 20Standard BaudConsult Abnormal CCO
        return targetBaud == 115200 || targetBaud == 9600 || targetBaud == 2400
            ? BaudConsultNoResponse : BaudConsultWrongResponse;
    case 187: // 20Standard BaudConsult ConfirmDelay CCO
        return BaudConsultConfirmDelay;
    case 188: // 20Standard BaudConsult NotSearchMeterRate CCO
        return BaudConsultNoResponse;
    default:
        return BaudConsultConfirm;
    }
}

int VirtualMeter::effectiveBaudConsultFallbackBaud() const
{
    if (m_baudConsultMode == BaudConsultAuto && m_currentCaseId == 189)
        return 9600;
    return m_baudConsultFallbackBaud;
}

void VirtualMeter::loadDefaultItems()
{
    m_enabled645Items = QSet<QString>()
        << "READ_ADDR"
        << "01040004" << "01008104"
        << "0C010004" << "01150004" << "0B040004"
        << "00010102" << "00FF0102"
        << "00010202" << "00FF0202" << "01008002"
        << "00000302" << "00010302" << "00FF0302"
        << "00000402" << "00010402" << "00FF0402"
        << "00000602" << "00010602" << "00FF0602"
        << "00010702" << "00FF0702"
        << "00000000" << "00000100" << "00000200"
        << "00FF0000" << "00FF0100" << "00FF0200"
        << "00008002" << "02008002" << "04008002" << "05008002"
        << "07050004" << "FF050004"
        << "01010004" << "02010004" << "000D3003"
        << "01000605" << "01010605" << "01020605"
        << "020A0004" << "02000106";

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
    const QString key = diKey(di);
    if (isBaudConsultCase(m_currentCaseId) && isBaudConsultCoreDi(key))
        return true;
    return m_enabled645Items.contains(key);
}

bool VirtualMeter::is645SearchEnabled() const
{
    if (isCommAddrRequestCase(m_currentCaseId))
        return commAddrReplies645(m_currentCaseId);
    if (isBaudConsultCase(m_currentCaseId))
        return true;
    return m_enabled645Items.contains("READ_ADDR");
}

bool VirtualMeter::is698ItemEnabled(quint16 oi) const
{
    if (isCommAddrRequestCase(m_currentCaseId) && oi == 0x4001)
        return commAddrReplies698(m_currentCaseId);
    if (isBaudConsultCase(m_currentCaseId) && isBaudConsultCoreOi(oi))
        return true;
    return m_enabled698Items.contains(oi);
}

bool VirtualMeter::isVirtualMeterEndpoint(DvcType dvcType) const
{
    return dvcType == SingleSTA || dvcType == ThreeSTA
        || dvcType == SingleMeter || dvcType == ThreeMeter;
}

bool VirtualMeter::handleBytes(DvcType dvcType, int dvcId, const QByteArray &data)
{
    if (!isVirtualMeterEndpoint(dvcType) || data.isEmpty())
        return false;

    QByteArray &rx = m_rxBuffers[rxKey(dvcType, dvcId)];
    rx.append(data);
    return processBuffer(dvcType, dvcId, rx);
}

QString VirtualMeter::rxKey(DvcType dvcType, int dvcId) const
{
    return QString("%1:%2").arg(int(dvcType)).arg(dvcId);
}

bool VirtualMeter::processBuffer(DvcType dvcType, int dvcId, QByteArray &rx)
{
    bool handled = false;

    while (!rx.isEmpty()) {
        const int start = rx.indexOf(char(0x68));
        if (start < 0) {
            if (tryAutoSyncBaud(dvcType, dvcId, rx))
                return true;
            if (rx.size() > 32)
                rx.clear();
            return true;
        }
        if (start > 0)
            rx.remove(0, start);

        if (rx.size() < 8)
            return true;

        if (rx.size() >= 8 && u8(rx.at(7)) == 0x68) {
            if (rx.size() < 12)
                return true;
            const int len = u8(rx.at(9));
            const int frameLen = 12 + len;
            if (frameLen < 12 || frameLen > 512) {
                rx.remove(0, 1);
                continue;
            }
            if (rx.size() < frameLen)
                return true;
            const QByteArray frame = rx.left(frameLen);
            rx.remove(0, frameLen);
            handled = process645Frame(dvcType, dvcId, frame) || handled;
            continue;
        }

        const int frameLen = int(u16le(rx, 1)) + 2;
        if (frameLen < 10 || frameLen > 4096) {
            rx.remove(0, 1);
            continue;
        }
        if (rx.size() < frameLen)
            return true;
        const QByteArray frame = rx.left(frameLen);
        rx.remove(0, frameLen);
        handled = process698Frame(dvcType, dvcId, frame) || handled;
    }

    return handled;
}

bool VirtualMeter::tryAutoSyncBaud(DvcType dvcType, int dvcId, QByteArray &rx)
{
    if (!m_baudAutoSyncPending || !m_serial || rx.size() < 3)
        return false;

    bool onlyPreamble = true;
    for (char value : rx) {
        if (u8(value) != 0xFE) {
            onlyPreamble = false;
            break;
        }
    }
    if (onlyPreamble)
        return false;

    const qint32 current = m_serial->baudRate(dvcType, dvcId);
    qint32 next = 9600;
    switch (current) {
    case 9600: next = 2400; break;
    case 2400: next = 4800; break;
    case 4800: next = 19200; break;
    case 19200: next = 115200; break;
    case 115200: next = 1200; break;
    default: next = 9600; break;
    }

    const QString garbage = QString(rx.toHex(' ').toUpper());
    rx.clear();
    const bool ok = m_serial->setBaudRate(dvcType, dvcId, next);
    Logger::instance().info(QString("[虚拟表] 首次F209前检测到错速字节 %1，自动探测速率 %2 -> %3：%4")
                            .arg(garbage).arg(current).arg(next)
                            .arg(ok ? "success" : "failed"));
    return true;
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
    QString detail;

    if (ctrl == 0x13 && m_currentCaseId == 158)
        m_commAddr158Saw645 = true;

    if (ctrl == 0x13 && isCommAddrRequestCase(m_currentCaseId)
        && !commAddrReplies645(m_currentCaseId)) {
        Logger::instance().info(QString("[虚拟表] TC=%1 645通信地址请求，按用例不响应")
                                .arg(m_currentCaseId));
        return true;
    }

    if (ctrl == 0x13 && is645SearchEnabled()) {
        response = build645Frame(0x93, m_addrWire, true);
        desc = "645 搜表/读通信地址";
    } else if (ctrl == 0x11 && plain.size() >= 4) {
        const QByteArray di = plain.left(4);
        detail = QString("DI=%1 enabled=%2")
                     .arg(QString(di.toHex().toUpper()))
                     .arg(is645ItemEnabled(di) ? "true" : "false");
        response = build645ReadDataResponse(plain.left(4));
        desc = QString("645 抄表 DI=%1").arg(QString(plain.left(4).toHex().toUpper()));
        if (response.isEmpty()) {
            response = build645Frame(0xD1, QByteArray(1, char(0x00)), true);
            desc = QString("645 抄表异常应答 %1").arg(detail);
        }
    } else if (ctrl == 0x11) {
        response = build645Frame(0xD1, QByteArray(1, char(0x00)), true);
        desc = "645 抄表数据域异常应答";
    } else if (ctrl == 0x12) {
        response = build645Frame(0xD2, QByteArray(1, char(0x00)), true);
        desc = "645 读后续数据异常应答";
    } else if (ctrl == 0x14) {
        response = build645Frame(0xD4, QByteArray(1, char(0x00)), true);
        desc = "645 写数据异常应答";
    } else if (ctrl == 0x94) {
        Logger::instance().info(QString("[VirtualMeter] STA local 645 write ACK observed: %1")
                                .arg(QString(frame.toHex(' ').toUpper())));
        return true;
    } else if (ctrl == 0x9F) {
        Logger::instance().info(QString("[VirtualMeter] STA local 645 ID read response observed: %1")
                                .arg(QString(frame.toHex(' ').toUpper())));
        return true;
    } else if (ctrl == 0x15) {
        response = build645Frame(0x95, QByteArray(), false);
        desc = "645 写通信地址确认";
    } else if (ctrl == 0x16) {
        response = build645Frame(0xD6, QByteArray(1, char(0x00)), true);
        desc = "645 冻结命令异常应答";
    } else if (ctrl == 0x1C) {
        response = build645Frame(0x9C, QByteArray(), false);
        desc = "645 拉合闸报警保持正常应答";
    } else if (ctrl == 0x08) {
        Logger::instance().info("[虚拟表] 收到 645 广播校时，按规约不回包");
        return true;
    } else if (is645BroadcastAddr(addr) && is645SearchEnabled()) {
        response = build645Frame(0x93, m_addrWire, true);
        desc = "645 广播搜表";
    }

    if (response.isEmpty()) {
        if (!detail.isEmpty()) {
            Logger::instance().info(QString("[VirtualMeter] 645 no response: %1 frame=%2")
                                    .arg(detail, QString(frame.toHex(' ').toUpper())));
        }
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

QByteArray VirtualMeter::valueFor645Di(const QByteArray &di, bool requireEnabled) const
{
    if (requireEnabled && !is645ItemEnabled(di))
        return QByteArray();

    if (isSameDi(di, "00010102"))
        return bcdLe(2200, 2);           // 220.0 V
    if (isSameDi(di, "00020102") || isSameDi(di, "00030102"))
        return QByteArray();
    if (isSameDi(di, "00ff0102"))
        return bcdLe(2200, 2) + QByteArray(2, char(0xFF)) + QByteArray(2, char(0xFF));

    if (isSameDi(di, "00010202") || isSameDi(di, "01008002"))
        return bcdLe(0, 3);              // 0.000 A
    if (isSameDi(di, "00020202") || isSameDi(di, "00030202"))
        return QByteArray();
    if (isSameDi(di, "00ff0202"))
        return bcdLe(0, 3) + QByteArray(3, char(0xFF)) + QByteArray(3, char(0xFF));

    if (isSameDi(di, "00000302") || isSameDi(di, "00010302"))
        return bcdLe(0, 3);              // 0.000 kW
    if (isSameDi(di, "00020302") || isSameDi(di, "00030302"))
        return QByteArray();
    if (isSameDi(di, "00ff0302"))
        return bcdLe(0, 3) + QByteArray(3, char(0xFF)) + QByteArray(3, char(0xFF));

    if (isSameDi(di, "00000402") || isSameDi(di, "00010402"))
        return bcdLe(0, 3);
    if (isSameDi(di, "00020402") || isSameDi(di, "00030402"))
        return QByteArray();
    if (isSameDi(di, "00ff0402"))
        return bcdLe(0, 3) + QByteArray(3, char(0xFF)) + QByteArray(3, char(0xFF));

    if (isSameDi(di, "00000602") || isSameDi(di, "00010602"))
        return bcdLe(1000, 2);           // 1.000
    if (isSameDi(di, "00020602") || isSameDi(di, "00030602"))
        return QByteArray();
    if (isSameDi(di, "00ff0602"))
        return bcdLe(1000, 2) + QByteArray(2, char(0xFF)) + QByteArray(2, char(0xFF));

    if (isSameDi(di, "00010702"))
        return bcdLe(0, 2);
    if (isSameDi(di, "00020702") || isSameDi(di, "00030702"))
        return QByteArray();
    if (isSameDi(di, "00ff0702"))
        return bcdLe(0, 2) + QByteArray(2, char(0xFF)) + QByteArray(2, char(0xFF));

    if (isSameDi(di, "01008104"))
        return QByteArray::fromHex("01 02 03 04 05 06 07 08 09 10 11 12 13 14 15"); // 厂商信息，脚本期望 15 字节值

    if (isSameDi(di, "00000000") || isSameDi(di, "00000100") || isSameDi(di, "00000200"))
        return bcdLe(100, 4);            // 1.00 kWh
    if (isSameDi(di, "00ff0000") || isSameDi(di, "00ff0100") || isSameDi(di, "00ff0200"))
        return bcdLe(400, 4) + bcdLe(100, 4) + bcdLe(100, 4) + bcdLe(100, 4) + bcdLe(100, 4);

    if (isSameDi(di, "01040004"))
        return m_addrWire;               // 通信地址

    if (isSameDi(di, "0B040004"))
        return QByteArray("DDZY1710-Z");

    if (isSameDi(di, "01150004"))
        return QByteArray::fromHex("000000000000000000000000AAAA");

    if (isSameDi(di, "07050004"))
        return QByteArray(2, char(0xFF));
    if (isSameDi(di, "FF050004"))
        return QByteArray(14, char(0xFF));

    if (isSameDi(di, "01010004"))
        return dateAndWeek645();
    if (isSameDi(di, "02010004"))
        return time645();
    if (isSameDi(di, "000D3003"))
        return QByteArray(3, char(0x00));

    if (isSameDi(di, "01000605"))
        return lastDailyFreezeTime645();
    if (isSameDi(di, "01010605") || isSameDi(di, "01020605"))
        return bcdLe(400, 4) + bcdLe(100, 4) + bcdLe(100, 4) + bcdLe(100, 4) + bcdLe(100, 4);

    if (isSameDi(di, "020A0004"))
        return QByteArray::fromHex("0F 00");
    if (isSameDi(di, "02000106"))
        return QByteArray::fromHex("A0 A0 16 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 AA 00 E5");

    if (isSameDi(di, "0C010004"))
        return QByteArray(1, char(0x7E)); // 通信速率特征字：1200/2400/4800/9600/19200/115200

    if (isSameDi(di, "00008002") || isSameDi(di, "02008002")
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
    if (hcsPos + 1 >= frame.size() || apduLen <= 0)
        return false;
    if (u16le(frame, hcsPos) != crc16X25(frame.mid(1, hcsPos - 1)))
        return false;
    if (u16le(frame, frame.size() - 3) != crc16X25(frame.mid(1, frame.size() - 4)))
        return false;

    const QByteArray apdu = frame.mid(apduPos, apduLen);
    QByteArray responseApdu;
    QString desc;
    int pendingBaudSwitch = 0;
    bool baudNoResponseHandled = false;

    if (u8(apdu.at(0)) == 0x05 && apdu.size() >= 2) {
        if (u8(apdu.at(1)) == 0x01) {
            if (apdu.size() >= 7 && isCommAddrRequestCase(m_currentCaseId)) {
                const quint16 oi = (quint16(u8(apdu.at(3))) << 8)
                                 | quint16(u8(apdu.at(4)));
                if (oi == 0x0010) {
                    Logger::instance().info(QString("[虚拟表] TC=%1 超时恢复触发帧 OI=0x0010，按流程不响应")
                                            .arg(m_currentCaseId));
                    return true;
                }
                if (oi == 0x4001 && m_currentCaseId == 158
                        && !m_commAddr158Saw645) {
                    Logger::instance().info("[虚拟表] TC=158 首次OOP通信地址请求不响应，等待STA切换到645@2400");
                    QTimer::singleShot(0, this, [this, dvcType, dvcId]() {
                        if (!m_serial)
                            return;
                        const bool ok = m_serial->setBaudRate(dvcType, dvcId, 2400);
                        Logger::instance().info(QString("[虚拟表] TC=158 虚拟表串口切至2400等待645通信地址请求：%1")
                                                .arg(ok ? "success" : "failed"));
                    });
                    return true;
                }
                if (oi == 0x4001 && !commAddrReplies698(m_currentCaseId)) {
                    Logger::instance().info(QString("[虚拟表] TC=%1 OOP通信地址请求，按用例不响应")
                                            .arg(m_currentCaseId));
                    return true;
                }
            }
            responseApdu = build698GetNormalResponse(apdu);
            desc = "698 GetRequestNormal";
        } else if (u8(apdu.at(1)) == 0x02) {
            responseApdu = build698GetNormalListResponse(apdu);
            desc = "698 GetRequestNormalList";
        }
    } else if (u8(apdu.at(0)) == 0x07 && apdu.size() >= 7) {
        const quint16 oi = (quint16(u8(apdu.at(3))) << 8)
                         | quint16(u8(apdu.at(4)));
        if (oi == 0x4000 && u8(apdu.at(5)) == 0x7F) {
            Logger::instance().info("[虚拟表] 收到 698 广播校时 OI=4000 method=7F，按规范不应答");
            return true;
        }
        int targetBaud = 0;
        m_baudAutoSyncPending = false;
        responseApdu = build698ActionResponse(apdu, &targetBaud);
        const BaudConsultMode mode = effectiveBaudConsultMode(targetBaud);
        if (targetBaud > 0 && responseApdu.isEmpty() && mode == BaudConsultNoResponse)
            baudNoResponseHandled = true;
        if (!responseApdu.isEmpty() && targetBaud > 0
            && mode == BaudConsultConfirm && m_baudConsultSwitchSerial)
            pendingBaudSwitch = targetBaud;
        desc = QString("698 ActionRequest F209 波特率协商 %1bps").arg(targetBaud > 0 ? targetBaud : effectiveBaudConsultFallbackBaud());
    }

    if (baudNoResponseHandled)
        return true;

    if (responseApdu.isEmpty()) {
        Logger::instance().info(QString("[虚拟表] 暂不支持的 698 APDU：%1").arg(QString(apdu.toHex(' ').toUpper())));
        return true;
    }

    const QByteArray response = build698Frame(frame, responseApdu);
    const int targetBaud = (u8(apdu.at(0)) == 0x07) ? baudRateFrom698Action(apdu) : 0;
    const BaudConsultMode mode = effectiveBaudConsultMode(targetBaud);
    if (m_serial && targetBaud > 0 && mode == BaudConsultConfirmDelay
        && m_baudConsultSwitchSerial) {
        m_serial->setBaudRate(dvcType, dvcId, targetBaud);
        QTimer::singleShot(500, this, [this, dvcType, dvcId, response, desc]() {
            if (!m_serial)
                return;
            m_serial->send(dvcType, dvcId, response);
            Logger::instance().info(QString("[虚拟表] %1，先切速并延时500ms应答，表地址 %2：%3")
                                    .arg(desc, meterAddress(), QString(response.toHex(' ').toUpper())));
        });
        return true;
    }

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
        // The legacy script oracle uses CA=0x10 for normal data responses
        // (communication-address OI 4001 is the exception and uses CA=0x00).
        if (apdu.size() >= 7 && u8(apdu.at(0)) == 0x85 && u8(apdu.at(1)) == 0x01) {
            const quint16 oi = (quint16(u8(apdu.at(3))) << 8) | quint16(u8(apdu.at(4)));
            addrField[addrField.size() - 1] = char(oi == 0x4001 ? 0x00 : 0x10);
        }
    }

    QByteArray frame;
    frame.append(char(0x68));
    frame.append(char(0x00));
    frame.append(char(0x00));
    frame.append(char(u8(request.at(3)) | 0x80));
    frame.append(addrField);

    // DLT698 length covers the two length bytes through FCS; only the leading
    // 0x68 and trailing 0x16 are excluded. The previous value was short by 2,
    // so otherwise valid replies were rejected by the STA.
    const quint16 length = quint16(2 + 1 + addrField.size() + 2 + apdu.size() + 2);
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

QByteArray VirtualMeter::build698ActiveEventFrame() const
{
    // REPORT-Notification-List: OAD 2000-02-01, long-unsigned value 0x08D6.
    const QByteArray apdu = QByteArray::fromHex("8801000120000201011208D60000");

    QByteArray addrField;
    addrField.append(char(0x05));
    addrField.append(m_addrWire);
    addrField.append(char(0x10));

    QByteArray frame;
    frame.append(char(0x68));
    frame.append(char(0x00));
    frame.append(char(0x00));
    frame.append(char(0x81));
    frame.append(addrField);

    const quint16 length = quint16(2 + 1 + addrField.size() + 2 + apdu.size() + 2);
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

    QByteArray apdu;
    apdu.append(char(0x85));
    apdu.append(char(0x01));
    apdu.append(char(piid));
    apdu.append(oad);
    if (data.isEmpty()) {
        apdu.append(char(0x00)); // GetResultError
        apdu.append(char(0x06)); // object not exist / unsupported
        Logger::instance().info(QString("[虚拟表] 698 OI=%1 无数据，返回错误结果").arg(oiText(oi)));
    } else {
        apdu.append(char(0x01)); // GetResultData
        apdu.append(data);
        Logger::instance().info(QString("[虚拟表] 698 OI=%1").arg(oiText(oi)));
    }

    apdu.append(char(0x00)); // no follow report
    apdu.append(char(0x00)); // no time tag
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

    apdu.append(char(0x00)); // no follow report
    apdu.append(char(0x00)); // no time tag
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
    if (oi != 0xF209 || !is698ItemEnabled(oi)) {
        const uchar piid = u8(requestApdu.at(2));
        QByteArray apdu;
        apdu.append(char(0xC7));
        apdu.append(char(0x01));
        apdu.append(char(piid));
        apdu.append(omd);
        apdu.append(char(0x06)); // object not exist / unsupported
        apdu.append(char(0x00)); // no follow report
        apdu.append(char(0x00)); // no time tag
        Logger::instance().info(QString("[虚拟表] 698 Action OI=%1 非波特率协商，返回错误结果").arg(oiText(oi)));
        return apdu;
    }

    const int baud = baudRateFrom698Action(requestApdu);
    const BaudConsultMode mode = effectiveBaudConsultMode(baud);
    if (targetBaud)
        *targetBaud = baud;

    if (mode == BaudConsultNoResponse) {
        Logger::instance().info(QString("[虚拟表] 698 F209 波特率协商自动/配置不响应，TC=%1，目标波特率 %2bps")
                                .arg(m_currentCaseId).arg(baud));
        return QByteArray();
    }

    uchar dar = 0x00; // success
    if (mode == BaudConsultDeny)
        dar = 0x10;  // kCommRateCannotChange
    else if (mode == BaudConsultWrongResponse)
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

    Logger::instance().info(QString("[虚拟表] 698 F209 波特率协商应答，TC=%1，mode=%2，DAR=%3，目标波特率 %4bps")
                            .arg(m_currentCaseId).arg(int(mode)).arg(int(dar)).arg(baud));
    return apdu;
}

QByteArray VirtualMeter::resultDataFor698Oi(quint16 oi, bool requireEnabled) const
{
    if (requireEnabled && !is698ItemEnabled(oi))
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
        values << uint32Be(8) << uint32Be(0) << uint32Be(8) << uint32Be(0) << uint32Be(0);
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
    const int fallbackBaud = effectiveBaudConsultFallbackBaud();
    if (requestApdu.size() < 8)
        return fallbackBaud;

    int pos = 7; // after service/subtype/piid/OMD
    if (pos + 1 < requestApdu.size() && u8(requestApdu.at(pos)) == 0x02) {
        const int count = u8(requestApdu.at(pos + 1));
        pos += 2;
        for (int i = 0; i < count && pos < requestApdu.size(); ++i) {
            const uchar type = u8(requestApdu.at(pos++));
            if (type == 0x51 && pos + 4 <= requestApdu.size()) {          // OAD
                pos += 4;
            } else if (type == 0x5F && pos + 5 <= requestApdu.size()) {   // COMDCB
                return baudFromCode(u8(requestApdu.at(pos)), fallbackBaud);
            } else {
                break;
            }
        }
    }

    const QByteArray marker = QByteArray::fromHex("F20902FD5F");
    const int idx = requestApdu.indexOf(marker);
    if (idx >= 0 && idx + marker.size() < requestApdu.size())
        return baudFromCode(u8(requestApdu.at(idx + marker.size())), fallbackBaud);

    return fallbackBaud;
}
