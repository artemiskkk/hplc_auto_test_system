#ifndef AFN11F127_BEIJING_H
#define AFN11F127_BEIJING_H

#include <QObject>

#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"
namespace qgdw_3762_protocol {

/**
 * @brief The NodeAreaBelongParameter
 */
struct NodeAreaBelongParameter
{
    Address mac_address_;//!<归属台区CCO
    char area_belong_flag_;//!<归属标识
    char preserve_;//!<保留
    Address node_address_;//!<从节点地址

    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const NodeAreaBelongParameter &unit_) const
    {
        if(this->mac_address_ == unit_.mac_address_
                &&this->area_belong_flag_ == unit_.area_belong_flag_
                && this->node_address_ == unit_.node_address_
                && this->preserve_ == unit_.preserve_)
            return true;
        else
            return false;
    }
};
/**
 * @brief The NodeAreaBelongUnit
 */
struct NodeAreaBelongUnit
{
    char operate_type_;//!<操作类型
    uchar data_len_;//!<数据长度
    char preserve_[2];//!<保留
    QList<NodeAreaBelongParameter> node_area_belong_list_;
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const NodeAreaBelongUnit &unit_) const
    {
        if(this->operate_type_ == unit_.operate_type_
                && this->data_len_ == unit_.data_len_
                && memcmp(this->preserve_,unit_.preserve_,2)==0
                && this->node_area_belong_list_.size() == unit_.node_area_belong_list_.size())
        {
            for(int i=0;i<this->node_area_belong_list_.size();i++)
            {
                if(!(this->node_area_belong_list_.at(i)==unit_.node_area_belong_list_.at(i)))
                    return false;
            }
            return true;
        }
        else
            return false;
    }
};
#ifdef UNIT_TEST
class Afn11F127_Beijing : public Frame3762Base
#else
/**
 * @brief The Afn11F127_Beijing
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT Afn11F127_Beijing : public Frame3762Base
#endif
{
public:
    /**
     * @brief Afn11F127_Beijing
     */
    Afn11F127_Beijing();
    /**
     * @brief Afn11F127_Beijing
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    Afn11F127_Beijing(CtrlField ctrl_field,InfoField info_field,AddressField address_field);

    NodeAreaBelongUnit data_unit_;//!<数据单元
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
#endif // AFN11F127_BEIJING_H
