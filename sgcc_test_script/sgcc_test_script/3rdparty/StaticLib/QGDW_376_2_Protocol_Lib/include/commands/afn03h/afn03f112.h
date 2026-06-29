#ifndef AFN03F112_H
#define AFN03F112_H

#include <QObject>
#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"

namespace qgdw_3762_protocol {
/**
 * @brief The NodeInfo03F112 节点信息结构体
 */
struct NodeInfo03F112
{
    Address node_address_;//!< 节点地址
    char node_type_;//!< 节点设备类型
    char node_chip_id_info_[24];//!< 节点芯片ID信息
    char node_chip_software_version_[2];//!< 节点芯片软件版本
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const NodeInfo03F112 &unit_)const
    {
        if(this->node_address_==unit_.node_address_
                &&this->node_type_==unit_.node_type_
                &&memcmp(this->node_chip_id_info_,unit_.node_chip_id_info_,24)==0
                &&memcmp(this->node_chip_software_version_,unit_.node_chip_software_version_,2)==0)
        {
            return true;
        }
        else
            return false;
    }
};
/**
 * @brief The CarriarChipInfoUnit 载波芯片信息单元结构体
 */
struct CarriarChipInfoUnit
{
    ushort node_total_num_;//!< 节点总数量
    ushort start_node_no_;//!< 节点起始序号
    uchar response_node_num_;//!< 应答的节点数量n
    QList<NodeInfo03F112> node_info_list;//!< 节点信息QList
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const CarriarChipInfoUnit &unit_)const
    {
        if(this->node_total_num_==unit_.node_total_num_
                &&this->start_node_no_==unit_.start_node_no_
                &&this->response_node_num_==unit_.response_node_num_
                &&this->node_info_list.size()==unit_.node_info_list.size())
        {
            for(int i=0;i<this->node_info_list.size();i++)
            {
                if(!(this->node_info_list.at(i)==unit_.node_info_list.at(i)))
                    return false;
            }
            return true;
        }
        else
            return false;
    }
};

#ifdef UNIT_TEST
class Afn03F112 : public Frame3762Base
#else
/**
 * @brief The Afn03F112
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT Afn03F112 : public Frame3762Base
#endif
{
public:
    /**
     * @brief Afn03F112
     */
    Afn03F112();
    /**
     * @brief Afn03F112
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    Afn03F112(CtrlField ctrl_field,InfoField info_field,AddressField address_field);

    ushort start_node_no_;//!< 节点起始序号
    uchar node_num_;//!< 节点数量n

    CarriarChipInfoUnit carriar_chip_info_unit_;//!< 载波芯片信息单元
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
#endif // AFN03F112_H
