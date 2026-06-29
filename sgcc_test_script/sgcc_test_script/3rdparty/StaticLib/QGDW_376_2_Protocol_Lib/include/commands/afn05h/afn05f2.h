#ifndef AFN05F2_H
#define AFN05F2_H

#include <QObject>

#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"
namespace qgdw_3762_protocol {

#ifdef UNIT_TEST
class Afn05F2 : public Frame3762Base
#else
/**
 * @brief The AFN05F2 允许/禁止载波从节点上报
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT Afn05F2 : public Frame3762Base
#endif
{
public:
    /**
     * @brief Afn05F2
     */
    Afn05F2();
    /**
     * @brief Afn05F2
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    Afn05F2(CtrlField ctrl_field,InfoField info_field,AddressField address_field);
    char event_report_state_flag_;//!<事件上报状态标志
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
#endif // AFN05F2_H
