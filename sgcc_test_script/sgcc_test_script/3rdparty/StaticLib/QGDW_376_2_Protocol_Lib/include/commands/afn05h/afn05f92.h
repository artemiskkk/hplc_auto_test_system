#ifndef AFN05F92_H
#define AFN05F92_H


#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"
namespace qgdw_3762_protocol {

#ifdef UNIT_TEST
class Afn05F92 : public Frame3762Base
#else
/**
 * @brief The Afn05F92 精准校时
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT Afn05F92 : public Frame3762Base
#endif
{
public:
    Afn05F92();
    /**
     * @brief Afn05F92
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */

    Afn05F92(CtrlField ctrl_field,InfoField info_field,AddressField address_field);
    Address node_address_;//!<表地址
    ushort frame_len_;//!<转发数据长度
    QByteArray frame_content_;//!<数据
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
#endif // AFN05F92_H
