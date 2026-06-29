#include "afnf0f102.h"

qgdw_3762_protocol::AfnF0F102::AfnF0F102()
{
    this->afn_=char(0xF0);
    this->dt1_=0x20;
    this->dt2_=0x0C;
}

qgdw_3762_protocol::AfnF0F102::AfnF0F102(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
:Frame3762Base(ctrl_field,info_field,address_field,char(0xF0),0x20,0x0C)
{
    this->ctrl_field_=ctrl_field;
    this->address_field_=address_field;
    this->info_field_=info_field;//

    this->afn_=char(0xF0);
    this->dt1_=0x20;
    this->dt2_=0x0C;
}
void qgdw_3762_protocol::AfnF0F102::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if(data.length()<46/*data.length()<53*/)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->network_node_info_unit_.node_total_num_=ushort(short(data.at(1))<<8)+uchar(data.at(0));
        this->network_node_info_unit_.this_node_num_=ushort(short(data.at(3))<<8)+uchar(data.at(2));
        this->network_node_info_unit_.node_start_no_=ushort(short(data.at(5))<<8)+uchar(data.at(4));
        data.remove(0,6);
        if(this->network_node_info_unit_.this_node_num_!=data.size()/46)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->network_node_info_unit_.node_info_structure_list_.clear();
        for(int i=0;i<this->network_node_info_unit_.this_node_num_;i++)
        {
            NodeInfoStructure network_info_structure_;
            for(int j=0;j<6;j++)
            {
                network_info_structure_.node_address_.addr[j]=data.at(5-j);
            }
            network_info_structure_.network_typelogy_info_.node_tei_=ushort(short(data.at(7))<<8)+uchar(data.at(6));
            network_info_structure_.network_typelogy_info_.proxy_node_tei_=ushort(short(data.at(9))<<8)+uchar(data.at(8));
            data.remove(0,10);
            memcpy(&network_info_structure_.network_typelogy_info_.node_info_,data,1);
            network_info_structure_.network_info_=data.at(1);
            network_info_structure_.device_type_[1]=data.at(3);
            network_info_structure_.device_type_[0]=data.at(2);
            network_info_structure_.phase_=data.at(4);
            network_info_structure_.proxy_change_times_=ushort(short(data.at(6))<<8)+uchar(data.at(5));
            network_info_structure_.node_offline_times_=ushort(short(data.at(8))<<8)+uchar(data.at(7));
            network_info_structure_.node_offline_time_=uint(int(data.at(12))<<24)+uint(int(data.at(11))<<16)+uint(int(data.at(10))<<8)+uchar(data.at(9));
            network_info_structure_.node_offline_max_time_=uint(int(data.at(16))<<24)+uint(int(data.at(15))<<16)+uint(int(data.at(14))<<8)+uchar(data.at(13));
            network_info_structure_.communication_success_rate_up_=uchar(data.at(17));
            network_info_structure_.communication_success_rate_down_=uchar(data.at(18));
            data.remove(0,19);
            memcpy(&network_info_structure_.primary_version_,data,3);
            data.remove(0,3);
            memcpy(&network_info_structure_.secondary_version_,data,2);
            data.remove(0,2);
            network_info_structure_.next_info_=ushort(short(data.at(1))<<8)+uchar(data.at(0));
            network_info_structure_.channel_type_=data.at(2);
            network_info_structure_.protocol_type_=data.at(3);
            network_info_structure_.area_state_=data.at(4);
            for(int j=0;j<6;j++)
            {
                network_info_structure_.area_address_.addr[j]=data.at(10-j);
            }
            network_info_structure_.hang_down_num_=uchar(data.at(11));
            data.remove(0,12);
            network_info_structure_.hang_down_list_.clear();
            for(int j=0;j<network_info_structure_.hang_down_num_;j++)
            {
                NodeHangDownInfo node_hang_down_info_;
                for(int m=0;m<6;m++)
                    node_hang_down_info_.node_address_.addr[m]=data.at(5-m);
                node_hang_down_info_.protocol_type_=data.at(6);
                node_hang_down_info_.device_type_=data.at(7);
                data.remove(0,8);
                network_info_structure_.hang_down_list_.append(node_hang_down_info_);
            }
            this->network_node_info_unit_.node_info_structure_list_.append(network_info_structure_);
        }
    }
    else
    {
        if(data.length()!=3)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->node_start_no_=ushort(short(data.at(1))<<8)+uchar(data.at(0));
        this->node_num_=uchar(data.at(2));
    }
}
QByteArray qgdw_3762_protocol::AfnF0F102::EncodeFrameDataField()
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
        data.append(char(this->network_node_info_unit_.this_node_num_&0xFF));
        data.append(char(this->network_node_info_unit_.this_node_num_>>8));
        data.append(char(this->network_node_info_unit_.node_start_no_&0xFF));
        data.append(char(this->network_node_info_unit_.node_start_no_>>8));
        for(int i=0;i<this->network_node_info_unit_.node_info_structure_list_.size();i++)
        {
            for(int j=0;j<6;j++)
                data.append(this->network_node_info_unit_.node_info_structure_list_.at(i).node_address_.addr[5-j]);
            data.append(char(this->network_node_info_unit_.node_info_structure_list_.at(i).network_typelogy_info_.node_tei_&0xFF));
            data.append(char(this->network_node_info_unit_.node_info_structure_list_.at(i).network_typelogy_info_.node_tei_>>8));
            data.append(char(this->network_node_info_unit_.node_info_structure_list_.at(i).network_typelogy_info_.proxy_node_tei_&0xFF));
            data.append(char(this->network_node_info_unit_.node_info_structure_list_.at(i).network_typelogy_info_.proxy_node_tei_>>8));
            data.append(*reinterpret_cast<char*>(&this->network_node_info_unit_.node_info_structure_list_[i].network_typelogy_info_.node_info_));

            data.append(this->network_node_info_unit_.node_info_structure_list_.at(i).network_info_);
            data.append(this->network_node_info_unit_.node_info_structure_list_.at(i).device_type_[0]);
            data.append(this->network_node_info_unit_.node_info_structure_list_.at(i).device_type_[1]);
            data.append(this->network_node_info_unit_.node_info_structure_list_.at(i).phase_);
            data.append(char(this->network_node_info_unit_.node_info_structure_list_.at(i).proxy_change_times_&0xFF));
            data.append(char(this->network_node_info_unit_.node_info_structure_list_.at(i).proxy_change_times_>>8));
            data.append(char(this->network_node_info_unit_.node_info_structure_list_.at(i).node_offline_times_&0xFF));
            data.append(char(this->network_node_info_unit_.node_info_structure_list_.at(i).node_offline_times_>>8));
            data.append(char(this->network_node_info_unit_.node_info_structure_list_.at(i).node_offline_time_&0xFF));
            data.append(char(this->network_node_info_unit_.node_info_structure_list_.at(i).node_offline_time_>>8));
            data.append(char(this->network_node_info_unit_.node_info_structure_list_.at(i).node_offline_time_>>16));
            data.append(char(this->network_node_info_unit_.node_info_structure_list_.at(i).node_offline_time_>>24));
            data.append(char(this->network_node_info_unit_.node_info_structure_list_.at(i).node_offline_max_time_&0xFF));
            data.append(char(this->network_node_info_unit_.node_info_structure_list_.at(i).node_offline_max_time_>>8));
            data.append(char(this->network_node_info_unit_.node_info_structure_list_.at(i).node_offline_max_time_>>16));
            data.append(char(this->network_node_info_unit_.node_info_structure_list_.at(i).node_offline_max_time_>>24));
            data.append(char(this->network_node_info_unit_.node_info_structure_list_.at(i).communication_success_rate_up_));
            data.append(char(this->network_node_info_unit_.node_info_structure_list_.at(i).communication_success_rate_down_));
            for(int j=0;j<3;j++)
                data.append(this->network_node_info_unit_.node_info_structure_list_.at(i).primary_version_[j]);
            for(int j=0;j<2;j++)
                data.append(this->network_node_info_unit_.node_info_structure_list_.at(i).secondary_version_[j]);
            data.append(char(this->network_node_info_unit_.node_info_structure_list_.at(i).next_info_&0xFF));
            data.append(char(this->network_node_info_unit_.node_info_structure_list_.at(i).next_info_>>8));
            data.append(this->network_node_info_unit_.node_info_structure_list_.at(i).channel_type_);
            data.append(this->network_node_info_unit_.node_info_structure_list_.at(i).protocol_type_);
            data.append(this->network_node_info_unit_.node_info_structure_list_.at(i).area_state_);
            for(int j=0;j<6;j++)
                data.append(this->network_node_info_unit_.node_info_structure_list_.at(i).area_address_.addr[5-j]);
            data.append(char(this->network_node_info_unit_.node_info_structure_list_.at(i).hang_down_num_));
            if(this->network_node_info_unit_.node_info_structure_list_.at(i).hang_down_num_!=this->network_node_info_unit_.node_info_structure_list_.at(i).hang_down_list_.size())
            {
                throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataContentError,QString("DataField Content Error"));
            }
            for(int j=0;j<this->network_node_info_unit_.node_info_structure_list_.at(i).hang_down_list_.size();j++)
            {
                for(int m=0;m<6;m++)
                    data.append(this->network_node_info_unit_.node_info_structure_list_.at(i).hang_down_list_.at(j).node_address_.addr[5-m]);
                data.append(this->network_node_info_unit_.node_info_structure_list_.at(i).hang_down_list_.at(j).protocol_type_);
                data.append(this->network_node_info_unit_.node_info_structure_list_.at(i).hang_down_list_.at(j).device_type_);
            }

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

