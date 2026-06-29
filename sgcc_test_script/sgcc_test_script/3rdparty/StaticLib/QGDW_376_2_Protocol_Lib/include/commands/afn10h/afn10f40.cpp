#include "afn10f40.h"

qgdw_3762_protocol::Afn10F40::Afn10F40()
{
    this->afn_=0x10;
    this->dt1_=char(0x80);
    this->dt2_=0x04;
}

qgdw_3762_protocol::Afn10F40::Afn10F40(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
:Frame3762Base(ctrl_field,info_field,address_field,0x10,char(0x80),0x04)
{
    this->ctrl_field_=ctrl_field;
    this->address_field_=address_field;
    this->info_field_=info_field;//

    this->afn_=0x10;
    this->dt1_=char(0x80);
    this->dt2_=0x04;
}
void qgdw_3762_protocol::Afn10F40::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if(data.length()<8)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->read_chip_id_unit_up_.device_type_=data.at(0);
        data.remove(0,1);
        for(int i=0;i<6;i++)
            this->read_chip_id_unit_up_.mac_address_.addr[i]=data.at(5-i);
        this->read_chip_id_unit_up_.id_type_=data.at(6);

        this->read_chip_id_unit_up_.id_length_=uchar(data.at(7));
        if(this->read_chip_id_unit_up_.id_length_!=data.size()-8)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->read_chip_id_unit_up_.id_content_=data.mid(8);
    }
    else
    {
        if(data.length()!=8)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->read_chip_id_unit_down_.device_type_=data.at(0);
        for(int i=0;i<6;i++)
            this->read_chip_id_unit_down_.mac_address_.addr[i]=data.at(6-i);
        this->read_chip_id_unit_down_.id_type_=data.at(7);
    }
}
QByteArray qgdw_3762_protocol::Afn10F40::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if(this->read_chip_id_unit_up_.id_length_!=this->read_chip_id_unit_up_.id_content_.size())
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        data.append(this->read_chip_id_unit_up_.device_type_);
        for(int i=0;i<6;i++)
            data.append(this->read_chip_id_unit_up_.mac_address_.addr[5-i]);
        data.append(this->read_chip_id_unit_up_.id_type_);
        data.append(char(this->read_chip_id_unit_up_.id_length_));
        data.append(this->read_chip_id_unit_up_.id_content_);
    }
    else
    {
        data.append(this->read_chip_id_unit_down_.device_type_);
        for(int i=0;i<6;i++)
            data.append(this->read_chip_id_unit_down_.mac_address_.addr[5-i]);
        data.append(this->read_chip_id_unit_down_.id_type_);
    }
    return  data;
}
