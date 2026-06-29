#ifndef AFNF0F7_H
#define AFNF0F7_H

#include <QObject>

#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"
namespace qgdw_3762_protocol {

struct ParameterQueryDataUnitUp
{
    char response_state_;
    char parameter_id_[2]={0x00,0x00};
    ushort parameter_length_;
    QByteArray parameter_content_;
};
struct ParameterQueryDataUnitDown
{
    char parameter_id_[2]={0x00,0x00};
};
#ifdef UNIT_TEST
class AfnF0F7 : public Frame3762Base
#else
/**
 * @brief The AFNF0F7
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT AfnF0F7 : public Frame3762Base
#endif
{
public:
    /**
     * @brief AfnF0F7
     */
    AfnF0F7();
    /**
     * @brief AfnF0F7
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    AfnF0F7(CtrlField ctrl_field,InfoField info_field,AddressField address_field);

    ParameterQueryDataUnitUp data_unit_up_;
    ParameterQueryDataUnitDown data_unit_down_;

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

#endif // AFNF0F7_H
