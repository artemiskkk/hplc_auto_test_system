#include "afn06f6.h"

qgdw_3762_protocol::Afn06F6::Afn06F6()
{
    this->afn_=0x06;
    this->dt1_=0x20;
    this->dt2_=0x00;
}

qgdw_3762_protocol::Afn06F6::Afn06F6(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
:Frame3762Base(ctrl_field,info_field,address_field,0x06,0x20,0x00)
{
    this->ctrl_field_=ctrl_field;
    this->address_field_=address_field;
    this->info_field_=info_field;//

    this->afn_=0x06;
    this->dt1_=0x20;
    this->dt2_=0x00;
}
void qgdw_3762_protocol::Afn06F6::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if(data.length()<1)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->report_node_num_=uchar(data.at(0));
        data.remove(0,1);
        if(this->report_node_num_!=data.size()/6)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->report_node_address_list_.clear();
        for(int i=0;i<this->report_node_num_;i++)
        {
            Address address;
            for(int j=0;j<6;j++)
                address.addr[j]=data.at(5-j);
            this->report_node_address_list_.append(address);
            data.remove(0,6);
        }
    }
    else
    {
        throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("Unknow Error"));
    }
}
QByteArray qgdw_3762_protocol::Afn06F6::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if(this->report_node_num_!=this->report_node_address_list_.size())
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        data.append(char(this->report_node_num_));
        for(int i=0;i<this->report_node_address_list_.size();i++)
        {
            for(int j=0;j<6;j++)
                data.append(this->report_node_address_list_.at(i).addr[5-j]);
        }
    }
    else
    {
        throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("Unknow Error"));
    }
    return  data;
}
