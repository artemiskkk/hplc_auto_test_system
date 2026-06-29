#ifndef AFN10F104_H
#define AFN10F104_H

#include <QObject>

#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"
namespace qgdw_3762_protocol {
/**
 * @brief The NodeInfo10F104 节点信息结构体
 */
struct NodeInfo10F104
{
    char software_version_[2];
    char ver_day_;
    char ver_month_;
    char ver_year_;
    char vendor_code_[2];
    char chip_code_[2];
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const NodeInfo10F104 &unit_)const
    {
        if(this->ver_day_==unit_.ver_day_
                &&this->ver_month_==unit_.ver_month_
                 &&this->ver_year_==unit_.ver_year_
                 &&memcmp(this->software_version_,unit_.software_version_,2)==0
                &&memcmp(this->vendor_code_,unit_.vendor_code_,2)==0
                &&memcmp(this->chip_code_,unit_.chip_code_,2)==0)
            return true;
        else
            return false;
    }
};
/**
 * @brief The NodeInfoGroup 从节点信息组结构体
 */
struct NodeInfoGroup10F104
{
    Address node_address_;//!<从节点地址
    NodeInfo10F104 node_info_;//!<从节点信息
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const NodeInfoGroup10F104 &unit_)const
    {
        if(this->node_address_==unit_.node_address_
                &&this->node_info_==unit_.node_info_)
            return true;
        else
            return false;
    }
};
/**
 * @brief The NodeInfoDataUnit 从节点信息数据单元结构体
 */
struct NodeInfoDataUnit10F104
{
    ushort node_total_num_;//!<从节点总数量
    uchar this_node_num_;//!<本次应答的从节点数量
    QList<NodeInfoGroup10F104> node_info_group_list_;//!<从节点信息组QList
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const NodeInfoDataUnit10F104 &unit_)const
    {
        if(this->node_total_num_==unit_.node_total_num_
                &&this->this_node_num_==unit_.this_node_num_
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
class Afn10F104 : public Frame3762Base
#else
/**
 * @brief The Afn10F104 查询 HPLC 载波模块节点信息
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT Afn10F104 : public Frame3762Base
#endif
{
public:
    /**
     * @brief Afn10F104
     */
    Afn10F104();
    /**
     * @brief Afn10F104
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    Afn10F104(CtrlField ctrl_field,InfoField info_field,AddressField address_field);
    ushort start_no_;//!<起始序号
    uchar this_query_num_;//!<本次查询数量
    QList<Address> node_address_list_;//!<节点地址QList

    NodeInfoDataUnit10F104 network_node_info_unit_;//!<网络拓扑信息数据单元
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
#endif // AFN10F104_H
