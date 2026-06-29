#include "afn11f126_beijing.h"

qgdw_3762_protocol::Afn11F126_Beijing::Afn11F126_Beijing()
{
    this->afn_=0x11;
    this->dt1_=0x20;
    this->dt2_=0x0F;
}

qgdw_3762_protocol::Afn11F126_Beijing::Afn11F126_Beijing(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
:Frame3762Base(ctrl_field,info_field,address_field,0x11,0x20,0x0F)
{
    this->ctrl_field_=ctrl_field;
    this->address_field_=address_field;
    this->info_field_=info_field;//

    this->afn_=0x11;
    this->dt1_=0x20;
    this->dt2_=0x0F;
}
void qgdw_3762_protocol::Afn11F126_Beijing::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirDown)
    {
        if(data.length()<4)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->data_unit_.operate_type_=data.at(0);
        this->data_unit_.data_len_=uchar(data.at(1));
        this->data_unit_.preserve_[0]=data.at(2);
        this->data_unit_.preserve_[1]=data.at(3);
        data.remove(0,4);
        if(this->data_unit_.data_len_!=data.size())
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        this->data_unit_.node_net_lock_list_.clear();
        for(int i=0;i<this->data_unit_.data_len_/8;i++)
        {
            NodeNetLockParameter node_info_;
            node_info_.lock_or_unlock_flag_=data.at(0);
            node_info_.preserve_=data.at(1);
            for(int j=2;j<8;j++)
            {
                node_info_.node_address_.addr[j]=data.at(7-j);
            }
            this->data_unit_.node_net_lock_list_.append(node_info_);
            data.remove(0,8);
        }
    }
    else
    {
        throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("Unknow Error"));
    }
}
QByteArray qgdw_3762_protocol::Afn11F126_Beijing::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirDown)
    {
        data.append(this->data_unit_.operate_type_);
        data.append(char(this->data_unit_.data_len_));
        data.append(QByteArray(this->data_unit_.preserve_,2));
        for(int i=0;i<this->data_unit_.node_net_lock_list_.size();i++)
        {
            data.append(this->data_unit_.node_net_lock_list_.at(i).lock_or_unlock_flag_);
            data.append(this->data_unit_.node_net_lock_list_.at(i).preserve_);
            for(int j=0;j<6;j++)
                data.append(this->data_unit_.node_net_lock_list_.at(i).node_address_.addr[5-j]);
        }
    }
    else
    {
        throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("Unknow Error"));
    }
    return  data;
}

