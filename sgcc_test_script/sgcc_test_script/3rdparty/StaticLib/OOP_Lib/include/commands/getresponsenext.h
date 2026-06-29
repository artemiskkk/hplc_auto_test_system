#ifndef GETRESPONSENEXT_H
#define GETRESPONSENEXT_H

#include <QObject>
#include "../frameoopbase.h"

using namespace std;

namespace  object_oriented_electic_data_exchange_protocol {

/**
 * @brief 分帧响应一个数据块
 */
#ifdef UNIT_TEST
class GetResponseNext : public FrameOOPBase
#else
class OOP_LIB_EXPORT GetResponseNext : public FrameOOPBase
#endif
{
public:
    GetResponseNext();
    ~GetResponseNext() override;
    /**
     * @brief GetResponseNext
     * @param ctrl_field  控制域
     * @param address_field  地址域
     * @param framing_field  分帧控制域
     */
    //GetResponseNext(CtrlField ctrl_field,AddressField address_field,FrameFormatField  framing_field);
    GetResponseNext(CtrlField ctrl_field,AddressField address_field);

    PIID_ACD piid_acd_;//!< 服务序号-优先级-ACD
    bool end_frame_flag_;//!< 末帧标志
    ushort sub_framing_seq_;//!< 分帧序号
    std::shared_ptr<SubFramingResponseParent> sub_frame_response_data_ptr_;//!< 分帧响应

    FollowReportField follow_report_field_;//!< 跟随上报信息域
public:
    /**
     * @brief 将16进制格式数据，解析为数据单元
     * @param data
     */
    void DecodeFrameDataField(QByteArray data) override;

private:
    /**
     * @brief 将数据单元编码为16进制格式数据
     * @return 返回编码好的字节串
     */
    QByteArray EncodeFrameDataField() override;
};

}

#endif // GETRESPONSENEXT_H
