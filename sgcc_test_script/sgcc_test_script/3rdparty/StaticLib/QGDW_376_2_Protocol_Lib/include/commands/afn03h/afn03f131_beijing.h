#ifndef AFN03F131_BEIJING_H
#define AFN03F131_BEIJING_H

#include <QObject>
#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"
namespace qgdw_3762_protocol {


#ifdef UNIT_TEST
class Afn03F131_Beijing : public Frame3762Base
#else
/**
 * @brief The Afn03F131_Beijing
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT Afn03F131_Beijing : public Frame3762Base
#endif
{
public:
    /**
     * @brief Afn03F131_Beijing
     */
    Afn03F131_Beijing();
    /**
     * @brief Afn03F131_Beijing
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    Afn03F131_Beijing(CtrlField ctrl_field,InfoField info_field,AddressField address_field);

    char white_list_switch_;    //!< 白名单开关
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

#endif // AFN03F131_BEIJING_H
