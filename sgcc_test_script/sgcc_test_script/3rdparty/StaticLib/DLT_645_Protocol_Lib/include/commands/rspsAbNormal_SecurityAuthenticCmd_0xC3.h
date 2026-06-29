#ifndef RspsAbNormal_SecurityAuthenticCmd_0xC3_H
#define RspsAbNormal_SecurityAuthenticCmd_0xC3_H

#include <QObject>
#include "../datatypedef.h"
#include "../DLT_645_Protocol_Lib_global.h"
#include "../frame645base.h"
#include "../exceptions/decodeexception.h"


namespace dlt_645_Protocol {


class DLT_645_PROTOCOL_LIB_EXPORT RspsAbNormal_SecurityAuthenticCmd_0xC3 : public Frame645Base
{
public:
    RspsAbNormal_SecurityAuthenticCmd_0xC3(uchar *addr, uchar dataLen);


public:
    ushort serr;


    void DecodeFrameDataField(QByteArray data) override;


protected:
    QByteArray EncodeFrameDataField() override;
};


}

#endif // RspsAbNormal_SecurityAuthenticCmd_0xC3_H
