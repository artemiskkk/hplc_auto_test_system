#ifndef AFN10F121_H
#define AFN10F121_H

#include <QObject>

#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"
namespace qgdw_3762_protocol {
/**
 * @brief The AreaIdentifyInfo 台区识别信息结构体
 */
struct AreaIdentifyInfo
{
    Address node_address_;//!<从节点地址
    char node_identify_result_;//!<从节点识别结果
    Address cco_address_;//!<所属台区CCO地址
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const AreaIdentifyInfo &unit_)const
    {
        if(this->node_address_==unit_.node_address_
                &&this->node_identify_result_==unit_.node_identify_result_
                &&this->cco_address_==unit_.cco_address_)
            return true;
        else
            return false;
    }
};
/**
 * @brief The AreaIdentifyInfoUnit 台区识别结果数据单元结构体
 */
struct AreaIdentifyInfoUnit
{
    ushort node_total_num_;//!<从节点总数量
    ushort node_start_no_;//!<从节点起始序号
    uchar this_node_num_;//!<本次从节点数量
    QList<AreaIdentifyInfo> area_identify_info_list_;//!<台区识别信息QList
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const AreaIdentifyInfoUnit &unit_)const
    {
        if(this->node_total_num_==unit_.node_total_num_
                &&this->node_start_no_==unit_.node_start_no_
                &&this->this_node_num_==unit_.this_node_num_
                &&this->area_identify_info_list_.size()==unit_.area_identify_info_list_.size())
        {
            for(int i=0;i<this->area_identify_info_list_.size();i++)
            {
                if(!(this->area_identify_info_list_.at(i)==unit_.area_identify_info_list_.at(i)))
                    return false;
            }
            return true;
        }
        else
            return false;
    }
};

#ifdef UNIT_TEST
class Afn10F121 : public Frame3762Base
#else
/**
 * @brief The AFN10F121 查询台区识别结果
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT Afn10F121 : public Frame3762Base
#endif
{
public:
    /**
     * @brief Afn10F121
     */
    Afn10F121();
    /**
     * @brief Afn10F121
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    Afn10F121(CtrlField ctrl_field,InfoField info_field,AddressField address_field);
    ushort node_start_no_;//!<从节点起始序号
    uchar node_num_;//!<从节点数量

    AreaIdentifyInfoUnit area_identify_info_unit_;//!<台区识别结果数据单元
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
#endif // AFN10F121_H
