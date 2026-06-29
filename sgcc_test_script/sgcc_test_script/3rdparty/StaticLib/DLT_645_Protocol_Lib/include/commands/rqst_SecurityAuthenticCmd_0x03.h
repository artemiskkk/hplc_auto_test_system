#ifndef Rqst_SecurityAuthenticCmd_0x03_H
#define Rqst_SecurityAuthenticCmd_0x03_H

#include <QObject>
#include "../datatypedef.h"
#include "../DLT_645_Protocol_Lib_global.h"
#include "../frame645base.h"
#include "../exceptions/decodeexception.h"


namespace dlt_645_Protocol {


class DLT_645_PROTOCOL_LIB_EXPORT Rqst_SecurityAuthenticCmd_0x03 : public Frame645Base
{
public:
    Rqst_SecurityAuthenticCmd_0x03(uchar *addr, uchar dataLen);


public:
    uchar di[4];
    uchar cn[4];
    uchar nn[4];


    void DecodeFrameDataField(QByteArray data) override;


protected:
    QByteArray EncodeFrameDataField() override;
};


}

#endif // Rqst_SecurityAuthenticCmd_0x03_H
