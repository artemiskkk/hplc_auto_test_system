#ifndef Rqst_WriteAddr_0x15_H
#define Rqst_WriteAddr_0x15_H

#include <QObject>
#include "../datatypedef.h"
#include "../DLT_645_Protocol_Lib_global.h"
#include "../frame645base.h"
#include "../exceptions/decodeexception.h"


namespace dlt_645_Protocol {


class DLT_645_PROTOCOL_LIB_EXPORT Rqst_WriteAddr_0x15 : public Frame645Base
{
public:
    Rqst_WriteAddr_0x15(uchar *addr, uchar dataLen);


public:
    uchar addr[6];

    void DecodeFrameDataField(QByteArray data) override;


protected:
    QByteArray EncodeFrameDataField() override;
};


}

#endif // Rqst_WriteAddr_0x15_H
