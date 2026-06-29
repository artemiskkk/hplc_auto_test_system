#ifndef AFN03F8_H
#define AFN03F8_H

#include <QObject>
#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"

namespace qgdw_3762_protocol {

#ifdef UNIT_TEST
class Afn03F8 : public Frame3762Base
#else
/**
 * @brief The Afn03F8
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT Afn03F8 : public Frame3762Base
#endif
{
public:
    /**
     * @brief Afn03F8
     */
    Afn03F8();
    /**
     * @brief Afn03F8
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    Afn03F8(CtrlField ctrl_field,InfoField info_field,AddressField address_field);

    uchar channel_group_; //!< 无线信道组：0~63
    uchar emit_power_; //!< 无线主节点发射功率：0表示最高发射功率；1表示次高发射功率；2表示次低发射功率；3表示最低发射功率。
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
#endif // AFN03F8_H
