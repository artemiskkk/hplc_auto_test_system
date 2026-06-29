#ifndef ACTIONRESPONSENORMALLIST_H
#define ACTIONRESPONSENORMALLIST_H

#include <QObject>
#include "../frameoopbase.h"

using namespace std;

namespace  object_oriented_electic_data_exchange_protocol {

/**
 * @brief 操作若干个对象方法的响应
 */
#ifdef UNIT_TEST
class ActionResponseNormalList : public FrameOOPBase
#else
class OOP_LIB_EXPORT ActionResponseNormalList : public FrameOOPBase
#endif
{
public:
    ActionResponseNormalList();
    ~ActionResponseNormalList() override;

    /**
     * @brief ActionResponseNormalList
     * @param ctrl_field  控制域
     * @param address_field  地址域
     * @param framing_field  分帧控制域
     */
    //ActionResponseNormalList(CtrlField ctrl_field,AddressField address_field,FrameFormatField  framing_field);
    ActionResponseNormalList(CtrlField ctrl_field,AddressField address_field);

    PIID_ACD piid_acd_;//!< 服务序号-优先级-ACD
    QList<AActionResult> list_action_result_;//!< 若干个对象方法操作结果
    //TimeTagField time_tag_field_;
    FollowReportField follow_report_field_;//!< 跟随上报信息域
public:
    /**
     * @brief 将16进制格式数据，解析为数据单元
     * @param data
     */
    void DecodeFrameDataField(QByteArray data) override;

    uchar GetActionNormalResultListSize();
public:
    /**
     * @brief 将数据单元编码为16进制格式数据
     * @return 返回编码好的字节串
     */
    QByteArray EncodeFrameDataField() override;

    uchar size_;//!< 对象方法操作结果个数
};

}

#endif // ACTIONRESPONSENORMALLIST_H
