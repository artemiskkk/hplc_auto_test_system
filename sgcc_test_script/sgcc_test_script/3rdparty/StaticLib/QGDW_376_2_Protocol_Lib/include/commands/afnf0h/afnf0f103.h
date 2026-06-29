#ifndef AFNF0F103_H
#define AFNF0F103_H

#include <QObject>

#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"
namespace qgdw_3762_protocol {
/**
 * @brief The NodeInfoF0F103 节点信息结构体
 */
struct NodeInfoF0F103
{
    uchar node_level_:4;//!<节点层级
    uchar node_role_:4;//!<节点角色
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const NodeInfoF0F103 &unit_)const
    {
        if(this->node_level_==unit_.node_level_
                &&this->node_role_==unit_.node_role_)
            return true;
        else
            return false;
    }
};
/**
 * @brief The NetworkTypelogyInfoF0F103 网络拓扑结构体
 */
struct NetworkTypelogyInfoF0F103
{
    ushort node_tei_;//!<节点标识
    ushort proxy_node_tei_;//!<代理节点标识
    NodeInfoF0F103 node_info_;//!<节点信息
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const NetworkTypelogyInfoF0F103 &unit_)const
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
 * @brief The NodeHangDownInfoF0F103 从节点下接节点信息结构体
 */
struct NodeHangDownInfoF0F103
{
    Address node_address_;//!<从节点地址
    char protocol_type_;//!<规约类型
    char device_type_;//!<设备类型
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const NodeHangDownInfoF0F103 &unit_)const
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
 * @brief The NodeInfoStructureF0F103 节点信息结构结构体
 */
struct NodeInfoStructureF0F103
{
    Address node_address_;//!<从节点地址6
    NetworkTypelogyInfoF0F103 network_typelogy_info_;//!<网络拓扑信息5
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
    QList<NodeHangDownInfoF0F103> hang_down_list_;//!<下接节点信息Qlist
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const NodeInfoStructureF0F103 &unit_)const
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
struct NetworkNodeInfoUnitF0F103
{
    ushort this_node_num_;//!<本次上报节点数量
    QList<NodeInfoStructureF0F103> node_info_structure_list_;//!<节点信息结构QList
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const NetworkNodeInfoUnitF0F103 &unit_)const
    {
        if(this->this_node_num_==unit_.this_node_num_
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
class AfnF0F103 : public Frame3762Base
#else
/**
 * @brief The AFNF0F103 查询网络节点信息
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT AfnF0F103 : public Frame3762Base
#endif
{
public:
    /**
     * @brief AfnF0F103
     */
    AfnF0F103();
    /**
     * @brief AfnF0F103
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    AfnF0F103(CtrlField ctrl_field,InfoField info_field,AddressField address_field);

    uchar node_num_;//!<从节点数量
    QList<Address> node_address_list_;//!<节点地址QList

    NetworkNodeInfoUnitF0F103 network_node_info_unit_;//!<网络拓扑信息数据单元
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

#endif // AFNF0F103_H
