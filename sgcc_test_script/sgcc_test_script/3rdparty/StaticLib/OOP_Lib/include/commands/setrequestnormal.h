#ifndef SETREQUESTNORMAL_H
#define SETREQUESTNORMAL_H

#include <QObject>
#include "../frameoopbase.h"

using namespace std;

namespace  object_oriented_electic_data_exchange_protocol {

/**
 * @brief 设置一个对象属性请求
 */
#ifdef UNIT_TEST
class SetRequestNormal : public FrameOOPBase
#else
class OOP_LIB_EXPORT SetRequestNormal : public FrameOOPBase
#endif
{
public:
    SetRequestNormal();
    ~SetRequestNormal() override;

    /**
     * @brief SetRequestNormal
     * @param ctrl_field  控制域
     * @param address_field  地址域
     * @param framing_field  分帧控制域
     */
    //SetRequestNormal(CtrlField ctrl_field,AddressField address_field,FrameFormatField  framing_field);
    SetRequestNormal(CtrlField ctrl_field,AddressField address_field);

    PIID piid_;//!< 服务序号-优先级
    ASetNormal a_set_normal_;//!< 设置一个对象属性
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

#endif // SETREQUESTNORMAL_H
