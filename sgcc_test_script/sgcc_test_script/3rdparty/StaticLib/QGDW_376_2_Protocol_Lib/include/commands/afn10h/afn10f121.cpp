#include "afn10f121.h"

qgdw_3762_protocol::Afn10F121::Afn10F121()
{
    this->afn_=0x10;
    this->dt1_=0x01;
    this->dt2_=0x0F;
}

qgdw_3762_protocol::Afn10F121::Afn10F121(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
:Frame3762Base(ctrl_field,info_field,address_field,0x10,0x01,0x0F)
{
    this->ctrl_field_=ctrl_field;
    this->address_field_=address_field;
    this->info_field_=info_field;//

    this->afn_=0x10;
    this->dt1_=0x01;
    this->dt2_=0x0F;
}
void qgdw_3762_protocol::Afn10F121::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if(data.length()<5)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->area_identify_info_unit_.node_total_num_=ushort((short(data.at(1))<<8)+data.at(0));
        this->area_identify_info_unit_.node_start_no_=ushort((short(data.at(3))<<8)+data.at(2));
        this->area_identify_info_unit_.this_node_num_=uchar(data.at(4));
        data.remove(0,5);
        if(this->area_identify_info_unit_.this_node_num_!=data.size()/13)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->area_identify_info_unit_.area_identify_info_list_.clear();
        for(int i=0;i<this->area_identify_info_unit_.this_node_num_;i++)
        {
            AreaIdentifyInfo area_identify_info_;
            for(int j=0;j<6;j++)
            {
                area_identify_info_.node_address_.addr[j]=data.at(5-j);
            }
            area_identify_info_.node_identify_result_=data.at(6);
            for(int j=0;j<6;j++)
            {
                area_identify_info_.cco_address_.addr[j]=data.at(12-j);
            }
            data.remove(0,13);
            this->area_identify_info_unit_.area_identify_info_list_.append(area_identify_info_);
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
QByteArray qgdw_3762_protocol::Afn10F121::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if(this->area_identify_info_unit_.this_node_num_!=this->area_identify_info_unit_.area_identify_info_list_.size())
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataContentError,QString("DataField Content Error"));
        }
        data.append(char(this->area_identify_info_unit_.node_total_num_&0xFF));
        data.append(char(this->area_identify_info_unit_.node_total_num_>>8));
        data.append(char(this->area_identify_info_unit_.node_start_no_&0xFF));
        data.append(char(this->area_identify_info_unit_.node_start_no_>>8));
        data.append(char(this->area_identify_info_unit_.this_node_num_));
        for(int i=0;i<this->area_identify_info_unit_.area_identify_info_list_.size();i++)
        {
            for(int j=0;j<6;j++)
                data.append(this->area_identify_info_unit_.area_identify_info_list_.at(i).node_address_.addr[5-j]);
            data.append(this->area_identify_info_unit_.area_identify_info_list_.at(i).node_identify_result_);
            for(int j=0;j<6;j++)
                data.append(this->area_identify_info_unit_.area_identify_info_list_.at(i).cco_address_.addr[5-j]);
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
