#include "afn06f4.h"

qgdw_3762_protocol::Afn06F4::Afn06F4()
{
    this->afn_=0x06;
    this->dt1_=0x08;
    this->dt2_=0x00;
}

qgdw_3762_protocol::Afn06F4::Afn06F4(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
:Frame3762Base(ctrl_field,info_field,address_field,0x06,0x08,0x00)
{
    this->ctrl_field_=ctrl_field;
    this->address_field_=address_field;
    this->info_field_=info_field;//

    this->afn_=0x06;
    this->dt1_=0x08;
    this->dt2_=0x00;
}
void qgdw_3762_protocol::Afn06F4::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if(data.length()<1)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->report_node_info_unit_.report_node_num_=uchar(data.at(0));
        data.remove(0,1);
        if(this->report_node_info_unit_.report_node_num_!=0)
        {
            if(data.length()<11)
            {
                throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
            }
            for(int i=0;i<6;i++)
            {
                this->report_node_info_unit_.report_node_address_.addr[i]=data.at(5-i);
            }
            this->report_node_info_unit_.report_node_protocol_=data.at(6);
            this->report_node_info_unit_.report_node_no_=ushort(short(data.at(8))<<8)+uchar(data.at(7));
            this->report_node_info_unit_.report_node_device_type_=data.at(9);
            this->report_node_info_unit_.attached_node_num_=uchar(data.at(10));
            this->report_node_info_unit_.transmit_node_num_=uchar(data.at(11));
            data.remove(0,12);
            if(this->report_node_info_unit_.transmit_node_num_!=data.size()/7)
            {
                throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
            }
            this->report_node_info_unit_.node_info_list_.clear();
            for(int i=0;i<this->report_node_info_unit_.transmit_node_num_;i++)
            {
                NodeInfo06F4 node_info_;
                for(int j=0;j<6;j++)
                {
                    node_info_.node_address_.addr[j]=data.at(5-j);
                }
                node_info_.node_protocol_=data.at(6);
                this->report_node_info_unit_.node_info_list_.append(node_info_);
                data.remove(0,7);
            }
        }
    }
    else
    {
        throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("Unknow Error"));
    }
}
QByteArray qgdw_3762_protocol::Afn06F4::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if(this->report_node_info_unit_.transmit_node_num_!=this->report_node_info_unit_.node_info_list_.size())
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        if(this->report_node_info_unit_.transmit_node_num_==0)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        data.append(char(this->report_node_info_unit_.report_node_num_));
        for(int i=0;i<6;i++)
        {
            data.append(this->report_node_info_unit_.report_node_address_.addr[5-i]);
        }
        data.append(this->report_node_info_unit_.report_node_protocol_);
        data.append(char(this->report_node_info_unit_.report_node_no_&0xFF));
        data.append(char(this->report_node_info_unit_.report_node_no_>>8));
        data.append(this->report_node_info_unit_.report_node_device_type_);
        data.append(char(this->report_node_info_unit_.attached_node_num_));
        data.append(char(this->report_node_info_unit_.transmit_node_num_));
        this->report_node_info_unit_.node_info_list_.clear();
        for(int i=0;i<this->report_node_info_unit_.node_info_list_.size();i++)
        {
            for(int j=0;j<6;j++)
            {
                data.append(this->report_node_info_unit_.node_info_list_.at(i).node_address_.addr[5-j]);
            }
            data.append(this->report_node_info_unit_.node_info_list_.at(i).node_protocol_);
        }
    }
    else
    {
        throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("Unknow Error"));
    }
    return  data;
}
