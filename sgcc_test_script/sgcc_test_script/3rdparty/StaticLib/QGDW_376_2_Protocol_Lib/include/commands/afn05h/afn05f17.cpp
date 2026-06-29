#include "afn05f17.h"
#include "afn05f16.h"
qgdw_3762_protocol::Afn05F17::Afn05F17()
{
    this->afn_=0x05;
    this->dt1_=char(0x01);
    this->dt2_=0x02;
}
qgdw_3762_protocol::Afn05F17::Afn05F17(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
:Frame3762Base(ctrl_field,info_field,address_field,0x05,char(0x01),0x02)
{
    this->ctrl_field_=ctrl_field;
    this->address_field_=address_field;
    this->info_field_=info_field;//

    this->afn_=0x05;
    this->dt1_=char(0x01);
    this->dt2_=0x05;
}
void qgdw_3762_protocol::Afn05F17::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirDown)
    {

        this->wireless_module_=data.at(0);
        this->wireless_channel_=data.at(1);
        this->channel_enable_flag_=data.at(2);
    }
    else
    {
        throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("Unknow Error"));
    }
}
QByteArray qgdw_3762_protocol::Afn05F17::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirDown)
    {

        data.append(this->wireless_module_);
        data.append(this->wireless_channel_);
        data.append(this->channel_enable_flag_);
    }
    else
    {
        throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("Unknow Error"));
    }
    return  data;
}


