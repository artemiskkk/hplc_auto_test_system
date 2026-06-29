#ifndef AFN11F4_H
#define AFN11F4_H

#include <QObject>

#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"
namespace qgdw_3762_protocol {
/**
 * @brief The WorkMode 工作模式结构体
 */
struct WorkMode11F4
{
    uchar work_state_flag_:1;//!<工作状态标志
    uchar register_allow_flag_:1;//!<注册允许状态标志
    uchar event_process_flag_:1;//!<事件处理状态标志
    uchar reverse_:1;//!<备用
    uchar error_correction_coding_:4;//!<纠错编码
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const WorkMode11F4 &unit_) const
    {
        if(this->work_state_flag_ == unit_.work_state_flag_
                && this->register_allow_flag_ == unit_.register_allow_flag_
                && this->event_process_flag_ == unit_.event_process_flag_
                && this->reverse_ == unit_.reverse_
                && this->error_correction_coding_ == unit_.error_correction_coding_
                )
            return true;
        else
            return false;
    }
};
/**
 * @brief The ComRateUnit 通讯速率单元结构体
 */
struct ComRateUnit11F4
{
    ushort com_rate_:15; //!< 通信速率
    ushort rate_unit_identift_:1; //!< 速率单位标识
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const ComRateUnit11F4 &unit_) const
    {
        if(this->com_rate_ == unit_.com_rate_
                && this->rate_unit_identift_ == unit_.rate_unit_identift_
                )
            return true;
        else
            return false;
    }
};

#ifdef UNIT_TEST
class Afn11F4 : public Frame3762Base
#else
/**
 * @brief The AFN11F4 设置路由工作模式
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT Afn11F4 : public Frame3762Base
#endif
{
public:
    /**
     * @brief Afn11F4
     */
    Afn11F4();
    /**
     * @brief Afn11F4
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    Afn11F4(CtrlField ctrl_field,InfoField info_field,AddressField address_field);

    WorkMode11F4 work_mode_;//!<工作状态

    ComRateUnit11F4 com_rate_unit_;//!< 通信速率单元
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
#endif // AFN11F4_H
