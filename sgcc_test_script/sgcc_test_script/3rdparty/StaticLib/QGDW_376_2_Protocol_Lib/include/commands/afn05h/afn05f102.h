#ifndef AFN05F102_H
#define AFN05F102_H

#include <QObject>

#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"
namespace qgdw_3762_protocol {

#ifdef UNIT_TEST
class Afn05F102 : public Frame3762Base
#else
/**
 * @brief The Afn05F102 设置全网广播周期
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT Afn05F102 : public Frame3762Base
#endif
{
public:
    Afn05F102();
    /**
     * @brief Afn05F102
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */

    Afn05F102(CtrlField ctrl_field,InfoField info_field,AddressField address_field);
    uchar period_value_;//!<广播周期数值
    char period_unit_;//!<广播周期单位
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
#endif // AFN05F102_H
