#ifndef GETRESPONSENORMAL_H
#define GETRESPONSENORMAL_H

#include <QObject>
#include "../frameoopbase.h"

using namespace std;

namespace  object_oriented_electic_data_exchange_protocol {

/**
 * @brief 读取一个对象属性的响应
 */
#ifdef UNIT_TEST
class GetResponseNormal : public FrameOOPBase
#else
class OOP_LIB_EXPORT GetResponseNormal : public FrameOOPBase
#endif
{
public:
    GetResponseNormal();
    ~GetResponseNormal() override;
    /**
     * @brief GetResponseNormal
     * @param ctrl_field  控制域
     * @param address_field  地址域
     * @param framing_field  分帧控制域
     */
    //GetResponseNormal(CtrlField ctrl_field,AddressField address_field,FrameFormatField  framing_field);
    GetResponseNormal(CtrlField ctrl_field,AddressField address_field);

    PIID_ACD piid_acd_;//!< 服务序号-优先级-ACD
    AResultNormal a_result_normal_;//!< 一个对象属性及其结果

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

#endif // GETRESPONSENORMAL_H
