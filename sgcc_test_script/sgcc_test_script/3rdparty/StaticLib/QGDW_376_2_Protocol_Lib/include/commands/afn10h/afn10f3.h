#ifndef AFN10F3_H
#define AFN10F3_H

#include <QObject>

#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"
namespace qgdw_3762_protocol {
/**
 * @brief The NodeInfo10F3 节点信息结构体
 */
struct NodeInfo10F3
{
    uchar relay_level_:4;//!<中继级别
    uchar signal_quality_:4;//!<侦听信号品质
    uchar phase_:3;//!<相位
    uchar protocol_type_:3;//!<通信协议类型
    uchar reverse_:2;//!<备用
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const NodeInfo10F3 &unit_)const
    {
        if(this->signal_quality_==unit_.signal_quality_
                &&this->relay_level_==unit_.relay_level_
                 &&this->reverse_==unit_.reverse_
                 &&this->protocol_type_==unit_.protocol_type_
                &&this->phase_==unit_.phase_)
            return true;
        else
            return false;
    }
};
/**
 * @brief The NodeInfoGroup10F3 从节点信息组结构体
 */
struct NodeInfoGroup10F3
{
    Address node_address_;//!<从节点地址
    NodeInfo10F3 node_info_;//!<从节点信息
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const NodeInfoGroup10F3 &unit_)const
    {
        if(this->node_address_==unit_.node_address_
                &&this->node_info_==unit_.node_info_)
            return true;
        else
            return false;
    }
};
/**
 * @brief The NodeInfoDataUnit 指定从节点的上一级中继路由信息单元结构体
 */
struct LastRelayDataUnit
{
    uchar router_node_total_num_;//!<指定从节点的上一级中继路由信息
    QList<NodeInfoGroup10F3> node_info_group_list_;//!<从节点信息组QList
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const LastRelayDataUnit &unit_)const
    {
        if(this->router_node_total_num_==unit_.router_node_total_num_
                &&this->node_info_group_list_.size()==unit_.node_info_group_list_.size())
        {
            for(int i=0;i<this->node_info_group_list_.size();i++)
            {
                if(!(this->node_info_group_list_.at(i)==unit_.node_info_group_list_.at(i)))
                    return false;
            }
            return true;
        }
        else
            return false;
    }
};

#ifdef UNIT_TEST
class Afn10F3 : public Frame3762Base
#else
/**
 * @brief The AFN10F3 指定从节点的上一级中继路由信息
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT Afn10F3 : public Frame3762Base
#endif
{
public:
    /**
     * @brief Afn10F3
     */
    Afn10F3();
    /**
     * @brief Afn10F3
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    Afn10F3(CtrlField ctrl_field,InfoField info_field,AddressField address_field);
    Address node_address_;//!<从节点地址


    LastRelayDataUnit last_relay_data_unit_;//!<指定从节点的上一级中继路由信息
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
#endif // AFN10F3_H
