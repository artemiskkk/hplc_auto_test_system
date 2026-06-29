#include "afn01f3.h"
qgdw_3762_protocol::Afn01F3::Afn01F3()
{
    this->afn_=0x01;
    this->dt1_=0x04;
    this->dt2_=0x00;
}
qgdw_3762_protocol::Afn01F3::Afn01F3(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
        :Frame3762Base(ctrl_field,info_field,address_field,0x01,0x04,0x00)
{
    this->ctrl_field_=ctrl_field;
    this->address_field_=address_field;
    this->info_field_=info_field;//

    this->afn_=0x01;
    this->dt1_=0x04;
    this->dt2_=0x00;
}

void qgdw_3762_protocol::Afn01F3::DecodeFrameDataField(QByteArray data)
{
    if(char(this->ctrl_field_.dir&0x01)==qgdw_3762_protocol::kDirDown)
    {
        if(data.length()!=0)
        {
            throw DecodeException(ExceptionCatalogue::kDataLengthError,"DataField Length != 0");
        }
    }
    else
    {
        throw DecodeException(ExceptionCatalogue::kUnknow,"Unknow Error");
    }
}

QByteArray qgdw_3762_protocol::Afn01F3::EncodeFrameDataField()
{
    QByteArray data;
    if(char(this->ctrl_field_.dir&0x01)==qgdw_3762_protocol::kDirDown)
    {
    }
    else
    {
        throw DecodeException(ExceptionCatalogue::kUnknow,"Unknow Error");
    }
    return data;
}
