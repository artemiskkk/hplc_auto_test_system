#ifndef SETREQUESTNORMALLIST_H
#define SETREQUESTNORMALLIST_H

#include <QObject>
#include "../frameoopbase.h"
#include"data/datatypedef.h"
using namespace std;

namespace  object_oriented_electic_data_exchange_protocol {

/**
 * @brief 设置若干个对象属性请求
 */
#ifdef UNIT_TEST
class SetRequestNormalList : public FrameOOPBase
#else
class OOP_LIB_EXPORT SetRequestNormalList : public FrameOOPBase
#endif
{
public:
    SetRequestNormalList();
    ~SetRequestNormalList() override;

    /**
     * @brief SetRequestNormalList
     * @param ctrl_field  控制域
     * @param address_field  地址域
     * @param framing_field  分帧控制域
     */
    //SetRequestNormalList(CtrlField ctrl_field,AddressField address_field,FrameFormatField  framing_field);
    SetRequestNormalList(CtrlField ctrl_field,AddressField address_field);

    PIID piid_;//!< 服务序号-优先级
    QList<ASetNormal> list_set_normal_;//!< 若干个对象属性
    //TimeTagField time_tag_field_;
public:
    /**
     * @brief 将16进制格式数据，解析为数据单元
     * @param data
     */
    void DecodeFrameDataField(QByteArray data) override;

    uchar GetSetNormalListSize();
public:
    /**
     * @brief 将数据单元编码为16进制格式数据
     * @return 返回编码好的字节串
     */
    QByteArray EncodeFrameDataField() override;

    uchar size_;//!< 对象属性个数
};

}
#endif // SETREQUESTNORMALLIST_H
