#ifndef RspsNormal_ReadFollowData_0xB2_H
#define RspsNormal_ReadFollowData_0xB2_H

#include <QObject>
#include "../datatypedef.h"
#include "../DLT_645_Protocol_Lib_global.h"
#include "../frame645base.h"
#include "../exceptions/decodeexception.h"


namespace dlt_645_Protocol {


class DLT_645_PROTOCOL_LIB_EXPORT RspsNormal_ReadFollowData_0xB2 : public Frame645Base
{
public:
    RspsNormal_ReadFollowData_0xB2(uchar *addr, uchar dataLen);


public:
    uchar di[4];
    QByteArray data;
    uchar seq;


    void DecodeFrameDataField(QByteArray data) override;


protected:
    QByteArray EncodeFrameDataField() override;
};


}

#endif // RspsNormal_ReadFollowData_0xB2_H
