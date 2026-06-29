#ifndef RspsNormal_SecurityAuthenticCmd_0x83_H
#define RspsNormal_SecurityAuthenticCmd_0x83_H

#include <QObject>
#include "../datatypedef.h"
#include "../DLT_645_Protocol_Lib_global.h"
#include "../frame645base.h"
#include "../exceptions/decodeexception.h"


namespace dlt_645_Protocol {


class DLT_645_PROTOCOL_LIB_EXPORT RspsNormal_SecurityAuthenticCmd_0x83 : public Frame645Base
{
public:
    RspsNormal_SecurityAuthenticCmd_0x83(uchar *addr, uchar dataLen);


public:
    uchar di[4];
    uchar nn[4];

    void DecodeFrameDataField(QByteArray data) override;


protected:
    QByteArray EncodeFrameDataField() override;
};


}

#endif // RspsNormal_SecurityAuthenticCmd_0x83_H
