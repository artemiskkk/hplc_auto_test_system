#include "afnf0f100.h"

qgdw_3762_protocol::AfnF0F100::AfnF0F100()
{
    this->afn_=char(0xF0);
    this->dt1_=0x08;
    this->dt2_=0x0C;
}

qgdw_3762_protocol::AfnF0F100::AfnF0F100(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
:Frame3762Base(ctrl_field,info_field,address_field,char(0xF0),0x08,0x0C)
{
    this->ctrl_field_=ctrl_field;
    this->address_field_=address_field;
    this->info_field_=info_field;//

    this->afn_=char(0xF0);
    this->dt1_=0x08;
    this->dt2_=0x0C;
}
void qgdw_3762_protocol::AfnF0F100::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if(data.length()<16)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length != 9"));
        }
        this->network_basic_unit_.node_total_num_=ushort(short(data.at(1))<<8)+uchar(data.at(0));
        this->network_basic_unit_.node_online_num_=ushort(short(data.at(3))<<8)+uchar(data.at(2));
        this->network_basic_unit_.begin_build_network_time_=uint(int(data.at(7))<<24)+uint(int(data.at(6))<<16)+uint(int(data.at(5))<<8)+uchar(data.at(4));
        this->network_basic_unit_.build_network_time_=ushort(short(data.at(9))<<8)+uchar(data.at(8));
        this->network_basic_unit_.beacon_period_=uchar(data.at(10));
        this->network_basic_unit_.router_period_=ushort(short(data.at(12))<<8)+uchar(data.at(11));
        this->network_basic_unit_.topology_level_=ushort(short(data.at(14))<<8)+uchar(data.at(13));
        this->network_basic_unit_.total_level_=uchar(data.at(15));
        data.remove(0,16);
        if(this->network_basic_unit_.total_level_!=data.size()/3)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length != 9"));
        }
        this->network_basic_unit_.level_info_list_.clear();
        for(int i=0;i<this->network_basic_unit_.total_level_;i++)
        {
            LevelInfo level_info_;
            level_info_.current_level_=data.at(0);
            level_info_.node_num_=ushort(short(data.at(2))<<8)+uchar(data.at(1));
            data.remove(0,3);
            this->network_basic_unit_.level_info_list_.append(level_info_);
        }
    }
    else
    {
        if(data.length() != 0)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length != 0"));
        }
    }
}
QByteArray qgdw_3762_protocol::AfnF0F100::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if(this->network_basic_unit_.total_level_!=this->network_basic_unit_.level_info_list_.size())
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length != 9"));
        }
        data.append(char(this->network_basic_unit_.node_total_num_&0xFF));
        data.append(char(this->network_basic_unit_.node_total_num_>>8));

        data.append(char(this->network_basic_unit_.node_online_num_&0xFF));
        data.append(char(this->network_basic_unit_.node_online_num_>>8));

        data.append(char(this->network_basic_unit_.begin_build_network_time_&0xFF));
        data.append(char(this->network_basic_unit_.begin_build_network_time_>>8));
        data.append(char(this->network_basic_unit_.begin_build_network_time_>>16));
        data.append(char(this->network_basic_unit_.begin_build_network_time_>>24));

        data.append(char(this->network_basic_unit_.build_network_time_&0xFF));
        data.append(char(this->network_basic_unit_.build_network_time_>>8));

        data.append(char(this->network_basic_unit_.beacon_period_&0xFF));

        data.append(char(this->network_basic_unit_.router_period_&0xFF));
        data.append(char(this->network_basic_unit_.router_period_>>8));

        data.append(char(this->network_basic_unit_.topology_level_&0xFF));
        data.append(char(this->network_basic_unit_.topology_level_>>8));

        data.append(char(this->network_basic_unit_.total_level_&0xFF));
        for(int i=0;i<this->network_basic_unit_.level_info_list_.size();i++)
        {
            data.append(this->network_basic_unit_.level_info_list_.at(i).current_level_);

            data.append(char(this->network_basic_unit_.level_info_list_.at(i).node_num_&0xFF));
            data.append(char(this->network_basic_unit_.level_info_list_.at(i).node_num_>>8));
        }
    }
    return  data;
}
