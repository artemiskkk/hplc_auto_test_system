#include "afn03f16.h"

qgdw_3762_protocol::Afn03F16::Afn03F16()
{
    this->afn_=0x03;
    this->dt1_=char(0x80);
    this->dt2_=0x01;
}

qgdw_3762_protocol::Afn03F16::Afn03F16(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
:Frame3762Base(ctrl_field,info_field,address_field,0x03,char(0x80),0x01)
{
    this->ctrl_field_=ctrl_field;
    this->info_field_=info_field;//
    this->address_field_=address_field;
    this->afn_=0x03;
    this->dt1_=char(0x80);
    this->dt2_=0x01;
}
void qgdw_3762_protocol::Afn03F16::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if(data.size()!=1)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error "));
        }
        this->carrier_frequence_range_=data.at(0);
    }
    else
    {
        if(data.length()!=0)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length error "));
        }
    }
}
QByteArray qgdw_3762_protocol::Afn03F16::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        data.append(this->carrier_frequence_range_);
    }
    return data;
}
