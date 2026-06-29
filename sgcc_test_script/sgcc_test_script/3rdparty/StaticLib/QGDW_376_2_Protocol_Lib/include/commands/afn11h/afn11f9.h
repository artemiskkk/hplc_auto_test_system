#ifndef AFN11F9_H
#define AFN11F9_H

#include <QObject>

#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"
namespace qgdw_3762_protocol {


#ifdef UNIT_TEST
class Afn11F9 : public Frame3762Base
#else
/**
 * @brief The AFN11F9 设置附属从节点绑定关系
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT Afn11F9 : public Frame3762Base
#endif
{
public:
    /**
     * @brief Afn11F9
     */
    Afn11F9();
    /**
     * @brief Afn11F9
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    Afn11F9(CtrlField ctrl_field,InfoField info_field,AddressField address_field);

    Address node_address_;//!<从节点地址
    uchar subsidiary_node_num_;//!<绑定附属从节点数量
    QList<Address> subsidiary_node_address_list_;//附属从节点地址QList
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
#endif // AFN11F9_H
