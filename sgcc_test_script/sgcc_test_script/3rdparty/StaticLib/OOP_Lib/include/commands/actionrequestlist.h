#ifndef ACTIONREQUESTLIST_H
#define ACTIONREQUESTLIST_H

#include <QObject>
#include "../frameoopbase.h"

using namespace std;

namespace  object_oriented_electic_data_exchange_protocol {

/**
 * @brief 操作若干个对象方法请求
 */
#ifdef UNIT_TEST
class ActionRequestList : public FrameOOPBase
#else
class OOP_LIB_EXPORT ActionRequestList : public FrameOOPBase
#endif
{
public:
    ActionRequestList();
    ~ActionRequestList() override;

    /**
     * @brief ActionRequestList
     * @param ctrl_field  控制域
     * @param address_field  地址域
     * @param framing_field  分帧控制域
     */
    //ActionRequestList(CtrlField ctrl_field,AddressField address_field,FrameFormatField  framing_field);
    ActionRequestList(CtrlField ctrl_field,AddressField address_field);

    PIID piid_;//!< 服务序号-优先级
    QList<AAction> list_action_;//!< 若干个方法
    //TimeTagField time_tag_field_;
public:
    /**
     * @brief 将16进制格式数据，解析为数据单元
     * @param data
     */
    void DecodeFrameDataField(QByteArray data) override;

    uchar GetActionListSize();
public:
    /**
     * @brief 将数据单元编码为16进制格式数据
     * @return 返回编码好的字节串
     */
    QByteArray EncodeFrameDataField() override;

    uchar size_;//!< 对象方法个数
};

}

#endif // ACTIONREQUESTLIST_H
