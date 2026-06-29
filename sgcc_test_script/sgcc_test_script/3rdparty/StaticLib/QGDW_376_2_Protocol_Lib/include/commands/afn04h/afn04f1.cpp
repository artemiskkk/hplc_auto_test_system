#include "afn04f1.h"

qgdw_3762_protocol::Afn04F1::Afn04F1()
{
    this->afn_=0x04;
    this->dt1_=0x01;
    this->dt2_=0x00;
}

qgdw_3762_protocol::Afn04F1::Afn04F1(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
:Frame3762Base(ctrl_field,info_field,address_field,0x04,0x01,0x00)
{
    this->ctrl_field_=ctrl_field;
    this->address_field_=address_field;
    this->info_field_=info_field;//

    this->afn_=0x04;
    this->dt1_=0x01;
    this->dt2_=0x00;
}
void qgdw_3762_protocol::Afn04F1::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirDown)
    {
        if(data.length()!=1)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->durate_time_=uchar(data.at(0));
    }
    else
    {
        throw DecodeException(ExceptionCatalogue::kDataLengthError,"Unknow Error");
    }
}
QByteArray qgdw_3762_protocol::Afn04F1::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirDown)
    {
        data.append(char(this->durate_time_));
    }
    else
    {
        throw DecodeException(ExceptionCatalogue::kDataLengthError,"Unknow Error");
    }
    return  data;
}
