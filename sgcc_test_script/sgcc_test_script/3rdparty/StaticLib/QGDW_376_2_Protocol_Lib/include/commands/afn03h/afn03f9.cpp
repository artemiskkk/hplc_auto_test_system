#include "afn03f9.h"

qgdw_3762_protocol::Afn03F9::Afn03F9()
{
    this->afn_=0x03;
    this->dt1_=0x01;
    this->dt2_=0x01;
}

qgdw_3762_protocol::Afn03F9::Afn03F9(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
:Frame3762Base(ctrl_field,info_field,address_field,0x03,0x01,0x01)
{
    this->ctrl_field_=ctrl_field;
    this->info_field_=info_field;//
    this->address_field_=address_field;
    this->afn_=0x03;
    this->dt1_=0x01;
    this->dt2_=0x01;
}
void qgdw_3762_protocol::Afn03F9::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if(data.length()<4)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        if(uchar(data.at(3))!=data.size()-4)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Content Error,报文内容错误"));
        }
        this->data_unit_up_.broadcast_communication_time_=ushort(short(data.at(1))<<8)+uchar(data.at(0));
        this->data_unit_up_.communtication_protocol_=data.at(2);
        this->data_unit_up_.frame_length_=uchar(data.at(3));
        this->data_unit_up_.frame_content_=data.mid(4);
    }
    else
    {
        if(data.length()<3)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        if(uchar(data.at(1))!=data.size()-3)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Content Error,报文内容错误"));
        }
        this->data_unit_down_.communtication_protocol_=data.at(0);
        this->data_unit_down_.frame_length_=uchar(data.at(1));
        this->data_unit_down_.frame_content_=data.mid(2,this->data_unit_down_.frame_length_);
        this->data_unit_down_.delay_related_flag_=uchar(data.at(data.size()-1));
    }
}
QByteArray qgdw_3762_protocol::Afn03F9::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {

        if(this->data_unit_up_.frame_length_!=this->data_unit_up_.frame_content_.size())
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Content Error,报文内容错误"));
        }
        data.append(char(this->data_unit_up_.broadcast_communication_time_&0xff));
        data.append(char(this->data_unit_up_.broadcast_communication_time_>>8));
        data.append(char(this->data_unit_up_.communtication_protocol_));
        data.append(char(this->data_unit_up_.frame_length_));
        data.append(this->data_unit_up_.frame_content_);
    }
    else
    {

        if(this->data_unit_down_.frame_length_!=this->data_unit_down_.frame_content_.size())
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Content Error,报文内容错误"));
        }
        data.append(char(this->data_unit_down_.communtication_protocol_));
        data.append(char(this->data_unit_down_.frame_length_));
        data.append(this->data_unit_down_.frame_content_);
        data.append(char(this->data_unit_down_.delay_related_flag_));
    }
    return  data;
}
