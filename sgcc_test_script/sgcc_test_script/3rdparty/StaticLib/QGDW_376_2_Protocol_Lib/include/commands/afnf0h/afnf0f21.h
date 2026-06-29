#ifndef AFNF0F21_H
#define AFNF0F21_H

#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"
namespace qgdw_3762_protocol {


#ifdef UNIT_TEST
class AfnF0F21 : public Frame3762Base
#else
/**
 * @brief The AFNF0F21
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT AfnF0F21 : public Frame3762Base
#endif
{
public:
    AfnF0F21();
    /**
     * @brief AfnF0F21
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    AfnF0F21(CtrlField ctrl_field,InfoField info_field,AddressField address_field);
    char dac_voltage_;//!<DAC电压
    char peak_clipping_factor_;//!<削峰因子
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
#endif // AFNF0F21_H
