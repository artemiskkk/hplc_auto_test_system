#ifndef SECURITYREQUEST_H
#define SECURITYREQUEST_H

#include <QObject>
#include "../frameoopbase.h"
#include"data/datatypedef.h"
using namespace std;

namespace  object_oriented_electic_data_exchange_protocol {


struct AppDataUnit
{
    uchar data_type_;
    QByteArray data_unit_;
};
struct DataVerifyInfo
{
    uchar data_type_;
    SID_MAC sid_mac_;
    RN rn_;
    RN_MAC rn_mac_;
    SID sid_;
};


/**
 * @brief 请求安全传输
 */
#ifdef UNIT_TEST
class SecurityRequest : public FrameOOPBase
#else
class OOP_LIB_EXPORT SecurityRequest : public FrameOOPBase
#endif
{
public:
    SecurityRequest();
    ~SecurityRequest() override;

    /**
     * @brief SecurityRequest
     * @param ctrl_field  控制域
     * @param address_field  地址域
     * @param framing_field  分帧控制域
     */
    //SecurityRequest(CtrlField ctrl_field,AddressField address_field,FrameFormatField  framing_field);
    SecurityRequest(CtrlField ctrl_field,AddressField address_field);


    AppDataUnit app_data_unit_;
    DataVerifyInfo data_verify_info_;
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
#endif // SECURITYREQUEST_H
