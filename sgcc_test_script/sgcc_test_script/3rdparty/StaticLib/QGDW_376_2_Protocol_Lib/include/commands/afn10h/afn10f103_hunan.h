#ifndef AFN10F103_HUNAN_H
#define AFN10F103_HUNAN_H

#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"
namespace qgdw_3762_protocol {
/**
 * @brief The ModuleConfig10F103_Hunan 模块配置信息结构体
 */
struct ModuleConfig10F103_Hunan
{
    Address node_address_;//!<节点地址
    char config_state_;//!<配置状态
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const ModuleConfig10F103_Hunan &unit_)const
    {
        if(this->node_address_==unit_.node_address_
                &&this->config_state_==unit_.config_state_)
            return true;
        else
            return false;
    }
};
/**
 * @brief The ModuleConfigInfoUnit10F103_Hunan 模块配置信息单元结构体
 */
struct ModuleConfigInfoUnit10F103_Hunan
{
    ushort node_total_num_;//!<从节点总数量
    uchar this_node_num_;//!<本次从节点数量
    QList<ModuleConfig10F103_Hunan> node_info_list_;//!<节点信息结构QList
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const ModuleConfigInfoUnit10F103_Hunan &unit_)const
    {
        if(this->node_total_num_==unit_.node_total_num_
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
class Afn10F103_Hunan : public Frame3762Base
#else
/**
 * @brief The Afn10F103_Hunan
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT Afn10F103_Hunan : public Frame3762Base
#endif
{
public:
    /**
     * @brief Afn10F103_Hunan
     */
    Afn10F103_Hunan();
    /**
     * @brief Afn10F103_Hunan
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    Afn10F103_Hunan(CtrlField ctrl_field,InfoField info_field,AddressField address_field);
    ushort node_start_no_;//!<从节点起始序号
    uchar node_num_;//!<从节点数量


    ModuleConfigInfoUnit10F103_Hunan module_config_unit_;//!<模块配置信息数据单元
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

#endif // AFN10F103_HUNAN_H
