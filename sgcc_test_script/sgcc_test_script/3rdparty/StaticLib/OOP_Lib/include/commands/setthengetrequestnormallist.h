#ifndef SETTHENGETREQUESTNORMALLIST_H
#define SETTHENGETREQUESTNORMALLIST_H

#include <QObject>
#include "../frameoopbase.h"

using namespace std;

namespace  object_oriented_electic_data_exchange_protocol {

/**
 * @brief 设置后读取若干个对象属性请求
 */
#ifdef UNIT_TEST
class SetThenGetRequestNormalList : public FrameOOPBase
#else
class OOP_LIB_EXPORT SetThenGetRequestNormalList : public FrameOOPBase
#endif
{
public:
    SetThenGetRequestNormalList();
    ~SetThenGetRequestNormalList() override;

    /**
     * @brief SetThenGetRequestNormalList
     * @param ctrl_field  控制域
     * @param address_field  地址域
     * @param framing_field  分帧控制域
     */
    //SetThenGetRequestNormalList(CtrlField ctrl_field,AddressField address_field,FrameFormatField  framing_field);
    SetThenGetRequestNormalList(CtrlField ctrl_field,AddressField address_field);

    PIID piid_;//!< 服务序号-优先级
    QList<ASetThenGetNormal> list_set_then_get_normal_;//!< 若干个设置后读取对象属性
    //TimeTagField time_tag_field_;
public:
    /**
     * @brief 将16进制格式数据，解析为数据单元
     * @param data
     */
    void DecodeFrameDataField(QByteArray data) override;

    uchar GetSetThenGetNormalListSize();
private:
    /**
     * @brief 将数据单元编码为16进制格式数据
     * @return 返回编码好的字节串
     */
    QByteArray EncodeFrameDataField() override;

    uchar size_;//!< 设置后读取对象属性
};

}

#endif // SETTHENGETREQUESTNORMALLIST_H
