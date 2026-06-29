#ifndef REPORTNOTIFICATIONLIST_H
#define REPORTNOTIFICATIONLIST_H

#include <QObject>
#include "../frameoopbase.h"

using namespace std;

namespace  object_oriented_electic_data_exchange_protocol {

/**
 * @brief 上报若干个对象属性
 */
#ifdef UNIT_TEST
class ReportNotificationList : public FrameOOPBase
#else
class OOP_LIB_EXPORT ReportNotificationList : public FrameOOPBase
#endif
{
public:
    ReportNotificationList();
    ~ReportNotificationList() override;
    /**
     * @brief ReportNotificationList
     * @param ctrl_field  控制域
     * @param address_field  地址域
     * @param framing_field  分帧控制域
     */
    //ReportNotificationList(CtrlField ctrl_field,AddressField address_field,FrameFormatField  framing_field);
    ReportNotificationList(CtrlField ctrl_field,AddressField address_field);

    PIID_ACD piid_acd_;//!< 服务序号-优先级-ACD
    QList<AResultNormal> list_result_normal_;//!< 若干个对象属性及其数据

    FollowReportField follow_report_field_;//!< 跟随上报信息域
public:
    /**
     * @brief 将16进制格式数据，解析为数据单元
     * @param data
     */
    void DecodeFrameDataField(QByteArray data) override;

    uchar GetResultNormalSize();
public:
    /**
     * @brief 将数据单元编码为16进制格式数据
     * @return 返回编码好的字节串
     */
    QByteArray EncodeFrameDataField() override;

    uchar size_;//!< 对象属性个数
};

}

#endif // REPORTNOTIFICATIONLIST_H
