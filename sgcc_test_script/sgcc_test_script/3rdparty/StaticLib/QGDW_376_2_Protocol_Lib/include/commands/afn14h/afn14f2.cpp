#include "afn14f2.h"
qgdw_3762_protocol::Afn14F2::Afn14F2()
{
    this->afn_=0x14;
    this->dt1_=0x02;
    this->dt2_=0x00;
}
qgdw_3762_protocol::Afn14F2::Afn14F2(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
:Frame3762Base(ctrl_field,info_field,address_field,0x14,0x02,0x00)
{
    this->ctrl_field_=ctrl_field;
    this->address_field_=address_field;
    this->info_field_=info_field;//

    this->afn_=0x14;
    this->dt1_=0x02;
    this->dt2_=0x00;
}
void qgdw_3762_protocol::Afn14F2::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirDown)
    {
        if(data.length()!=6)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        memcpy(&this->current_time_,data,6);
    }
    else
    {
        if(data.length()!=0)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
    }
}
QByteArray qgdw_3762_protocol::Afn14F2::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirDown)
    {
        char temp[6];
        memcpy(temp,&this->current_time_,6);
        for(int i=0;i<6;i++)
            data.append(temp[i]);
    }
    return  data;
}
