#ifndef Rqst_WriteData_0x14_H
#define Rqst_WriteData_0x14_H

#include <QObject>
#include "../datatypedef.h"
#include "../DLT_645_Protocol_Lib_global.h"
#include "../frame645base.h"
#include "../exceptions/decodeexception.h"


namespace dlt_645_Protocol {


class DLT_645_PROTOCOL_LIB_EXPORT Rqst_WriteData_0x14 : public Frame645Base
{
public:
    Rqst_WriteData_0x14(uchar *addr, uchar dataLen);


public:
    uchar di[4];
    uchar pwd[4];
    uchar opd[4];
    QByteArray data;


    void DecodeFrameDataField(QByteArray data) override;


protected:
    QByteArray EncodeFrameDataField() override;
};


}

#endif // Rqst_WriteData_0x14_H
