#ifndef AFN10F93_H
#define AFN10F93_H

#include <QObject>

#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"
namespace qgdw_3762_protocol {


#ifdef UNIT_TEST
class Afn10F93 : public Frame3762Base
#else
/**
 * @brief The Afn10F93 时钟维护周期查询
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT Afn10F93 : public Frame3762Base
#endif
{
public:
    /**
     * @brief Afn10F93
     */
    Afn10F93();
    /**
     * @brief Afn10F93
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    Afn10F93(CtrlField ctrl_field,InfoField info_field,AddressField address_field);
    ushort period_;//!<周期
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
#endif // AFN10F93_H
