#ifndef AFN06F10_H
#define AFN06F10_H

#include <QObject>

#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"
namespace qgdw_3762_protocol {
/**
 * @brief The NodeStateInfo 从节点状态信息
 */
struct NodeStateInfo
{
    Address node_address_;//!<从节点地址
    char node_state_;//!<从节点状态变化
    uint offline_time_;//!<离线时长
    char offline_reason_;//!<离线原因
    QByteArray chip_id_;//!<芯片ID
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const NodeStateInfo &unit_)const
    {
        if(this->node_address_==unit_.node_address_
                &&this->node_state_==unit_.node_state_
                 &&this->offline_time_==unit_.offline_time_
                 &&this->offline_reason_==unit_.offline_reason_
                 &&this->chip_id_==unit_.chip_id_)
            return true;
        else
            return false;
    }
};
/**
 * @brief The ReportNodeStateUnit 上报从节点状态变化数据单元结构体
 */
struct ReportNodeStateUnit
{
    ushort report_node_total_num_;//!<上报从节点总数
    uchar this_report_node_num_;//!<本次上报从节点的数量
    ushort report_start_no_;//!<本次上报开始序号
    QList<NodeStateInfo> node_state_info_list_;//!<从从节点状态信息QList
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const ReportNodeStateUnit &unit_)const
    {
        if(this->report_node_total_num_==unit_.report_node_total_num_
                &&this->this_report_node_num_==unit_.this_report_node_num_
                 &&this->report_start_no_==unit_.report_start_no_
                 &&this->node_state_info_list_.size()==unit_.node_state_info_list_.size())
        {
            for(int i=0;i<this->node_state_info_list_.size();i++)
            {
                if(!(this->node_state_info_list_.at(i)==unit_.node_state_info_list_.at(i)))
                    return false;
            }
            return true;
        }
        else
            return false;
    }
};

#ifdef UNIT_TEST
class Afn06F10 : public Frame3762Base
#else
/**
 * @brief The AFN06F10 上报从节点状态变化
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT Afn06F10 : public Frame3762Base
#endif
{
public:
    /**
     * @brief Afn06F10
     */
    Afn06F10();
    /**
     * @brief Afn06F10
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    Afn06F10(CtrlField ctrl_field,InfoField info_field,AddressField address_field);

    ReportNodeStateUnit report_node_state_unit_;//!<上报从节点状态变化数据单元
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
#endif // AFN06F10_H
