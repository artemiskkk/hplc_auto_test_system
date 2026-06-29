#include "afnf0f89.h"

qgdw_3762_protocol::AfnF0F89::AfnF0F89()
{
    this->afn_=char(0xF0);
    this->dt1_=char(0x01);
    this->dt2_=0x0B;
}

qgdw_3762_protocol::AfnF0F89::AfnF0F89(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
    :Frame3762Base(ctrl_field,info_field,address_field,char(0xF0),0x01,0x0B)
{
    this->afn_=char(0xF0);
    this->dt1_=char(0x01);
    this->dt2_=0x0B;
}

void qgdw_3762_protocol::AfnF0F89::DecodeFrameDataField(QByteArray data)
{
    if(data.length()!=0)
    {
        throw DecodeException(ExceptionCatalogue::kDataLengthError,"DataField Length != 0");
    }
}

QByteArray qgdw_3762_protocol::AfnF0F89::EncodeFrameDataField()
{
    QByteArray data_frame;
    data_frame.append(char(freq_));
    return data_frame;
}
