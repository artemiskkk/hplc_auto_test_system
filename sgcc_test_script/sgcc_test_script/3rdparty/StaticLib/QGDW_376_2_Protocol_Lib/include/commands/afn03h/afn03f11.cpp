#include "afn03f11.h"

qgdw_3762_protocol::Afn03F11::Afn03F11()
{
    this->afn_=0x03;
    this->dt1_=0x04;
    this->dt2_=0x01;
}

qgdw_3762_protocol::Afn03F11::Afn03F11(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
:Frame3762Base(ctrl_field,info_field,address_field,0x03,0x04,0x01)
{
    this->ctrl_field_=ctrl_field;
    this->info_field_=info_field;//
    this->address_field_=address_field;
    this->afn_=0x03;
    this->dt1_=0x04;
    this->dt2_=0x01;
    memset(this->fn_support_,0,32);
}
void qgdw_3762_protocol::Afn03F11::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if(data.length()!=33)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error "));
        }
        this->afn_code_=data.at(0);
        data.remove(0,1);
        memcpy(fn_support_,data,32);
    }
    else
    {
        if(data.length()!=1)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length error "));
        }
        this->afn_code_=data.at(0);
    }
}
QByteArray qgdw_3762_protocol::Afn03F11::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        data.append(this->afn_code_);
        for(int i=0;i<32;i++)
        {
            data.append(this->fn_support_[i]);
        }
    }
    else
    {
        data.append(this->afn_code_);
    }
    return data;
}
