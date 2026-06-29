#include "afn03f131_beijing.h"

qgdw_3762_protocol::Afn03F131_Beijing::Afn03F131_Beijing()
{
    this->afn_=0x03;
    this->dt1_=0x04;
    this->dt2_=0x10;
}

qgdw_3762_protocol::Afn03F131_Beijing::Afn03F131_Beijing(const qgdw_3762_protocol::CtrlField ctrl_field, qgdw_3762_protocol::InfoField info_field, qgdw_3762_protocol::AddressField address_field)
:Frame3762Base(ctrl_field,info_field,address_field,0x03,0x04,0x10)
{
    this->ctrl_field_=ctrl_field;
    this->address_field_=address_field;
    this->info_field_=info_field;//

    this->afn_=0x03;
    this->dt1_=0x04;
    this->dt2_=0x10;
}
void qgdw_3762_protocol::Afn03F131_Beijing::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if(data.length()!=1)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length != 9"));
        }
        this->white_list_switch_=data.at(0);
    }
    else
    {
        if(data.length() != 0)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length != 0"));
        }
    }
}
QByteArray qgdw_3762_protocol::Afn03F131_Beijing::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        data.append(this->white_list_switch_);
    }
    else
    {

    }
    return  data;
}
