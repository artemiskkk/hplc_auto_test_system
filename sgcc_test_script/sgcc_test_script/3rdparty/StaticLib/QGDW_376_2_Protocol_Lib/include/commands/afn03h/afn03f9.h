#ifndef AFN03F9_H
#define AFN03F9_H

#include <QObject>

#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"

namespace qgdw_3762_protocol {
/**
 * @brief The DelayRelatedDataUnit_Down struct
 */
struct DelayRelatedDataUnit_Down
{
    char communtication_protocol_; //!< 通信协议类型
    uchar frame_length_; //!< 报文长度L
    QByteArray frame_content_; //!< 报文内容
    ushort delay_related_flag_; //!< 通信延时相关性标志
    /**
     * @brief operator ==  重载运算符“==”
     * @param delay_related_down
     * @return 返回比较结果
     */
    bool operator==(const DelayRelatedDataUnit_Down &delay_related_down) const
    {
        if(this->communtication_protocol_ == delay_related_down.communtication_protocol_
                && this->frame_length_ == delay_related_down.frame_length_
                && this->frame_content_ == delay_related_down.frame_content_
                && this->delay_related_flag_ == delay_related_down.delay_related_flag_
                )
            return true;
        else
            return false;
    }
};
/**
 * @brief The DelayRelatedDataUnit_Up struct
 */
struct DelayRelatedDataUnit_Up
{
    ushort broadcast_communication_time_; //!< 广播通信延迟时间
    char communtication_protocol_; //!< 通信协议类型
    uchar frame_length_; //!< 报文长度L
    QByteArray frame_content_; //!< 报文内容
    /**
     * @brief operator ==
     * @param delay_related_up
     * @return
     */
    bool operator==(const DelayRelatedDataUnit_Up &delay_related_up) const
    {
        if(this->communtication_protocol_ == delay_related_up.communtication_protocol_
                && this->frame_length_ == delay_related_up.frame_length_
                && this->frame_content_ == delay_related_up.frame_content_
                && this->broadcast_communication_time_ == delay_related_up.broadcast_communication_time_
                )
            return true;
        else
            return false;
    }
};

#ifdef UNIT_TEST
class Afn03F9 : public Frame3762Base
#else
/**
 * @brief The Afn03F9
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT Afn03F9 : public Frame3762Base
#endif
{
public:
    /**
     * @brief Afn03F9
     */
    Afn03F9();
    /**
     * @brief Afn03F9
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    Afn03F9(CtrlField ctrl_field,InfoField info_field,AddressField address_field);

    DelayRelatedDataUnit_Down data_unit_down_; //!< 下行数据单元

    DelayRelatedDataUnit_Up data_unit_up_; //!< 上行数据单元
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
#endif // AFN03F9_H
