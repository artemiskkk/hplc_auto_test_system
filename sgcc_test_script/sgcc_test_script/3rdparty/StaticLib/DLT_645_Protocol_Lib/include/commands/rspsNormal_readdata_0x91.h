#ifndef RspsNormal_ReadData_0x91_H
#define RspsNormal_ReadData_0x91_H

#include <QObject>
#include "../datatypedef.h"
#include "../DLT_645_Protocol_Lib_global.h"
#include "../frame645base.h"
#include "../exceptions/decodeexception.h"


namespace dlt_645_Protocol {


class DLT_645_PROTOCOL_LIB_EXPORT RspsNormal_ReadData_0x91 : public Frame645Base
{
public:
    RspsNormal_ReadData_0x91(uchar *addr, uchar dataLen);


public:
    uchar di[4];
    QByteArray data;


    void DecodeFrameDataField(QByteArray data) override;


protected:
    QByteArray EncodeFrameDataField() override;
};


}

#endif // RspsNormal_ReadData_0x91_H
