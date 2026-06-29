#include "afn06f1.h"

qgdw_3762_protocol::Afn06F1::Afn06F1()
{
    this->afn_=0x06;
    this->dt1_=0x01;
    this->dt2_=0x00;
}

qgdw_3762_protocol::Afn06F1::Afn06F1(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
:Frame3762Base(ctrl_field,info_field,address_field,0x06,0x01,0x00)
{
    this->ctrl_field_=ctrl_field;
    this->address_field_=address_field;
    this->info_field_=info_field;//

    this->afn_=0x06;
    this->dt1_=0x01;
    this->dt2_=0x00;
}
void qgdw_3762_protocol::Afn06F1::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if(data.length()<1)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        //此处为了支持抄控器扩展功能
        if(data.size()==7)
        {
            for(int i=0;i<6;i++)
            {
                this->report_address[i]=data.at(5-i);
            }
            this->connec_status=data.at(6);
            return;
        }

        this->report_node_num_=uchar(data.at(0));
        data.remove(0,1);
        if(this->report_node_num_!=data.size()/9)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->node_info_list_.clear();
        for(int i=0;i<this->report_node_num_;i++)
        {
            NodeInfo06F1 node_info_;
            for(int j=0;j<6;j++)
            {
                node_info_.node_address_.addr[j]=data.at(5-j);
            }
            node_info_.node_protocol_type_=data.at(6);
            node_info_.node_no_=ushort(short(data.at(8))<<8)+uchar(data.at(7));
            this->node_info_list_.append(node_info_);
            data.remove(0,9);
        }
    }
    else
    {
        throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("Unknow Error"));
    }
}
QByteArray qgdw_3762_protocol::Afn06F1::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if(this->report_node_num_!=this->node_info_list_.size())
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        data.append(char(this->report_node_num_));
        for(int i=0;i<this->node_info_list_.size();i++)
        {
            for(int j=0;j<6;j++)
            {
                data.append(this->node_info_list_.at(i).node_address_.addr[5-j]);
            }
            data.append(this->node_info_list_.at(i).node_protocol_type_);
            data.append(char(this->node_info_list_.at(i).node_no_&0xFF));
            data.append(char(this->node_info_list_.at(i).node_no_>>8));
        }
    }
    else
    {
        throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("Unknow Error"));
    }
    return  data;
}
