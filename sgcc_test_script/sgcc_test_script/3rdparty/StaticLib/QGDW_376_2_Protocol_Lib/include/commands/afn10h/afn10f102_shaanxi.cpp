#include "afn10f102_shaanxi.h"

qgdw_3762_protocol::Afn10F102_Shaanxi::Afn10F102_Shaanxi()
{
    this->afn_=0x10;
    this->dt1_=0x20;
    this->dt2_=0x0C;
}

qgdw_3762_protocol::Afn10F102_Shaanxi::Afn10F102_Shaanxi(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
:Frame3762Base(ctrl_field,info_field,address_field,0x10,0x20,0x0C)
{
    this->ctrl_field_=ctrl_field;
    this->address_field_=address_field;
    this->info_field_=info_field;//

    this->afn_=0x10;
    this->dt1_=0x20;
    this->dt2_=0x0C;
}
void qgdw_3762_protocol::Afn10F102_Shaanxi::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if(data.length()<3)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->node_info_unit_.node_total_num_=ushort((short(data.at(1))<<8))+uchar(data.at(0));
        this->node_info_unit_.this_node_num_=uchar(data.at(2));
        data.remove(0,3);
        if(this->node_info_unit_.this_node_num_!=data.size()/8)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->node_info_unit_.node_info_list_.clear();
        for(int i=0;i<this->node_info_unit_.this_node_num_;i++)
        {
            NodeInfo10F102_Shaanxi node_info;
            for(int j=0;j<6;j++)
            {
                node_info.node_address_.addr[j]=data.at(5-j);
            }
            data.remove(0,6);
            memcpy(&node_info.node_phase_info_,data,2);
            data.remove(0,2);
            this->node_info_unit_.node_info_list_.append(node_info);
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
QByteArray qgdw_3762_protocol::Afn10F102_Shaanxi::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if(this->node_info_unit_.this_node_num_!=this->node_info_unit_.node_info_list_.size())
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataContentError,QString("DataField Content Error"));
        }
        data.append(char(this->node_info_unit_.node_total_num_&0xFF));
        data.append(char(this->node_info_unit_.node_total_num_>>8));
        data.append(char(this->node_info_unit_.this_node_num_));
        for(int i=0;i<this->node_info_unit_.node_info_list_.size();i++)
        {
            for(int j=0;j<6;j++)
                data.append(this->node_info_unit_.node_info_list_.at(i).node_address_.addr[5-j]);
            char temp[2];
            memcpy(temp,&this->node_info_unit_.node_info_list_.at(i).node_phase_info_,2);
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
