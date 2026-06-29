#ifndef AFNF0F11_H
#define AFNF0F11_H

#include <QObject>

#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"

namespace qgdw_3762_protocol {
/**
 * @brief The QueryNodeClusterUnit 查询节点聚类信息数据单元结构体
 */
struct QueryNodeClusterUnit
{
    char response_state_;//!<应答状态
    ushort report_node_tei_;//!<上报节点TEI
    char phase_;//!<相线
    Address report_node_address_;//!<上报节点MAC地址
    uchar report_node_meter_num_;//!<上报节点下挂电表个数
    QList<Address> mac_address_list_;//!<mac地址QList
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const QueryNodeClusterUnit &unit_)const
    {
        if(this->report_node_tei_==unit_.report_node_tei_
                &&this->response_state_==unit_.response_state_
                &&this->phase_==unit_.phase_
                &&this->report_node_address_==unit_.report_node_address_
                &&this->report_node_meter_num_==unit_.report_node_meter_num_
                &&this->mac_address_list_.size()==unit_.mac_address_list_.size())
        {
            for(int i=0;i<this->mac_address_list_.size();i++)
            {
                if(!(this->mac_address_list_.at(i)==unit_.mac_address_list_.at(i)))
                    return false;
            }
            return true;
        }
        else
            return false;
    }
};

#ifdef UNIT_TEST
class AfnF0F11 : public Frame3762Base
#else
/**
 * @brief The AfnF0F11 查询节点聚类信息
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT AfnF0F11 : public Frame3762Base
#endif
{
public:
    /**
     * @brief AfnF0F11
     */
    AfnF0F11();
    /**
     * @brief AfnF0F11
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    AfnF0F11(CtrlField ctrl_field,InfoField info_field,AddressField address_field);

    QueryNodeClusterUnit query_node_cluster_unit_;//!<查询节点聚类信息数据单元
    Address query_node_address_;//!<查询节点地址
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

#endif // AFNF0F11_H
