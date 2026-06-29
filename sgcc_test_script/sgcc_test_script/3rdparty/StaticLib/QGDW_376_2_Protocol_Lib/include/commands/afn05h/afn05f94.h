#ifndef AFN05F94_H
#define AFN05F94_H


#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"
namespace qgdw_3762_protocol {

#ifdef UNIT_TEST
class Afn05F94 : public Frame3762Base
#else
/**
 * @brief The Afn05F94 超差上报阈值设置
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT Afn05F94 : public Frame3762Base
#endif
{
public:
    Afn05F94();
    /**
     * @brief Afn05F94
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */

    Afn05F94(CtrlField ctrl_field,InfoField info_field,AddressField address_field);
    Address node_address_;//!<表地址
    uchar threshold_;//!<阈值(分钟)
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
#endif // AFN05F94_H
