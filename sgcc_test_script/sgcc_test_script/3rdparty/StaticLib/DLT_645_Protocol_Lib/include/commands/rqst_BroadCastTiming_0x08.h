#ifndef Rqst_BroadCastTiming_0x08_H
#define Rqst_BroadCastTiming_0x08_H

#include <QObject>
#include "../datatypedef.h"
#include "../DLT_645_Protocol_Lib_global.h"
#include "../frame645base.h"
#include "../exceptions/decodeexception.h"


namespace dlt_645_Protocol {


class DLT_645_PROTOCOL_LIB_EXPORT Rqst_BroadCastTiming_0x08 : public Frame645Base
{
public:
    Rqst_BroadCastTiming_0x08(uchar *addr, uchar dataLen);


public:
    uchar dateTime[6];


    void DecodeFrameDataField(QByteArray data) override;


protected:
    QByteArray EncodeFrameDataField() override;
};


}

#endif // Rqst_BroadCastTiming_0x08_H
