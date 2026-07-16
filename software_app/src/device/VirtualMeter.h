#ifndef VIRTUALMETER_H
#define VIRTUALMETER_H

#include <QObject>
#include <QByteArray>
#include <QHash>
#include <QSet>
#include <QString>
#include <QtGlobal>
#include "PublicDataStruct/commdatatype.h"

class SerialComm;

class VirtualMeter : public QObject
{
public:
    enum BaudConsultMode {
        BaudConsultAuto = 0,
        BaudConsultConfirm,
        BaudConsultDeny,
        BaudConsultNoResponse,
        BaudConsultWrongResponse,
        BaudConsultConfirmDelay
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
    void setCurrentCaseId(int tcID);
    bool acceptsLegacyFixtureBaudControl() const;
    bool triggerActiveEvent(DvcType dvcType, int dvcId);
    QByteArray buildManual645Response(const QString &itemKey) const;
    QByteArray buildManual698Response(quint16 oi) const;
    bool handleBytes(DvcType dvcType, int dvcId, const QByteArray &data);

private:
    void loadDefaultItems();
    bool is645ItemEnabled(const QByteArray &di) const;
    bool is645SearchEnabled() const;
    bool is698ItemEnabled(quint16 oi) const;

    bool isVirtualMeterEndpoint(DvcType dvcType) const;
    QString rxKey(DvcType dvcType, int dvcId) const;
    bool processBuffer(DvcType dvcType, int dvcId, QByteArray &rx);
    bool tryAutoSyncBaud(DvcType dvcType, int dvcId, QByteArray &rx);
    bool process645Frame(DvcType dvcType, int dvcId, const QByteArray &frame);
    bool process698Frame(DvcType dvcType, int dvcId, const QByteArray &frame);

    QByteArray build645Frame(uchar ctrl, const QByteArray &plainData, bool encodeData) const;
    QByteArray build645ReadDataResponse(const QByteArray &di) const;
    QByteArray valueFor645Di(const QByteArray &di, bool requireEnabled = true) const;

    QByteArray build698Frame(const QByteArray &request, const QByteArray &apdu) const;
    QByteArray build698ActiveEventFrame() const;
    QByteArray build698GetNormalResponse(const QByteArray &requestApdu) const;
    QByteArray build698GetNormalListResponse(const QByteArray &requestApdu) const;
    QByteArray build698ActionResponse(const QByteArray &requestApdu, int *targetBaud) const;
    QByteArray resultDataFor698Oi(quint16 oi, bool requireEnabled = true) const;
    int baudRateFrom698Action(const QByteArray &requestApdu) const;
    BaudConsultMode effectiveBaudConsultMode(int targetBaud = 0) const;
    int effectiveBaudConsultFallbackBaud() const;

    SerialComm *m_serial = nullptr;
    QHash<QString, QByteArray> m_rxBuffers;
    QByteArray m_addrDisplay; // 112233445566
    QByteArray m_addrWire;    // 665544332211
    QSet<QString> m_enabled645Items;
    QSet<quint16> m_enabled698Items;
    BaudConsultMode m_baudConsultMode = BaudConsultAuto;
    int m_baudConsultFallbackBaud = 9600;
    bool m_baudConsultSwitchSerial = true;
    int m_currentCaseId = 0;
    bool m_baudAutoSyncPending = false;
    bool m_commAddr158Saw645 = false;
};

#endif // VIRTUALMETER_H
