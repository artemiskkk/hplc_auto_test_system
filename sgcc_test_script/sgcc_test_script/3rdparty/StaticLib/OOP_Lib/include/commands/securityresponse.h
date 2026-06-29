#ifndef SECURITYRESPONSE_H
#define SECURITYRESPONSE_H

#include <QObject>
#include "../frameoopbase.h"
#include"data/datatypedef.h"
using namespace std;

namespace  object_oriented_electic_data_exchange_protocol {

struct AppDataUnit_Up
{
    uchar data_type_;
    QByteArray data_unit_;
    DAR dar_;
};
struct DataVerifyInfo_Up
{
    uchar optional_;
    uchar data_type_;
    MAC mac_;
};

/**
 * @brief 设置一个对象属性请求
 */
#ifdef UNIT_TEST
class SecurityResponse : public FrameOOPBase
#else
class OOP_LIB_EXPORT SecurityResponse : public FrameOOPBase
#endif
{
public:
    SecurityResponse();
    ~SecurityResponse() override;

    /**
     * @brief SecurityResponse
     * @param ctrl_field  控制域
     * @param address_field  地址域
     * @param framing_field  分帧控制域
     */
    //SecurityResponse(CtrlField ctrl_field,AddressField address_field,FrameFormatField  framing_field);
    SecurityResponse(CtrlField ctrl_field,AddressField address_field);

    AppDataUnit_Up app_data_unit_;
    DataVerifyInfo_Up data_verify_info_;
    //TimeTagField time_tag_field_;
public:
    /**
     * @brief 将16进制格式数据，解析为数据单元
     * @param data
     */
    void DecodeFrameDataField(QByteArray data) override;

public:
    /**
     * @brief 将数据单元编码为16进制格式数据
     * @return 返回编码好的字节串
     */
    QByteArray EncodeFrameDataField() override;
private:
    void Decode_OctetString(uchar lenflag,QByteArray &data,QByteArray &msg);
};

}
#endif // SECURITYRESPONSE_H
