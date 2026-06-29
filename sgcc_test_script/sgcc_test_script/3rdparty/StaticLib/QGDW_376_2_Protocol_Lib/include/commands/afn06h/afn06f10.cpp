#include "afn06f10.h"

qgdw_3762_protocol::Afn06F10::Afn06F10()
{
    this->afn_=0x06;
    this->dt1_=0x02;
    this->dt2_=0x01;
}

qgdw_3762_protocol::Afn06F10::Afn06F10(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
:Frame3762Base(ctrl_field,info_field,address_field,0x06,0x02,0x01)
{
    this->ctrl_field_=ctrl_field;
    this->address_field_=address_field;
    this->info_field_=info_field;//

    this->afn_=0x06;
    this->dt1_=0x02;
    this->dt2_=0x01;
}
void qgdw_3762_protocol::Afn06F10::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if(data.length()==1)
            return;
        if(data.length()<5)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->report_node_state_unit_.report_node_total_num_=ushort(short(data.at(1))<<8)+uchar(data.at(0));
        this->report_node_state_unit_.this_report_node_num_=uchar(data.at(2));
        if(!(this->report_node_state_unit_.this_report_node_num_<10))
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->report_node_state_unit_.report_start_no_=ushort(short(data.at(4))<<8)+uchar(data.at(3));
        data.remove(0,5);
        if(this->report_node_state_unit_.this_report_node_num_==data.size()/36)
        {
            for(int i=0;i<this->report_node_state_unit_.this_report_node_num_;i++)
            {
                NodeStateInfo node_state_info_;
                for(int j=0;j<6;j++)
                {
                    node_state_info_.node_address_.addr[j]=data.at(5-j);
                }
                node_state_info_.node_state_=data.at(6);
                node_state_info_.offline_time_=uchar(data.at(7))+uint(int(data.at(8))<<8)+uint(int(data.at(9))<<16)+uint(int(data.at(10))<<24);
                node_state_info_.offline_reason_=data.at(11);
                node_state_info_.chip_id_=data.mid(12,24);
                this->report_node_state_unit_.node_state_info_list_.append(node_state_info_);
                data.remove(0,36);
            }
        }
        else if(this->report_node_state_unit_.this_report_node_num_==data.size()/12)
        {
            for(int i=0;i<this->report_node_state_unit_.this_report_node_num_;i++)
            {
                NodeStateInfo node_state_info_;
                for(int j=0;j<6;j++)
                {
                    node_state_info_.node_address_.addr[j]=data.at(5-j);
                }
                node_state_info_.node_state_=data.at(6);
                node_state_info_.offline_time_=uchar(data.at(7))+uint(int(data.at(8))<<8)+uint(int(data.at(9))<<16)+uint(int(data.at(10))<<24);
                node_state_info_.offline_reason_=data.at(11);
                this->report_node_state_unit_.node_state_info_list_.append(node_state_info_);
                data.remove(0,12);
            }
        }
        else
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("Unknow Error"));
        }

    }
    else
    {
        throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("Unknow Error"));
    }
}
QByteArray qgdw_3762_protocol::Afn06F10::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if(this->report_node_state_unit_.this_report_node_num_!=this->report_node_state_unit_.node_state_info_list_.size())
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        data.append(char(this->report_node_state_unit_.report_node_total_num_&0xFF));
        data.append(char(this->report_node_state_unit_.report_node_total_num_>>8));
        data.append(char(this->report_node_state_unit_.this_report_node_num_));
        data.append(char(this->report_node_state_unit_.report_start_no_&0xFF));
        data.append(char(this->report_node_state_unit_.report_start_no_>>8));

        for(int i=0;i<this->report_node_state_unit_.node_state_info_list_.size();i++)
        {
            for(int j=0;j<6;j++)
            {
                data.append(this->report_node_state_unit_.node_state_info_list_.at(i).node_address_.addr[5-j]);
            }
            data.append(this->report_node_state_unit_.node_state_info_list_.at(i).node_state_);
            data.append(char(this->report_node_state_unit_.node_state_info_list_.at(i).offline_time_&0xFF));
            data.append(char(this->report_node_state_unit_.node_state_info_list_.at(i).offline_time_>>8));
            data.append(char(this->report_node_state_unit_.node_state_info_list_.at(i).offline_time_>>16));
            data.append(char(this->report_node_state_unit_.node_state_info_list_.at(i).offline_time_>>24));

            data.append(this->report_node_state_unit_.node_state_info_list_.at(i).offline_reason_);
            data.append(this->report_node_state_unit_.node_state_info_list_.at(i).chip_id_);
        }
    }
    else
    {
        throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("Unknow Error"));
    }
    return  data;
}
