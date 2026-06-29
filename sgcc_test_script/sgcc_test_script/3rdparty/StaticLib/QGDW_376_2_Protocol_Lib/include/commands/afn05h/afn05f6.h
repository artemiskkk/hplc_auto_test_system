#ifndef AFN05F6_H
#define AFN05F6_H

#include <QObject>

#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"
namespace qgdw_3762_protocol {

#ifdef UNIT_TEST
class Afn05F6 : public Frame3762Base
#else
/**
 * @brief The AFN05F6 允许/禁止台区识别
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT Afn05F6 : public Frame3762Base
#endif
{
public:
    /**
     * @brief Afn05F6
     */
    Afn05F6();
    /**
     * @brief Afn05F6
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    Afn05F6(CtrlField ctrl_field,InfoField info_field,AddressField address_field);
    char area_identify_enable_flag_;//!<台区识别使能标志
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
#endif // AFN05F6_H
