#ifndef RQST_WRITEDATA_CURRENT_0X14_H
#define RQST_WRITEDATA_CURRENT_0X14_H

#include <QObject>
#include "../datatypedef.h"
#include "../DLT_645_Protocol_Lib_global.h"
#include "../frame645base.h"
#include "../exceptions/decodeexception.h"
namespace dlt_645_Protocol {

struct CurrentStruct
{
    char devive_type_;
    char send_time_[6];
    ushort bit_width_;
    bool need_Frequence_=false;
    double frequence_;
    uchar word_info_length_=0x02;
    QByteArray word_info_;
    ushort high_pulse_width_;
    ushort low_pulse_width_;
};

class DLT_645_PROTOCOL_LIB_EXPORT rqst_WriteData_Current_0x14 : public Frame645Base
{
public:
    rqst_WriteData_Current_0x14(uchar *addr, uchar dataLen);


public:
    uchar di[4];
    CurrentStruct current_data_;


    void DecodeFrameDataField(QByteArray data) override;

protected:
    QByteArray EncodeFrameDataField() override;
};


}
#endif // RQST_WRITEDATA_CURRENT_0X14_H
