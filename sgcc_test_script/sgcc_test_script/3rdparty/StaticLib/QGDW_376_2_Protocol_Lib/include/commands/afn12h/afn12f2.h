#ifndef AFN12F2_H
#define AFN12F2_H

#include <QObject>

#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"
namespace qgdw_3762_protocol {


#ifdef UNIT_TEST
class Afn12F2 : public Frame3762Base
#else
/**
 * @brief The AFN12F2 暂停
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT Afn12F2 : public Frame3762Base
#endif
{
public:
    /**
     * @brief Afn12F2
     */
    Afn12F2();
    /**
     * @brief Afn12F2
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    Afn12F2(CtrlField ctrl_field,InfoField info_field,AddressField address_field);
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
#endif // AFN12F2_H
