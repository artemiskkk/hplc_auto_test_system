#ifndef RspsNormal_MultiFuncTermOtptCtrlCmd_0x9D_H
#define RspsNormal_MultiFuncTermOtptCtrlCmd_0x9D_H

#include <QObject>
#include "../datatypedef.h"
#include "../DLT_645_Protocol_Lib_global.h"
#include "../frame645base.h"
#include "../exceptions/decodeexception.h"


namespace dlt_645_Protocol {


class DLT_645_PROTOCOL_LIB_EXPORT RspsNormal_MultiFuncTermOtptCtrlCmd_0x9D : public Frame645Base
{
public:
    RspsNormal_MultiFuncTermOtptCtrlCmd_0x9D(uchar *addr, uchar dataLen);


public:
    uchar otptWord;

    void DecodeFrameDataField(QByteArray data) override;


protected:
    QByteArray EncodeFrameDataField() override;
};


}

#endif // RspsNormal_MultiFuncTermOtptCtrlCmd_0x9D_H
