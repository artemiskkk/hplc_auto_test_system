#ifndef Rpt_ClkErrRptHuNan_0x9F_H
#define Rpt_ClkErrRptHuNan_0x9F_H

#include <QObject>
#include "../datatypedef.h"
#include "../DLT_645_Protocol_Lib_global.h"
#include "../frame645base.h"
#include "../exceptions/decodeexception.h"



namespace dlt_645_Protocol {


#ifdef UNIT_TEST
class Rpt_ClkErrRptHuNan_0x9F : public Frame645Base
#else
class DLT_645_PROTOCOL_LIB_EXPORT Rpt_ClkErrRptHuNan_0x9F : public Frame645Base
#endif
{
public:
    Rpt_ClkErrRptHuNan_0x9F(uchar *addr, uchar dataLen);


public:
    uchar errMode;
    uchar errDateTime[6];

    void DecodeFrameDataField(QByteArray data) override;


protected:
    QByteArray EncodeFrameDataField() override;
};


}

#endif // Rpt_ClkErrRptHuNan_0x9F_H
