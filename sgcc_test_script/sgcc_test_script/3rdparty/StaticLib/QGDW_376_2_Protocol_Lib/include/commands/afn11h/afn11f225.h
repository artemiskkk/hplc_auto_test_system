#ifndef AFN11F225_H
#define AFN11F225_H

#include <QObject>

#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"
namespace qgdw_3762_protocol {
/**
 * @brief The StartTime 开始时间结构体
 */
struct StartTime11F225
{
    char second_;//!<秒
    char minute_;//!<分
    char hour_;//!<时
    char day_;//!<日
    char month;//!<月
    char year_;//!<年
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const StartTime11F225 &unit_) const
    {
        if(this->second_ == unit_.second_
                && this->minute_ == unit_.minute_
                && this->hour_ == unit_.hour_
                && this->day_ == unit_.day_
                && this->month == unit_.month
                && this->year_ == unit_.year_
                )
            return true;
        else
            return false;
    }
};

#ifdef UNIT_TEST
class Afn11F225 : public Frame3762Base
#else
/**
 * @brief The AFN11F225 启动新增电表上报
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT Afn11F225 : public Frame3762Base
#endif
{
public:
    /**
     * @brief Afn11F225
     */
    Afn11F225();
    /**
     * @brief Afn11F225
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    Afn11F225(CtrlField ctrl_field,InfoField info_field,AddressField address_field);

    StartTime11F225 start_time_;//!<开始时间
    ushort last_time_;//!<持续时间
    uchar retransmit_times_;//!<从节点重发次数
    uchar wait_time_slice_num_;//!<随机等待时间片个数
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
#endif // AFN11F225_H
