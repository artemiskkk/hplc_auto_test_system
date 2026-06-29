#include "afn14f1.h"
qgdw_3762_protocol::Afn14F1::Afn14F1()
{
    this->afn_=0x14;
    this->dt1_=0x01;
    this->dt2_=0x00;
}
qgdw_3762_protocol::Afn14F1::Afn14F1(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
:Frame3762Base(ctrl_field,info_field,address_field,0x14,0x01,0x00)
{
    this->ctrl_field_=ctrl_field;
    this->address_field_=address_field;
    this->info_field_=info_field;//

    this->afn_=0x14;
    this->dt1_=0x01;
    this->dt2_=0x00;
}
void qgdw_3762_protocol::Afn14F1::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirDown)
    {
        if(data.length()<4)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->router_request_read_unit_.read_flag_=data.at(0);
        this->router_request_read_unit_.delay_related_flag_=data.at(1);
        this->router_request_read_unit_.frame_length_=uchar(data.at(2));
        this->router_request_read_unit_.frame_content_.clear();
        this->router_request_read_unit_.frame_content_=data.mid(3,this->router_request_read_unit_.frame_length_);
        this->router_request_read_unit_.subsidiary_node_num_=uchar(data.at(3+this->router_request_read_unit_.frame_length_));
        data.remove(0,4+this->router_request_read_unit_.frame_length_);
        this->router_request_read_unit_.subsidiary_node_address_list_.clear();
        for(int i=0;i<this->router_request_read_unit_.subsidiary_node_num_;i++)
        {
            Address subsidiary_node_address_;
            for(int j=0;j<6;j++)
                subsidiary_node_address_.addr[j]=data.at(5-j);
            data.remove(0,6);
            this->router_request_read_unit_.subsidiary_node_address_list_.append(subsidiary_node_address_);
        }
    }
    else
    {
        if(data.length()!=9)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->phase_=data.at(0);
        for (int i=0;i<6;i++)
        {
            this->node_address_.addr[i]=data.at(6-i);
        }
        this->node_no_=ushort(short(data.at(8))<<8)+uchar(data.at(7));
    }
}
QByteArray qgdw_3762_protocol::Afn14F1::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirDown)
    {
        if(this->router_request_read_unit_.frame_length_!=this->router_request_read_unit_.frame_content_.size())
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        data.append(this->router_request_read_unit_.read_flag_);
        data.append(this->router_request_read_unit_.delay_related_flag_);
        data.append(char(this->router_request_read_unit_.frame_length_));
        data.append(this->router_request_read_unit_.frame_content_);
        data.append(char(this->router_request_read_unit_.subsidiary_node_num_));
        if(this->router_request_read_unit_.subsidiary_node_num_!=this->router_request_read_unit_.subsidiary_node_address_list_.size())
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        for(int i=0;i<this->router_request_read_unit_.subsidiary_node_address_list_.size();i++)
        {
            for(int j=0;j<6;j++)
                data.append(this->router_request_read_unit_.subsidiary_node_address_list_.at(i).addr[5-j]);
        }
    }
    else
    {
        data.append(this->phase_);
        for(int i=0;i<6;i++)
            data.append(this->node_address_.addr[5-i]);
        data.append(char(this->node_no_&0xFF));
        data.append(char(this->node_no_>>8));
    }
    return  data;
}
