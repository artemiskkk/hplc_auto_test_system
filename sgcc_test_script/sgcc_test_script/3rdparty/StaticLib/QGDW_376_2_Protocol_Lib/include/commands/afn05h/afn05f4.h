#ifndef AFN05F4_H
#define AFN05F4_H

#include <QObject>

#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"
namespace qgdw_3762_protocol {

#ifdef UNIT_TEST
class Afn05F4 : public Frame3762Base
#else
/**
 * @brief The AFN05F4 设置从节点监控最大超时时间
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT Afn05F4 : public Frame3762Base
#endif
{
public:
    /**
     * @brief Afn05F4
     */
    Afn05F4();
    /**
     * @brief Afn05F4
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    Afn05F4(CtrlField ctrl_field,InfoField info_field,AddressField address_field);
    uchar max_monitor_time_;//!<最大监控时间
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
#endif // AFN05F4_H
