#include "afn03f201.h"

qgdw_3762_protocol::Afn03F201::Afn03F201()
{
    this->afn_=0x03;
    this->dt1_=0x01;
    this->dt2_=0x19;
}
qgdw_3762_protocol::Afn03F201::Afn03F201(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
:Frame3762Base(ctrl_field,info_field,address_field,0x03,0x01,0x19)
{
    this->ctrl_field_=ctrl_field;
    this->address_field_=address_field;
    this->info_field_=info_field;//

    this->afn_=0x03;
    this->dt1_=0x01;
    this->dt2_=0x19;
}
void qgdw_3762_protocol::Afn03F201::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if(data.length()!=1)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->sta_attest_flag_=data.at(0);
    }
}
QByteArray qgdw_3762_protocol::Afn03F201::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        data.append(this->sta_attest_flag_);
    }
    return  data;
}
