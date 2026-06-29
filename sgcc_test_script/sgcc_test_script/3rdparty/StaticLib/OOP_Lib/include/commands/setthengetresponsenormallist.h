#ifndef SETTHENGETRESPONSENORMALLIST_H
#define SETTHENGETRESPONSENORMALLIST_H

#include <QObject>
#include "../frameoopbase.h"

using namespace std;

namespace  object_oriented_electic_data_exchange_protocol {

/**
 * @brief 设置若干个对象属性的确认信息以及读取若干个对象属性的响应
 */
#ifdef UNIT_TEST
class SetThenGetResponseNormalList : public FrameOOPBase
#else
class OOP_LIB_EXPORT SetThenGetResponseNormalList : public FrameOOPBase
#endif
{
public:
    SetThenGetResponseNormalList();
    ~SetThenGetResponseNormalList() override;

    /**
     * @brief SetThenGetResponseNormalList
     * @param ctrl_field  控制域
     * @param address_field  地址域
     * @param framing_field  分帧控制域
     */
    //SetThenGetResponseNormalList(CtrlField ctrl_field,AddressField address_field,FrameFormatField  framing_field);
    SetThenGetResponseNormalList(CtrlField ctrl_field,AddressField address_field);

    PIID_ACD piid_acd_;//!< 服务序号-优先级-ACD
    QList<ASetThenGetNormalResult> list_set_get_normal_result_;//!< 若干个对象属性设置后读取结果
    //TimeTagField time_tag_field_;
    FollowReportField follow_report_field_;//!< 跟随上报信息域
public:
    /**
     * @brief 将16进制格式数据，解析为数据单元
     * @param data
     */
    void DecodeFrameDataField(QByteArray data) override;

    uchar GetSetGetNormalResultListSize();
private:
    /**
     * @brief 将数据单元编码为16进制格式数据
     * @return 返回编码好的字节串
     */
    QByteArray EncodeFrameDataField() override;

    uchar size_;//!< 对象属性设置后读取结果个数
};

}

#endif // SETTHENGETRESPONSENORMALLIST_H
