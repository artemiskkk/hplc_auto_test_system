#ifndef AFN12F1_H
#define AFN12F1_H

#include <QObject>

#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"
namespace qgdw_3762_protocol {


#ifdef UNIT_TEST
class Afn12F1 : public Frame3762Base
#else
/**
 * @brief The AFN12F1 重启
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT Afn12F1 : public Frame3762Base
#endif
{
public:
    /**
     * @brief Afn12F1
     */
    Afn12F1();
    /**
     * @brief Afn12F1
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    Afn12F1(CtrlField ctrl_field,InfoField info_field,AddressField address_field);
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
#endif // AFN12F1_H
