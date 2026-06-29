#ifndef AFN14F3_H
#define AFN14F3_H

#include <QObject>

#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"
namespace qgdw_3762_protocol {
/**
 * @brief The RequestModifyDataUnitUp 请求依通信延时修正通信数据数据单元上行
 */
struct RequestModifyDataUnitUp
{
    Address node_address_;//!<从节点地址
    ushort delay_time_;//!<预计延迟时间
    uchar frame_length_;//!<抄读信息长度
    QByteArray frame_content_;//!<抄读数据内容
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const RequestModifyDataUnitUp &unit_)const
    {
        if(this->node_address_==unit_.node_address_
                &&this->delay_time_==unit_.delay_time_
                &&this->frame_length_==unit_.frame_length_
                &&this->frame_length_==unit_.frame_length_
                &&this->frame_content_==unit_.frame_content_)
            return true;
        else
            return false;
    }
};
/**
 * @brief The RequestModifyDataUnitDown 请求依通信延时修正通信数据数据单元下行
 */
struct RequestModifyDataUnitDown
{
    uchar frame_length_;//!<数据长度
    QByteArray frame_content_;//!<修正通信数据内容
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const RequestModifyDataUnitDown &unit_)const
    {
        if(this->frame_length_==unit_.frame_length_
                &&this->frame_content_==unit_.frame_content_)
            return true;
        else
            return false;
    }
};

#ifdef UNIT_TEST
class Afn14F3 : public Frame3762Base
#else
/**
 * @brief The AFN14F3 请求依通信延时修正通信数据
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT Afn14F3 : public Frame3762Base
#endif
{
public:
    /**
     * @brief Afn14F3
     */
    Afn14F3();
    /**
     * @brief Afn14F3
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    Afn14F3(CtrlField ctrl_field,InfoField info_field,AddressField address_field);

    RequestModifyDataUnitUp request_unit_up_;//!<请求依通信延时修正通信数据数据单元上行
    RequestModifyDataUnitDown request_unit_down_;//!<请求依通信延时修正通信数据数据单元下行
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
#endif // AFN14F3_H
