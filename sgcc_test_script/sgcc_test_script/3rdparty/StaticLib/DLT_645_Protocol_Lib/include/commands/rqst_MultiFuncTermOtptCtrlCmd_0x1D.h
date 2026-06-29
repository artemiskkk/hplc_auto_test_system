#ifndef Rqst_MultiFuncTermOtptCtrlCmd_0x1D_H
#define Rqst_MultiFuncTermOtptCtrlCmd_0x1D_H

#include <QObject>
#include "../datatypedef.h"
#include "../DLT_645_Protocol_Lib_global.h"
#include "../frame645base.h"
#include "../exceptions/decodeexception.h"


namespace dlt_645_Protocol {


class DLT_645_PROTOCOL_LIB_EXPORT Rqst_MultiFuncTermOtptCtrlCmd_0x1D : public Frame645Base
{
public:
    Rqst_MultiFuncTermOtptCtrlCmd_0x1D(uchar *addr, uchar dataLen);


public:
    uchar otptWord;

    void DecodeFrameDataField(QByteArray data) override;


protected:
    QByteArray EncodeFrameDataField() override;
};


}

#endif // Rqst_MultiFuncTermOtptCtrlCmd_0x1D_H
