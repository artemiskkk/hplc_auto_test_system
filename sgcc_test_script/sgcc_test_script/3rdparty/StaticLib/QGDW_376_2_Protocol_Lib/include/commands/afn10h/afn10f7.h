#ifndef AFN10F7_H
#define AFN10F7_H

#include <QObject>

#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"
namespace qgdw_3762_protocol {
/**
 * @brief The NodeType 节点类型结构体
 */
struct NodeType
{
    uchar node_module_type_:4;//!<从节点模块类型
    uchar minute_collection_:1;//!<分钟采集
    uchar reverse_:2;//!<备用
    uchar node_id_update_flag:1;//!<从节点ID信息更新标志
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const NodeType &unit_)const
    {
        if(this->node_module_type_==unit_.node_module_type_
                &&this->reverse_==unit_.reverse_
                &&this->node_id_update_flag==unit_.node_id_update_flag)
        {
            return true;
        }
        else
            return false;
    }
};
/**
 * @brief The NodeInfo 节点信息结构体
 */
struct NodeIdInfo
{
    Address node_address_;//!<从节点地址
    NodeType node_type_;//!<从节点节点类型
    QString vendor_code_;    //!< 厂商代码
    uchar node_id_length_;//!< 从节点模块ID号长度
    char node_id_format_;//!< 从节点模块ID号格式
    QByteArray node_id_;//!< 从节点模块ID号
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const NodeIdInfo &unit_)const
    {
        if(this->node_address_==unit_.node_address_
                &&this->node_type_==unit_.node_type_
                &&this->vendor_code_==unit_.vendor_code_
                &&this->node_id_length_==unit_.node_id_length_
                &&this->node_id_format_==unit_.node_id_format_
                &&this->node_id_==unit_.node_id_)
        {
            return true;
        }
        else
            return false;
    }
};
/**
 * @brief The NodeIdInfoDataUnit 从节点ID信息数据单元结构体
 */
struct NodeIdInfoDataUnit
{
    ushort node_total_num_;//!<从节点总数量
    uchar this_node_num_;//!<本次应答的从节点数量
    QList<NodeIdInfo> node_id_info_list_;//!<从节点ID信息QList
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const NodeIdInfoDataUnit &unit_)const
    {
        if(this->node_total_num_==unit_.node_total_num_
                &&this->this_node_num_==unit_.this_node_num_
                &&this->node_id_info_list_.size()==unit_.node_id_info_list_.size())
        {
            for(int i=0;i<this->node_id_info_list_.size();i++)
            {
                if(!(this->node_id_info_list_.at(i)==unit_.node_id_info_list_.at(i)))
                    return false;
            }
            return true;
        }
        else
            return false;
    }
};

#ifdef UNIT_TEST
class Afn10F7 : public Frame3762Base
#else
/**
 * @brief The AFN10F7 从节点ID信息
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT Afn10F7 : public Frame3762Base
#endif
{
public:
    /**
     * @brief Afn10F7
     */
    Afn10F7();
    /**
     * @brief Afn10F7
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    Afn10F7(CtrlField ctrl_field,InfoField info_field,AddressField address_field);
    ushort node_start_no_;//!<从节点起始序号
    uchar node_num_;//!<从节点数量

    NodeIdInfoDataUnit node_id_info_unit_;//!<从节点ID信息数据单元
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
#endif // AFN10F7_H
