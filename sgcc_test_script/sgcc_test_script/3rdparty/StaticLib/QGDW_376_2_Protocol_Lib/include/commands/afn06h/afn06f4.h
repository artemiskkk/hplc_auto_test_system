#ifndef AFN06F4_H
#define AFN06F4_H

#include <QObject>

#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"
namespace qgdw_3762_protocol {
/**
 * @brief The NodeInfo 节点信息结构体
 */
struct NodeInfo06F4
{
    Address node_address_;//!<从节点地址
    char node_protocol_;//!<从节点协议
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const NodeInfo06F4 &unit_)const
    {
        if(this->node_address_==unit_.node_address_
                &&this->node_protocol_==unit_.node_protocol_)
            return true;
        else
            return false;
    }
};
/**
 * @brief The ReportNodeInfoAndDataUnit 上报从节点信息及设备类型数据单元
 */
struct ReportNodeInfoAndDataUnit
{
    uchar report_node_num_;//!<上报从节点的数量n
    Address report_node_address_;//!<从节点1通信地址
    char report_node_protocol_;//!<从节点1通信协议类型
    ushort report_node_no_;//!<从节点1序号
    char report_node_device_type_;//!<从节点1设备类型
    uchar attached_node_num_;//!<从节点1下接从节点数量M
    uchar transmit_node_num_;//!<本报文传输的节点数量m
    QList<NodeInfo06F4> node_info_list_;//!<从节点地址QList
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const ReportNodeInfoAndDataUnit &unit_)const
    {
        if(this->report_node_num_==unit_.report_node_num_
                &&this->report_node_address_==unit_.report_node_address_
                &&this->report_node_protocol_==unit_.report_node_protocol_
                &&this->report_node_no_==unit_.report_node_no_
                &&this->report_node_device_type_==unit_.report_node_device_type_
                &&this->attached_node_num_==unit_.attached_node_num_
                &&this->transmit_node_num_==unit_.transmit_node_num_
                &&this->node_info_list_.size()==unit_.node_info_list_.size())
        {
            for(int i=0;i<this->node_info_list_.size();i++)
            {
                if(!(this->node_info_list_.at(i)==unit_.node_info_list_.at(i)))
                {
                    return false;
                }
            }
            return true;
        }
        else
            return false;
    }
};

#ifdef UNIT_TEST
class Afn06F4 : public Frame3762Base
#else
/**
 * @brief The AFN06F4 上报从节点信息及设备类型
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT Afn06F4 : public Frame3762Base
#endif
{
public:
    /**
     * @brief Afn06F4
     */
    Afn06F4();
    /**
     * @brief Afn06F4
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    Afn06F4(CtrlField ctrl_field,InfoField info_field,AddressField address_field);

    ReportNodeInfoAndDataUnit report_node_info_unit_;//!<上报从节点信息及设备类型数据单元
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
#endif // AFN06F4_H
