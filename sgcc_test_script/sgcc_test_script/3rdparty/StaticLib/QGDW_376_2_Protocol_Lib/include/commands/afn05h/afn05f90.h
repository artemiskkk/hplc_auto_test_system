#ifndef AFN05F90_H
#define AFN05F90_H

#include <QObject>

#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"
namespace qgdw_3762_protocol {

#ifdef UNIT_TEST
class Afn05F90 : public Frame3762Base
#else
/**
 * @brief The Afn05F90 时钟管理开关设置
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT Afn05F90 : public Frame3762Base
#endif
{
public:
    Afn05F90();
    /**
     * @brief Afn05F90
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */

    Afn05F90(CtrlField ctrl_field,InfoField info_field,AddressField address_field);
    Address node_address_;//!<表地址
    char function_flag_;//!<功能标识
    char function_state_;//!<功能状态
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

#endif // AFN05F90_H
