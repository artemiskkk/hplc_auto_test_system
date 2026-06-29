#include "afn05f103.h"

qgdw_3762_protocol::Afn05F103::Afn05F103()
{
    this->afn_=0x05;
    this->dt1_=0x40;
    this->dt2_=0x0C;
}
qgdw_3762_protocol::Afn05F103::Afn05F103(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
:Frame3762Base(ctrl_field,info_field,address_field,0x05,0x40,0x0C)
{
    this->ctrl_field_=ctrl_field;
    this->address_field_=address_field;
    this->info_field_=info_field;//

    this->afn_=0x05;
    this->dt1_=0x40;
    this->dt2_=0x0C;
}
void qgdw_3762_protocol::Afn05F103::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirDown)
    {
        if(data.length()<9)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->curve_config_data_.set_flag_=data.at(0);
        this->curve_config_data_.read_period_=uchar(data.at(1));
        this->curve_config_data_.protocol=data.at(2);
        this->curve_config_data_.single_meter_=data.at(3);
        this->curve_config_data_.single_data_id_num_=uchar(data.at(4));
        this->curve_config_data_.single_reply_data_len_=uchar(data.at(5));
        data.remove(0,6);
        if(data.length()<this->curve_config_data_.single_data_id_num_*5)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->curve_config_data_.single_data_id_len_list_.clear();
        for(int i=0;i<this->curve_config_data_.single_data_id_num_;i++)
        {
            DataIdLen data_id_len_;
            memcpy(data_id_len_.data_id_,data,4);
            data_id_len_.reply_data_len_=uchar(data.at(4));
            this->curve_config_data_.single_data_id_len_list_.append(data_id_len_);
            data.remove(0,5);
        }
        this->curve_config_data_.tri_meter_=data.at(0);
        this->curve_config_data_.tri_data_id_num_=uchar(data.at(1));
        this->curve_config_data_.tri_reply_data_len_=uchar(data.at(2));
        data.remove(0,3);
        if(data.length()<this->curve_config_data_.tri_data_id_num_*5)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->curve_config_data_.tri_data_id_len_list_.clear();
        for(int i=0;i<this->curve_config_data_.tri_data_id_num_;i++)
        {
            DataIdLen data_id_len_;
            memcpy(data_id_len_.data_id_,data,4);
            data_id_len_.reply_data_len_=uchar(data.at(4));
            this->curve_config_data_.tri_data_id_len_list_.append(data_id_len_);
            data.remove(0,5);
        }
    }
    else
    {
        throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("Unknow Error"));
    }
}
QByteArray qgdw_3762_protocol::Afn05F103::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirDown)
    {
        data.append(this->curve_config_data_.set_flag_);
        data.append(char(this->curve_config_data_.read_period_));
        data.append(this->curve_config_data_.protocol);
        data.append(this->curve_config_data_.single_meter_);
        data.append(char(this->curve_config_data_.single_data_id_num_));
        data.append(char(this->curve_config_data_.single_reply_data_len_));
        if(this->curve_config_data_.single_data_id_num_!=this->curve_config_data_.single_data_id_len_list_.size())
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("Unknow Error"));
        }
        for(int i=0;i<this->curve_config_data_.single_data_id_len_list_.size();i++)
        {
            data.append(this->curve_config_data_.single_data_id_len_list_.at(i).data_id_,4);
            data.append(char(this->curve_config_data_.single_data_id_len_list_.at(i).reply_data_len_));
        }
        data.append(this->curve_config_data_.tri_meter_);
        data.append(char(this->curve_config_data_.tri_data_id_num_));
        data.append(char(this->curve_config_data_.tri_reply_data_len_));
        if(this->curve_config_data_.tri_data_id_num_!=this->curve_config_data_.tri_data_id_len_list_.size())
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("Unknow Error"));
        }
        for(int i=0;i<this->curve_config_data_.tri_data_id_len_list_.size();i++)
        {
            data.append(this->curve_config_data_.tri_data_id_len_list_.at(i).data_id_,4);
            data.append(char(this->curve_config_data_.tri_data_id_len_list_.at(i).reply_data_len_));
        }
    }
    else
    {
        throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("Unknow Error"));
    }
    return  data;
}
