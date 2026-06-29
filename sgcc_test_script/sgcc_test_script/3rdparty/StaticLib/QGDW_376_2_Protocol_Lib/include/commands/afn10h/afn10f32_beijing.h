#ifndef AFN10F32_BEIJING_H
#define AFN10F32_BEIJING_H

#include <QObject>

#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"
namespace qgdw_3762_protocol {

/**
 * @brief The NodeCarrierChannelInfo
 */
struct NodeCarrierChannelInfo
{
    Address node_address_;//!<从节点地址
    uchar node_channel_quality_up_;//!<上行信道品质
    uchar node_channel_quality_down_;//!<下行信道品质
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const NodeCarrierChannelInfo &unit_)const
    {
        if(this->node_address_==unit_.node_address_
                &&this->node_channel_quality_up_==unit_.node_channel_quality_up_
                &&this->node_channel_quality_down_==unit_.node_channel_quality_down_)
        {
            return true;
        }
        else
            return false;
    }
};
/**
 * @brief The CarrierChannelUnit10F32_Beijing
 */
struct CarrierChannelUnit10F32_Beijing
{
    ushort node_total_num_;//!<从节点总数量
    ushort node_start_no_;//!<从节点起始序号
    uchar this_node_num_;//!<本次从节点数量
    QList<NodeCarrierChannelInfo> node_info_list_;//!<从节点信息QList
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const CarrierChannelUnit10F32_Beijing &unit_)const
    {
        if(this->node_total_num_==unit_.node_total_num_
                &&this->node_start_no_==unit_.node_start_no_
                &&this->this_node_num_==unit_.this_node_num_
                &&this->node_info_list_.size()==unit_.node_info_list_.size())
        {
            for(int i=0;i<this->node_info_list_.size();i++)
            {
                if(!(this->node_info_list_.at(i)==unit_.node_info_list_.at(i)))
                    return false;
            }
            return true;
        }
        else
            return false;
    }
};

#ifdef UNIT_TEST
class Afn10F32_Beijing : public Frame3762Base
#else
/**
 * @brief The Afn10F32_Beijing
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT Afn10F32_Beijing : public Frame3762Base
#endif
{
public:
    /**
     * @brief Afn10F32_Beijing
     */
    Afn10F32_Beijing();
    /**
     * @brief Afn10F32_Beijing
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    Afn10F32_Beijing(CtrlField ctrl_field,InfoField info_field,AddressField address_field);
    ushort node_start_no_;//!<从节点起始序号
    uchar node_num_;//!<从节点数量

    CarrierChannelUnit10F32_Beijing node_phase_info_unit_;//!<节点相线信息单元
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
#endif // AFN10F32_BEIJING_H
