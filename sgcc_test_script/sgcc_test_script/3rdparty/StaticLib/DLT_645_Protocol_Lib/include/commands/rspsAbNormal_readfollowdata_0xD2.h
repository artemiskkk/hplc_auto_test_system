#ifndef RspsAbNormal_ReadFollowData_0xD2_H
#define RspsAbNormal_ReadFollowData_0xD2_H

#include <QObject>
#include "../datatypedef.h"
#include "../DLT_645_Protocol_Lib_global.h"
#include "../frame645base.h"
#include "../exceptions/decodeexception.h"


namespace dlt_645_Protocol {


class DLT_645_PROTOCOL_LIB_EXPORT RspsAbNormal_ReadFollowData_0xD2 : public Frame645Base
{
public:
    RspsAbNormal_ReadFollowData_0xD2(uchar *addr, uchar dataLen);


public:
    uchar err;


    void DecodeFrameDataField(QByteArray data) override;


protected:
    QByteArray EncodeFrameDataField() override;
};


}

#endif // RspsAbNormal_ReadFollowData_0xD2_H
