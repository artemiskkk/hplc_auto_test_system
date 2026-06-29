#include "afn03f5.h"

qgdw_3762_protocol::Afn03F5::Afn03F5()
{
    this->afn_=0x03;
    this->dt1_=0x10;
    this->dt2_=0x00;
}

qgdw_3762_protocol::Afn03F5::Afn03F5(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
:Frame3762Base(ctrl_field,info_field,address_field,0x03,0x10,0x00)
{
    this->ctrl_field_=ctrl_field;
    this->info_field_=info_field;//
    this->address_field_=address_field;
    this->afn_=0x03;
    this->dt1_=0x10;
    this->dt2_=0x00;
}
void qgdw_3762_protocol::Afn03F5::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if(data.length()<2)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length < 2 "));
        }
        memcpy(&this->state_word_,data,2);
        if(this->state_word_.rate_num_!=(data.size()/2-1))
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataContentFormatError,QString("DataField Content Format Error"));
        }        
        data.remove(0,2);
        com_rate_unit_list_.clear();

        for(int i=0;i<this->state_word_.rate_num_;i++)
        {
            ComRateUnit com_rate_unit;
            memcpy(&com_rate_unit,data,2);
            com_rate_unit_list_.append(com_rate_unit);
            data.remove(0,2);
        }
    }
    else
    {
        if(data.length() != 0)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length != 0"));
        }
    }
}
QByteArray qgdw_3762_protocol::Afn03F5::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {        
        char temp[2];
        memcpy(temp,&this->state_word_,2);
        data.append(temp);
        if(this->state_word_.rate_num_!=this->com_rate_unit_list_.size())
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        for(int i=0;i<this->com_rate_unit_list_.size();i++)
        {
            char temp_1_[2];
            memcpy(temp_1_,&this->com_rate_unit_list_.at(i),2);
            data.append(temp_1_[1]);
            data.append(temp_1_[0]);
        }
    }
    return data;
}
