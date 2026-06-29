#ifndef AFN03F5_H
#define AFN03F5_H

#include <QObject>
#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"

namespace qgdw_3762_protocol {
/**
 * @brief The StateWord 状态字结构体
 */
struct StateWord
{    
    uchar rate_num_:4;   //!< 速率数量
    uchar master_channel_feature_:2;    //!< 主节点信道特征
    uchar period_read_mode_:2;   //!< 周期抄表模式

    uchar channel_num_:4;    //!< 信道数量
    uchar reverse_:4;    //!< 备用
    /**
     * @brief operator ==
     * @param state_word_
     * @return
     */
    bool operator==(const StateWord &state_word_) const
    {
        if(this->period_read_mode_ == state_word_.period_read_mode_
                && this->master_channel_feature_ == state_word_.master_channel_feature_
                && this->rate_num_ == state_word_.rate_num_
                && this->reverse_ == state_word_.reverse_
                && this->channel_num_ == state_word_.channel_num_
                )
            return true;
        else
            return false;
    }
};

/**
 * @brief The ComRateUnit 通讯速率单元结构体
 */
struct ComRateUnit
{    
    ushort com_rate_:15; //!< 通信速率
    ushort rate_unit_identift_:1; //!< 速率单位标识
    /**
     * @brief operator ==
     * @param com_rate_unit
     * @return
     */
    bool operator==(const ComRateUnit &com_rate_unit) const
    {
        if(this->rate_unit_identift_ == com_rate_unit.rate_unit_identift_
                && this->com_rate_ == com_rate_unit.com_rate_
                )
            return true;
        else
            return false;
    }
};


#ifdef UNIT_TEST
class Afn03F5 : public Frame3762Base
#else
/**
 * @brief The Afn03F5
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT Afn03F5 : public Frame3762Base
#endif
{
public:
    /**
     * @brief Afn03F5
     */
    Afn03F5();
    /**
     * @brief Afn03F5
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    Afn03F5(CtrlField ctrl_field,InfoField info_field,AddressField address_field);
    StateWord state_word_;  //!< 状态字
    QList<ComRateUnit> com_rate_unit_list_; //!<通讯速率单元QList
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
#endif // AFN03F5_H
