#include "afnf0f88.h"

qgdw_3762_protocol::AfnF0F88::AfnF0F88()
{
    this->afn_=char(0xF0);
    this->dt1_=char(0x80);
    this->dt2_=0x0A;
}

qgdw_3762_protocol::AfnF0F88::AfnF0F88(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
    :Frame3762Base(ctrl_field,info_field,address_field,char(0xF0),char(0x80),0x0A)
{
    this->afn_=char(0xF0);
    this->dt1_=char(0x80);
    this->dt2_=0x0A;
}

void qgdw_3762_protocol::AfnF0F88::DecodeFrameDataField(QByteArray data)
{
    if(data.length()!=0)
    {
        throw DecodeException(ExceptionCatalogue::kDataLengthError,"DataField Length != 0");
    }
}

QByteArray qgdw_3762_protocol::AfnF0F88::EncodeFrameDataField()
{
    QByteArray data_frame;
    data_frame.append(char(freq_));
    return data_frame;
}
