#ifndef AFN10F31_H
#define AFN10F31_H

#include <QObject>

#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"
namespace qgdw_3762_protocol {
/**
 * @brief The NodePhaseInfo10F31 节点相线信息结构体
 */
struct NodePhaseInfo10F31
{
    uchar phase_:3;//!<相位信息
    uchar meter_type_flag_:1;//!<电表类型
    uchar line_abnormal_flag_:1;//!<线路异常
    uchar phase_sequence_:3;//!<三相表相序类型
    uchar reverse_1_;//!<备用
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const NodePhaseInfo10F31 &unit_)const
    {
        if(this->phase_==unit_.phase_
                &&this->meter_type_flag_==unit_.meter_type_flag_
                &&this->line_abnormal_flag_==unit_.line_abnormal_flag_
                &&this->phase_sequence_==unit_.phase_sequence_
                &&this->reverse_1_==unit_.reverse_1_)
        {
            return true;
        }
        else
            return false;
    }
};
/**
 * @brief The NodeInfo10F31 节点信息结构体
 */
struct NodeInfo10F31
{
    Address node_address_;//!<从节点地址
    NodePhaseInfo10F31 node_phase_info_;//!<从节点相线信息
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const NodeInfo10F31 &unit_)const
    {
        if(this->node_address_==unit_.node_address_
                &&this->node_phase_info_==unit_.node_phase_info_)
        {
            return true;
        }
        else
            return false;
    }
};
/**
 * @brief The NodePhaseInfoUnit10F31 节点相线信息单元结构体
 */
struct NodePhaseInfoUnit10F31
{
    ushort node_total_num_;//!<从节点总数量
    ushort node_start_no_;//!<从节点起始序号
    uchar this_node_num_;//!<本次从节点数量
    QList<NodeInfo10F31> node_info_list_;//!<从节点信息QList
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const NodePhaseInfoUnit10F31 &unit_)const
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
class Afn10F31 : public Frame3762Base
#else
/**
 * @brief The AFN10F31 查询相线信息
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT Afn10F31 : public Frame3762Base
#endif
{
public:
    /**
     * @brief Afn10F31
     */
    Afn10F31();
    /**
     * @brief Afn10F31
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    Afn10F31(CtrlField ctrl_field,InfoField info_field,AddressField address_field);
    ushort node_start_no_;//!<从节点起始序号
    uchar node_num_;//!<从节点数量

    NodePhaseInfoUnit10F31 node_phase_info_unit_;//!<节点相线信息单元
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
#endif // AFN10F31_H
