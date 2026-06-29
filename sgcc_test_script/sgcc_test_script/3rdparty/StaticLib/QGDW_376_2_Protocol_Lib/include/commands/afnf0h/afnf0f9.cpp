#include "afnf0f9.h"

qgdw_3762_protocol::AfnF0F9::AfnF0F9()
{
    this->afn_=char(0xF0);
    this->dt1_=0x01;
    this->dt2_=0x01;
}

qgdw_3762_protocol::AfnF0F9::AfnF0F9(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
:Frame3762Base(ctrl_field,info_field,address_field,char(0xF0),0x01,0x01)
{
    this->ctrl_field_=ctrl_field;
    this->address_field_=address_field;
    this->info_field_=info_field;//

    this->afn_=char(0xF0);
    this->dt1_=0x01;
    this->dt2_=0x01;
}
void qgdw_3762_protocol::AfnF0F9::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirDown)
    {
        if(data.length()!=6)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        for(int i=0;i<6;i++)
            this->query_node_address_.addr[i]=data.at(5-i);
    }
    else
    {
        if(data.length()<4)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->query_node_proxy_unit_.report_node_tei_=ushort(short(data.at(1))<<8)+uchar(data.at(0));
        this->query_node_proxy_unit_.report_node_level_=uchar(data.at(2));
        this->query_node_proxy_unit_.report_node_phase_=data.at(3);
        data.remove(0,4);
        this->query_node_proxy_unit_.mac_address_list_.clear();
        for(int i=0;i<this->query_node_proxy_unit_.report_node_level_;i++)
        {
            Address mac_address_;
            for(int j=0;j<6;j++)
                mac_address_.addr[j]=data.at(5-j);
            data.remove(0,6);
            this->query_node_proxy_unit_.mac_address_list_.append(mac_address_);
        }
    }
}
QByteArray qgdw_3762_protocol::AfnF0F9::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirDown)
    {
        for(int i=0;i<6;i++)
            data.append(this->query_node_address_.addr[5-i]);
    }
    else
    {
        if(this->query_node_proxy_unit_.report_node_level_!=this->query_node_proxy_unit_.mac_address_list_.size())
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        data.append(char(this->query_node_proxy_unit_.report_node_tei_&0xFF));
        data.append(char(this->query_node_proxy_unit_.report_node_tei_>>8));
        data.append(char(this->query_node_proxy_unit_.report_node_level_));
        data.append(this->query_node_proxy_unit_.report_node_phase_);
        for(int i=0;i<this->query_node_proxy_unit_.mac_address_list_.size();i++)
        {
            for(int j=0;j<6;j++)
                data.append(this->query_node_proxy_unit_.mac_address_list_.at(i).addr[5-j]);
        }
    }
    return  data;
}
