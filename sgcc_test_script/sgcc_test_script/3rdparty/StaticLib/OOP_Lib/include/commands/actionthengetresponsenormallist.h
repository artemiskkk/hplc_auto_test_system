#ifndef ACTIONTHENGETRESPONSENORMALLIST_H
#define ACTIONTHENGETRESPONSENORMALLIST_H

#include <QObject>
#include "../frameoopbase.h"

using namespace std;

namespace  object_oriented_electic_data_exchange_protocol {

/**
 * @brief 操作若干个对象方法后读取若干个属性的响应
 */
#ifdef UNIT_TEST
class ActionThenGetResponseNormalList : public FrameOOPBase
#else
class OOP_LIB_EXPORT ActionThenGetResponseNormalList : public FrameOOPBase
#endif
{
public:
    ActionThenGetResponseNormalList();
    ~ActionThenGetResponseNormalList() override;

    /**
     * @brief ActionThenGetResponseNormalList
     * @param ctrl_field  控制域
     * @param address_field  地址域
     * @param framing_field  分帧控制域
     */
    //ActionThenGetResponseNormalList(CtrlField ctrl_field,AddressField address_field,FrameFormatField  framing_field);
    ActionThenGetResponseNormalList(CtrlField ctrl_field,AddressField address_field);

    PIID_ACD piid_acd_;//!< 服务序号-优先级-ACD
    QList<AActionThenGetNormalResult> list_action_get_normal_result_;//!< 操作若干个对象方法后读取属性的结果
    //TimeTagField time_tag_field_;
    FollowReportField follow_report_field_;//!< 跟随上报信息域
public:
    /**
     * @brief 将16进制格式数据，解析为数据单元
     * @param data
     */
    void DecodeFrameDataField(QByteArray data) override;

    uchar GetActionGetNormalResultListSize();
private:
    /**
     * @brief 将数据单元编码为16进制格式数据
     * @return 返回编码好的字节串
     */
    QByteArray EncodeFrameDataField() override;

    uchar size_;//!< 对象方法操作后读取结果个数
};

}

#endif // ACTIONTHENGETRESPONSENORMALLIST_H
