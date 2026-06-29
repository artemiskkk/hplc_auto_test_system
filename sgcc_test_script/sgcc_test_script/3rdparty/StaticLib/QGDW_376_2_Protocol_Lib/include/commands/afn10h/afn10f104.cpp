#include "afn10f104.h"

qgdw_3762_protocol::Afn10F104::Afn10F104()
{
    this->afn_=0x10;
    this->dt1_=char(0x80);
    this->dt2_=0x0C;
}

qgdw_3762_protocol::Afn10F104::Afn10F104(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
:Frame3762Base(ctrl_field,info_field,address_field,0x10,char(0x80),0x0C)
{
    this->ctrl_field_=ctrl_field;
    this->address_field_=address_field;
    this->info_field_=info_field;//

    this->afn_=0x10;
    this->dt1_=char(0x80);
    this->dt2_=0x0C;
}
void qgdw_3762_protocol::Afn10F104::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if(data.length()<2)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->network_node_info_unit_.node_total_num_=ushort((short(data.at(1))<<8))+uchar(data.at(0));
        data.remove(0,2);
        this->network_node_info_unit_.this_node_num_=uchar(data.at(0));
        data.remove(0,1);
        if(this->network_node_info_unit_.this_node_num_!=data.size()/15)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->network_node_info_unit_.node_info_group_list_.clear();
        for(int i=0;i<this->network_node_info_unit_.this_node_num_;i++)
        {
            NodeInfoGroup10F104 node_info_group_;
            for(int j=0;j<6;j++)
            {
                node_info_group_.node_address_.addr[j]=data.at(5-j);
            }
            data.remove(0,6);
            node_info_group_.node_info_.software_version_[0]=data.at(1);
            node_info_group_.node_info_.software_version_[1]=data.at(0);
            node_info_group_.node_info_.ver_day_=data.at(2);
            node_info_group_.node_info_.ver_month_=data.at(3);
            node_info_group_.node_info_.ver_year_=data.at(4);
            node_info_group_.node_info_.vendor_code_[0]=data.at(6);
            node_info_group_.node_info_.vendor_code_[1]=data.at(5);
            node_info_group_.node_info_.chip_code_[0]=data.at(8);
            node_info_group_.node_info_.chip_code_[1]=data.at(7);
            data.remove(0,9);
            this->network_node_info_unit_.node_info_group_list_.append(node_info_group_);
        }
    }
    else
    {
        if(data.length()!=3)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->start_no_=ushort((short(data.at(1))<<8))+uchar(data.at(0));
        this->this_query_num_=uchar(data.at(2));
        data.remove(0,3);
    }
}
QByteArray qgdw_3762_protocol::Afn10F104::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if(this->network_node_info_unit_.this_node_num_!=this->network_node_info_unit_.node_info_group_list_.size())
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataContentError,QString("DataField Content Error"));
        }
        data.append(char(this->network_node_info_unit_.node_total_num_&0xFF));
        data.append(char(this->network_node_info_unit_.node_total_num_>>8));
        data.append(char(this->network_node_info_unit_.this_node_num_));
        for(int i=0;i<this->network_node_info_unit_.node_info_group_list_.size();i++)
        {
            for(int j=0;j<6;j++)
                data.append(this->network_node_info_unit_.node_info_group_list_.at(i).node_address_.addr[5-j]);
            data.append(this->network_node_info_unit_.node_info_group_list_.at(i).node_info_.software_version_);
            data.append(this->network_node_info_unit_.node_info_group_list_.at(i).node_info_.ver_day_);
            data.append(this->network_node_info_unit_.node_info_group_list_.at(i).node_info_.ver_month_);
            data.append(this->network_node_info_unit_.node_info_group_list_.at(i).node_info_.ver_year_);
            data.append(this->network_node_info_unit_.node_info_group_list_.at(i).node_info_.vendor_code_);
            data.append(this->network_node_info_unit_.node_info_group_list_.at(i).node_info_.chip_code_);
        }
    }
    else
    {
        data.append(char(this->start_no_&0xFF));
        data.append(char((this->start_no_>>8)&0xFF));
        data.append(char(this->this_query_num_));
    }
    return  data;
}
