#ifndef AFN11F126_BEIJING_H
#define AFN11F126_BEIJING_H

#include <QObject>

#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"
namespace qgdw_3762_protocol {

/**
 * @brief The NodeNetLockParameter
 */
struct NodeNetLockParameter
{
    char lock_or_unlock_flag_;//!<锁定或解锁标识
    char preserve_;//!<保留
    Address node_address_;//!<从节点地址

    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const NodeNetLockParameter &unit_) const
    {
        if(this->lock_or_unlock_flag_ == unit_.lock_or_unlock_flag_
                && this->node_address_ == unit_.node_address_
                && this->preserve_ == unit_.preserve_)
            return true;
        else
            return false;
    }
};
/**
 * @brief The NodeNetLockUnit
 */
struct NodeNetLockUnit
{
    char operate_type_;//!<操作类型
    uchar data_len_;//!<数据长度
    char preserve_[2];//!<保留
    QList<NodeNetLockParameter> node_net_lock_list_;
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const NodeNetLockUnit &unit_) const
    {
        if(this->operate_type_ == unit_.operate_type_
                && this->data_len_ == unit_.data_len_
                && memcmp(this->preserve_,unit_.preserve_,2)==0
                && this->node_net_lock_list_.size() == unit_.node_net_lock_list_.size())
        {
            for(int i=0;i<this->node_net_lock_list_.size();i++)
            {
                if(!(this->node_net_lock_list_.at(i)==unit_.node_net_lock_list_.at(i)))
                    return false;
            }
            return true;
        }
        else
            return false;
    }
};
#ifdef UNIT_TEST
class Afn11F126_Beijing : public Frame3762Base
#else
/**
 * @brief The Afn11F126_Beijing
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT Afn11F126_Beijing : public Frame3762Base
#endif
{
public:
    /**
     * @brief Afn11F126_Beijing
     */
    Afn11F126_Beijing();
    /**
     * @brief Afn11F126_Beijing
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    Afn11F126_Beijing(CtrlField ctrl_field,InfoField info_field,AddressField address_field);

    NodeNetLockUnit data_unit_;//!<数据单元
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
#endif // AFN11F126_BEIJING_H
