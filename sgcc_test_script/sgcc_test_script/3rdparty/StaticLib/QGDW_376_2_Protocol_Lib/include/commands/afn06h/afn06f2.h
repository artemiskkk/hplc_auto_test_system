#ifndef AFN06F2_H
#define AFN06F2_H

#include <QObject>

#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"
namespace qgdw_3762_protocol {
/**
 * @brief The ReportDataUnit 上报抄读数据数据单元结构体
 */
struct ReportDataUnit
{
    ushort node_no_;//!<节点序号
    char node_protocol_type_;//!<节点协议
    ushort communication_up_time_;//!<当前报文本地通信上行时长
    uchar frame_length_;//!<报文长度L
    QByteArray frame_content_;//!<报文内容
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const ReportDataUnit &unit_)const
    {
        if(this->communication_up_time_==unit_.communication_up_time_
                &&this->node_protocol_type_==unit_.node_protocol_type_
                &&this->node_no_==unit_.node_no_
                &&this->frame_length_==unit_.frame_length_
                &&this->frame_content_==unit_.frame_content_)
            return true;
        else
            return false;
    }
};
#ifdef UNIT_TEST
class Afn06F2 : public Frame3762Base
#else
/**
 * @brief The AFN06F2 上报抄读数据
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT Afn06F2 : public Frame3762Base
#endif
{
public:
    /**
     * @brief Afn06F2
     */
    Afn06F2();
    /**
     * @brief Afn06F2
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    Afn06F2(CtrlField ctrl_field,InfoField info_field,AddressField address_field);

    ReportDataUnit report_data_unit_;//!<上报抄读数据数据单元
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
#endif // AFN06F2_H
