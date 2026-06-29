#ifndef AFN06F1_H
#define AFN06F1_H

#include <QObject>

#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"
namespace qgdw_3762_protocol {
/**
 * @brief The NodeInfo 节点信息结构体
 */
struct NodeInfo06F1
{
    Address node_address_;//!<节点地址
    char node_protocol_type_;//!<节点协议
    ushort node_no_;//!<节点序号
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const NodeInfo06F1 &unit_)const
    {
        if(this->node_address_==unit_.node_address_
                &&this->node_protocol_type_==unit_.node_protocol_type_
                &&this->node_no_==unit_.node_no_)
            return true;
        else
            return false;
    }
};
#ifdef UNIT_TEST
class Afn06F1 : public Frame3762Base
#else
/**
 * @brief The AFN06F1 上报从节点信息
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT Afn06F1 : public Frame3762Base
#endif
{
public:
    /**
     * @brief Afn06F1
     */
    Afn06F1();
    /**
     * @brief Afn06F1
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    Afn06F1(CtrlField ctrl_field,InfoField info_field,AddressField address_field);

    uchar report_node_num_;//!<上报从节点的数量n
    QList<NodeInfo06F1> node_info_list_;//!<节点信息QList
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
#endif // AFN06F1_H
