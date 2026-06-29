#include "afn03f70.h"

qgdw_3762_protocol::Afn03F70::Afn03F70()
{
    this->afn_=0x03;
    this->dt1_=char(0x20);
    this->dt2_=0x08;
}

qgdw_3762_protocol::Afn03F70::Afn03F70(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
:Frame3762Base(ctrl_field,info_field,address_field,0x03,char(0x20),0x08)
{
    this->ctrl_field_=ctrl_field;
    this->info_field_=info_field;//
    this->address_field_=address_field;
    this->afn_=0x03;
    this->dt1_=char(0x20);
    this->dt2_=0x08;
}
void qgdw_3762_protocol::Afn03F70::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if(data.length()!=6)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error "));
        }
        for(int i=0;i<6;i++)
        {
            this->mac_[i]=data.at(5-i);
        }
    }
    else
    {
        if(data.length()!=0)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length error "));
        }
    }
}
QByteArray qgdw_3762_protocol::Afn03F70::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirDown)
    {

    }
    else
    {
        for(int i=0;i<6;i++)
        {
            data.append(this->mac_[5-i]);
        }
    }
    return data;
}
