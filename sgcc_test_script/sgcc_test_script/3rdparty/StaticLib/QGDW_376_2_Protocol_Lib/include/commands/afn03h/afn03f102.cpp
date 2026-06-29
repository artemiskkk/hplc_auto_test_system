#include "afn03f102.h"

qgdw_3762_protocol::Afn03F102::Afn03F102()
{
    this->afn_=0x03;
    this->dt1_=0x20;
    this->dt2_=0x0C;
}
qgdw_3762_protocol::Afn03F102::Afn03F102(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
:Frame3762Base(ctrl_field,info_field,address_field,0x03,0x20,0x0C)
{
    this->ctrl_field_=ctrl_field;
    this->address_field_=address_field;
    this->info_field_=info_field;//

    this->afn_=0x03;
    this->dt1_=0x20;
    this->dt2_=0x0C;
}
void qgdw_3762_protocol::Afn03F102::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirDown)
    {
        if(data.length()!=1)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->protocol_=data.at(0);
    }
    else
    {
        if(data.length()<7)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->data_unit_up_.protocol=data.at(0);
        this->data_unit_up_.single_meter_=data.at(1);
        this->data_unit_up_.single_data_id_num_=uchar(data.at(2));
        this->data_unit_up_.single_reply_data_len_=uchar(data.at(3));
        data.remove(0,4);
        if(data.length()<this->data_unit_up_.single_data_id_num_*5)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->data_unit_up_.single_data_id_len_list_.clear();
        for(int i=0;i<this->data_unit_up_.single_data_id_num_;i++)
        {
            DataIdLen data_id_len_;
            memcpy(data_id_len_.data_id_,data,4);
            data_id_len_.reply_data_len_=uchar(data.at(4));
            this->data_unit_up_.single_data_id_len_list_.append(data_id_len_);
            data.remove(0,5);
        }
        this->data_unit_up_.tri_meter_=data.at(0);
        this->data_unit_up_.tri_data_id_num_=uchar(data.at(1));
        this->data_unit_up_.tri_reply_data_len_=uchar(data.at(2));
        data.remove(0,3);
        if(data.length()<this->data_unit_up_.tri_data_id_num_*5)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->data_unit_up_.tri_data_id_len_list_.clear();
        for(int i=0;i<this->data_unit_up_.tri_data_id_num_;i++)
        {
            DataIdLen data_id_len_;
            memcpy(data_id_len_.data_id_,data,4);
            data_id_len_.reply_data_len_=uchar(data.at(4));
            this->data_unit_up_.tri_data_id_len_list_.append(data_id_len_);
            data.remove(0,5);
        }
    }
}
QByteArray qgdw_3762_protocol::Afn03F102::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirDown)
    {
        data.append(this->protocol_);
    }
    else
    {
        data.append(this->data_unit_up_.protocol);
        data.append(this->data_unit_up_.single_meter_);
        data.append(char(this->data_unit_up_.single_data_id_num_));
        data.append(char(this->data_unit_up_.single_reply_data_len_));
        if(this->data_unit_up_.single_data_id_num_!=this->data_unit_up_.single_data_id_len_list_.size())
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("Unknow Error"));
        }
        for(int i=0;i<this->data_unit_up_.single_data_id_len_list_.size();i++)
        {
            data.append(this->data_unit_up_.single_data_id_len_list_.at(i).data_id_,4);
            data.append(char(this->data_unit_up_.single_data_id_len_list_.at(i).reply_data_len_));
        }
        data.append(this->data_unit_up_.tri_meter_);
        data.append(char(this->data_unit_up_.tri_data_id_num_));
        data.append(char(this->data_unit_up_.tri_reply_data_len_));
        if(this->data_unit_up_.tri_data_id_num_!=this->data_unit_up_.tri_data_id_len_list_.size())
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("Unknow Error"));
        }
        for(int i=0;i<this->data_unit_up_.tri_data_id_len_list_.size();i++)
        {
            data.append(this->data_unit_up_.tri_data_id_len_list_.at(i).data_id_,4);
            data.append(char(this->data_unit_up_.tri_data_id_len_list_.at(i).reply_data_len_));
        }
    }
    return  data;
}
