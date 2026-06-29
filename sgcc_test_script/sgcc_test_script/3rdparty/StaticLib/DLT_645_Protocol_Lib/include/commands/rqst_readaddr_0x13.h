#ifndef Rqst_ReadAddr_0x13_H
#define Rqst_ReadAddr_0x13_H

#include <QObject>
#include "../datatypedef.h"
#include "../DLT_645_Protocol_Lib_global.h"
#include "../frame645base.h"
#include "../exceptions/decodeexception.h"


namespace dlt_645_Protocol {


class DLT_645_PROTOCOL_LIB_EXPORT Rqst_ReadAddr_0x13 : public Frame645Base
{
public:
    Rqst_ReadAddr_0x13(uchar *addr, uchar dataLen);


public:
    void DecodeFrameDataField(QByteArray data) override;


protected:
    QByteArray EncodeFrameDataField() override;
};


}

#endif // Rqst_ReadAddr_0x13_H
