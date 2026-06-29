#ifndef AFNF1F1_H
#define AFNF1F1_H

#include <QObject>

#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"

namespace qgdw_3762_protocol {
/**
 * @brief The ConcurrentReadMeterUnitDown 并发抄表数据单元下行
 */
struct ConcurrentReadMeterUnitDown
{
    char protocol_type_;//!<规约类型
    uchar subsidiary_node_num_;//!<附属从节点数量
    QList<Address> subsidiary_node_address_list_;//附属从节点地址QList
    ushort frame_length_;//!<报文长度
    QByteArray frame_content_;//!<报文内容
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const ConcurrentReadMeterUnitDown &unit_)const
    {
        if(this->protocol_type_==unit_.protocol_type_
                &&this->subsidiary_node_num_==unit_.subsidiary_node_num_
                &&this->frame_length_==unit_.frame_length_
                &&this->frame_content_==unit_.frame_content_
                &&this->subsidiary_node_address_list_.size()==unit_.subsidiary_node_address_list_.size())
        {
            for(int i=0;i<this->subsidiary_node_address_list_.size();i++)
            {
                if(!(this->subsidiary_node_address_list_.at(i)==unit_.subsidiary_node_address_list_.at(i)))
                    return false;
            }
            return true;
        }
        else
            return false;
    }
};
/**
 * @brief The ConcurrentReadMeterUnitUp 并发抄表数据单元上行
 */
struct ConcurrentReadMeterUnitUp
{
    //char read_state_;//!<抄读结果状态
    char protocol_type_;//!<规约类型
    ushort frame_length_;//!<报文长度
    QByteArray frame_content_;//!<报文内容
    /**
    * @brief operator ==
    * @param unit_
    * @return
    */
    bool operator==(const ConcurrentReadMeterUnitUp &unit_)const
    {
       if(this->protocol_type_==unit_.protocol_type_
               //&&this->read_state_==unit_.read_state_
               &&this->frame_length_==unit_.frame_length_
               &&this->frame_content_==unit_.frame_content_)
           return true;

       else
           return false;
    }
};

#ifdef UNIT_TEST
class AfnF1F1 : public Frame3762Base
#else
/**
 * @brief The AfnF1F1 集中器主动并发
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT AfnF1F1 : public Frame3762Base
#endif
{
public:
    /**
     * @brief AfnF1F1
     */
    AfnF1F1();
    /**
     * @brief AfnF1F1
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    AfnF1F1(CtrlField ctrl_field,InfoField info_field,AddressField address_field);

    ConcurrentReadMeterUnitDown unit_down_;//!<并发抄表数据单元

    ConcurrentReadMeterUnitUp unit_up_;//!<并发抄表数据单元
public:
    /**
     * @brief 将16进制格式数据，解析为数据单元
     * @param data
     */
    void DecodeFrameDataField(QByteArray data) override;
private:
    /**
     * @brief 将数据单元编码为16进制格式数据
     * @return 反回编码好的字节串
     */
    QByteArray EncodeFrameDataField() override;
};
}
#endif // AFNF1F1_H
