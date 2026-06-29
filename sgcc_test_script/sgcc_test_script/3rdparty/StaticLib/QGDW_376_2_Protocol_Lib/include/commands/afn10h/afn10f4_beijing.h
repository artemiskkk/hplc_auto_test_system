#ifndef AFN10F4_BEIJING_H
#define AFN10F4_BEIJING_H

#include <QObject>

#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"
namespace qgdw_3762_protocol {
/**
 * @brief The OperateStateWord_Beijing 运行状态字结构体
 */
struct OperateStateWord_Beijing
{
    uchar router_complete_flag_:1;//!<路由完成标志
    uchar report_work_flag_:1;//!<工作标志
    uchar report_event_flag_:1;//!<上报事件标志
    uchar reverse_:1;//!<备用
    uchar error_correction_coding_:4;//!<纠错编码
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const OperateStateWord_Beijing &unit_)const
    {
        if(this->router_complete_flag_==unit_.router_complete_flag_
                &&this->report_work_flag_==unit_.report_work_flag_
                 &&this->reverse_==unit_.reverse_
                 &&this->report_event_flag_==unit_.report_event_flag_
                &&this->error_correction_coding_==unit_.error_correction_coding_)
            return true;
        else
            return false;
    }
};
/**
 * @brief The WorkSwitch_Beijing 工作开关结构体
 */
struct WorkSwitch_Beijing
{
    uchar work_state_:1;//!<工作状态
    uchar register_allow_flag_:1;//!<注册允许状态
    uchar work_state_1:2;//!<工作状态1
    uchar area_difference_flag_:1;//!<台区区分标志
    uchar reverse_:3;//!<备用
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const WorkSwitch_Beijing &unit_)const
    {
        if(this->work_state_==unit_.work_state_
                &&this->register_allow_flag_==unit_.register_allow_flag_
                 &&this->reverse_==unit_.reverse_
                 &&this->work_state_1==unit_.work_state_1
                &&this->area_difference_flag_==unit_.area_difference_flag_
                &&this->reverse_==unit_.reverse_)
            return true;
        else
            return false;
    }
};
/**
 * @brief The RouterOperateStateUnit 路由运行状态数据单元结构体
 */
struct RouterOperateStateUnit_Beijing
{
    OperateStateWord_Beijing operate_state_word_;//!<运行状态字
    ushort node_total_num_;//!<从节点总数量
    ushort have_read_node_num_;//!<已抄从节点数量
    ushort read_by_relay_node_num_;//!<中继抄到从节点数量
    WorkSwitch_Beijing work_switch_;//!<工作开关
    ushort communication_rate_;//!<载波通信速率
    uchar relay_level_phase_1_;//!<第1相中继级别
    uchar relay_level_phase_2_;//!<第2相中继级别
    uchar relay_level_phase_3_;//!<第3相中继级别
    char work_procedure_phase_1_;//!<第1相工作步骤
    char work_procedure_phase_2_;//!<第2相工作步骤
    char work_procedure_phase_3_;//!<第3相工作步骤
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const RouterOperateStateUnit_Beijing &unit_)const
    {
        if(this->operate_state_word_==unit_.operate_state_word_
                &&this->node_total_num_==unit_.node_total_num_
                 &&this->have_read_node_num_==unit_.have_read_node_num_
                 &&this->read_by_relay_node_num_==unit_.read_by_relay_node_num_
                &&this->work_switch_==unit_.work_switch_
                &&this->communication_rate_==unit_.communication_rate_
                 &&this->relay_level_phase_1_==unit_.relay_level_phase_1_
                 &&this->relay_level_phase_2_==unit_.relay_level_phase_2_
                &&this->relay_level_phase_3_==unit_.relay_level_phase_3_
                &&this->work_procedure_phase_1_==unit_.work_procedure_phase_1_
               &&this->work_procedure_phase_2_==unit_.work_procedure_phase_2_
                &&this->work_procedure_phase_3_==unit_.work_procedure_phase_3_)
            return true;
        else
            return false;
    }
};

#ifdef UNIT_TEST
class Afn10F4 : public Frame3762Base
#else
/**
 * @brief The Afn10F4_Beijing ：路由运行状态
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT Afn10F4_Beijing : public Frame3762Base
#endif
{
public:
    /**
     * @brief Afn10F4_Beijing
     */
    Afn10F4_Beijing();
    /**
     * @brief Afn10F4_Beijing
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    Afn10F4_Beijing(CtrlField ctrl_field,InfoField info_field,AddressField address_field);

    RouterOperateStateUnit_Beijing router_operate_state_unit_;//!<路由运行状态数据单元
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
#endif // AFN10F4_BEIJING_H
