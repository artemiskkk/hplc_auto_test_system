#ifndef REPORTNOTIFICATIONTRANSDATA_H
#define REPORTNOTIFICATIONTRANSDATA_H

#include <QObject>
#include "../frameoopbase.h"

using namespace std;

namespace  object_oriented_electic_data_exchange_protocol {

/**
 * @brief 上报透明数据通知
 */
#ifdef UNIT_TEST
class ReportNotificationTransData : public FrameOOPBase
#else
class OOP_LIB_EXPORT ReportNotificationTransData : public FrameOOPBase
#endif
{
public:
    ReportNotificationTransData();
    ~ReportNotificationTransData() override;
    /**
     * @brief ReportNotificationTransData
     * @param ctrl_field  控制域
     * @param address_field  地址域
     * @param framing_field  分帧控制域
     */
    //ReportNotificationTransData(CtrlField ctrl_field,AddressField address_field,FrameFormatField  framing_field);
    ReportNotificationTransData(CtrlField ctrl_field,AddressField address_field);

    PIID_ACD piid_acd_;//!< 服务序号-优先级-ACD
    OAD oad_;//!< 数据来源端口号
    QList<QByteArray> list_trans_data_;//!< 透明数据

    FollowReportField follow_report_field_;//!< 跟随上报信息域
public:
    /**
     * @brief 将16进制格式数据，解析为数据单元
     * @param data
     */
    void DecodeFrameDataField(QByteArray data) override;

    uchar GetTransDataSize();
private:
    /**
     * @brief 将数据单元编码为16进制格式数据
     * @return 返回编码好的字节串
     */
    QByteArray EncodeFrameDataField() override;

    uchar size_;//!< 透明数据个数
};

}

#endif // REPORTNOTIFICATIONTRANSDATA_H
