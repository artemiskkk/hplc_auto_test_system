#include "afn10f6.h"

qgdw_3762_protocol::Afn10F6::Afn10F6()
{
    this->afn_=0x10;
    this->dt1_=0x20;
    this->dt2_=0x00;
}

qgdw_3762_protocol::Afn10F6::Afn10F6(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
:Frame3762Base(ctrl_field,info_field,address_field,0x10,0x20,0x00)
{
    this->ctrl_field_=ctrl_field;
    this->address_field_=address_field;
    this->info_field_=info_field;//
    this->afn_=0x10;
    this->dt1_=0x20;
    this->dt2_=0x00;

}
void qgdw_3762_protocol::Afn10F6::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if(data.length()<3)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->register_node_info_unit_.node_total_num_=ushort((short(data.at(1))<<8))+uchar(data.at(0));
        this->register_node_info_unit_.this_node_num_=uchar(data.at(2));
        data.remove(0,3);
        if(this->register_node_info_unit_.this_node_num_!=data.size()/8)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->register_node_info_unit_.node_info_group_list_.clear();
        for(int i=0;i<this->register_node_info_unit_.this_node_num_;i++)
        {
            NodeInfoGroup10F6 node_info_group_;
            for(int j=0;j<6;j++)
            {
                node_info_group_.node_address_.addr[j]=data.at(5-j);
            }
            data.remove(0,6);
            memcpy(&node_info_group_.node_info_,data,2);
            this->register_node_info_unit_.node_info_group_list_.append(node_info_group_);
            data.remove(0,2);
        }
    }
    else
    {
        if(data.length()!=3)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->node_start_no_=ushort((short(data.at(1))<<8)+data.at(0));
        this->node_num_=uchar(data.at(2));
    }
}
QByteArray qgdw_3762_protocol::Afn10F6::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if(this->register_node_info_unit_.this_node_num_!=this->register_node_info_unit_.node_info_group_list_.size())
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataContentError,QString("DataField Content Error"));
        }
        data.append(char(this->register_node_info_unit_.node_total_num_&0xFF));
        data.append(char(this->register_node_info_unit_.node_total_num_>>8));
        data.append(char(this->register_node_info_unit_.this_node_num_));
        for(int i=0;i<this->register_node_info_unit_.node_info_group_list_.size();i++)
        {
            for(int j=0;j<6;j++)
                data.append(this->register_node_info_unit_.node_info_group_list_.at(i).node_address_.addr[5-j]);
            char temp[2];
            memcpy(temp,&this->register_node_info_unit_.node_info_group_list_.at(i).node_info_,2);
            data.append(temp[0]);
            data.append(temp[1]);
        }
    }
    else
    {
        data.append(char(this->node_start_no_&0xFF));
        data.append(char(this->node_start_no_>>8));
        data.append(char(this->node_num_));
    }
    return  data;
}
