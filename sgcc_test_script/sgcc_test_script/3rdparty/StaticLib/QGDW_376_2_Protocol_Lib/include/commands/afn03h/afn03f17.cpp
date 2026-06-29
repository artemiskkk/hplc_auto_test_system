#include "afn03f17.h"

qgdw_3762_protocol::Afn03F17::Afn03F17()
{
    this->afn_=0x03;
    this->dt1_=char(0x01);
    this->dt2_=0x02;
}

qgdw_3762_protocol::Afn03F17::Afn03F17(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
:Frame3762Base(ctrl_field,info_field,address_field,0x03,char(0x01),0x02)
{
    this->ctrl_field_=ctrl_field;
    this->info_field_=info_field;//
    this->address_field_=address_field;
    this->afn_=0x03;
    this->dt1_=char(0x01);
    this->dt2_=0x02;
}
void qgdw_3762_protocol::Afn03F17::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if(data.size()!=3)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error "));
        }
        this->wireless_module_=data.at(0);
        this->wireless_channel_=data.at(1);
        this->channel_enable_flag_=data.at(2);
    }
    else
    {
        if(data.length()!=0)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length error "));
        }
    }
}

QByteArray qgdw_3762_protocol::Afn03F17::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        data.append(this->wireless_module_);
        data.append(this->wireless_channel_);
        data.append(this->channel_enable_flag_);
    }
    return data;
}
