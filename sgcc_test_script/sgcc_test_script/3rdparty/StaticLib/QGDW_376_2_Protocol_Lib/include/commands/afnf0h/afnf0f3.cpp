#include "afnf0f3.h"

qgdw_3762_protocol::AfnF0F3::AfnF0F3()
{
    this->afn_=char(0xF0);
    this->dt1_=0x04;
    this->dt2_=0x00;
}

qgdw_3762_protocol::AfnF0F3::AfnF0F3(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
:Frame3762Base(ctrl_field,info_field,address_field,char(0xF0),0x04,0x00)
{
    this->ctrl_field_=ctrl_field;
    this->address_field_=address_field;
    this->info_field_=info_field;//

    this->afn_=char(0xF0);
    this->dt1_=0x04;
    this->dt2_=0x00;
}
void qgdw_3762_protocol::AfnF0F3::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirDown)
    {
        if(data.length()!=6)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->memory_setting_unit_.memory_type_=data.at(0);
        this->memory_setting_unit_.start_address_=uint(int(data.at(4))<<24)+uint(int(data.at(3))<<16)+uint(int(data.at(2))<<8)+uchar(data.at(1));
        this->memory_setting_unit_.data_length_=uchar(data.at(5));
    }
    else
    {
        throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("Unknow Error"));
    }
}
QByteArray qgdw_3762_protocol::AfnF0F3::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirDown)
    {
        data.append(this->memory_setting_unit_.memory_type_);

        data.append(char(this->memory_setting_unit_.start_address_&0xFF));
        data.append(char(this->memory_setting_unit_.start_address_>>8));
        data.append(char(this->memory_setting_unit_.start_address_>>16));
        data.append(char(this->memory_setting_unit_.start_address_>>24));

        data.append(char(this->memory_setting_unit_.data_length_));
    }
    else
    {
        throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("Unknow Error"));
    }   return  data;
}
