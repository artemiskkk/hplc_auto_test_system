#ifndef RspsNormal_ReadAddr_0x93_H
#define RspsNormal_ReadAddr_0x93_H

#include <QObject>
#include "../datatypedef.h"
#include "../DLT_645_Protocol_Lib_global.h"
#include "../frame645base.h"
#include "../exceptions/decodeexception.h"


namespace dlt_645_Protocol {


class DLT_645_PROTOCOL_LIB_EXPORT RspsNormal_ReadAddr_0x93 : public Frame645Base
{
public:
    RspsNormal_ReadAddr_0x93(uchar *addr, uchar dataLen);


public:
    uchar addr[6];


    void DecodeFrameDataField(QByteArray data) override;


protected:
    QByteArray EncodeFrameDataField() override;
};


}

#endif // RspsNormal_ReadAddr_0x93_H
