#ifndef AFN10F112_H
#define AFN10F112_H

#include <QObject>

#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"
namespace qgdw_3762_protocol {
/**
 * @brief The NodeChipInfo 节点芯片信息结构体
 */
struct NodeChipInfo
{
    Address node_address_;//!<从节点地址
    char node_device_type_;//!<从节点设备类型
    QByteArray node_chip_id_;//从节点芯片ID
    char node_software_version_[2];//从芯片软件版本信息
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const NodeChipInfo &unit_)const
    {
        if(this->node_address_==unit_.node_address_
                &&this->node_device_type_==unit_.node_device_type_
                &&this->node_chip_id_==unit_.node_chip_id_
                &&memcmp(this->node_software_version_,unit_.node_software_version_,2)==0)
            return true;
        else
            return false;
    }
};
/**
 * @brief The NodeChipInfoUnit 载波芯片信息数据单元结构体
 */
struct NodeChipInfoUnit
{
    ushort node_total_num_;//!<从节点总数量
    ushort node_start_no_;//!<从节点起始序号
    uchar this_node_num_;//!<本次从节点数量
    QList<NodeChipInfo> node_chip_info_list_;//!<节点芯片信息结构QList
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const NodeChipInfoUnit &unit_)const
    {
        if(this->node_total_num_==unit_.node_total_num_
                &&this->node_start_no_==unit_.node_start_no_
                &&this->this_node_num_==unit_.this_node_num_
                &&this->node_chip_info_list_.size()==unit_.node_chip_info_list_.size())
        {
            for(int i=0;i<this->node_chip_info_list_.size();i++)
            {
                if(!(this->node_chip_info_list_.at(i)==unit_.node_chip_info_list_.at(i)))
                    return false;
            }
            return true;
        }
        else
            return false;
    }
};

#ifdef UNIT_TEST
class Afn10F112 : public Frame3762Base
#else
/**
 * @brief The AFN10F112 查询宽带载波芯片信息
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT Afn10F112 : public Frame3762Base
#endif
{
public:
    /**
     * @brief Afn10F112
     */
    Afn10F112();
    /**
     * @brief Afn10F112
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    Afn10F112(CtrlField ctrl_field,InfoField info_field,AddressField address_field);
    ushort node_start_no_;//!<从节点起始序号
    uchar node_num_;//!<从节点数量

    NodeChipInfoUnit node_chip_info_unit_;//!<芯片信息数据单元
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
#endif // AFN10F112_H
