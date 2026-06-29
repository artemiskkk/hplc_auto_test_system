#ifndef AFN03F4_H
#define AFN03F4_H

#include <QObject>
#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"

namespace qgdw_3762_protocol {

#ifdef UNIT_TEST
class Afn03F4 : public Frame3762Base
#else
/**
 * @brief The Afn03F4
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT Afn03F4 : public Frame3762Base
#endif
{
public:
    /**
     * @brief Afn03F4
     */
    Afn03F4();
    /**
     * @brief Afn03F4
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    Afn03F4(CtrlField ctrl_field,InfoField info_field,AddressField address_field);

    Address master_node_address_ ;  //!<主节点地址

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
#endif // AFN03F4_H
