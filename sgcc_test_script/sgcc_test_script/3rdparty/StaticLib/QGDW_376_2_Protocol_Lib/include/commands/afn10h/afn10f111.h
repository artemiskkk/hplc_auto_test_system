#ifndef AFN10F111_H
#define AFN10F111_H

#include <QObject>

#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"
namespace qgdw_3762_protocol {
/**
 * @brief The NeigborNetworkTei 邻居节点网络标识号结构体
 */
struct NeighborNetworkId
{
    char network_id_[3];//!<邻居节点网络标识号
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const NeighborNetworkId &unit_)const
    {
        if(memcmp(this->network_id_,unit_.network_id_,3)==0)
            return true;
        else
            return false;
    }
};
/**
 * @brief The MultiNetworkInfoUnit 多网络信息数据单元结构体
 */
struct MultiNetworkInfoUnit
{
    uchar total_num_;//!<多网络节点总数量
    NeighborNetworkId this_network_id_;//!<本节点网络标识号
    Address this_node_address_;//!<本节点主节点地址
    QList<NeighborNetworkId> neighbor_network_id_List_;//!<邻居节点网络标识号QList
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const MultiNetworkInfoUnit &unit_)const
    {
        if(this->total_num_==unit_.total_num_
                &&this->this_network_id_==unit_.this_network_id_
                &&this->this_node_address_==unit_.this_node_address_
                &&this->neighbor_network_id_List_.size()==unit_.neighbor_network_id_List_.size())
        {
            for(int i=0;i<this->neighbor_network_id_List_.size();i++)
            {
                if(!(this->neighbor_network_id_List_.at(i)==unit_.neighbor_network_id_List_.at(i)))
                    return false;
            }
            return true;
        }
        else
            return false;
    }
};

#ifdef UNIT_TEST
class Afn10F111 : public Frame3762Base
#else
/**
 * @brief The AFN10F111 查询多网络信息
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT Afn10F111 : public Frame3762Base
#endif
{
public:
    /**
     * @brief Afn10F111
     */
    Afn10F111();
    /**
     * @brief Afn10F111
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    Afn10F111(CtrlField ctrl_field,InfoField info_field,AddressField address_field);

    MultiNetworkInfoUnit multi_network_info_unit_;//!<多网络信息数据
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
#endif // AFN10F111_H
