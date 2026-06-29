#include "afn13f1.h"

qgdw_3762_protocol::Afn13F1::Afn13F1()
{
    this->afn_=0x13;
    this->dt1_=0x01;
    this->dt2_=0x00;
}

qgdw_3762_protocol::Afn13F1::Afn13F1(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
    :Frame3762Base(ctrl_field,info_field,address_field,0x13,0x01,0x00)
{
    this->ctrl_field_=ctrl_field;
    this->address_field_=address_field;
    this->info_field_=info_field;//

    this->afn_=0x13;
    this->dt1_=0x01;
    this->dt2_=0x00;
}

void qgdw_3762_protocol::Afn13F1::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01)==qgdw_3762_protocol::kDirDown) //下行
    {
        if(data.length()<3)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->data_field_down_.protocol_type_=data.at(0);
        this->data_field_down_.delay_tag_=data.at(1);
        this->data_field_down_.sub_node_num_=uchar(data.at(2));
        if(data.length()<(4+6*this->data_field_down_.sub_node_num_))
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        data.remove(0,3);
        this->data_field_down_.sub_node_address_list_.clear();
        for(int i=0;i<this->data_field_down_.sub_node_num_;i++)
        {
            Address node_address_;
            for(int j=0;j<6;j++)
                node_address_.addr[j]=data.at(5-j);
            data.remove(0,6);
            this->data_field_down_.sub_node_address_list_.append(node_address_);
        }
        this->data_field_down_.frame_length_=uchar(data.at(0));
        if(this->data_field_down_.frame_length_!=data.size())
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->data_field_down_.frame_content_.clear();
        this->data_field_down_.frame_content_.append(data.mid(1));
    }
    else//上行
    {
        if(data.length()<4)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->data_field_up_.up_delay_time_=ushort((short(data.at(1))<<8))+uchar(data.at(0));
        this->data_field_up_.protocol_type_=data.at(2);
        this->data_field_up_.frame_length_=uchar(data.at(3));
        data.remove(0,4);
        if(this->data_field_up_.frame_length_!=data.size())
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        if(data.size()>0)
            this->data_field_up_.frame_content_.append(data);
    }
}
QByteArray qgdw_3762_protocol::Afn13F1::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01)==qgdw_3762_protocol::kDirDown) //下行
    {
        data.append(this->data_field_down_.protocol_type_);
        data.append(this->data_field_down_.delay_tag_);
        data.append(char(this->data_field_down_.sub_node_num_));
        if(this->data_field_down_.sub_node_num_!=this->data_field_down_.sub_node_address_list_.size())
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        for(int i=0;i<this->data_field_down_.sub_node_address_list_.size();i++)
        {
            for(int j=0;j<6;j++)
                data.append(this->data_field_down_.sub_node_address_list_.at(i).addr[5-j]);
        }
        data.append(char(this->data_field_down_.frame_length_));
        if(this->data_field_down_.frame_length_!=this->data_field_down_.frame_content_.size())
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        data.append(this->data_field_down_.frame_content_);
    }
    else//上行
    {
        data.append(char(this->data_field_up_.up_delay_time_&0xFF));
        data.append(char(this->data_field_up_.up_delay_time_>>8));
        data.append(this->data_field_up_.protocol_type_);
        data.append(char(this->data_field_up_.frame_length_));
        if(this->data_field_up_.frame_length_!=this->data_field_up_.frame_content_.size())
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        data.append(this->data_field_up_.frame_content_);
    }
    return data;
}
