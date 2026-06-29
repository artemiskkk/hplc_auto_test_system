#ifndef Rqst_ModifyPswd_0x18_H
#define Rqst_ModifyPswd_0x18_H

#include <QObject>
#include "../datatypedef.h"
#include "../DLT_645_Protocol_Lib_global.h"
#include "../frame645base.h"
#include "../exceptions/decodeexception.h"


namespace dlt_645_Protocol {


class DLT_645_PROTOCOL_LIB_EXPORT Rqst_ModifyPswd_0x18 : public Frame645Base
{
public:
    Rqst_ModifyPswd_0x18(uchar *addr, uchar dataLen);


public:
    uchar di[4];
    uchar pwd1[4];
    uchar pwd2[4];


    void DecodeFrameDataField(QByteArray data) override;


protected:
    QByteArray EncodeFrameDataField() override;
};


}

#endif // Rqst_ModifyPswd_0x18_H
