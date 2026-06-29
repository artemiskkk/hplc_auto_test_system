#ifndef AFN02F1_H
#define AFN02F1_H

#include <QObject>
#include <QByteArray>
#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"
namespace qgdw_3762_protocol {

#ifdef UNIT_TEST
class Afn02F1 : public Frame3762Base
#else
/**
 * @brief The Afn02F1 转发通信协议数据帧
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT Afn02F1 : public Frame3762Base
#endif
{
public:
    Afn02F1();
    Afn02F1(CtrlField ctrl_field,InfoField info_field,AddressField address_field);

    char protocol_type_ = 0x00; //<! 通信协议类型
    uchar frame_length_ = 0;         //<! 报文长度L
    QByteArray frame_content_;           //<! 报文内容
public:
    /**
     * @brief 将16进制格式数据，解析为数据单元
     * @param data
     */
    void DecodeFrameDataField(QByteArray data) override;
private:
    /**
     * @brief 将数据单元编码为16进制格式数据
     * @return 反回编码好的字节串
     */
    QByteArray EncodeFrameDataField() override;
};

}

#endif // AFN02F1_H
