#ifndef AFN10F8_H
#define AFN10F8_H

#include <QObject>

#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"
namespace qgdw_3762_protocol {
/**
 * @brief The NodeInfo10F8 节点信息结构体
 */
struct NodeInfo10F8
{
    uchar node_level_:4;//!<节点层级
    uchar node_role_:4;//!<节点角色
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const NodeInfo10F8 &unit_)const
    {
        if(this->node_level_==unit_.node_level_
                &&this->node_role_==unit_.node_role_)
        {
            return true;
        }
        else
            return false;
    }
};
/**
 * @brief The NetworkTypelogyInfo 网络拓扑结构体
 */
struct NetworkTypelogyInfo
{
    Address node_address_;//!<节点地址
    ushort node_tei_;//!<节点标识
    ushort proxy_node_tei_;//!<代理节点标识
    NodeInfo10F8 node_info_;//!<节点信息
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const NetworkTypelogyInfo &unit_)const
    {
        if(this->node_address_==unit_.node_address_
                &&this->node_tei_==unit_.node_tei_
                &&this->proxy_node_tei_==unit_.proxy_node_tei_
                &&this->node_info_==unit_.node_info_)
        {
            return true;
        }
        else
            return false;
    }
};
/**
 * @brief The NetworkTypelogyInfoUnit 网络拓扑信息数据单元结构体
 */
struct NetworkTypelogyInfoUnit
{
    ushort node_total_num_;//!<从节点总数量
    ushort node_start_no_;//!<从节点起始序号
    uchar this_node_num_;//!<本次从节点数量
    QList<NetworkTypelogyInfo> network_typelogy_info_List_;//!<网络拓扑QList
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const NetworkTypelogyInfoUnit &unit_)const
    {
        if(this->node_total_num_==unit_.node_total_num_
                &&this->node_start_no_==unit_.node_start_no_
                &&this->this_node_num_==unit_.this_node_num_
                &&this->network_typelogy_info_List_.size()==unit_.network_typelogy_info_List_.size())
        {
            for(int i=0;i<this->network_typelogy_info_List_.size();i++)
            {
                if(!(this->network_typelogy_info_List_.at(i)==unit_.network_typelogy_info_List_.at(i)))
                    return false;
            }
            return true;
        }
        else
            return false;
    }
};

#ifdef UNIT_TEST
class Afn10F8 : public Frame3762Base
#else
/**
 * @brief The AFN10F8 网络拓扑信息
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT Afn10F8 : public Frame3762Base
#endif
{
public:
    /**
     * @brief Afn10F8
     */
    Afn10F8();
    /**
     * @brief Afn10F8
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    Afn10F8(CtrlField ctrl_field,InfoField info_field,AddressField address_field);
    ushort node_start_no_;//!<从节点起始序号
    uchar node_num_;//!<从节点数量

    NetworkTypelogyInfoUnit network_typelogy_info_unit_;//!<网络拓扑信息数据单元
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
#endif // AFN10F8_H
