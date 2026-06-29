#ifndef AFN06F66_H
#define AFN06F66_H

#include <QObject>

#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"
namespace qgdw_3762_protocol {


#ifdef UNIT_TEST
class Afn06F66 : public Frame3762Base
#else
/**
 * @brief The AFN06F6 伴听主动上报数据
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT Afn06F66 : public Frame3762Base
#endif
{
public:
    /**
     * @brief Afn06F66
     */
    Afn06F66();
    /**
     * @brief Afn06F66
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    Afn06F66(CtrlField ctrl_field,InfoField info_field,AddressField address_field);

    uchar frame_type_;//!<上报事件从节点数量
    uchar protocol_;//!<通信类型
    ushort frame_len_;//!<报文长度
    QByteArray frame_;//!<上报从节点地址QList
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
#endif // AFN06F66_H
