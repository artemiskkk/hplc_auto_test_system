#include "afn10f3.h"

qgdw_3762_protocol::Afn10F3::Afn10F3()
{
    this->afn_=0x10;
    this->dt1_=0x04;
    this->dt2_=0x00;
}

qgdw_3762_protocol::Afn10F3::Afn10F3(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
:Frame3762Base(ctrl_field,info_field,address_field,0x10,0x04,0x00)
{
    this->ctrl_field_=ctrl_field;
    this->address_field_=address_field;
    this->info_field_=info_field;//

    this->afn_=0x10;
    this->dt1_=0x04;
    this->dt2_=0x00;


}
void qgdw_3762_protocol::Afn10F3::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if(data.length()<1)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->last_relay_data_unit_.router_node_total_num_=uchar(data.at(0));
        data.remove(0,1);
        if(this->last_relay_data_unit_.router_node_total_num_!=data.size()/8)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->last_relay_data_unit_.node_info_group_list_.clear();
        for(int i=0;i<this->last_relay_data_unit_.router_node_total_num_;i++)
        {
            NodeInfoGroup10F3 node_info_group_;
            for(int j=0;j<6;j++)
            {
                node_info_group_.node_address_.addr[j]=data.at(5-j);
            }
            data.remove(0,6);
            memcpy(&node_info_group_.node_info_,data,2);
            this->last_relay_data_unit_.node_info_group_list_.append(node_info_group_);
            data.remove(0,2);
        }
    }
    else
    {
        if(data.length()!=6)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        for(int i=0;i<6;i++)
        {
            this->node_address_.addr[i]=data.at(5-i);
        }
    }
}
QByteArray qgdw_3762_protocol::Afn10F3::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if(this->last_relay_data_unit_.router_node_total_num_!=this->last_relay_data_unit_.node_info_group_list_.size())
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataContentError,QString("DataField Content Error"));
        }
        data.append(char(this->last_relay_data_unit_.router_node_total_num_));
        for(int i=0;i<this->last_relay_data_unit_.node_info_group_list_.size();i++)
        {
            for(int j=0;j<6;j++)
                data.append(this->last_relay_data_unit_.node_info_group_list_.at(i).node_address_.addr[5-j]);
            char temp[2];
            memcpy(temp,&this->last_relay_data_unit_.node_info_group_list_.at(i).node_info_,2);
            data.append(temp[0]);
            data.append(temp[1]);
        }
    }
    else
    {
        for(int i=0;i<6;i++)
            data.append(this->node_address_.addr[5-i]);
    }
    return  data;
}
