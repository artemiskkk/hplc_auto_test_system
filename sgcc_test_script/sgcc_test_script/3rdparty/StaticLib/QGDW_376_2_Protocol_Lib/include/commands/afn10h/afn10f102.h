#ifndef AFN10F102_H
#define AFN10F102_H

#include <QObject>

#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"
namespace qgdw_3762_protocol {
/**
 * @brief The NodeInfo10F102 节点信息结构体
 */
struct NodeInfo10F102
{
    uchar node_level_:4;//!<节点层级
    uchar node_role_:4;//!<节点角色
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const NodeInfo10F102 &unit_)const
    {
        if(this->node_level_==unit_.node_level_
                &&this->node_level_==unit_.node_level_)
            return true;
        else
            return false;
    }
};
/**
 * @brief The NetworkTypelogyInfo10F102 网络拓扑结构体
 */
struct NetworkTypelogyInfo10F102
{
    Address node_address_;//!<节点地址
    ushort node_tei_;//!<节点标识
    ushort proxy_node_tei_;//!<代理节点标识
    NodeInfo10F102 node_info_;//!<节点信息
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const NetworkTypelogyInfo10F102 &unit_)const
    {
        if(this->node_address_==unit_.node_address_
                &&this->node_tei_==unit_.node_tei_
                &&this->proxy_node_tei_==unit_.proxy_node_tei_
                &&this->node_info_==unit_.node_info_
                )
            return true;
        else
            return false;
    }
};
/**
 * @brief The NodeInfoStructure10F102 节点信息结构结构体
 */
struct NodeInfoStructure10F102
{
    Address node_address_;//!<从节点地址
    NetworkTypelogyInfo10F102 network_typelogy_info_;//!<网络拓扑信息
    char device_property;//!<设备属性
    QString vendor_code_;//!< 厂商代码
    uchar communication_success_rate_up_;//!< 厂商代码
    uchar communication_success_rate_down_;//!< 厂商代码
    char software_version_[3];//!<从节点软件版本
    char reverse_[5];//!<保留
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const NodeInfoStructure10F102 &unit_)const
    {
        if(this->node_address_==unit_.node_address_
                &&this->network_typelogy_info_==unit_.network_typelogy_info_
                &&this->device_property==unit_.device_property
                &&this->vendor_code_==unit_.vendor_code_
                &&this->communication_success_rate_up_==unit_.communication_success_rate_up_
                &&this->communication_success_rate_down_==unit_.communication_success_rate_down_
                &&memcmp(this->software_version_,unit_.software_version_,3)==0
                &&memcmp(this->reverse_,unit_.reverse_,5)==0
                )
            return true;
        else
            return false;
    }
};
/**
 * @brief The NetworkNodeInfoUnit10F102 网络节点信息单元结构体
 */
struct NetworkNodeInfoUnit10F102
{
    ushort node_total_num_;//!<从节点总数量
    ushort node_start_no_;//!<从节点起始序号
    uchar this_node_num_;//!<本次从节点数量
    QList<NodeInfoStructure10F102> node_info_structure_list_;//!<节点信息结构QList
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const NetworkNodeInfoUnit10F102 &unit_)const
    {
        if(this->node_total_num_==unit_.node_total_num_
                &&this->node_start_no_==unit_.node_start_no_
                &&this->this_node_num_==unit_.this_node_num_
                &&this->node_info_structure_list_.size()==unit_.node_info_structure_list_.size())
        {
            for(int i=0;i<this->node_info_structure_list_.size();i++)
            {
                if(!(this->node_info_structure_list_.at(i)==unit_.node_info_structure_list_.at(i)))
                    return false;
            }
            return true;
        }
        else
            return false;
    }
};

#ifdef UNIT_TEST
class Afn10F102 : public Frame3762Base
#else
/**
 * @brief The AFN10F102 查询网络节点信息
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT Afn10F102 : public Frame3762Base
#endif
{
public:
    /**
     * @brief Afn10F102
     */
    Afn10F102();
    /**
     * @brief Afn10F102
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    Afn10F102(CtrlField ctrl_field,InfoField info_field,AddressField address_field);
    ushort node_start_no_;//!<从节点起始序号
    uchar node_num_;//!<从节点数量


    NetworkNodeInfoUnit10F102 network_node_info_unit_;//!<网络拓扑信息数据单元
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
#endif // AFN10F102_H
