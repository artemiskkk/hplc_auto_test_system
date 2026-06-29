#ifndef AFNF0F6_H
#define AFNF0F6_H

#include <QObject>

#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"

namespace qgdw_3762_protocol {
/**
 * @brief The UartTestUnitDown UART测试数据单元下行结构体
 */
struct UartTestUnitDown
{
    char uart_index_;//!<UART索引
    char baud_rate_;//!<存储器类型
    uchar max_timeout_;//!<最大超时时间
    char protocol_type_;//!<规约类型
    uchar frame_length_;//!<报文长度
    QByteArray frame_content_;//!<报文内容
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const UartTestUnitDown &unit_)const
    {
        if(this->uart_index_==unit_.uart_index_
                &&this->baud_rate_==unit_.baud_rate_
                &&this->max_timeout_==unit_.max_timeout_
                &&this->protocol_type_==unit_.protocol_type_
                &&this->frame_length_==unit_.frame_length_
                &&this->frame_content_==unit_.frame_content_)
            return true;
        else
            return false;
    }
};
/**
 * @brief The UartTestUnitUp UART测试数据单元上行结构体
 */
struct UartTestUnitUp
{
    char response_state_;//!<应答状态
    char protocol_type_;//!<规约类型
    uchar frame_length_;//!<报文长度
    QByteArray frame_content_;//!<报文内容
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const UartTestUnitUp &unit_)const
    {
        if(this->response_state_==unit_.response_state_
                &&this->protocol_type_==unit_.protocol_type_
                &&this->frame_length_==unit_.frame_length_
                &&this->frame_content_==unit_.frame_content_)
            return true;
        else
            return false;
    }
};

#ifdef UNIT_TEST
class AfnF0F6 : public Frame3762Base
#else
/**
 * @brief The AfnF0F6 UART测试
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT AfnF0F6 : public Frame3762Base
#endif
{
public:
    /**
     * @brief AfnF0F6
     */
    AfnF0F6();
    /**
     * @brief AfnF0F6
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    AfnF0F6(CtrlField ctrl_field,InfoField info_field,AddressField address_field);

    UartTestUnitDown uart_test_unit_down_;//!<UART测试数据单元下行
    UartTestUnitUp uart_test_unit_up_;//!<UART测试数据单元上行
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
#endif // AFNF0F6_H
