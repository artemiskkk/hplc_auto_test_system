#include "afnf0f39.h"

qgdw_3762_protocol::AfnF0F39::AfnF0F39()
{
    this->afn_=char(0xF0);
    this->dt1_=0x40;
    this->dt2_=0x04;
}

qgdw_3762_protocol::AfnF0F39::AfnF0F39(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
:Frame3762Base(ctrl_field,info_field,address_field,char(0xF0),0x40,0x04)
{
    this->ctrl_field_=ctrl_field;
    this->address_field_=address_field;
    this->info_field_=info_field;//

    this->afn_=char(0xF0);
    this->dt1_=0x40;
    this->dt2_=0x04;
}
void qgdw_3762_protocol::AfnF0F39::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirDown)
    {
        if(data.length()<9)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        for(int i=0;i<6;i++)
            this->config_chip_id_unit_.mac_address_.addr[i]=data.at(5-i);
        this->config_chip_id_unit_.id_type_=data.at(6);
        this->config_chip_id_unit_.id_format_=data.at(7);
        this->config_chip_id_unit_.id_length_=uchar(data.at(8));
        if(this->config_chip_id_unit_.id_length_!=data.size()-9)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->config_chip_id_unit_.id_content_.clear();
        this->config_chip_id_unit_.id_content_=data.mid(9);
    }
    else
    {
        throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("Unknow Error"));
    }
}
QByteArray qgdw_3762_protocol::AfnF0F39::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirDown)
    {
        if(this->config_chip_id_unit_.id_length_!=this->config_chip_id_unit_.id_content_.size())
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        for(int i=0;i<6;i++)
            data.append(this->config_chip_id_unit_.mac_address_.addr[5-i]);
        data.append(this->config_chip_id_unit_.id_type_);
        data.append(this->config_chip_id_unit_.id_format_);
        data.append(char(this->config_chip_id_unit_.id_length_));
        data.append(this->config_chip_id_unit_.id_content_);
    }
    else
    {
        throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("Unknow Error"));
    }
    return  data;
}

