#include "afn10f7.h"

qgdw_3762_protocol::Afn10F7::Afn10F7()
{
    this->afn_=0x10;
    this->dt1_=0x40;
    this->dt2_=0x00;
}

qgdw_3762_protocol::Afn10F7::Afn10F7(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
:Frame3762Base(ctrl_field,info_field,address_field,0x10,0x40,0x00)
{
    this->ctrl_field_=ctrl_field;
    this->address_field_=address_field;
    this->info_field_=info_field;//

    this->afn_=0x10;
    this->dt1_=0x40;
    this->dt2_=0x00;

}
void qgdw_3762_protocol::Afn10F7::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if(data.length()<3)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->node_id_info_unit_.node_total_num_=ushort((short(data.at(1))<<8))+uchar(data.at(0));
        this->node_id_info_unit_.this_node_num_=uchar(data.at(2));
        data.remove(0,3);
        this->node_id_info_unit_.node_id_info_list_.clear();
        for(int i=0;i<this->node_id_info_unit_.this_node_num_;i++)
        {
            NodeIdInfo node_id_info_;
            for(int j=0;j<6;j++)
            {
                node_id_info_.node_address_.addr[j]=data.at(5-j);
            }
            *reinterpret_cast<char*>(&node_id_info_.node_type_)=data.at(6);
            node_id_info_.vendor_code_.append(data.at(8));
            node_id_info_.vendor_code_.append(data.at(7));
            node_id_info_.node_id_length_=uchar(data.at(9));
            node_id_info_.node_id_format_=data.at(10);
            node_id_info_.node_id_.append(data.mid(11,node_id_info_.node_id_length_));
            data.remove(0,11+node_id_info_.node_id_length_);
            this->node_id_info_unit_.node_id_info_list_.append(node_id_info_);
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
QByteArray qgdw_3762_protocol::Afn10F7::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if(this->node_id_info_unit_.this_node_num_!=this->node_id_info_unit_.node_id_info_list_.size())
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataContentError,QString("DataField Content Error"));
        }
        data.append(char(this->node_id_info_unit_.node_total_num_&0xFF));
        data.append(char(this->node_id_info_unit_.node_total_num_>>8));
        data.append(char(this->node_id_info_unit_.this_node_num_));
        for(int i=0;i<this->node_id_info_unit_.node_id_info_list_.size();i++)
        {
            for(int j=0;j<6;j++)
                data.append(this->node_id_info_unit_.node_id_info_list_.at(i).node_address_.addr[5-j]);
            data.append(*reinterpret_cast<char*>(&this->node_id_info_unit_.node_id_info_list_[i].node_type_));
            data.append(this->node_id_info_unit_.node_id_info_list_.at(i).vendor_code_.at(1).toUpper());
            data.append(this->node_id_info_unit_.node_id_info_list_.at(i).vendor_code_.at(0).toUpper());
            data.append(char(this->node_id_info_unit_.node_id_info_list_.at(i).node_id_length_));
            data.append(this->node_id_info_unit_.node_id_info_list_.at(i).node_id_format_);
            data.append(this->node_id_info_unit_.node_id_info_list_.at(i).node_id_);
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
