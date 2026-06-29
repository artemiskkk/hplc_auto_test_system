#ifndef AFN05F5_H
#define AFN05F5_H

#include <QObject>

#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"
namespace qgdw_3762_protocol {

#ifdef UNIT_TEST
class Afn05F5 : public Frame3762Base
#else
/**
 * @brief The AFN05F5 设置无线通信参数
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT Afn05F5 : public Frame3762Base
#endif
{
public:
    /**
     * @brief Afn05F5
     */
    Afn05F5();
    /**
     * @brief Afn05F5
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    Afn05F5(CtrlField ctrl_field,InfoField info_field,AddressField address_field);
    char wireless_channel_group_;//!<无线信道组
    char emit_power_;//!<无线发射功率
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
#endif // AFN05F5_H
