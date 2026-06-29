#include "afn11f1.h"
qgdw_3762_protocol::Afn11F1::Afn11F1()
{
    this->afn_=0x11;
    this->dt1_=0x01;
    this->dt2_=0x00;
}
qgdw_3762_protocol::Afn11F1::Afn11F1(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
:Frame3762Base(ctrl_field,info_field,address_field,0x11,0x01,0x00)
{
    this->ctrl_field_=ctrl_field;
    this->address_field_=address_field;
    this->info_field_=info_field;//

    this->afn_=0x11;
    this->dt1_=0x01;
    this->dt2_=0x00;
}
void qgdw_3762_protocol::Afn11F1::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirDown)
    {
        if(data.length()<1)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->node_num_=uchar(data.at(0));
        data.remove(0,1);
        if(this->node_num_!=data.size()/7)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataContentError,QString("DataField Content Error"));
        }
        this->node_parameter_list_.clear();
        for(int i=0;i<this->node_num_;i++)
        {
            NodeParameter node_parameter_;
            for(int j=0;j<6;j++)
                node_parameter_.node_address_.addr[j]=data.at(5-j);
            node_parameter_.protocol_type_=data.at(6);
            data.remove(0,7);
            this->node_parameter_list_.append(node_parameter_);
        }
    }
    else
    {
        throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("Unknow Error"));
    }
}
QByteArray qgdw_3762_protocol::Afn11F1::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirDown)
    {
        if(this->node_num_!=this->node_parameter_list_.size())
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataContentError,QString("DataField Content Error"));
        }
        data.append(char(this->node_num_));
        for(int i=0;i<this->node_parameter_list_.size();i++)
        {
            for(int j=0;j<6;j++)
                data.append(this->node_parameter_list_.at(i).node_address_.addr[5-j]);
            data.append(this->node_parameter_list_.at(i).protocol_type_);
        }
    }
    else
    {
        throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("Unknow Error"));
    }
    return  data;
}
