#ifndef AFNF0F100_H
#define AFNF0F100_H

#include <QObject>

#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"
namespace qgdw_3762_protocol {
/**
 * @brief The LevelInfo 层级信息结构体
 */
struct LevelInfo
{
    char current_level_;//!<网络节点总数
    ushort node_num_;//!<网络节点总数
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const LevelInfo &unit_)const
    {
        if(this->current_level_==unit_.current_level_
                &&this->node_num_==unit_.node_num_)
        {
            return true;
        }
        else
            return false;
    }
};
/**
 * @brief The NetworkBasicUnit 网络基本信息单元结构体
 */
struct NetworkBasicUnit
{
    ushort node_total_num_;//!<网络节点总数
    ushort node_online_num_;//!<在线站点个数
    uint begin_build_network_time_;//!<网络启动组网时长
    ushort build_network_time_;//!<组网时长
    uchar beacon_period_;//!<信标周期
    ushort router_period_;//!<路由周期
    ushort topology_level_;//!<拓扑变更次数
    uchar total_level_;//!<总层级
    QList<LevelInfo> level_info_list_;//!<层级信息QList
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const NetworkBasicUnit &unit_)const
    {
        if(this->node_total_num_==unit_.node_total_num_
                &&this->node_online_num_==unit_.node_online_num_
                &&this->begin_build_network_time_==unit_.begin_build_network_time_
                &&this->build_network_time_==unit_.build_network_time_
                &&this->beacon_period_==unit_.beacon_period_
                &&this->router_period_==unit_.router_period_
                &&this->topology_level_==unit_.topology_level_
                &&this->total_level_==unit_.total_level_
                &&this->level_info_list_.size()==unit_.level_info_list_.size())
        {
            for(int i=0;i<this->level_info_list_.size();i++)
            {
                if(!(this->level_info_list_.at(i)==unit_.level_info_list_.at(i)))
                    return false;
            }
            return true;
        }
        else
            return false;
    }
};

#ifdef UNIT_TEST
class AfnF0F100 : public Frame3762Base
#else
/**
 * @brief The AFNF0F100 查询网络基本信息
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT AfnF0F100 : public Frame3762Base
#endif
{
public:
    /**
     * @brief AfnF0F100
     */
    AfnF0F100();
    /**
     * @brief AfnF0F100
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    AfnF0F100(CtrlField ctrl_field,InfoField info_field,AddressField address_field);

    NetworkBasicUnit network_basic_unit_;//!<网络基本信息单元
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
#endif // AFNF0F100_H
