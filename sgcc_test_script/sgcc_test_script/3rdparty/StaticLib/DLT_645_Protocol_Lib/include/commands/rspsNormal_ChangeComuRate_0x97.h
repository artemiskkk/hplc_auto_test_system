#ifndef RspsNormal_ChangeComuRate_0x97_H
#define RspsNormal_ChangeComuRate_0x97_H

#include <QObject>
#include "../datatypedef.h"
#include "../DLT_645_Protocol_Lib_global.h"
#include "../frame645base.h"
#include "../exceptions/decodeexception.h"


namespace dlt_645_Protocol {


class DLT_645_PROTOCOL_LIB_EXPORT RspsNormal_ChangeComuRate_0x97 : public Frame645Base
{
public:
    RspsNormal_ChangeComuRate_0x97(uchar *addr, uchar dataLen);


public:
    uchar taggedWord;

    void DecodeFrameDataField(QByteArray data) override;


protected:
    QByteArray EncodeFrameDataField() override;
};


}

#endif // RspsNormal_ChangeComuRate_0x97_H
