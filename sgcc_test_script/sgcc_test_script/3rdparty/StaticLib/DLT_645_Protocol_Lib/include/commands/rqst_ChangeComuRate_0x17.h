#ifndef Rqst_ChangeComuRate_0x17_H
#define Rqst_ChangeComuRate_0x17_H

#include <QObject>
#include "../datatypedef.h"
#include "../DLT_645_Protocol_Lib_global.h"
#include "../frame645base.h"
#include "../exceptions/decodeexception.h"


namespace dlt_645_Protocol {


class DLT_645_PROTOCOL_LIB_EXPORT Rqst_ChangeComuRate_0x17 : public Frame645Base
{
public:
    Rqst_ChangeComuRate_0x17(uchar *addr, uchar dataLen);


public:
    uchar taggedWord;

    void DecodeFrameDataField(QByteArray data) override;


protected:
    QByteArray EncodeFrameDataField() override;
};


}

#endif // Rqst_ChangeComuRate_0x17_H
