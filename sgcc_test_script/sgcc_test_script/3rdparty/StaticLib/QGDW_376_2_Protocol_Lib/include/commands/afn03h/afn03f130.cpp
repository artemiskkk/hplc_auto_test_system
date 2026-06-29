#include "afn03f130.h"

qgdw_3762_protocol::Afn03F130::Afn03F130()
{
    this->afn_=0x03;
    this->dt1_=0x02;
    this->dt2_=0x10;
}

qgdw_3762_protocol::Afn03F130::Afn03F130(const qgdw_3762_protocol::CtrlField ctrl_field, qgdw_3762_protocol::InfoField info_field, qgdw_3762_protocol::AddressField address_field)
:Frame3762Base(ctrl_field,info_field,address_field,0x03,0x02,0x10)
{
    this->ctrl_field_=ctrl_field;
    this->address_field_=address_field;
    this->info_field_=info_field;//

    this->afn_=0x03;
    this->dt1_=0x02;
    this->dt2_=0x10;
}
void qgdw_3762_protocol::Afn03F130::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if(data.length()<11)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length != 9"));
        }
        this->data_unit_up_.cpu_no_=data.at(0);
        data.remove(0,1);
        memcpy(this->data_unit_up_.vendor_code_,data,2);
        data.remove(0,2);
        this->data_unit_up_.version_date_[0]=char(uchar(data.at(0))>>3);
        this->data_unit_up_.version_date_[1]=char(data.at(1)&0x0F);
        this->data_unit_up_.version_date_[2]=char((ushort(data.at(2))<<4)+(uchar(data.at(1)&0xF0)>>4)-2000);
        data.remove(0,3);
        memcpy(this->data_unit_up_.hardware_version_,data,2);
        data.remove(0,2);
        memcpy(this->data_unit_up_.software_version_,data,2);
        data.remove(0,2);
        this->data_unit_up_.mcu_type_len_=uchar(data.at(0));
        data.remove(0,1);
        if(data.length()!=this->data_unit_up_.mcu_type_len_)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length != 9"));
        }
        this->data_unit_up_.mcu_type_=data;
    }
    else
    {
        if(data.length() != 0)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length != 0"));
        }
    }
}
QByteArray qgdw_3762_protocol::Afn03F130::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if(this->data_unit_up_.mcu_type_.length()!=this->data_unit_up_.mcu_type_len_)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length != 9"));
        }
        data.append(this->data_unit_up_.cpu_no_);
        data.append(this->data_unit_up_.vendor_code_,2);
        data.append(char(this->data_unit_up_.version_date_[0]<<3));
        ushort temp=ushort(this->data_unit_up_.version_date_[2])+2000;
        data.append(char(uchar(this->data_unit_up_.version_date_[1]&0x0F)+uchar((temp&0x0F)<<4)));
        data.append(char(temp>>4));
        data.append(this->data_unit_up_.hardware_version_,2);
        data.append(this->data_unit_up_.software_version_,2);
        data.append(char(this->data_unit_up_.mcu_type_len_));
        data.append(this->data_unit_up_.mcu_type_);
    }
    return  data;
}
