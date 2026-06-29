#include "afn11f3.h"

qgdw_3762_protocol::Afn11F3::Afn11F3()
{
    this->afn_=0x11;
    this->dt1_=0x04;
    this->dt2_=0x00;
}

qgdw_3762_protocol::Afn11F3::Afn11F3(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
:Frame3762Base(ctrl_field,info_field,address_field,0x11,0x04,0x00)
{
    this->ctrl_field_=ctrl_field;
    this->address_field_=address_field;
    this->info_field_=info_field;//

    this->afn_=0x11;
    this->dt1_=0x04;
    this->dt2_=0x00;
}
void qgdw_3762_protocol::Afn11F3::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirDown)
    {
        if(data.length()<7)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        for(int j=0;j<6;j++)
            this->node_address_.addr[j]=data.at(5-j);
        this->relay_level_=uchar(data.at(6));
        data.remove(0,7);
        if(this->relay_level_!=data.size()/6)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataContentError,QString("DataField Content Error"));
        }
        this->relay_node_address_list_.clear();
        for(int i=0;i<this->relay_level_;i++)
        {
            Address relay_node_address_;
            for(int j=0;j<6;j++)
                relay_node_address_.addr[j]=data.at(5-j);
            data.remove(0,6);
            this->relay_node_address_list_.append(relay_node_address_);
        }
    }
    else
    {
        throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("Unknow Error"));
    }
}
QByteArray qgdw_3762_protocol::Afn11F3::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirDown)
    {
        if(this->relay_level_!=this->relay_node_address_list_.size())
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataContentError,QString("DataField Content Error"));
        }
        for(int i=0;i<6;i++)
            data.append(this->node_address_.addr[5-i]);
        data.append(char(this->relay_level_));
        for(int i=0;i<this->relay_node_address_list_.size();i++)
        {
            for(int j=0;j<6;j++)
                data.append(this->relay_node_address_list_.at(i).addr[5-j]);
        }
    }
    else
    {
        throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("Unknow Error"));
    }
    return  data;
}

