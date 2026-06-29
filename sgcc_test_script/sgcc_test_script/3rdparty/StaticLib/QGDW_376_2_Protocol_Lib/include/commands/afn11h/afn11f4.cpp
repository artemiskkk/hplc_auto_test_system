#include "afn11f4.h"

qgdw_3762_protocol::Afn11F4::Afn11F4()
{
    this->afn_=0x11;
    this->dt1_=0x08;
    this->dt2_=0x00;
}

qgdw_3762_protocol::Afn11F4::Afn11F4(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
:Frame3762Base(ctrl_field,info_field,address_field,0x11,0x08,0x00)
{
    this->ctrl_field_=ctrl_field;
    this->address_field_=address_field;
    this->info_field_=info_field;//

    this->afn_=0x11;
    this->dt1_=0x08;
    this->dt2_=0x00;
}
void qgdw_3762_protocol::Afn11F4::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirDown)
    {
        if(data.length()!=3)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        memcpy(&this->work_mode_,data,1);
        data.remove(0,1);
        memcpy(&this->com_rate_unit_,data,2);
    }
    else
    {
        throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("Unknow Error"));
    }
}
QByteArray qgdw_3762_protocol::Afn11F4::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirDown)
    {
        data.append(*reinterpret_cast<char*>(&this->work_mode_));
        char temp[2];
        memcpy(temp,&this->com_rate_unit_,2);
        data.append(temp[1]);
        data.append(temp[0]);
    }
    else
    {
        throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("Unknow Error"));
    }
    return  data;
}


