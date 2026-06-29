#ifndef ACTIONRESPONSENORMAL_H
#define ACTIONRESPONSENORMAL_H

#include <QObject>
#include "../frameoopbase.h"

using namespace std;

namespace  object_oriented_electic_data_exchange_protocol {

/**
 * @brief 操作一个对象方法的响应
 */
#ifdef UNIT_TEST
class ActionResponseNormal : public FrameOOPBase
#else
class OOP_LIB_EXPORT ActionResponseNormal : public FrameOOPBase
#endif
{
public:
    ActionResponseNormal();
    ~ActionResponseNormal() override;

    /**
     * @brief ActionResponseNormal
     * @param ctrl_field  控制域
     * @param address_field  地址域
     * @param framing_field  分帧控制域
     */
    //ActionResponseNormal(CtrlField ctrl_field,AddressField address_field,FrameFormatField  framing_field);
    ActionResponseNormal(CtrlField ctrl_field,AddressField address_field);

    PIID_ACD piid_acd_;//!< 服务序号-优先级-ACD
    AActionResult a_action_result_;//!< 一个对象方法操作执行结果
    //TimeTagField time_tag_field_;
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

#endif // ACTIONRESPONSENORMAL_H
