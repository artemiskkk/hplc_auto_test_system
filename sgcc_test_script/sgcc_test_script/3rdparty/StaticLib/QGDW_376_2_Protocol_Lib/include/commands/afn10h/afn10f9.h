#ifndef AFN10F9_H
#define AFN10F9_H

#include <QObject>

#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"
namespace qgdw_3762_protocol {


#ifdef UNIT_TEST
class Afn10F9 : public Frame3762Base
#else
/**
* @brief The AFN10F9 网络规模
*/
class QGDW_376_2_PROTOCOL_LIB_EXPORT Afn10F9 : public Frame3762Base
#endif
{
public:
    /**
     * @brief Afn10F9
     */
    Afn10F9();
    /**
     * @brief Afn10F9
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    Afn10F9(CtrlField ctrl_field,InfoField info_field,AddressField address_field);
    ushort network_scale_;//!<网络规模
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
#endif // AFN10F9_H
