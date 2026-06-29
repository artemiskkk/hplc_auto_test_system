#ifndef AFNF0F102_H
#define AFNF0F102_H

#include <QObject>

#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"
namespace qgdw_3762_protocol {
/**
 * @brief The NodeInfoF0F102 节点信息结构体
 */
struct NodeInfoF0F102
{
    uchar node_level_:4;//!<节点层级
    uchar node_role_:4;//!<节点角色
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const NodeInfoF0F102 &unit_)const
    {
        if(this->node_level_==unit_.node_level_
                &&this->node_role_==unit_.node_role_)
            return true;
        else
            return false;
    }
};
/**
 * @brief The NetworkTypelogyInfo 网络拓扑结构体
 */
struct NetworkTypelogyInfoF0F102
{
    ushort node_tei_;//!<节点标识
    ushort proxy_node_tei_;//!<代理节点标识
    NodeInfoF0F102 node_info_;//!<节点信息
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const NetworkTypelogyInfoF0F102 &unit_)const
    {
        if(this->node_tei_==unit_.node_tei_
                &&this->proxy_node_tei_==unit_.proxy_node_tei_
                &&this->node_info_==unit_.node_info_)
            return true;
        else
            return false;
    }
};
/**
 * @brief The NodeHangDownInfo_ 从节点下接节点信息结构体
 */
struct NodeHangDownInfo
{
    Address node_address_;//!<从节点地址
    char protocol_type_;//!<规约类型
    char device_type_;//!<设备类型
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const NodeHangDownInfo &unit_)const
    {
        if(this->node_address_==unit_.node_address_
                &&this->protocol_type_==unit_.protocol_type_
                &&this->device_type_==unit_.device_type_)
            return true;
        else
            return false;
    }
};
/**
 * @brief The NodeInfoStructure 节点信息结构结构体
 */
struct NodeInfoStructure
{
    Address node_address_;//!<从节点地址6
    NetworkTypelogyInfoF0F102 network_typelogy_info_;//!<网络拓扑信息5
    char network_info_;//!<网络信息1
    char device_type_[2];//!<设备类型2
    char phase_;//!<相位信息1
    ushort proxy_change_times_;//!<代理变更次数2
    ushort node_offline_times_;//!<站点离线次数2
    uint node_offline_time_;//!<站点离线时长4
    uint node_offline_max_time_;//!<站点离线最大时长4
    uint communication_success_rate_up_;//!< 厂商代码4
    uint communication_success_rate_down_;//!< 厂商代码4
    char primary_version_[3];//!<主版本3
    char secondary_version_[2];//!<次版本2
    ushort next_info_;//!<下一跳信息2
    char channel_type_[2];//!<信道类型2
    char protocol_type_;//!<规约类型1
    char area_state_;//!<台区状态1
    Address area_address_;//!<台区号地址6
    uchar hang_down_num_;//!<从节点下接从节点数量M1
    QList<NodeHangDownInfo> hang_down_list_;//!<下接节点信息Qlist
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const NodeInfoStructure &unit_)const
    {
        if(this->node_address_==unit_.node_address_
                &&this->network_typelogy_info_==unit_.network_typelogy_info_
                &&this->network_info_==unit_.network_info_
                &&memcmp(this->device_type_,unit_.device_type_,2)==0
                &&this->phase_==unit_.phase_
                &&this->proxy_change_times_==unit_.proxy_change_times_
                &&this->node_offline_times_==unit_.node_offline_times_
                &&this->node_offline_time_==unit_.node_offline_time_
                &&this->node_offline_max_time_==unit_.node_offline_max_time_
                &&this->communication_success_rate_up_==unit_.communication_success_rate_up_
                &&this->communication_success_rate_down_==unit_.communication_success_rate_down_
                &&memcmp(this->primary_version_,unit_.primary_version_,3)==0
                &&memcmp(this->secondary_version_,unit_.secondary_version_,2)==0
                &&this->next_info_==unit_.next_info_
                &&memcmp(this->channel_type_,unit_.channel_type_,2)==0
                &&this->protocol_type_==unit_.protocol_type_
                &&this->area_state_==unit_.area_state_
                &&this->area_address_==unit_.area_address_
                &&this->hang_down_num_==unit_.hang_down_num_
                &&this->hang_down_list_.size()==unit_.hang_down_list_.size())
        {
            for(int i=0;i<this->hang_down_list_.size();i++)
            {
                if(!(this->hang_down_list_.at(i)==unit_.hang_down_list_.at(i)))
                    return false;
            }
            return true;
        }
        else
            return false;
    }
};
/**
 * @brief The NetworkNodeInfoUnit 网络节点信息单元结构体
 */
struct NetworkNodeInfoUnit
{
    ushort node_total_num_;//!<从节点总数量
    ushort this_node_num_;//!<本次上报节点数量
    ushort node_start_no_;//!<从节点起始序号
    QList<NodeInfoStructure> node_info_structure_list_;//!<节点信息结构QList
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const NetworkNodeInfoUnit &unit_)const
    {
        if(this->node_total_num_==unit_.node_total_num_
                &&this->this_node_num_==unit_.this_node_num_
                &&this->node_start_no_==unit_.node_start_no_
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
class AfnF0F102 : public Frame3762Base
#else
/**
 * @brief The AFNF0F102 查询网络节点信息
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT AfnF0F102 : public Frame3762Base
#endif
{
public:
    /**
     * @brief AfnF0F102
     */
    AfnF0F102();
    /**
     * @brief AfnF0F102
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    AfnF0F102(CtrlField ctrl_field,InfoField info_field,AddressField address_field);
    ushort node_start_no_;//!<从节点起始序号
    uchar node_num_;//!<从节点数量

    NetworkNodeInfoUnit network_node_info_unit_;//!<网络拓扑信息数据单元
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
#endif // AFNF0F102_H
