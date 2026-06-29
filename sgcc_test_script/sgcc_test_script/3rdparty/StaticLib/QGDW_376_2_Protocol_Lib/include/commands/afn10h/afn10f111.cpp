#include "afn10f111.h"

qgdw_3762_protocol::Afn10F111::Afn10F111()
{
    this->afn_=0x10;
    this->dt1_=0x40;
    this->dt2_=0x0D;
}

qgdw_3762_protocol::Afn10F111::Afn10F111(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
:Frame3762Base(ctrl_field,info_field,address_field,0x10,0x40,0x0D)
{
    this->ctrl_field_=ctrl_field;
    this->address_field_=address_field;
    this->info_field_=info_field;//

    this->afn_=0x10;
    this->dt1_=0x40;
    this->dt2_=0x0D;
}
void qgdw_3762_protocol::Afn10F111::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if(data.length()<10)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->multi_network_info_unit_.total_num_=uchar(data.at(0));
        data.remove(0,1);
        memcpy(&this->multi_network_info_unit_.this_network_id_,data,3);
        data.remove(0,3);
        for(int i=0;i<6;i++)
        {
            this->multi_network_info_unit_.this_node_address_.addr[i]=data.at(5-i);
        }
        data.remove(0,6);
        if(this->multi_network_info_unit_.total_num_!=data.size()/3)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->multi_network_info_unit_.neighbor_network_id_List_.clear();
        for(int i=0;i<this->multi_network_info_unit_.total_num_;i++)
        {
            NeighborNetworkId neighbor_network_id_;
            memcpy(&neighbor_network_id_,data,3);
            data.remove(0,3);
            this->multi_network_info_unit_.neighbor_network_id_List_.append(neighbor_network_id_);
        }
    }
    else
    {
        if(data.length()!=0)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
    }
}
QByteArray qgdw_3762_protocol::Afn10F111::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if(this->multi_network_info_unit_.total_num_!=this->multi_network_info_unit_.neighbor_network_id_List_.size())
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataContentError,QString("DataField Content Error"));
        }
        data.append(char(this->multi_network_info_unit_.total_num_));
        for(int i=0;i<3;i++)
        {
            data.append(this->multi_network_info_unit_.this_network_id_.network_id_[i]);
        }
        for(int i=0;i<6;i++)
        {
            data.append(this->multi_network_info_unit_.this_node_address_.addr[5-i]);
        }
        for(int i=0;i<this->multi_network_info_unit_.neighbor_network_id_List_.size();i++)
        {
            for(int j=0;j<3;j++)
            {
                data.append(this->multi_network_info_unit_.neighbor_network_id_List_.at(i).network_id_[j]);
            }
        }
    }
    return  data;
}
