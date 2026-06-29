#ifndef RSPSABNORMAL_FROZECMD_0XD6_H
#define RSPSABNORMAL_FROZECMD_0XD6_H

#include <QObject>
#include "../datatypedef.h"
#include "../DLT_645_Protocol_Lib_global.h"
#include "../frame645base.h"
#include "../exceptions/decodeexception.h"

namespace dlt_645_Protocol {

class DLT_645_PROTOCOL_LIB_EXPORT RspsAbNormal_FrozeCmd_0xD6 : public Frame645Base
{
public:
    RspsAbNormal_FrozeCmd_0xD6(uchar *addr, uchar dataLen);

public:
    uchar err;


    void DecodeFrameDataField(QByteArray data) override;


protected:
    QByteArray EncodeFrameDataField() override;
};

}

#endif // RSPSABNORMAL_FROZECMD_0XD6_H
