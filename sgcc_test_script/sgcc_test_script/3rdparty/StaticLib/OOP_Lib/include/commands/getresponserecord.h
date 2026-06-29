#ifndef GETRESPONSERECORD_H
#define GETRESPONSERECORD_H

#include <QObject>
#include "../frameoopbase.h"

using namespace std;

namespace  object_oriented_electic_data_exchange_protocol {

/**
 * @brief 读取一个记录型对象属性的响应
 */
#ifdef UNIT_TEST
class GetResponseRecord : public FrameOOPBase
#else
class OOP_LIB_EXPORT GetResponseRecord : public FrameOOPBase
#endif
{
public:
    GetResponseRecord();
    ~GetResponseRecord() override;
    /**
     * @brief GetResponseRecord
     * @param ctrl_field  控制域
     * @param address_field  地址域
     * @param framing_field  分帧控制域
     */
    //GetResponseRecord(CtrlField ctrl_field,AddressField address_field,FrameFormatField  framing_field);
    GetResponseRecord(CtrlField ctrl_field,AddressField address_field);

    PIID_ACD piid_acd_;//!< 服务序号-优先级-ACD
    AResultRecord a_result_record_;//!< 一个记录型对象属性及其结果

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

#endif // GETRESPONSERECORD_H
