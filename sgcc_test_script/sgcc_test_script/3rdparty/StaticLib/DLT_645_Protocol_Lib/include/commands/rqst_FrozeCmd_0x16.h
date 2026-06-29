#ifndef Rqst_FrozeCmd_0x16_H
#define Rqst_FrozeCmd_0x16_H

#include <QObject>
#include "../datatypedef.h"
#include "../DLT_645_Protocol_Lib_global.h"
#include "../frame645base.h"
#include "../exceptions/decodeexception.h"


namespace dlt_645_Protocol {


class DLT_645_PROTOCOL_LIB_EXPORT Rqst_FrozeCmd_0x16 : public Frame645Base
{
public:
    Rqst_FrozeCmd_0x16(uchar *addr, uchar dataLen);


public:
    uchar date[4];


    void DecodeFrameDataField(QByteArray data) override;


protected:
    QByteArray EncodeFrameDataField() override;
};


}

#endif // Rqst_FrozeCmd_0x16_H
