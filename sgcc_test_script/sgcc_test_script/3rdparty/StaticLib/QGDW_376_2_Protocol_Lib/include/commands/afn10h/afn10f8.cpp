#include "afn10f8.h"

qgdw_3762_protocol::Afn10F8::Afn10F8()
{
    this->afn_=0x10;
    this->dt1_=char(0x80);
    this->dt2_=0x00;
}

qgdw_3762_protocol::Afn10F8::Afn10F8(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
:Frame3762Base(ctrl_field,info_field,address_field,0x10,char(0x80),0x00)
{
    this->ctrl_field_=ctrl_field;
    this->address_field_=address_field;
    this->info_field_=info_field;//

    this->afn_=0x10;
    this->dt1_=char(0x80);
    this->dt2_=0x00;
}
void qgdw_3762_protocol::Afn10F8::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if(data.length()<5)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->network_typelogy_info_unit_.node_total_num_=ushort((short(data.at(1))<<8))+uchar(data.at(0));
        this->network_typelogy_info_unit_.node_start_no_=ushort((short(data.at(3))<<8))+uchar(data.at(2));
        this->network_typelogy_info_unit_.this_node_num_=uchar(data.at(4));
        data.remove(0,5);
        if(this->network_typelogy_info_unit_.this_node_num_!=data.size()/11)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->network_typelogy_info_unit_.network_typelogy_info_List_.clear();
        for(int i=0;i<this->network_typelogy_info_unit_.this_node_num_;i++)
        {
            NetworkTypelogyInfo network_typelogy_info_;
            for(int j=0;j<6;j++)
            {
                network_typelogy_info_.node_address_.addr[j]=data.at(5-j);
            }
            network_typelogy_info_.node_tei_=ushort((short(data.at(7))<<8))+uchar(data.at(6));
            network_typelogy_info_.proxy_node_tei_=ushort((short(data.at(9))<<8))+uchar(data.at(8));
            data.remove(0,10);
            memcpy(&network_typelogy_info_.node_info_,data,1);
            this->network_typelogy_info_unit_.network_typelogy_info_List_.append(network_typelogy_info_);
            data.remove(0,1);
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
QByteArray qgdw_3762_protocol::Afn10F8::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if(this->network_typelogy_info_unit_.this_node_num_!=this->network_typelogy_info_unit_.network_typelogy_info_List_.size())
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataContentError,QString("DataField Content Error"));
        }
        data.append(char(this->network_typelogy_info_unit_.node_total_num_&0xFF));
        data.append(char(this->network_typelogy_info_unit_.node_total_num_>>8));
        data.append(char(this->network_typelogy_info_unit_.node_start_no_&0xFF));
        data.append(char(this->network_typelogy_info_unit_.node_start_no_>>8));
        data.append(char(this->network_typelogy_info_unit_.this_node_num_));
        for(int i=0;i<this->network_typelogy_info_unit_.network_typelogy_info_List_.size();i++)
        {
            for(int j=0;j<6;j++)
                data.append(this->network_typelogy_info_unit_.network_typelogy_info_List_.at(i).node_address_.addr[5-j]);

            data.append(char(this->network_typelogy_info_unit_.network_typelogy_info_List_.at(i).node_tei_&0xFF));
            data.append(char(this->network_typelogy_info_unit_.network_typelogy_info_List_.at(i).node_tei_>>8));
            data.append(char(this->network_typelogy_info_unit_.network_typelogy_info_List_.at(i).proxy_node_tei_&0xFF));
            data.append(char(this->network_typelogy_info_unit_.network_typelogy_info_List_.at(i).proxy_node_tei_>>8));
            data.append(*reinterpret_cast<char*>(&this->network_typelogy_info_unit_.network_typelogy_info_List_[i].node_info_));
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
