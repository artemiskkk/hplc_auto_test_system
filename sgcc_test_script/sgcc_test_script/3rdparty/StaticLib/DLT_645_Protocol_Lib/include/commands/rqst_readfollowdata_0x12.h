#ifndef Rqst_ReadFollowData_0x12_H
#define Rqst_ReadFollowData_0x12_H

#include <QObject>
#include "../datatypedef.h"
#include "../DLT_645_Protocol_Lib_global.h"
#include "../frame645base.h"
#include "../exceptions/decodeexception.h"


namespace dlt_645_Protocol {


class DLT_645_PROTOCOL_LIB_EXPORT Rqst_ReadFollowData_0x12 : public Frame645Base
{
public:
    Rqst_ReadFollowData_0x12(uchar *addr, uchar dataLen);


public:
    uchar di[4];
    uchar seq;


    void DecodeFrameDataField(QByteArray data) override;


protected:
    QByteArray EncodeFrameDataField() override;
};


}

#endif // Rqst_ReadFollowData_0x12_H
