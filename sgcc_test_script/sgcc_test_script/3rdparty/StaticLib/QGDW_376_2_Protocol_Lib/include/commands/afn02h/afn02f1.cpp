#include "afn02f1.h"
qgdw_3762_protocol::Afn02F1::Afn02F1()
{
    this->afn_=0x02;
    this->dt1_=0x01;
    this->dt2_=0x00;
}
qgdw_3762_protocol::Afn02F1::Afn02F1(const qgdw_3762_protocol::CtrlField ctrl_field, qgdw_3762_protocol::InfoField info_field, qgdw_3762_protocol::AddressField address_field)
    :Frame3762Base(ctrl_field,info_field,address_field,0x02,0x01,0x00)
{
    this->ctrl_field_=ctrl_field;
    this->address_field_=address_field;
    this->info_field_=info_field;//

    this->afn_=0x02;
    this->dt1_=0x01;
    this->dt2_=0x00;
}

void qgdw_3762_protocol::Afn02F1::DecodeFrameDataField(QByteArray data)
{
    if(char(this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirDown)
    {
        if(data.length()<2)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->protocol_type_=data.at(0);
        this->frame_length_=uchar(data.at(1));
        if(this->frame_length_!=data.size()-2)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->frame_content_.clear();
        this->frame_content_.append(data.mid(2));
    }
    else
    {
        if(data.length()<2)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->protocol_type_=data.at(0);
        this->frame_length_=uchar(data.at(1));
        if(this->frame_length_!=data.size()-2)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->frame_content_.append(data.mid(2));
    }
}

QByteArray qgdw_3762_protocol::Afn02F1::EncodeFrameDataField()
{
    QByteArray data;
    if(this->frame_length_!=this->frame_content_.size())
    {
        throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
    }
    data.append(this->protocol_type_);
    data.append(char(this->frame_length_));
    data.append(this->frame_content_);
    return data;
}
