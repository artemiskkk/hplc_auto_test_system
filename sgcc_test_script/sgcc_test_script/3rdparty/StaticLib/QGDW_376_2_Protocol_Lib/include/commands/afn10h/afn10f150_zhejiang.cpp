#include "afn10f150_zhejiang.h"

qgdw_3762_protocol::Afn10F150_Zhejiang::Afn10F150_Zhejiang()
{
    this->afn_=0x10;
    this->dt1_=0x20;
    this->dt2_=0x12;
}

qgdw_3762_protocol::Afn10F150_Zhejiang::Afn10F150_Zhejiang(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
:Frame3762Base(ctrl_field,info_field,address_field,0x10,0x20,0x12)
{
    this->ctrl_field_=ctrl_field;
    this->address_field_=address_field;
    this->info_field_=info_field;//

    this->afn_=0x10;
    this->dt1_=0x20;
    this->dt2_=0x12;
}
void qgdw_3762_protocol::Afn10F150_Zhejiang::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if(data.length()<3)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->node_info_data_unit_.node_total_num_=ushort((short(data.at(1))<<8))+uchar(data.at(0));
        this->node_info_data_unit_.this_start_no_=ushort((short(data.at(3))<<8))+uchar(data.at(2));
        this->node_info_data_unit_.this_node_num_=uchar(data.at(4));
        data.remove(0,5);
        if(this->node_info_data_unit_.this_node_num_!=data.size()/11)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->node_info_data_unit_.node_info_list_.clear();
        for(int i=0;i<this->node_info_data_unit_.this_node_num_;i++)
        {
            NodeInfo10F150 node_info_;
            for(int j=0;j<6;j++)
            {
                node_info_.slaved_address_.addr[j]=data.at(5-j);
            }
            data.remove(0,6);
            for(int j=0;j<6;j++)
            {
                node_info_.node_address_.addr[j]=data.at(5-j);
            }
            data.remove(0,6);
            node_info_.protocol_type_=data.at(0);
            data.remove(0,1);
            this->node_info_data_unit_.node_info_list_.append(node_info_);
        }
    }
    else
    {
        if(data.length()!=3)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->node_start_no_=ushort((short(data.at(1))<<8))+uchar(data.at(0));
        this->node_num_=uchar(data.at(2));
    }
}
QByteArray qgdw_3762_protocol::Afn10F150_Zhejiang::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if(this->node_info_data_unit_.this_node_num_!=this->node_info_data_unit_.node_info_list_.size())
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataContentError,QString("DataField Content Error"));
        }
        data.append(char(this->node_info_data_unit_.node_total_num_&0xFF));
        data.append(char(this->node_info_data_unit_.node_total_num_>>8));
        data.append(char(this->node_info_data_unit_.this_start_no_&0xFF));
        data.append(char(this->node_info_data_unit_.this_start_no_>>8));
        data.append(char(this->node_info_data_unit_.this_node_num_));
        for(int i=0;i<this->node_info_data_unit_.node_info_list_.size();i++)
        {
            for(int j=0;j<6;j++)
                data.append(this->node_info_data_unit_.node_info_list_.at(i).slaved_address_.addr[5-j]);
            for(int j=0;j<6;j++)
                data.append(this->node_info_data_unit_.node_info_list_.at(i).node_address_.addr[5-j]);
            data.append(this->node_info_data_unit_.node_info_list_.at(i).protocol_type_);
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
