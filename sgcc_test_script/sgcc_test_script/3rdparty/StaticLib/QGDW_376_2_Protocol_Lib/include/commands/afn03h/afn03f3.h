#ifndef AFN03F3_H
#define AFN03F3_H

#include <QObject>
#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"

namespace qgdw_3762_protocol {
/**
 * @brief The ListenNodeInfo 侦听节点信息结构体
 */
struct ListenNodeInfo
{
    Address node_address_;  //!< 开始节点指针
    uchar relay_level_:4;    //!< 中继级别
    uchar listen_signal_quality_:4;  //!< 侦听信号品质
    uchar listen_times_:5;   //!< 侦听次数
    uchar reverse_:3;    //!< 保留
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const ListenNodeInfo &unit_)const
    {
        if(this->node_address_==unit_.node_address_
                &&this->relay_level_==unit_.relay_level_
                &&this->listen_signal_quality_==unit_.listen_signal_quality_
                &&this->listen_times_==unit_.listen_times_
                &&this->reverse_==unit_.reverse_)
        {
            return true;
        }
        else
            return false;
    }
};

#ifdef UNIT_TEST
class Afn03F3 : public Frame3762Base
#else
/**
 * @brief The Afn03F3
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT Afn03F3 : public Frame3762Base
#endif
{
public:
    /**
     * @brief Afn03F3
     */
    Afn03F3();
    /**
     * @brief Afn03F3
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    Afn03F3(CtrlField ctrl_field,InfoField info_field,AddressField address_field);

    uchar start_node_pointer_; //!< 开始节点指针
    uchar read_node_num_; //!< 读取节点的数量

    uchar listen_node_total_num_; //!<侦听节点总数量
    uchar listen_this_frame_node_total_num_; //!<侦听到本帧节点总数量

    QList<ListenNodeInfo> listen_node_Info_list_;  //!< 侦听节点信息QList
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
#endif // AFN03F3_H
