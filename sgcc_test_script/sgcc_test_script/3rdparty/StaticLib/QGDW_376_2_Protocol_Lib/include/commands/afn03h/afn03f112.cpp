#include "afn03f112.h"

qgdw_3762_protocol::Afn03F112::Afn03F112()
{
    this->afn_=0x03;
    this->dt1_=char(0x80);
    this->dt2_=0x0D;
}

qgdw_3762_protocol::Afn03F112::Afn03F112(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
:Frame3762Base(ctrl_field,info_field,address_field,0x03,char(0x80),0x0E)
{
    this->ctrl_field_=ctrl_field;
    this->info_field_=info_field;//
    this->address_field_=address_field;
    this->afn_=0x03;
    this->dt1_=char(0x80);
    this->dt2_=0x0D;
}
void qgdw_3762_protocol::Afn03F112::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if(data.size()<5)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error "));
        }
        this->carriar_chip_info_unit_.node_total_num_=ushort(short(data.at(1))<<8)+uchar(data.at(0));
        this->carriar_chip_info_unit_.start_node_no_=ushort(short(data.at(3))<<8)+uchar(data.at(2));
        this->carriar_chip_info_unit_.response_node_num_=uchar(data.at(4));
        data.remove(0,5);
        if(this->carriar_chip_info_unit_.response_node_num_!=data.size()/33)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Content Error "));
        }
        for(int i=0;i<this->carriar_chip_info_unit_.response_node_num_;i++)
        {
            NodeInfo03F112 node_info_;
            memcpy(&node_info_,data,33);
            for(int j=0;j<6;j++)
            {
                node_info_.node_address_.addr[j]=data.at(5-j);
            }
            this->carriar_chip_info_unit_.node_info_list.append(node_info_);
            data.remove(0,33);
        }
    }
    else
    {
        if(data.length()!=3)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length error "));
        }
        this->start_node_no_=ushort(short(data.at(1))<<8)+uchar(data.at(0));
        this->node_num_=uchar(data.at(2));
    }
}
QByteArray qgdw_3762_protocol::Afn03F112::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if(this->carriar_chip_info_unit_.response_node_num_!=this->carriar_chip_info_unit_.node_info_list.size())
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Content Error "));
        }
        data.append(char(this->carriar_chip_info_unit_.node_total_num_&0xFF));
        data.append(char(this->carriar_chip_info_unit_.node_total_num_>>8));
        data.append(char(this->carriar_chip_info_unit_.start_node_no_&0xFF));
        data.append(char(this->carriar_chip_info_unit_.start_node_no_>>8));
        data.append(char(this->carriar_chip_info_unit_.response_node_num_));

        for(int i=0;i<this->carriar_chip_info_unit_.node_info_list.size();i++)
        {
            for(int j=0;j<6;j++)
            {
                data.append(this->carriar_chip_info_unit_.node_info_list.at(i).node_address_.addr[5-j]);
            }
            data.append(this->carriar_chip_info_unit_.node_info_list.at(i).node_type_);
            for(int j=0;j<24;j++)
            {
                data.append(this->carriar_chip_info_unit_.node_info_list.at(i).node_chip_id_info_[j]);
            }
            for(int j=0;j<2;j++)
            {
                data.append(this->carriar_chip_info_unit_.node_info_list.at(i).node_chip_software_version_[j]);
            }
        }
    }
    else
    {
        data.append(char(this->start_node_no_&0xFF));
        data.append(char(this->start_node_no_>>8));
        data.append(char(this->node_num_));
    }
    return data;
}
