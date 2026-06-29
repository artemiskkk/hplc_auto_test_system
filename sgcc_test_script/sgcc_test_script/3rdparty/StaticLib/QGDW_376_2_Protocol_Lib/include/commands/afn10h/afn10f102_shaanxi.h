#ifndef AFN10F102_SHAANXI_H
#define AFN10F102_SHAANXI_H

#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"
namespace qgdw_3762_protocol {
/**
 * @brief The Afn10F102_Shaanxi 节点信息结构体
 */
struct NodePhaseInfo10F102_Shaanxi
{
    uchar meter_source_:1;//!<表档案
    uchar phase_sequence_:3;//!<相序
    uchar preserve_:4;//!<保留
    uchar phase_3_:2;//!<接线柱3相位
    uchar phase_2_:2;//!<接线柱2相位
    uchar phase_1_:2;//!<接线柱1相位
    uchar meter_type_:2;//!<表计类型
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const NodePhaseInfo10F102_Shaanxi &unit_)const
    {
        if(this->meter_source_==unit_.meter_source_
                &&this->phase_sequence_==unit_.phase_sequence_
                &&this->preserve_==unit_.preserve_
                &&this->phase_3_==unit_.phase_3_
                &&this->phase_2_==unit_.phase_2_
                &&this->phase_1_==unit_.phase_1_
                &&this->meter_type_==unit_.meter_type_)
            return true;
        else
            return false;
    }
};
struct NodeInfo10F102_Shaanxi
{
    Address node_address_;//!<节点地址
    NodePhaseInfo10F102_Shaanxi node_phase_info_;//!<节点相位信息
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const NodeInfo10F102_Shaanxi &unit_)const
    {
        if(this->node_address_==unit_.node_address_
                &&this->node_phase_info_==unit_.node_phase_info_)
            return true;
        else
            return false;
    }
};
/**
 * @brief The NodePhaseInfoUnit10F102_Shaanxi 节点相位信息单元结构体
 */
struct NodePhaseInfoUnit10F102_Shaanxi
{
    ushort node_total_num_;//!<从节点总数量
    uchar this_node_num_;//!<本次从节点数量
    QList<NodeInfo10F102_Shaanxi> node_info_list_;//!<节点信息结构QList
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const NodePhaseInfoUnit10F102_Shaanxi &unit_)const
    {
        if(this->node_total_num_==unit_.node_total_num_
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
class Afn10F102_Shaanxi : public Frame3762Base
#else
/**
 * @brief The Afn10F102_Shaanxi 查询网络节点信息
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT Afn10F102_Shaanxi : public Frame3762Base
#endif
{
public:
    /**
     * @brief Afn10F102_Shaanxi
     */
    Afn10F102_Shaanxi();
    /**
     * @brief Afn10F102_Shaanxi
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    Afn10F102_Shaanxi(CtrlField ctrl_field,InfoField info_field,AddressField address_field);
    ushort node_start_no_;//!<从节点起始序号
    uchar node_num_;//!<从节点数量


    NodePhaseInfoUnit10F102_Shaanxi node_info_unit_;//!<节点信息数据单元
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

#endif // AFN10F102_SHAANXI_H
