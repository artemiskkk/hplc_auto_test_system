#include "afnf0f44.h"

qgdw_3762_protocol::AfnF0F44::AfnF0F44()
{
    this->afn_=char(0xF0);
    this->dt1_=char(0x08);
    this->dt2_=0x05;
}

qgdw_3762_protocol::AfnF0F44::AfnF0F44(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
:Frame3762Base(ctrl_field,info_field,address_field,char(0xF0),char(0x08),0x05)
{
    this->ctrl_field_=ctrl_field;
    this->address_field_=address_field;
    this->info_field_=info_field;//

    this->afn_=char(0xF0);
    this->dt1_=char(0x08);
    this->dt2_=0x05;
}
void qgdw_3762_protocol::AfnF0F44::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if(data.length()<9)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        for(int i=0;i<6;i++)
            this->read_chip_id_unit_up_.mac_address_.addr[i]=data.at(5-i);
        this->read_chip_id_unit_up_.id_type_=data.at(6);
        this->read_chip_id_unit_up_.id_format_=data.at(7);
        this->read_chip_id_unit_up_.id_length_=uchar(data.at(8))+((data.at(9)&0xff)*256);
        if(this->read_chip_id_unit_up_.id_length_!=data.size()-10)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->read_chip_id_unit_up_.id_content_.clear();
        this->read_chip_id_unit_up_.id_content_=data.mid(10,this->read_chip_id_unit_up_.id_length_);
    }
    else
    {
        if(data.length()!=10)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        for(int i=0;i<6;i++)
            this->read_chip_id_unit_down_.mac_address_.addr[i]=data.at(5-i);
        this->read_chip_id_unit_down_.id_type_=data.at(6);
        this->read_chip_id_unit_down_.id_format_=data.at(7);
        this->read_chip_id_unit_down_.id_length_=uchar(data.at(8))+((data.at(9)&0xff)*256);
    }
}
QByteArray qgdw_3762_protocol::AfnF0F44::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if(this->read_chip_id_unit_up_.id_length_!=this->read_chip_id_unit_up_.id_content_.size())
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        for(int i=0;i<6;i++)
            data.append(this->read_chip_id_unit_up_.mac_address_.addr[5-i]);
        data.append(this->read_chip_id_unit_up_.id_type_);
        data.append(this->read_chip_id_unit_up_.id_format_);
        data.append((const char *) (&this->read_chip_id_unit_up_.id_length_),2);
        data.append(this->read_chip_id_unit_up_.id_content_);
    }
    else
    {
        for(int i=0;i<6;i++)
            data.append(this->read_chip_id_unit_down_.mac_address_.addr[5-i]);
        data.append(this->read_chip_id_unit_down_.id_type_);
        data.append(this->read_chip_id_unit_down_.id_format_);
        data.append((const char *)(&this->read_chip_id_unit_down_.id_length_),2);
    }
    return  data;
}


