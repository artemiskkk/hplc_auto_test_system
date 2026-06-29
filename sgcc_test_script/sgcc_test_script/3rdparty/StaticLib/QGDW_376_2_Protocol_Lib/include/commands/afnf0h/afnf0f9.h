#ifndef AFNF0F9_H
#define AFNF0F9_H

#include <QObject>

#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"

namespace qgdw_3762_protocol {
/**
 * @brief The QueryNodeProxyUnit 查询节点代理主路径数据单元结构体
 */
struct QueryNodeProxyUnit
{
    ushort report_node_tei_;//!<上报节点TEI
    uchar report_node_level_;//!<上报节点级数
    char report_node_phase_;//!<上报节点相位
    QList<Address> mac_address_list_;//!<mac地址QList
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const QueryNodeProxyUnit &unit_)const
    {
        if(this->report_node_tei_==unit_.report_node_tei_
                &&this->report_node_level_==unit_.report_node_level_
                &&this->report_node_phase_==unit_.report_node_phase_
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
class AfnF0F9 : public Frame3762Base
#else
/**
 * @brief The AfnF0F9 查询节点代理主路径
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT AfnF0F9 : public Frame3762Base
#endif
{
public:
    /**
     * @brief AfnF0F9
     */
    AfnF0F9();
    /**
     * @brief AfnF0F9
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    AfnF0F9(CtrlField ctrl_field,InfoField info_field,AddressField address_field);

    QueryNodeProxyUnit query_node_proxy_unit_;//!<查询节点代理主路径数据单元
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
#endif // AFNF0F9_H
