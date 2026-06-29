#ifndef AFN13F51_H
#define AFN13F51_H


#include <QObject>

#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"

namespace qgdw_3762_protocol {
/**
 * @brief The Afn13F51FieldDown Afn13F51 下行数据域
 */
struct Afn13F51FieldDown
{
    char protocol_type_;//!<通信协议类型
    char reserve_;
    char delay_tag_;//!<通信延时相关性标志
    uchar sub_node_num_;//!<从节点附属节点数量
    QList<Address> sub_node_address_list_;//!<从节点附属节点地址QList
    ushort frame_length_;//!<报文长度
    QByteArray frame_content_;//!<报文内容
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const Afn13F51FieldDown &unit_)const
    {
        if(this->protocol_type_==unit_.protocol_type_
                &&this->reserve_==unit_.reserve_
                &&this->delay_tag_==unit_.delay_tag_
                &&this->sub_node_num_==unit_.sub_node_num_
                &&this->frame_length_==unit_.frame_length_
                &&this->frame_content_==unit_.frame_content_
                &&this->sub_node_address_list_.size()==unit_.sub_node_address_list_.size())
        {
            for(int i=0;i<this->sub_node_address_list_.size();i++)
            {
                if(!(this->sub_node_address_list_.at(i)==unit_.sub_node_address_list_.at(i)))
                    return false;
            }
            return true;
        }
        else
            return false;
    }
};
/**
 * @brief The Afn13F51FieldUp Afn13F51 上行数据域
 */
struct Afn13F51FieldUp
{
    ushort up_delay_time_;//!<当前报文本地通信上行时长
    char protocol_type_;//!<通信协议类型
    char reserve_;
    ushort frame_length_;//!<报文长度
    QByteArray frame_content_;//!<报文内容
    /**
    * @brief operator ==
    * @param unit_
    * @return
    */
    bool operator==(const Afn13F51FieldUp &unit_)const
    {
       if(this->protocol_type_==unit_.protocol_type_
               &&this->up_delay_time_==unit_.up_delay_time_
                &&this->reserve_==unit_.reserve_
               &&this->frame_length_==unit_.frame_length_
               &&this->frame_content_==unit_.frame_content_)
           return true;

       else
           return false;
    }
};

#ifdef UNIT_TEST
class Afn13F51 : public Frame3762Base
#else
/**
 * @brief The Afn13F51 监控从节点
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT Afn13F51 : public Frame3762Base
#endif
{
public:
    /**
     * @brief Afn13F51
     */
    Afn13F51();
    /**
     * @brief Afn13F51
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    Afn13F51(CtrlField ctrl_field,InfoField info_field,AddressField address_field);

    Afn13F51FieldDown data_field_down_;//!<下行数据域
    Afn13F51FieldUp data_field_up_;//!<上行数据域
public:
    void DecodeFrameDataField(QByteArray data) override;

protected:
    QByteArray EncodeFrameDataField() override;
};
}

#endif // AFN13F51_H
