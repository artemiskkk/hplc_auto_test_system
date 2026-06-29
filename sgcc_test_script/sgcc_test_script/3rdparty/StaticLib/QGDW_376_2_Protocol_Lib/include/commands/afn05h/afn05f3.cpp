#include "afn05f3.h"

qgdw_3762_protocol::Afn05F3::Afn05F3()
{
    this->afn_=0x05;
    this->dt1_=0x04;
    this->dt2_=0x00;
}

qgdw_3762_protocol::Afn05F3::Afn05F3(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
:Frame3762Base(ctrl_field,info_field,address_field,0x05,0x04,0x00)
{
    this->ctrl_field_=ctrl_field;
    this->address_field_=address_field;
    this->info_field_=info_field;//

    this->afn_=0x05;
    this->dt1_=0x04;
    this->dt2_=0x00;
}
void qgdw_3762_protocol::Afn05F3::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirDown)
    {
        if(data.length()<2)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->broadcast_data_unit_.ctrl_word_=data.at(0);
        this->broadcast_data_unit_.frame_length_=uchar(data.at(1));
        if(this->broadcast_data_unit_.frame_length_!=data.size()-2)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->broadcast_data_unit_.frame_content_.clear();
        this->broadcast_data_unit_.frame_content_.append(data.mid(2));
    }
    else
    {
        throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("Unknow Error"));
    }
}
QByteArray qgdw_3762_protocol::Afn05F3::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirDown)
    {
        if(this->broadcast_data_unit_.frame_length_!=this->broadcast_data_unit_.frame_content_.size())
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        data.append(this->broadcast_data_unit_.ctrl_word_);
        data.append(char(this->broadcast_data_unit_.frame_length_));
        data.append(this->broadcast_data_unit_.frame_content_);
    }
    else
    {
        throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("Unknow Error"));
    }
    return  data;
}
