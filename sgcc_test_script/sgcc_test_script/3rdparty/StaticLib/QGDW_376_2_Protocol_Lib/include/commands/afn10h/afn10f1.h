#ifndef AFN10F1_H
#define AFN10F1_H

#include <QObject>

#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"
namespace qgdw_3762_protocol {


#ifdef UNIT_TEST
class Afn10F1 : public Frame3762Base
#else
/**
 * @brief The AFN10F1 从节点数量
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT Afn10F1 : public Frame3762Base
#endif
{
public:
    /**
     * @brief Afn10F1
     */
    Afn10F1();
    /**
     * @brief Afn10F1
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    Afn10F1(CtrlField ctrl_field,InfoField info_field,AddressField address_field);
    ushort node_total_num_;//!<从节点总数量
    ushort router_support_max_num_;//!<路由支持最大从节点数量
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
#endif // AFN10F1_H
