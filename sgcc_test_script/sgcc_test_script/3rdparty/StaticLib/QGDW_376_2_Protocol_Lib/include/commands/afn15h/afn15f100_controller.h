#ifndef AFN15F100_CONTROLLER_H
#define AFN15F100_CONTROLLER_H

#include <QObject>
#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"
namespace qgdw_3762_protocol {

#ifdef UNIT_TEST
class Afn15F100_Controller : public Frame3762Base
#else
/**
 * @brief The Afn15F100_Controller 抄控器启动升级
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT Afn15F100_Controller : public Frame3762Base
#endif
{
public:
    Afn15F100_Controller();
    /**
     * @brief Afn05F1
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */

    Afn15F100_Controller(CtrlField ctrl_field,InfoField info_field,AddressField address_field);

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
#endif // AFN15F100_CONTROLLER_H
