#ifndef AFN14F2_H
#define AFN14F2_H

#include <QObject>

#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"
namespace qgdw_3762_protocol {
/**
 * @brief The StartTime 开始时间结构体
 */
struct CurrentTime
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
    bool operator==(const CurrentTime &unit_) const
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
class Afn14F2 : public Frame3762Base
#else
/**
 * @brief The AFN14F2 路由请求集中器时钟
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT Afn14F2 : public Frame3762Base
#endif
{
public:
    /**
     * @brief Afn14F2
     */
    Afn14F2();
    /**
     * @brief Afn14F2
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    Afn14F2(CtrlField ctrl_field,InfoField info_field,AddressField address_field);

    CurrentTime current_time_;//!<当前时间
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
#endif // AFN14F2_H
