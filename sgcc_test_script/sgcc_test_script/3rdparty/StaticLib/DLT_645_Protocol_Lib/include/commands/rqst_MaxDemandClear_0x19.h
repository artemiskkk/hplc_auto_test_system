#ifndef Rqst_MaxDemandClear_0x19_H
#define Rqst_MaxDemandClear_0x19_H

#include <QObject>
#include "../datatypedef.h"
#include "../DLT_645_Protocol_Lib_global.h"
#include "../frame645base.h"
#include "../exceptions/decodeexception.h"


namespace dlt_645_Protocol {


class DLT_645_PROTOCOL_LIB_EXPORT Rqst_MaxDemandClear_0x19 : public Frame645Base
{
public:
    Rqst_MaxDemandClear_0x19(uchar *addr, uchar dataLen);


public:
    uchar pwd[4];
    uchar cn[4];


    void DecodeFrameDataField(QByteArray data) override;


protected:
    QByteArray EncodeFrameDataField() override;
};


}

#endif // Rqst_MaxDemandClear_0x19_H
