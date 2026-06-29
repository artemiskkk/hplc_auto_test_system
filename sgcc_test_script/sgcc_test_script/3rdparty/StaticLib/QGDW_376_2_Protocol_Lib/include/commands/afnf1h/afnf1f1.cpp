#include "afnf1f1.h"

qgdw_3762_protocol::AfnF1F1::AfnF1F1()
{
    this->afn_=char(0xF1);
    this->dt1_=0x01;
    this->dt2_=0x00;
}

qgdw_3762_protocol::AfnF1F1::AfnF1F1(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
:Frame3762Base(ctrl_field,info_field,address_field,char(0xF1),0x01,0x00)
{
    this->ctrl_field_=ctrl_field;
    this->address_field_=address_field;
    this->info_field_=info_field;//

    this->afn_=char(0xF1);
    this->dt1_=0x01;
    this->dt2_=0x00;
}
void qgdw_3762_protocol::AfnF1F1::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirDown)
    {
        if(data.length()<4)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->unit_down_.protocol_type_=data.at(0);
        this->unit_down_.subsidiary_node_num_=uchar(data.at(1));
        data.remove(0,2);
        this->unit_down_.subsidiary_node_address_list_.clear();
        for(int i=0;i<this->unit_down_.subsidiary_node_num_;i++)
        {
            Address subsidiary_node_address_;
            for(int j=0;j<6;j++)
                subsidiary_node_address_.addr[j]=data.at(5-j);
            data.remove(0,6);
            this->unit_down_.subsidiary_node_address_list_.append(subsidiary_node_address_);
        }
        this->unit_down_.frame_length_=ushort((short(data.at(1))<<8))+uchar(data.at(0));
        if(this->unit_down_.frame_length_!=data.size()-2)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->unit_down_.frame_content_.clear();
        this->unit_down_.frame_content_.append(data.mid(2));
    }
    else
    {
        if(data.length()<3)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        //this->unit_up_.read_state_=data.at(0);
        this->unit_up_.protocol_type_=data.at(0);
        this->unit_up_.frame_length_=ushort((short(data.at(2))<<8))+uchar(data.at(1));
        if(this->unit_up_.frame_length_!=data.size()-3)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        if(this->unit_up_.frame_length_>0)
            this->unit_up_.frame_content_.append(data.mid(3));
    }
}
QByteArray qgdw_3762_protocol::AfnF1F1::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirDown)
    {
        if(this->unit_down_.frame_length_!=this->unit_down_.frame_content_.size())
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        if(this->unit_down_.subsidiary_node_num_!=this->unit_down_.subsidiary_node_address_list_.size())
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        data.append(this->unit_down_.protocol_type_);
        data.append(char(this->unit_down_.subsidiary_node_num_));
        for(int i=0;i<this->unit_down_.subsidiary_node_address_list_.size();i++)
        {
            for(int j=0;j<6;j++)
                data.append(this->unit_down_.subsidiary_node_address_list_.at(i).addr[5-j]);
        }
        data.append(char(this->unit_down_.frame_length_&0xFF));
        data.append(char(this->unit_down_.frame_length_>>8));
        data.append(this->unit_down_.frame_content_);
    }
    else
    {
        if(this->unit_up_.frame_length_!=this->unit_up_.frame_content_.size())
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        //data.append(this->unit_up_.read_state_);
        data.append(this->unit_up_.protocol_type_);
        data.append(char(this->unit_up_.frame_length_&0xFF));
        data.append(char(this->unit_up_.frame_length_>>8));
        data.append(this->unit_up_.frame_content_);
    }
    return  data;
}

