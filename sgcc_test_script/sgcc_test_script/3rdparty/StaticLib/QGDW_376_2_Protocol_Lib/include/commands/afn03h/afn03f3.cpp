#include "afn03f3.h"

qgdw_3762_protocol::Afn03F3::Afn03F3()
{
    this->afn_=0x03;
    this->dt1_=0x04;
    this->dt2_=0x00;
}

qgdw_3762_protocol::Afn03F3::Afn03F3(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field, const qgdw_3762_protocol::AddressField address_field)
    :Frame3762Base(ctrl_field,info_field,address_field,0x03,0x04,0x00)
{
    this->ctrl_field_=ctrl_field;
    this->info_field_=info_field;//
    this->address_field_=address_field;
    this->afn_=0x03;
    this->dt1_=0x04;
    this->dt2_=0x00;
}
void qgdw_3762_protocol::Afn03F3::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if(data.length()<2)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length < 2 "));
        }
        this->listen_node_total_num_=uchar(data.at(0));
        this->listen_this_frame_node_total_num_=uchar(data.at(1));
        if(this->listen_node_total_num_<this->listen_this_frame_node_total_num_)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataContentError,QString("DataField Content Error"));
        }
        if((data.length()-2)!=this->listen_this_frame_node_total_num_*8)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataContentFormatError,QString("DataField Content Format Error"));
        }
        data.remove(0,2);
        this->listen_node_Info_list_.clear();
        for(int i=0;i<listen_this_frame_node_total_num_;i++)
        {
            ListenNodeInfo listen_node_info_;
            memcpy(reinterpret_cast<char*>(&listen_node_info_),data.data(),8);
            Address address;
            for(int j=0;j<6;j++)
            {
                address.addr[j]=listen_node_info_.node_address_.addr[5-j];
            }
            memcpy(&listen_node_info_.node_address_,&address,6);
            this->listen_node_Info_list_.append(listen_node_info_);
            data.remove(0,8);
        }
    }
    else
    {
        if(data.length() != 2)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length != 0"));
        }
        this->start_node_pointer_=uchar(data.at(0));
        this->read_node_num_=uchar(data.at(1));
    }
}
QByteArray qgdw_3762_protocol::Afn03F3::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        if((this->listen_node_total_num_<this->listen_this_frame_node_total_num_)||(this->listen_node_Info_list_.size()!=this->listen_this_frame_node_total_num_))
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataContentError,QString("DataField Content Error"));
        }
        if(this->listen_node_Info_list_.size()==0)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("侦听节点信息QList元素个数为0"));
        }
        data.append(char(this->listen_node_total_num_));
        data.append(char(this->listen_this_frame_node_total_num_));
        for(int i=0;i<this->listen_node_Info_list_.size();i++)
        {
            Address address;
            for(int j=0;j<6;j++)
            {
                address.addr[j]=listen_node_Info_list_.at(i).node_address_.addr[5-j];
            }
            listen_node_Info_list_[i].node_address_=address;
            char listen_node_info[8];
            memcpy(listen_node_info,&this->listen_node_Info_list_.at(i),8);
            for(int j=0;j<8;j++)
                data.append(listen_node_info[j]);
        }
    }
    else
    {
        data.append(char(start_node_pointer_));
        data.append(char(read_node_num_));
    }
    return  data;
}
