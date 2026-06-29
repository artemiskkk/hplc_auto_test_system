#ifndef AFN04F3_H
#define AFN04F3_H

#include <QObject>

#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"
namespace qgdw_3762_protocol {
/**
 * @brief The CommunicationTestUnit 本地通信模块报文通信测试数据单元
 */
struct CommunicationTestUnit
{
    char test_rate_no_; //!<测试通信速率序号
    Address dst_address_;//!<目标地址
    char protocol_type_;//!<通信协议类型
    uchar frame_length_;//!<报文长度L
    QByteArray frame_content;//!<报文内容
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const CommunicationTestUnit &unit_)const
    {
        if(this->test_rate_no_==unit_.test_rate_no_
                &&this->dst_address_==unit_.dst_address_
                &&this->protocol_type_==unit_.protocol_type_
                &&this->frame_length_==unit_.frame_length_
                &&this->frame_content==unit_.frame_content)
            return true;
        else
            return false;
    }
};
#ifdef UNIT_TEST
class Afn04F3 : public Frame3762Base
#else
/**
 * @brief The AFN04F3 本地通信模块报文通信测试
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT Afn04F3 : public Frame3762Base
#endif
{
public:
    /**
     * @brief Afn04F3
     */
    Afn04F3();
    /**
     * @brief Afn04F3
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    Afn04F3(CtrlField ctrl_field,InfoField info_field,AddressField address_field);

    CommunicationTestUnit communication_test_unit_;//!<本地通信模块报文通信测试数据单元
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
#endif // AFN04F3_H
