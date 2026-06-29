#ifndef RspsNormal_FrozeCmd_0x96_H
#define RspsNormal_FrozeCmd_0x96_H

#include <QObject>
#include "../datatypedef.h"
#include "../DLT_645_Protocol_Lib_global.h"
#include "../frame645base.h"
#include "../exceptions/decodeexception.h"


namespace dlt_645_Protocol {


class DLT_645_PROTOCOL_LIB_EXPORT RspsNormal_FrozeCmd_0x96 : public Frame645Base
{
public:
    RspsNormal_FrozeCmd_0x96(uchar *addr, uchar dataLen);


public:
    void DecodeFrameDataField(QByteArray data) override;


protected:
    QByteArray EncodeFrameDataField() override;
};


}

#endif // RspsNormal_FrozeCmd_0x96_H
