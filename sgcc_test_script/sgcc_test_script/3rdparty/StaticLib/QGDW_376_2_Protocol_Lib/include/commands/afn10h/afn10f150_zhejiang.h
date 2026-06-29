#ifndef AFN10F150_ZHEJIANG_H
#define AFN10F150_ZHEJIANG_H

#include <QObject>
#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"
namespace qgdw_3762_protocol {
/**
 * @brief The NodeInfo10F150 节点信息结构体
 */
struct NodeInfo10F150
{
    Address slaved_address_;//!<隶属节点地址
    Address node_address_;//!<节点地址
    char protocol_type_;//!<通信协议类型
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const NodeInfo10F150 &unit_)const
    {
        if(this->slaved_address_==unit_.slaved_address_
                &&this->node_address_==unit_.node_address_
                 &&this->protocol_type_==unit_.protocol_type_)
            return true;
        else
            return false;
    }
    /**
     * @brief operator =  重载运算符“=”
     * @param NodeInfo10F150
     * @return 结构体赋值
     */
    NodeInfo10F150& operator=(const NodeInfo10F150 &node_)
    {
        this->slaved_address_=node_.slaved_address_;
        this->node_address_=node_.node_address_;
        this->protocol_type_=node_.protocol_type_;
        return *this;
    }
};
/**
 * @brief The NodeInfoDataUnit10F150 从节点信息数据单元结构体
 */
struct NodeInfoDataUnit10F150
{
    ushort node_total_num_;//!<从节点总数量
    ushort this_start_no_;//!<起始序号
    uchar this_node_num_;//本次应答数量
    QList<NodeInfo10F150> node_info_list_;//!<从节点信息组QList
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const NodeInfoDataUnit10F150 &unit_)const
    {
        if(this->node_total_num_==unit_.node_total_num_
                &&this->this_start_no_==unit_.this_start_no_
                &&this->this_node_num_==unit_.this_node_num_
                &&this->node_info_list_.size()==unit_.node_info_list_.size())
        {
            for(int i=0;i<this->node_info_list_.size();i++)
            {
                if(!(this->node_info_list_.at(i)==unit_.node_info_list_.at(i)))
                    return false;
            }
            return true;
        }
        else
            return false;
    }
};

#ifdef UNIT_TEST
class Afn10F150_Zhejiang : public Frame3762Base
#else
/**
* @brief The Afn10F150_Zhejiang
*/
class QGDW_376_2_PROTOCOL_LIB_EXPORT Afn10F150_Zhejiang : public Frame3762Base
#endif
{
public:
    /**
     * @brief Afn10F150_Zhejiang
     */
    Afn10F150_Zhejiang();
    /**
     * @brief Afn10F150_Zhejiang
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    Afn10F150_Zhejiang(CtrlField ctrl_field,InfoField info_field,AddressField address_field);
    ushort node_start_no_;//!<从节点起始序号
    uchar node_num_;//!<从节点数量

    NodeInfoDataUnit10F150 node_info_data_unit_;//!<从节点信息数据单元
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
#endif // AFN10F150_ZHEJIANG_H
