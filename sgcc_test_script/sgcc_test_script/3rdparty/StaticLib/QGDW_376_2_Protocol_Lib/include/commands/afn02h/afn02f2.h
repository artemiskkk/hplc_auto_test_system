#ifndef AFN02F2_H
#define AFN02F2_H

#include <QObject>
#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"

namespace qgdw_3762_protocol {
#ifdef UNIT_TEST
class Afn02F2 : public Frame3762Base
#else
/**
 * @brief The Afn02F2 抄控器扩展-测试帧转发
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT Afn02F2 : public Frame3762Base
#endif
{
public:
    Afn02F2();
    Afn02F2(CtrlField ctrl_field,InfoField info_field,AddressField address_field);

    char protocol_type_;       //<!  通信协议类型 00H为透明传输；01H为 DL/T 645—1997；02H为 DL/T 645—2007；03H为DL/T698.45; 04H为宽带载波应用层协议；05H为测试帧协议
    ushort test_count_;     //<!  测试次数
    char tmi_;               //<!  TIM
    uchar block_count_;      //<!  物理分块数
    char reserve_ = 0x00;   //<!  发送帧类型  0-Beacon 1-SOF
    ushort frame_length_;       //<!  报文长度L
    QByteArray frame_content_;  //<!  报文内容
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
#endif // AFN02F2_H
