#include "afn06f5.h"

qgdw_3762_protocol::Afn06F5::Afn06F5()
{
    this->afn_=0x06;
    this->dt1_=0x10;
    this->dt2_=0x00;
}

qgdw_3762_protocol::Afn06F5::Afn06F5(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
:Frame3762Base(ctrl_field,info_field,address_field,0x06,0x10,0x00)
{
    this->ctrl_field_=ctrl_field;
    this->address_field_=address_field;
    this->info_field_=info_field;//

    this->afn_=0x06;
    this->dt1_=0x10;
    this->dt2_=0x00;
}
void qgdw_3762_protocol::Afn06F5::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if(data.length()<3)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->report_event_unit_.node_device_type_=data.at(0);
        this->report_event_unit_.node_protocol_type_=data.at(1);
        this->report_event_unit_.frame_length_=uchar(data.at(2));
        data.remove(0,3);
        if(this->report_event_unit_.frame_length_!=data.size())
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->report_event_unit_.frame_content_.clear();
        this->report_event_unit_.frame_content_.append(data);
    }
    else
    {
        throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("Unknow Error"));
    }
}
QByteArray qgdw_3762_protocol::Afn06F5::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if(this->report_event_unit_.frame_length_!=this->report_event_unit_.frame_content_.size())
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        data.append(this->report_event_unit_.node_device_type_);
        data.append(this->report_event_unit_.node_protocol_type_);
        data.append(char(this->report_event_unit_.frame_length_));
        data.append(this->report_event_unit_.frame_content_);
    }
    else
    {
        throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("Unknow Error"));
    }
    return  data;
}
