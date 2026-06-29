#ifndef GETREQUESTMD5_H
#define GETREQUESTMD5_H

#include <QObject>
#include "../frameoopbase.h"

using namespace std;

namespace  object_oriented_electic_data_exchange_protocol {

/**
 * @brief 读取一个对象属性的MD5值
 */
#ifdef UNIT_TEST
class GetRequestMD5 : public FrameOOPBase
#else
class OOP_LIB_EXPORT GetRequestMD5 : public FrameOOPBase
#endif
{
public:
    GetRequestMD5();
    ~GetRequestMD5() override;
    /**
     * @brief GetRequestMD5
     * @param ctrl_field  控制域
     * @param address_field  地址域
     * @param framing_field  分帧控制域
     */
    //GetRequestMD5(CtrlField ctrl_field,AddressField address_field,FrameFormatField  framing_field);
    GetRequestMD5(CtrlField ctrl_field,AddressField address_field);

    PIID piid_;//!< 服务序号-优先级
    OAD oad_;//!< 一个对象属性描述符

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

#endif // GETREQUESTMD5_H
