#include "afn10f133.h"

qgdw_3762_protocol::Afn10F133::Afn10F133()
{
    this->afn_=0x10;
    this->dt1_=0x10;
    this->dt2_=0x10;
}
qgdw_3762_protocol::Afn10F133::Afn10F133(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
:Frame3762Base(ctrl_field,info_field,address_field,0x10,0x10,0x10)
{
    this->ctrl_field_=ctrl_field;
    this->address_field_=address_field;
    this->info_field_=info_field;//

    this->afn_=0x10;
    this->dt1_=0x10;
    this->dt2_=0x10;
}
void qgdw_3762_protocol::Afn10F133::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if(data.length()!=2)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        memcpy(&this->router_work_mode_unit_,data,2);
    }
    else
    {
        throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("Unknow Error"));
    }
}
QByteArray qgdw_3762_protocol::Afn10F133::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        char temp[2];
        memcpy(temp,&this->router_work_mode_unit_,2);
        data.append(temp[0]);
        data.append(temp[1]);
    }
    else
    {
    }
    return  data;
}
