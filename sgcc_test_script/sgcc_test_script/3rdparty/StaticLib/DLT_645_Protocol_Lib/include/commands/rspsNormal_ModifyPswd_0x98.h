#ifndef RspsNormal_ModifyPswd_0x98_H
#define RspsNormal_ModifyPswd_0x98_H

#include <QObject>
#include "../datatypedef.h"
#include "../DLT_645_Protocol_Lib_global.h"
#include "../frame645base.h"
#include "../exceptions/decodeexception.h"


namespace dlt_645_Protocol {


class DLT_645_PROTOCOL_LIB_EXPORT RspsNormal_ModifyPswd_0x98 : public Frame645Base
{
public:
    RspsNormal_ModifyPswd_0x98(uchar *addr, uchar dataLen);


public:
    uchar pwd2[4];

    void DecodeFrameDataField(QByteArray data) override;


protected:
    QByteArray EncodeFrameDataField() override;
};


}

#endif // RspsNormal_ModifyPswd_0x98_H
