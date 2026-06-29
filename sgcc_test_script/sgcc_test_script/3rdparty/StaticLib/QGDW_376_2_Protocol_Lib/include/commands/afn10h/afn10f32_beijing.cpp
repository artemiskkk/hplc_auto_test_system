#include "afn10f32_beijing.h"

qgdw_3762_protocol::Afn10F32_Beijing::Afn10F32_Beijing()
{
    this->afn_=0x10;
    this->dt1_=char(0x80);
    this->dt2_=0x03;
}

qgdw_3762_protocol::Afn10F32_Beijing::Afn10F32_Beijing(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
:Frame3762Base(ctrl_field,info_field,address_field,0x10,char(0x80),0x03)
{
    this->ctrl_field_=ctrl_field;
    this->address_field_=address_field;
    this->info_field_=info_field;//

    this->afn_=0x10;
    this->dt1_=char(0x80);
    this->dt2_=0x03;


}
void qgdw_3762_protocol::Afn10F32_Beijing::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if(data.length()<5)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->node_phase_info_unit_.node_total_num_=ushort((short(data.at(1))<<8))+uchar(data.at(0));
        this->node_phase_info_unit_.node_start_no_=ushort((short(data.at(3))<<8))+uchar(data.at(2));
        this->node_phase_info_unit_.this_node_num_=uchar(data.at(4));
        data.remove(0,5);
        if(this->node_phase_info_unit_.this_node_num_!=data.size()/8)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->node_phase_info_unit_.node_info_list_.clear();
        for(int i=0;i<this->node_phase_info_unit_.this_node_num_;i++)
        {
            NodeCarrierChannelInfo node_info_;
            for(int j=0;j<6;j++)
            {
                node_info_.node_address_.addr[j]=data.at(5-j);
            }
            node_info_.node_channel_quality_up_=uchar(data.at(6));
            node_info_.node_channel_quality_up_=uchar(data.at(7));
            this->node_phase_info_unit_.node_info_list_.append(node_info_);
            data.remove(0,8);
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
QByteArray qgdw_3762_protocol::Afn10F32_Beijing::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if(this->node_phase_info_unit_.this_node_num_!=this->node_phase_info_unit_.node_info_list_.size())
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataContentError,QString("DataField Content Error"));
        }
        data.append(char(this->node_phase_info_unit_.node_total_num_&0xFF));
        data.append(char(this->node_phase_info_unit_.node_total_num_>>8));
        data.append(char(this->node_phase_info_unit_.node_start_no_&0xFF));
        data.append(char(this->node_phase_info_unit_.node_start_no_>>8));
        data.append(char(this->node_phase_info_unit_.this_node_num_));
        for(int i=0;i<this->node_phase_info_unit_.node_info_list_.size();i++)
        {
            for(int j=0;j<6;j++)
                data.append(this->node_phase_info_unit_.node_info_list_.at(i).node_address_.addr[5-j]);
            data.append(char(this->node_phase_info_unit_.node_info_list_.at(i).node_channel_quality_up_));
            data.append(char(this->node_phase_info_unit_.node_info_list_.at(i).node_channel_quality_down_));
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
