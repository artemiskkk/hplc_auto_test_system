#ifndef AFN10F101_BEIJING_H
#define AFN10F101_BEIJING_H

#include <QObject>

#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"
#include "afn10f101.h"
namespace qgdw_3762_protocol {

/**
 * @brief The NodeInfoGroup10F101_Beijing 从节点信息组结构体
 */
struct NodeInfoGroup10F101_Beijing
{
    Address node_address_;//!<从节点地址
    NodeInfo10F101 node_info_;//!<从节点信息
    char node_software_version_[3];//!<从节点软件版本
    char bootloader_version_;//!<bootloader版本
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const NodeInfoGroup10F101_Beijing &unit_)const
    {
        if(this->node_address_==unit_.node_address_
                &&this->node_info_==unit_.node_info_
                &&memcmp(this->node_software_version_,unit_.node_software_version_,3)==0
                &&this->bootloader_version_==unit_.bootloader_version_)
            return true;
        else
            return false;
    }
};
/**
 * @brief The NodeInfoDataUnit10F101_Beijing
 */
struct NodeInfoDataUnit10F101_Beijing
{
    ushort node_total_num_;//!<从节点总数量
    uchar this_node_num_;//!<本次应答的从节点数量
    QList<NodeInfoGroup10F101_Beijing> node_info_group_list_;//!<从节点信息组QList
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const NodeInfoDataUnit10F101_Beijing &unit_)const
    {
        if(this->node_total_num_==unit_.node_total_num_
                &&this->this_node_num_==unit_.this_node_num_
                &&this->node_info_group_list_.size()==unit_.node_info_group_list_.size())
        {
            for(int i=0;i<this->node_info_group_list_.size();i++)
            {
                if(!(this->node_info_group_list_.at(i)==unit_.node_info_group_list_.at(i)))
                    return false;
            }
            return true;
        }
        else
            return false;
    }
};

#ifdef UNIT_TEST
class Afn10F101 : public Frame3762Base
#else
/**
 * @brief The Afn10F101_Beijing 查询从节点信息
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT Afn10F101_Beijing : public Frame3762Base
#endif
{
public:
    /**
     * @brief Afn10F101_Beijing
     */
    Afn10F101_Beijing();
    /**
     * @brief Afn10F101_Beijing
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    Afn10F101_Beijing(CtrlField ctrl_field,InfoField info_field,AddressField address_field);
    ushort node_start_no_;//!<从节点起始序号
    uchar node_num_;//!<从节点数量

    NodeInfoDataUnit10F101_Beijing node_info_data_unit_;//!<从节点信息数据单元
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
#endif // AFN10F101_BEIJING_H
