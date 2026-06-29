#ifndef AFN03F201_H
#define AFN03F201_H
#include <QObject>
#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"
namespace qgdw_3762_protocol {

#ifdef UNIT_TEST
class Afn03F201 : public Frame3762Base
#else
/**
 * @brief The Afn03F201 STA认证使能开启/禁止
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT Afn03F201 : public Frame3762Base
#endif
{
public:
    Afn03F201();
    /**
     * @brief Afn03F201
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */

    Afn03F201(CtrlField ctrl_field,InfoField info_field,AddressField address_field);
    char sta_attest_flag_;//!<STA认证使能标志
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
#endif // AFN03F201_H
