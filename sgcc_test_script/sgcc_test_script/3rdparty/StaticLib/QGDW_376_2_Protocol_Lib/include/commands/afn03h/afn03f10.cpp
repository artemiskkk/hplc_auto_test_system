#include "afn03f10.h"

qgdw_3762_protocol::Afn03F10::Afn03F10()
{
    this->afn_=0x03;
    this->dt1_=0x02;
    this->dt2_=0x01;
}

qgdw_3762_protocol::Afn03F10::Afn03F10(const qgdw_3762_protocol::CtrlField ctrl_field, const qgdw_3762_protocol::InfoField info_field,const  qgdw_3762_protocol::AddressField address_field)
:Frame3762Base(ctrl_field,info_field,address_field,0x03,0x02,0x01)
{
    this->ctrl_field_=ctrl_field;
    this->info_field_=info_field;//
    this->address_field_=address_field;
    this->afn_=0x03;
    this->dt1_=0x02;
    this->dt2_=0x01;
}
void qgdw_3762_protocol::Afn03F10::DecodeFrameDataField(QByteArray data)
{
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        int com_rate_count=data.at(3)&0x0F;
        if(data.length()!=39+com_rate_count*2)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        memcpy(&this->operate_mode_info_unit_.local_communication_mode_,data,6);
        this->operate_mode_info_unit_.node_monitor_max_timeout_=uchar(data.at(6));
        this->operate_mode_info_unit_.broadcast_command_max_timeout_=ushort(short(data.at(8))<<8)+uchar(data.at(7));
        this->operate_mode_info_unit_.max_support_frame_length_=ushort(short(data.at(10))<<8)+uchar(data.at(9));
        this->operate_mode_info_unit_.max_transmit_data_length_=ushort(short(data.at(12))<<8)+uchar(data.at(11));
        this->operate_mode_info_unit_.upgrade_wait_time_=uchar(data.at(13));

        for(int i=0;i<6;i++)
        {
            this->operate_mode_info_unit_.master_node_address_.addr[i]=data.at(19-i);
        }
        this->operate_mode_info_unit_.max_support_node_num_=ushort(short(data.at(21))<<8)+uchar(data.at(20));
        this->operate_mode_info_unit_.current_node_num_=ushort(short(data.at(23))<<8)+uchar(data.at(22));
        data.remove(0,24);
        memcpy(&this->operate_mode_info_unit_.protocol_publish_date_,data,3);
        data.remove(0,3);
        memcpy(&this->operate_mode_info_unit_.protocol_record_date_,data,3);
        data.remove(0,3);
        QByteArray temp;
        for(int i=0;i<9;i++)
        {
            temp.append(data.at(8-i));
        }
        this->operate_mode_info_unit_.vendor_code_and_version_.version_ = QString::fromLatin1(temp.mid(0,2));
        this->operate_mode_info_unit_.vendor_code_and_version_.version_time_ = QString::fromLatin1(temp.mid(2,3));
        this->operate_mode_info_unit_.vendor_code_and_version_.chip_code_ = QString::fromLatin1(temp.mid(5,2));
        this->operate_mode_info_unit_.vendor_code_and_version_.vendor_code_ = QString::fromLatin1(temp.mid(7,2));
        data.remove(0,9);
        this->operate_mode_info_unit_.com_rate_unit_list_.clear();
        for(int i=0;i<com_rate_count;i++)
        {
            ComRateUnit03F10 com_rate_unit;
            memcpy(&com_rate_unit,data,2);
            this->operate_mode_info_unit_.com_rate_unit_list_.append(com_rate_unit);
            data.remove(0,2);
        }
    }
    else
    {
        if(data.length()!=0)
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
    }
}
QByteArray qgdw_3762_protocol::Afn03F10::EncodeFrameDataField()
{
    QByteArray data;
    if((this->ctrl_field_.dir&0x01) == qgdw_3762_protocol::kDirUp)
    {
        char temp[6];
        memcpy(temp,&this->operate_mode_info_unit_.local_communication_mode_,6);
        for(int i=0;i<6;i++)
            data.append(temp[i]);
        if(this->operate_mode_info_unit_.local_communication_mode_.rate_num_!=this->operate_mode_info_unit_.com_rate_unit_list_.size())
        {
            throw DecodeException(qgdw_3762_protocol::ExceptionCatalogue::kDataLengthError,QString("DataField Length Error"));
        }
        data.append(char(this->operate_mode_info_unit_.node_monitor_max_timeout_));

        data.append(char(this->operate_mode_info_unit_.broadcast_command_max_timeout_&0xff));
        data.append(char(this->operate_mode_info_unit_.broadcast_command_max_timeout_>>8));
        data.append(char(this->operate_mode_info_unit_.max_support_frame_length_&0xff));
        data.append(char(this->operate_mode_info_unit_.max_support_frame_length_>>8));
        data.append(char(this->operate_mode_info_unit_.max_transmit_data_length_&0xff));
        data.append(char(this->operate_mode_info_unit_.max_transmit_data_length_>>8));

        data.append(char(this->operate_mode_info_unit_.upgrade_wait_time_));
        for(int i=0;i<6;i++)
        {
            data.append(this->operate_mode_info_unit_.master_node_address_.addr[5-i]);
        }
        data.append(char(this->operate_mode_info_unit_.max_support_node_num_&0xff));
        data.append(char(this->operate_mode_info_unit_.max_support_node_num_>>8));
        data.append(char(this->operate_mode_info_unit_.current_node_num_&0xff));
        data.append(char(this->operate_mode_info_unit_.current_node_num_>>8));

        char temp1[3];
        memcpy(temp1,&this->operate_mode_info_unit_.protocol_publish_date_,3);
        for(int i=0;i<3;i++)
            data.append(temp1[i]);
        char temp1_[3];
        memcpy(temp1_,&this->operate_mode_info_unit_.protocol_record_date_,3);
        for(int i=0;i<3;i++)
            data.append(temp1_[i]);

        QByteArray temp2;
        temp2.append(this->operate_mode_info_unit_.vendor_code_and_version_.vendor_code_.at(1).toUpper());
        temp2.append(this->operate_mode_info_unit_.vendor_code_and_version_.vendor_code_.at(0).toUpper());

        temp2.append(this->operate_mode_info_unit_.vendor_code_and_version_.chip_code_.at(1).toUpper());
        temp2.append(this->operate_mode_info_unit_.vendor_code_and_version_.chip_code_.at(0).toUpper());

        temp2.append(this->operate_mode_info_unit_.vendor_code_and_version_.version_time_.at(2).toUpper());
        temp2.append(this->operate_mode_info_unit_.vendor_code_and_version_.version_time_.at(1).toUpper());
        temp2.append(this->operate_mode_info_unit_.vendor_code_and_version_.version_time_.at(0).toUpper());

        temp2.append(this->operate_mode_info_unit_.vendor_code_and_version_.version_.at(1).toUpper());
        temp2.append(this->operate_mode_info_unit_.vendor_code_and_version_.version_.at(0).toUpper());

        data.append(temp2);

        for(int i=0;i<this->operate_mode_info_unit_.com_rate_unit_list_.size();i++)
        {
            char temp_1_[2];
            memcpy(temp_1_,&this->operate_mode_info_unit_.com_rate_unit_list_.at(i),2);
            data.append(temp_1_[0]);
            data.append(temp_1_[1]);
        }
    }
    return  data;
}
