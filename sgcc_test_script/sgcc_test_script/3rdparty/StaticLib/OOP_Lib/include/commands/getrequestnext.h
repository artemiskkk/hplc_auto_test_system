#ifndef GETREQUESTNEXT_H
#define GETREQUESTNEXT_H

#include <QObject>
#include "../frameoopbase.h"

using namespace std;

namespace  object_oriented_electic_data_exchange_protocol {

/**
 * @brief 读取分帧响应的下一个数据块请求
 */
#ifdef UNIT_TEST
class GetRequestNext : public FrameOOPBase
#else
class OOP_LIB_EXPORT GetRequestNext : public FrameOOPBase
#endif
{
public:
    GetRequestNext();
    ~GetRequestNext() override;
    /**
     * @brief GetRequestNext
     * @param ctrl_field  控制域
     * @param address_field  地址域
     * @param framing_field  分帧控制域
     */
    //GetRequestNext(CtrlField ctrl_field,AddressField address_field,FrameFormatField  framing_field);
    GetRequestNext(CtrlField ctrl_field,AddressField address_field);

    PIID piid_;//!< 服务序号-优先级
    ushort latest_seq_;//!< 正确接收的最近一次数据块序号

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

#endif // GETREQUESTNEXT_H
