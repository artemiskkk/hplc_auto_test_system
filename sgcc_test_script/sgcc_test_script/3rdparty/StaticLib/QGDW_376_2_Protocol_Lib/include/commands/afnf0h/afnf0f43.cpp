#include "afnf0f43.h"

qgdw_3762_protocol::AfnF0F43::AfnF0F43()
{
    this->afn_=char(0xF0);
    this->dt1_=0x04;
    this->dt2_=0x05;
}

qgdw_3762_protocol::AfnF0F43::AfnF0F43(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
:Frame3762Base(ctrl_field,info_field,address_field,char(0xF0),0x04,0x05)
{
    this->ctrl_field_=ctrl_field;
    this->address_field_=address_field;
    this->info_field_=info_field;//

    this->afn_=char(0xF0);
    this->dt1_=0x04;
    this->dt2_=0x05;
}
void qgdw_3762_protocol::AfnF0F43::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirDown)
    {
        if(data.length()!=26)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        for(int i=0;i<6;i++)
            this->mac_address_.addr[i]=data.at(5-i);
        this->id_type_=data.at(6);
        this->id_formate_=data.at(7);
        this->id_length_=data.at(8);
        this->id_length_=(data.at(9)&0xff)+((data.at(10)&0xff)*256);
        if(data.length()!=10+ this->id_length_)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->id_content_.clear();
        this->id_content_=data.mid(11,this->id_length_);
    }
    else
    {
        throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("Unknow Error"));
    }
}
QByteArray qgdw_3762_protocol::AfnF0F43::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirDown)
    {
        if(this->id_content_.size()!=this->id_length_)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }

        for(int i=0;i<6;i++)
            data.append(this->mac_address_.addr[5-i]);
        data.append(this->id_type_);
        data.append(this->id_formate_);
        data.append((const char*)&this->id_length_,2);
        data.append(this->id_content_);
    }
    else
    {
        throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("Unknow Error"));
    }
    return  data;
}

