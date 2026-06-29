#ifndef RspsNormal_EvntClear_0x9B_H
#define RspsNormal_EvntClear_0x9B_H

#include <QObject>
#include "../datatypedef.h"
#include "../DLT_645_Protocol_Lib_global.h"
#include "../frame645base.h"
#include "../exceptions/decodeexception.h"


namespace dlt_645_Protocol {


class DLT_645_PROTOCOL_LIB_EXPORT RspsNormal_EvntClear_0x9B : public Frame645Base
{
public:
    RspsNormal_EvntClear_0x9B(uchar *addr, uchar dataLen);


public:

    void DecodeFrameDataField(QByteArray data) override;


protected:
    QByteArray EncodeFrameDataField() override;
};


}

#endif // RspsNormal_EvntClear_0x9B_H
