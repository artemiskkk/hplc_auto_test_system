#ifndef GETRESPONSEMD5_H
#define GETRESPONSEMD5_H

#include <QObject>
#include "../frameoopbase.h"

using namespace std;

namespace  object_oriented_electic_data_exchange_protocol {

/**
 * @brief 响应读取对象属性MD5值
 */
#ifdef UNIT_TEST
class GetResponseMD5 : public FrameOOPBase
#else
class OOP_LIB_EXPORT GetResponseMD5 : public FrameOOPBase
#endif
{
public:
    GetResponseMD5();
    ~GetResponseMD5() override;
    /**
     * @brief GetResponseMD5
     * @param ctrl_field  控制域
     * @param address_field  地址域
     * @param framing_field  分帧控制域
     */
    //GetResponseMD5(CtrlField ctrl_field,AddressField address_field,FrameFormatField  framing_field);
    GetResponseMD5(CtrlField ctrl_field,AddressField address_field);

    PIID_ACD piid_acd_;//!< 服务序号-优先级-ACD
    OAD oad_;//!< 对象属性描述符
    std::shared_ptr<MD5ResultParent> md5_result_value_ptr_ = nullptr;//!<  MD5结果

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

#endif // GETRESPONSEMD5_H
