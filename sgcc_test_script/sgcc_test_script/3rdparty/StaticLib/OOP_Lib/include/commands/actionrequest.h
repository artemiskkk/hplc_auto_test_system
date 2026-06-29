#ifndef ACTIONREQUEST_H
#define ACTIONREQUEST_H

#include <QObject>
#include "../frameoopbase.h"

using namespace std;

namespace  object_oriented_electic_data_exchange_protocol {

/**
 * @brief 操作一个对象方法请求
 */
#ifdef UNIT_TEST
class ActionRequest : public FrameOOPBase
#else
class OOP_LIB_EXPORT ActionRequest : public FrameOOPBase
#endif
{
public:
    ActionRequest();
    ~ActionRequest() override;

    /**
     * @brief ActionRequest
     * @param ctrl_field  控制域
     * @param address_field  地址域
     * @param framing_field  分帧控制域
     */
    //ActionRequest(CtrlField ctrl_field,AddressField address_field,FrameFormatField  framing_field);
    ActionRequest(CtrlField ctrl_field,AddressField address_field);

    PIID piid_;//!< 服务序号-优先级
    AAction a_action_;//!< 操作一个对象方法
    //TimeTagField time_tag_field_;
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

#endif // ACTIONREQUEST_H
