#include "afn10f102.h"

qgdw_3762_protocol::Afn10F102::Afn10F102()
{
    this->afn_=0x10;
    this->dt1_=0x20;
    this->dt2_=0x0C;
}

qgdw_3762_protocol::Afn10F102::Afn10F102(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
:Frame3762Base(ctrl_field,info_field,address_field,0x10,0x20,0x0C)
{
    this->ctrl_field_=ctrl_field;
    this->address_field_=address_field;
    this->info_field_=info_field;//

    this->afn_=0x10;
    this->dt1_=0x20;
    this->dt2_=0x0C;
}
void qgdw_3762_protocol::Afn10F102::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if(data.length()<5)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->network_node_info_unit_.node_total_num_=ushort((short(data.at(1))<<8))+uchar(data.at(0));
        this->network_node_info_unit_.node_start_no_=ushort((short(data.at(3))<<8))+uchar(data.at(2));
        this->network_node_info_unit_.this_node_num_=uchar(data.at(4));
        data.remove(0,5);
        if(this->network_node_info_unit_.this_node_num_!=data.size()/25)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->network_node_info_unit_.node_info_structure_list_.clear();
        for(int i=0;i<this->network_node_info_unit_.this_node_num_;i++)
        {
            NodeInfoStructure10F102 network_info_structure_;
            for(int j=0;j<6;j++)
            {
                network_info_structure_.node_address_.addr[j]=data.at(5-j);
            }
            network_info_structure_.network_typelogy_info_.node_tei_=ushort((short(data.at(7))<<8))+uchar(data.at(6));
            network_info_structure_.network_typelogy_info_.proxy_node_tei_=ushort((short(data.at(9))<<8))+uchar(data.at(8));
            data.remove(0,10);
            memcpy(&network_info_structure_.network_typelogy_info_.node_info_,data,1);
            network_info_structure_.device_property=data.at(1);
            network_info_structure_.vendor_code_.append(data.at(3));
            network_info_structure_.vendor_code_.append(data.at(2));
            network_info_structure_.communication_success_rate_up_=uchar(data.at(4));
            network_info_structure_.communication_success_rate_down_=uchar(data.at(5));
            data.remove(0,6);
            memcpy(&network_info_structure_.software_version_,data,3);
            data.remove(0,3);
            memcpy(&network_info_structure_.reverse_,data,5);
            data.remove(0,5);
            this->network_node_info_unit_.node_info_structure_list_.append(network_info_structure_);
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
QByteArray qgdw_3762_protocol::Afn10F102::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if(this->network_node_info_unit_.this_node_num_!=this->network_node_info_unit_.node_info_structure_list_.size())
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataContentError,QString("DataField Content Error"));
        }
        data.append(char(this->network_node_info_unit_.node_total_num_&0xFF));
        data.append(char(this->network_node_info_unit_.node_total_num_>>8));
        data.append(char(this->network_node_info_unit_.node_start_no_&0xFF));
        data.append(char(this->network_node_info_unit_.node_start_no_>>8));
        data.append(char(this->network_node_info_unit_.this_node_num_));
        for(int i=0;i<this->network_node_info_unit_.node_info_structure_list_.size();i++)
        {
            for(int j=0;j<6;j++)
                data.append(this->network_node_info_unit_.node_info_structure_list_.at(i).node_address_.addr[5-j]);
            char temp[2];
            memcpy(temp,&this->network_node_info_unit_.node_info_structure_list_.at(i).network_typelogy_info_.node_tei_,2);
            data.append(temp[0]);
            data.append(temp[1]);
            char temp1[2];
            memcpy(temp1,&this->network_node_info_unit_.node_info_structure_list_.at(i).network_typelogy_info_.proxy_node_tei_,2);
            data.append(temp1[0]);
            data.append(temp1[1]);
            data.append(*reinterpret_cast<char*>(&this->network_node_info_unit_.node_info_structure_list_[i].network_typelogy_info_.node_info_));
            data.append(this->network_node_info_unit_.node_info_structure_list_.at(i).device_property);
            data.append(this->network_node_info_unit_.node_info_structure_list_.at(i).vendor_code_.at(1));
            data.append(this->network_node_info_unit_.node_info_structure_list_.at(i).vendor_code_.at(0));
            data.append(char(this->network_node_info_unit_.node_info_structure_list_.at(i).communication_success_rate_up_));
            data.append(char(this->network_node_info_unit_.node_info_structure_list_.at(i).communication_success_rate_down_));
            for(int j=0;j<3;j++)
                data.append(this->network_node_info_unit_.node_info_structure_list_.at(i).software_version_[j]);
            for(int j=0;j<5;j++)
                data.append(this->network_node_info_unit_.node_info_structure_list_.at(i).reverse_[j]);
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
