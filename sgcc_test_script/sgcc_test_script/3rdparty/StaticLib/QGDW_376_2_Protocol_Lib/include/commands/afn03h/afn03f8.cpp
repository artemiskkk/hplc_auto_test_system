#include "afn03f8.h"

qgdw_3762_protocol::Afn03F8::Afn03F8()
{
    this->afn_=0x03;
    this->dt1_=char(0x80);
    this->dt2_=0x00;
}

qgdw_3762_protocol::Afn03F8::Afn03F8(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
:Frame3762Base(ctrl_field,info_field,address_field,0x03,char(0x80),0x00)
{
    this->ctrl_field_=ctrl_field;
    this->info_field_=info_field;//
    this->address_field_=address_field;
    this->afn_=0x03;
    this->dt1_=char(0x80);
    this->dt2_=0x00;
}
void qgdw_3762_protocol::Afn03F8::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if(data.length()!=2)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error "));
        }
        this->channel_group_=uchar(data.at(0));
        this->emit_power_=uchar(data.at(1));
    }
    else
    {
        if(data.length()!=0)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length error "));
        }
    }
}
QByteArray qgdw_3762_protocol::Afn03F8::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {        
        data.append(char(this->channel_group_));
        data.append(char(this->emit_power_));
    }
    return data;
}
