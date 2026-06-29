#ifndef AFN11F1_H
#define AFN11F1_H

#include <QObject>

#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"
namespace qgdw_3762_protocol {
/**
 * @brief The NodeParameter 从节点参数
 */
struct NodeParameter
{
    Address node_address_;//!<从节点地址
    char protocol_type_;//!<协议类型
};

#ifdef UNIT_TEST
class Afn11F1 : public Frame3762Base
#else
/**
 * @brief The AFN11F1 添加从节点
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT Afn11F1 : public Frame3762Base
#endif
{
public:
    /**
     * @brief Afn11F1
     */
    Afn11F1();
    /**
     * @brief Afn11F1
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    Afn11F1(CtrlField ctrl_field,InfoField info_field,AddressField address_field);

    uchar node_num_;//!<从节点数量
    QList<NodeParameter> node_parameter_list_;//从节点参数QList
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
#endif // AFN11F1_H
