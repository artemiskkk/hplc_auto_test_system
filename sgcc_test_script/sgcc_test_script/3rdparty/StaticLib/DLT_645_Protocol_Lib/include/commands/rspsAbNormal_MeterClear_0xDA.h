#ifndef RspsAbNormal_MeterClear_0xDA_H
#define RspsAbNormal_MeterClear_0xDA_H

#include <QObject>
#include "../datatypedef.h"
#include "../DLT_645_Protocol_Lib_global.h"
#include "../frame645base.h"
#include "../exceptions/decodeexception.h"


namespace dlt_645_Protocol {


class DLT_645_PROTOCOL_LIB_EXPORT RspsAbNormal_MeterClear_0xDA : public Frame645Base
{
public:
    RspsAbNormal_MeterClear_0xDA(uchar *addr, uchar dataLen);


public:
    uchar err;


    void DecodeFrameDataField(QByteArray data) override;


protected:
    QByteArray EncodeFrameDataField() override;
};


}

#endif // RspsAbNormal_MeterClear_0xDA_H
