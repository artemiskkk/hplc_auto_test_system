#ifndef AFN01F2_H
#define AFN01F2_H

#include <QObject>
#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"
namespace qgdw_3762_protocol {
#ifdef UNIT_TEST
class Afn01F2 : public Frame3762Base
#else
/**
 * @brief The Afn01F2 参数区初始化
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT Afn01F2 : public Frame3762Base
#endif
{
public:
    Afn01F2();
    Afn01F2(CtrlField ctrl_field,InfoField info_field,AddressField address_field);

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
#endif // AFN01F2_H
