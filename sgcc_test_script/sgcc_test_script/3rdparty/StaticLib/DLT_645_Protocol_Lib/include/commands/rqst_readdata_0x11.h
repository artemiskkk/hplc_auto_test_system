#ifndef RQST_READDATA_0X11_H
#define RQST_READDATA_0X11_H

#include <QObject>
#include "../datatypedef.h"
#include "../DLT_645_Protocol_Lib_global.h"
#include "../frame645base.h"
#include "../exceptions/decodeexception.h"


namespace dlt_645_Protocol {


#ifdef UNIT_TEST
class Rqst_ReadData_0x11 : public Frame645Base
#else
class DLT_645_PROTOCOL_LIB_EXPORT Rqst_ReadData_0x11 : public Frame645Base
#endif
{
public:
    Rqst_ReadData_0x11(uchar *addr, uchar dataLen, uchar m_DataLen);


public:
    uchar m_DataLen=0;

    uchar di[4];
    uchar blockOfRcrd;
    uchar dateTime[5];
    QByteArray programCompareData;//!<湖南协议程序比对


    void DecodeFrameDataField(QByteArray data) override;


protected:
    QByteArray EncodeFrameDataField() override;
};


}

#endif // RQST_READDATA_0X11_H
