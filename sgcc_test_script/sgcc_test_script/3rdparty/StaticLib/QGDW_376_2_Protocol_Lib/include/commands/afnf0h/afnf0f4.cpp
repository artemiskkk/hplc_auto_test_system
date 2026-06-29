#include "afnf0f4.h"

qgdw_3762_protocol::AfnF0F4::AfnF0F4()
{
    this->afn_=char(0xF0);
    this->dt1_=0x08;
    this->dt2_=0x00;
}

qgdw_3762_protocol::AfnF0F4::AfnF0F4(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
:Frame3762Base(ctrl_field,info_field,address_field,char(0xF0),0x08,0x00)
{
    this->ctrl_field_=ctrl_field;
    this->address_field_=address_field;
    this->info_field_=info_field;//

    this->afn_=char(0xF0);
    this->dt1_=0x08;
    this->dt2_=0x00;
}
void qgdw_3762_protocol::AfnF0F4::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if(data.length()<3)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->zero_test_unit_.response_state_=data.at(0);
        this->zero_test_unit_.phase_=data.at(1);
        this->zero_test_unit_.element_num_=uchar(data.at(2));
        data.remove(0,3);
        if(this->zero_test_unit_.element_num_!=data.size()/4)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->zero_test_unit_.zero_list_.clear();
        for(int i=0;i<this->zero_test_unit_.element_num_;i++)
        {
            uint temp=uint(int(data.at(3))<<24)+uint(int(data.at(2))<<16)+uint(int(data.at(1))<<8)+uchar(data.at(0));
            data.remove(0,4);
            this->zero_test_unit_.zero_list_.append(temp);
        }
    }
    else
    {
        if(data.length()!=2)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->phase_=data.at(0);
        this->element_num_=uchar(data.at(1));
    }
}
QByteArray qgdw_3762_protocol::AfnF0F4::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if(this->zero_test_unit_.element_num_!=this->zero_test_unit_.zero_list_.size())
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        data.append(this->zero_test_unit_.response_state_);
        data.append(this->zero_test_unit_.phase_);
        data.append(char(this->zero_test_unit_.element_num_));
        for(int i=0;i<this->zero_test_unit_.zero_list_.size();i++)
        {
            data.append(char(this->zero_test_unit_.zero_list_.at(i)&0xFF));
            data.append(char(this->zero_test_unit_.zero_list_.at(i)>>8));
            data.append(char(this->zero_test_unit_.zero_list_.at(i)>>16));
            data.append(char(this->zero_test_unit_.zero_list_.at(i)>>24));
        }
    }
    else
    {
        data.append(this->phase_);
        data.append(char(this->element_num_));
    }
    return  data;
}

