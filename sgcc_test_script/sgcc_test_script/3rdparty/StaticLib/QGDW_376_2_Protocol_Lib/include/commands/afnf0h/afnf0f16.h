#ifndef AFNF0F16_H
#define AFNF0F16_H

#include <QObject>

#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"

namespace qgdw_3762_protocol {
/**
 * @brief The QueryMeterBoxUnit 查询表箱聚类数据单元结构体
 */
struct QueryMeterBoxUnit
{
    char report_meter_box_role_;//!<上报节点表箱角色
    Address report_node_address_;//!<上报节点MAC地址
    ushort report_meter_box_id_;//!<上报节点表箱ID
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const QueryMeterBoxUnit &unit_)const
    {
        if(this->report_meter_box_role_==unit_.report_meter_box_role_
                &&this->report_node_address_==unit_.report_node_address_
                &&this->report_meter_box_id_==unit_.report_meter_box_id_)
            return true;
        else
            return false;
    }
};

#ifdef UNIT_TEST
class AfnF0F16 : public Frame3762Base
#else
/**
 * @brief The AfnF0F16 查询表箱聚类信息
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT AfnF0F16 : public Frame3762Base
#endif
{
public:
    /**
     * @brief AfnF0F16
     */
    AfnF0F16();
    /**
     * @brief AfnF0F16
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    AfnF0F16(CtrlField ctrl_field,InfoField info_field,AddressField address_field);

    QueryMeterBoxUnit query_meter_box_unit_;//!<查询表箱聚类数据单元
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

#endif // AFNF0F16_H
