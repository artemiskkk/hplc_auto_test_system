#ifndef RspsNormal_MaxDemandClear_0x99_H
#define RspsNormal_MaxDemandClear_0x99_H

#include <QObject>
#include "../datatypedef.h"
#include "../DLT_645_Protocol_Lib_global.h"
#include "../frame645base.h"
#include "../exceptions/decodeexception.h"


namespace dlt_645_Protocol {


class DLT_645_PROTOCOL_LIB_EXPORT RspsNormal_MaxDemandClear_0x99 : public Frame645Base
{
public:
    RspsNormal_MaxDemandClear_0x99(uchar *addr, uchar dataLen);


public:

    void DecodeFrameDataField(QByteArray data) override;


protected:
    QByteArray EncodeFrameDataField() override;
};


}

#endif // RspsNormal_MaxDemandClear_0x99_H
