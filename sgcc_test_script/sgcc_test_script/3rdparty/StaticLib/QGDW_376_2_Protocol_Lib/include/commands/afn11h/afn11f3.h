#ifndef AFN11F3_H
#define AFN11F3_H

#include <QObject>

#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"
namespace qgdw_3762_protocol {


#ifdef UNIT_TEST
class Afn11F3 : public Frame3762Base
#else
/**
 * @brief The AFN11F3 设置从节点固定中继路径
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT Afn11F3 : public Frame3762Base
#endif
{
public:
    /**
     * @brief Afn11F3
     */
    Afn11F3();
    /**
     * @brief Afn11F3
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    Afn11F3(CtrlField ctrl_field,InfoField info_field,AddressField address_field);

    Address node_address_;//!<从节点地址
    uchar relay_level_;//!<中级级别
    QList<Address> relay_node_address_list_;//中继从节点地址QList
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
#endif // AFN11F3_H
