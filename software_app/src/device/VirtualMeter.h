#ifndef VIRTUALMETER_H
#define VIRTUALMETER_H

#include <QObject>
#include <QByteArray>
#include <QSet>
#include <QString>
#include <QtGlobal>
#include "PublicDataStruct/commdatatype.h"

class SerialComm;

class VirtualMeter : public QObject
{
public:
    enum BaudConsultMode {
        BaudConsultConfirm = 0,
        BaudConsultDeny,
        BaudConsultNoResponse,
        BaudConsultWrongResponse
    };

    explicit VirtualMeter(QObject *parent = nullptr);

    void setSerial(SerialComm *serial);
    QString meterAddress() const;
    bool setMeterAddress(const QString &hex12);
    QSet<QString> enabled645Items() const;
    QSet<quint16> enabled698Items() const;
    void setEnabled645Items(const QSet<QString> &items);
    void setEnabled698Items(const QSet<quint16> &items);
    BaudConsultMode baudConsultMode() const;
    int baudConsultFallbackBaud() const;
    bool baudConsultSwitchSerial() const;
    void setBaudConsultMode(BaudConsultMode mode);
    void setBaudConsultFallbackBaud(int baud);
    void setBaudConsultSwitchSerial(bool enabled);
    bool handleBytes(DvcType dvcType, int dvcId, const QByteArray &data);

private:
    void loadDefaultItems();
    bool is645ItemEnabled(const QByteArray &di) const;
    bool is645SearchEnabled() const;
    bool is698ItemEnabled(quint16 oi) const;

    bool isStaEndpoint(DvcType dvcType) const;
    bool processBuffer(DvcType dvcType, int dvcId);
    bool process645Frame(DvcType dvcType, int dvcId, const QByteArray &frame);
    bool process698Frame(DvcType dvcType, int dvcId, const QByteArray &frame);

    QByteArray build645Frame(uchar ctrl, const QByteArray &plainData, bool encodeData) const;
    QByteArray build645ReadDataResponse(const QByteArray &di) const;
    QByteArray valueFor645Di(const QByteArray &di) const;

    QByteArray build698Frame(const QByteArray &request, const QByteArray &apdu) const;
    QByteArray build698GetNormalResponse(const QByteArray &requestApdu) const;
    QByteArray build698GetNormalListResponse(const QByteArray &requestApdu) const;
    QByteArray build698ActionResponse(const QByteArray &requestApdu, int *targetBaud) const;
    QByteArray resultDataFor698Oi(quint16 oi) const;
    int baudRateFrom698Action(const QByteArray &requestApdu) const;

    SerialComm *m_serial = nullptr;
    QByteArray m_rx;
    QByteArray m_addrDisplay; // 112233445566
    QByteArray m_addrWire;    // 665544332211
    QSet<QString> m_enabled645Items;
    QSet<quint16> m_enabled698Items;
    BaudConsultMode m_baudConsultMode = BaudConsultConfirm;
    int m_baudConsultFallbackBaud = 9600;
    bool m_baudConsultSwitchSerial = true;
};

#endif // VIRTUALMETER_H
