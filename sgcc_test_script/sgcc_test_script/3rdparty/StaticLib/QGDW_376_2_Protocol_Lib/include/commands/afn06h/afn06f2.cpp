#include "afn06f2.h"

qgdw_3762_protocol::Afn06F2::Afn06F2()
{
    this->afn_=0x06;
    this->dt1_=0x02;
    this->dt2_=0x00;
}

qgdw_3762_protocol::Afn06F2::Afn06F2(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
:Frame3762Base(ctrl_field,info_field,address_field,0x06,0x02,0x00)
{
    this->ctrl_field_=ctrl_field;
    this->address_field_=address_field;
    this->info_field_=info_field;//

    this->afn_=0x06;
    this->dt1_=0x02;
    this->dt2_=0x00;
}
void qgdw_3762_protocol::Afn06F2::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if(data.length()<6)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->report_data_unit_.node_no_=ushort(short(data.at(1))<<8)+uchar(data.at(0));
        this->report_data_unit_.node_protocol_type_=data.at(2);
        this->report_data_unit_.communication_up_time_=ushort(short(data.at(4))<<8)+uchar(data.at(3));
        this->report_data_unit_.frame_length_=uchar(data.at(5));
        data.remove(0,6);
        if(this->report_data_unit_.frame_length_!=data.size())
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->report_data_unit_.frame_content_.clear();
        this->report_data_unit_.frame_content_.append(data);
    }
    else
    {
        throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("Unknow Error"));
    }
}
QByteArray qgdw_3762_protocol::Afn06F2::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if(this->report_data_unit_.frame_length_!=this->report_data_unit_.frame_content_.size())
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        data.append(char(this->report_data_unit_.node_no_&0xFF));
        data.append(char(this->report_data_unit_.node_no_>>8));
        data.append(this->report_data_unit_.node_protocol_type_);
        data.append(char(this->report_data_unit_.communication_up_time_&0xFF));
        data.append(char(this->report_data_unit_.communication_up_time_>>8));
        data.append(char(this->report_data_unit_.frame_length_));
        data.append(this->report_data_unit_.frame_content_);
    }
    else
    {
        throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("Unknow Error"));
    }
    return  data;
}
