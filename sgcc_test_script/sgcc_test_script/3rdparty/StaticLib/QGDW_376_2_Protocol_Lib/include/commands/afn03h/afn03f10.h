#ifndef AFN03F10_H
#define AFN03F10_H

#include <QObject>
#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"

namespace qgdw_3762_protocol {
/**
 * @brief The LocalCommunicationMode 本地通信模式结构体
 */
struct LocalCommunicationMode
{
    uchar communication_mode_:4;    //!< 通信方式
    uchar router_manage_mode_:1;    //!< 路由管理方式
    uchar slave_node_info_mode_:1;    //!< 从节点信息模式
    uchar period_read_mode_:2;   //!< 周期抄表模式

    uchar transmission_delay_parameter_support_:3; //!< 传输延时参数支持
    uchar fail_node_change_mode_:2; //!< 失败从节点切换发起方式
    uchar broadcast_command_confirm_mode_:1; //!< 广播命令确认方式
    uchar broadcast_command_channel_execution_mode_:2; //!< 广播命令信道执行方式

    uchar channel_num_:5;    //!< 信道数量
    uchar power_down_info_:3; //!< 低压电网掉电信息

//    uchar reverse_:4;    //!< 保留
//    uchar rate_num_:4;   //!< 速率数量

    uchar rate_num_:4;   //!< 速率数量
    uchar minute_collection_:1;    //!< 分钟采集
    uchar reverse_:3;    //!< 保留

    uchar reverse_2_;    //!< 保留2
    uchar reverse_1_;    //!< 保留1
    /**
     * @brief operator ==
     * @param local_mode_
     * @return
     */
    bool operator==(const LocalCommunicationMode &local_mode_) const
    {
        if(this->communication_mode_ == local_mode_.communication_mode_
                && this->router_manage_mode_ == local_mode_.router_manage_mode_
                && this->slave_node_info_mode_ == local_mode_.slave_node_info_mode_
                && this->period_read_mode_ == local_mode_.period_read_mode_
                && this->transmission_delay_parameter_support_ == local_mode_.transmission_delay_parameter_support_
                && this->fail_node_change_mode_ == local_mode_.fail_node_change_mode_
                && this->broadcast_command_confirm_mode_ == local_mode_.broadcast_command_confirm_mode_
                && this->broadcast_command_channel_execution_mode_ == local_mode_.broadcast_command_channel_execution_mode_
                && this->channel_num_ == local_mode_.channel_num_
                && this->power_down_info_ == local_mode_.power_down_info_
                && this->reverse_ == local_mode_.reverse_
                && this->rate_num_ == local_mode_.rate_num_
                && this->reverse_2_ == local_mode_.reverse_2_
                && this->reverse_1_ == local_mode_.reverse_1_
                )
            return true;
        else
            return false;
    }
};
/**
 * @brief The Date 日期结构体
 */
struct Date
{
    char year_;    //!< 年
    char month_;   //!< 月
    char day_;    //!< 日
    /**
     * @brief operator ==
     * @param date_
     * @return
     */
    bool operator==(const Date &date_) const
    {
        if(this->year_ == date_.year_
                && this->month_ == date_.month_
                && this->day_ == date_.day_
                )
            return true;
        else
            return false;
    }
};
/**
 * @brief The VendorCodeAndVersion 通信模块厂商代码及版本信息结构体
 */
struct VendorCodeAndVersion
{
    QString vendor_code_;    //!< 厂商代码,例“TC"
    QString chip_code_;      //!< 芯片代码,例“R5"
    QString version_time_;   //!< 版本时间,年月日,例“200516"
    QString version_;        //!< 版本,例“0388"
    /**
     * @brief operator ==
     * @param code_version_
     * @return
     */
    bool operator==(const VendorCodeAndVersion &code_version_) const
    {
        if(this->vendor_code_ == code_version_.vendor_code_
                && this->chip_code_ == code_version_.chip_code_
                && this->version_time_ == code_version_.version_time_
                && this->version_ == code_version_.version_
                )
            return true;
        else
            return false;
    }
};
/**
 * @brief The ComRateUnit 通讯速率单元结构体
 */
struct ComRateUnit03F10
{
    ushort com_rate_:15; //!< 通信速率
    ushort rate_unit_identift_:1; //!< 速率单位标识
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const ComRateUnit03F10 &unit_) const
    {
        if(this->com_rate_ == unit_.com_rate_
                && this->rate_unit_identift_ == unit_.rate_unit_identift_
                )
            return true;
        else
            return false;
    }
    /**
     * @brief operator !=
     * @param unit_
     * @return
     */
    bool operator!=(const ComRateUnit03F10 &unit_) const
    {
        if(this->com_rate_ != unit_.com_rate_
                || this->rate_unit_identift_ != unit_.rate_unit_identift_
                )
            return true;
        else
            return false;
    }
};
/**
 * @brief The OperateModeInfoUnit 运行模式信息单元结构体
 */
struct OperateModeInfoUnit
{
    LocalCommunicationMode local_communication_mode_;//!< 本地通信模式
    uchar node_monitor_max_timeout_;//!< 从节点监控最大超时时间
    ushort broadcast_command_max_timeout_;//!< 广播命令最大超时时间
    ushort max_support_frame_length_;//!< 最大支持的报文长度
    ushort max_transmit_data_length_;//!< 文件传输支持的最大单个数据包长度
    uchar upgrade_wait_time_;//!< 升级操作等待时间
    Address master_node_address_;//!< 主节点地址
    ushort max_support_node_num_;//!< 支持的最大从节点数量
    ushort current_node_num_;//!< 当前从节点数量
    Date protocol_publish_date_;//!< 通信模块使用的协议发布日期
    Date protocol_record_date_;//!< 通信模块使用的协议最后备案日期
    VendorCodeAndVersion vendor_code_and_version_;//!< 通信模块厂商代码及版本信息
    QList<ComRateUnit03F10> com_rate_unit_list_; //!<通讯速率单元QList
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const OperateModeInfoUnit &unit_) const
    {
        if(this->local_communication_mode_ == unit_.local_communication_mode_
                && this->node_monitor_max_timeout_ == unit_.node_monitor_max_timeout_
                && this->broadcast_command_max_timeout_ == unit_.broadcast_command_max_timeout_
                && this->max_support_frame_length_ == unit_.max_support_frame_length_
                && this->max_transmit_data_length_ == unit_.max_transmit_data_length_
                && this->upgrade_wait_time_ == unit_.upgrade_wait_time_
                && this->master_node_address_ == unit_.master_node_address_
                && this->max_support_node_num_ == unit_.max_support_node_num_
                && this->current_node_num_ == unit_.current_node_num_
                && this->protocol_publish_date_ == unit_.protocol_publish_date_
                && this->protocol_record_date_ == unit_.protocol_record_date_
                && this->vendor_code_and_version_ == unit_.vendor_code_and_version_
                && this->com_rate_unit_list_.size() == unit_.com_rate_unit_list_.size()
                )
        {
            bool result_=true;
            for(int i=0;i<this->com_rate_unit_list_.size();i++)
            {
                if(this->com_rate_unit_list_.at(i)!=unit_.com_rate_unit_list_.at(i))
                    result_=false;
            }
            return result_;
        }

        else
            return false;
    }
};

#ifdef UNIT_TEST
class Afn03F10 : public Frame3762Base
#else
/**
 * @brief The Afn03F10
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT Afn03F10 : public Frame3762Base
#endif
{
public:
    /**
     * @brief Afn03F10
     */
    Afn03F10();
    /**
     * @brief Afn03F10
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    Afn03F10(CtrlField ctrl_field,InfoField info_field,AddressField address_field);

    OperateModeInfoUnit operate_mode_info_unit_; //!< 运行模式信息单元
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
#endif // AFN03F10_H
