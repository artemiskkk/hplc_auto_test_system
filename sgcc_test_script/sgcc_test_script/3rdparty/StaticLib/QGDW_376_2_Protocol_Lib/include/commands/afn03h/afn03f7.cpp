#include "afn03f7.h"

qgdw_3762_protocol::Afn03F7::Afn03F7()
{
    this->afn_=0x03;
    this->dt1_=0x40;
    this->dt2_=0x00;
}

qgdw_3762_protocol::Afn03F7::Afn03F7(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field,const  qgdw_3762_protocol::AddressField address_field)
:Frame3762Base(ctrl_field,info_field,address_field,0x03,0x40,0x00)
{
    this->ctrl_field_=ctrl_field;
    this->info_field_=info_field;//
    this->address_field_=address_field;
    this->afn_=0x03;
    this->dt1_=0x40;
    this->dt2_=0x00;
}
void qgdw_3762_protocol::Afn03F7::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if(data.length()!=1)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length error "));
        }
        this->communication_timeout_=uchar(data.at(0));
    }
    else
    {
        if(data.length()!=0)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length error "));
        }
    }
}
QByteArray qgdw_3762_protocol::Afn03F7::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        data.append(char(this->communication_timeout_));
    }
    return data;
}
