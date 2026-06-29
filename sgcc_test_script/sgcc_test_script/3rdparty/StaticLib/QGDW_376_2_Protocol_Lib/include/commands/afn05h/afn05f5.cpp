#include "afn05f5.h"

qgdw_3762_protocol::Afn05F5::Afn05F5()
{
    this->afn_=0x05;
    this->dt1_=0x10;
    this->dt2_=0x00;

}

qgdw_3762_protocol::Afn05F5::Afn05F5(const qgdw_3762_protocol::CtrlField ctrl_field,const  qgdw_3762_protocol::InfoField info_field,const  qgdw_3762_protocol::AddressField address_field)
:Frame3762Base(ctrl_field,info_field,address_field,0x05,0x10,0x00)
{
    this->ctrl_field_=ctrl_field;
    this->address_field_=address_field;
    this->info_field_=info_field;//

    this->afn_=0x05;
    this->dt1_=0x10;
    this->dt2_=0x00;
}
void qgdw_3762_protocol::Afn05F5::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirDown)
    {
        if(data.length()!=2)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        if(uchar(data.at(0))>63)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataContentError,QString("DataField Content Error"));
        }
        this->wireless_channel_group_=data.at(0);
        this->emit_power_=data.at(1);
    }
    else
    {
        throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("Unknow Error"));
    }
}
QByteArray qgdw_3762_protocol::Afn05F5::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirDown)
    {
        if(uchar(this->wireless_channel_group_)>63)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataContentError,QString("DataField Content Error"));
        }
        data.append(this->wireless_channel_group_);
        data.append(this->emit_power_);
    }
    else
    {
        throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("Unknow Error"));
    }
    return  data;
}
