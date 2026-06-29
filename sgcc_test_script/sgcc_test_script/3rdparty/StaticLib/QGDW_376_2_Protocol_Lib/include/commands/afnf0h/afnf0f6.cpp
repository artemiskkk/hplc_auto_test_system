#include "afnf0f6.h"

qgdw_3762_protocol::AfnF0F6::AfnF0F6()
{
    this->afn_=char(0xF0);
    this->dt1_=0x20;
    this->dt2_=0x00;
}

qgdw_3762_protocol::AfnF0F6::AfnF0F6(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
:Frame3762Base(ctrl_field,info_field,address_field,char(0xF0),0x20,0x00)
{
    this->ctrl_field_=ctrl_field;
    this->address_field_=address_field;
    this->info_field_=info_field;//

    this->afn_=char(0xF0);
    this->dt1_=0x20;
    this->dt2_=0x00;
}
void qgdw_3762_protocol::AfnF0F6::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if(data.length()<3)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->uart_test_unit_up_.response_state_=data.at(0);
        this->uart_test_unit_up_.protocol_type_=data.at(1);
        this->uart_test_unit_up_.frame_length_=uchar(data.at(2));
        if(this->uart_test_unit_up_.frame_length_!=data.size()-3)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->uart_test_unit_up_.frame_content_.clear();
        this->uart_test_unit_up_.frame_content_.append(data.mid(3));
    }
    else
    {
        if(data.length()<5)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->uart_test_unit_down_.uart_index_=data.at(0);
        this->uart_test_unit_down_.baud_rate_=data.at(1);
        this->uart_test_unit_down_.max_timeout_=uchar(data.at(2));
        this->uart_test_unit_down_.protocol_type_=data.at(3);
        this->uart_test_unit_down_.frame_length_=uchar(data.at(4));
        if(this->uart_test_unit_down_.frame_length_!=data.size()-5)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->uart_test_unit_down_.frame_content_.append(data.mid(5));
    }
}
QByteArray qgdw_3762_protocol::AfnF0F6::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if(this->uart_test_unit_up_.frame_length_!=this->uart_test_unit_up_.frame_content_.size())
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        data.append(this->uart_test_unit_up_.response_state_);
        data.append(this->uart_test_unit_up_.protocol_type_);
        data.append(char(this->uart_test_unit_up_.frame_length_));
        data.append(this->uart_test_unit_up_.frame_content_);
    }
    else
    {
        if(this->uart_test_unit_down_.frame_length_!=this->uart_test_unit_down_.frame_content_.size())
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        data.append(this->uart_test_unit_down_.uart_index_);
        data.append(this->uart_test_unit_down_.baud_rate_);
        data.append(char(this->uart_test_unit_down_.max_timeout_));
        data.append(this->uart_test_unit_down_.protocol_type_);
        data.append(char(this->uart_test_unit_down_.frame_length_));
        data.append(this->uart_test_unit_down_.frame_content_);
    }
    return  data;
}
