#ifndef AFN01F1_H
#define AFN01F1_H

#include <QObject>
#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"

namespace qgdw_3762_protocol {
#ifdef UNIT_TEST
class Afn01F1 : public Frame3762Base
#else
/**
 * @brief The Afn01F1 硬件初始化（复位）
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT Afn01F1 : public Frame3762Base
#endif
{
public:
    Afn01F1();
    Afn01F1(CtrlField ctrl_field,InfoField info_field,AddressField address_field);

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
#endif // AFN01F1_H
