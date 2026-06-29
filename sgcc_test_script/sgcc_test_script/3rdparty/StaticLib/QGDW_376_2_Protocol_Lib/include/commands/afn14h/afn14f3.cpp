#include "afn14f3.h"

qgdw_3762_protocol::Afn14F3::Afn14F3()
{
    this->afn_=0x14;
    this->dt1_=0x04;
    this->dt2_=0x00;
}

qgdw_3762_protocol::Afn14F3::Afn14F3(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
:Frame3762Base(ctrl_field,info_field,address_field,0x14,0x04,0x00)
{
    this->ctrl_field_=ctrl_field;
    this->address_field_=address_field;
    this->info_field_=info_field;//

    this->afn_=0x14;
    this->dt1_=0x04;
    this->dt2_=0x00;
}
void qgdw_3762_protocol::Afn14F3::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if(data.length()<9)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        for(int i=0;i<6;i++)
        {
            this->request_unit_up_.node_address_.addr[i]=data.at(5-i);
        }
        this->request_unit_up_.delay_time_=ushort(short(data.at(7))<<8)+uchar(data.at(6));
        this->request_unit_up_.frame_length_=uchar(data.at(8));
        this->request_unit_up_.frame_content_.clear();
        this->request_unit_up_.frame_content_.append(data.mid(9));
    }
    else
    {
        if(data.length()<1)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->request_unit_down_.frame_length_=uchar(data.at(0));
        this->request_unit_down_.frame_content_.append(data.mid(1));
    }
}
QByteArray qgdw_3762_protocol::Afn14F3::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if(this->request_unit_up_.frame_length_!=this->request_unit_up_.frame_content_.size())
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        for(int i=0;i<6;i++)
        {
            data.append(this->request_unit_up_.node_address_.addr[5-i]);
        }
        data.append(char(this->request_unit_up_.delay_time_&0xFF));
        data.append(char(this->request_unit_up_.delay_time_>>8));
        data.append(char(this->request_unit_up_.frame_length_));
        data.append(this->request_unit_up_.frame_content_);
    }
    else
    {
        if(this->request_unit_down_.frame_length_!=this->request_unit_down_.frame_content_.size())
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        data.append(char(this->request_unit_down_.frame_length_));
        data.append(this->request_unit_down_.frame_content_);
    }
    return  data;
}
