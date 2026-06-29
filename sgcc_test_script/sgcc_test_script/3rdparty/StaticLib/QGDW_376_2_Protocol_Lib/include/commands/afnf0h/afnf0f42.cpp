#include "afnf0f42.h"

qgdw_3762_protocol::AfnF0F42::AfnF0F42()
{
    this->afn_=char(0xF0);
    this->dt1_=0x02;
    this->dt2_=0x05;
}

qgdw_3762_protocol::AfnF0F42::AfnF0F42(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
:Frame3762Base(ctrl_field,info_field,address_field,char(0xF0),0x02,0x05)
{
    this->ctrl_field_=ctrl_field;
    this->address_field_=address_field;
    this->info_field_=info_field;//

    this->afn_=char(0xF0);
    this->dt1_=0x02;
    this->dt2_=0x05;
}
void qgdw_3762_protocol::AfnF0F42::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirDown)
    {
        if(data.length()!=26)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->device_type_=data.at(0);
        for(int i=0;i<6;i++)
            this->mac_address_.addr[i]=data.at(6-i);
        this->sn_content_.clear();
        this->sn_content_=data.mid(7,19);
    }
    else
    {
        throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("Unknow Error"));
    }
}
QByteArray qgdw_3762_protocol::AfnF0F42::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirDown)
    {
        if(this->sn_content_.size()!=19)
        {
            //throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        data.append(this->device_type_);
        for(int i=0;i<6;i++)
            data.append(this->mac_address_.addr[5-i]);
        data.append(this->sn_content_);
    }
    else
    {
        throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("Unknow Error"));
    }
    return  data;
}

