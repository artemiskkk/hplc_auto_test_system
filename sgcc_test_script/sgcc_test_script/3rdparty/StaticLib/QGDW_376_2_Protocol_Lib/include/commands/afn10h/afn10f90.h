#ifndef AFN10F90_H
#define AFN10F90_H

#include <QObject>

#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"
namespace qgdw_3762_protocol {
/**
 * @brief The DataUnit10F90Up
 */
struct DataUnit10F90Up
{
    Address node_address_;//!<表地址
    char timing_state_;//!<校时状态
    char preserve_=0x00;//!<备用
    uchar threshold_;//!<阈值(分钟)
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const DataUnit10F90Up &unit_)const
    {
        if(this->node_address_==unit_.node_address_
                &&this->timing_state_==unit_.timing_state_
                 &&this->preserve_==unit_.preserve_
                 &&this->threshold_==unit_.threshold_)
            return true;
        else
            return false;
    }
};
/**
 * @brief The DataUnit10F90Down
 */
struct DataUnit10F90Down
{
    Address node_address_;//!<表地址
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const DataUnit10F90Down &unit_)const
    {
        if(this->node_address_==unit_.node_address_)
            return true;
        else
            return false;
    }
};

#ifdef UNIT_TEST
class Afn10F90 : public Frame3762Base
#else
/**
 * @brief The Afn10F90 时钟管理开关/超差阈值查询
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT Afn10F90 : public Frame3762Base
#endif
{
public:
    /**
     * @brief Afn10F90
     */
    Afn10F90();
    /**
     * @brief Afn10F90
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    Afn10F90(CtrlField ctrl_field,InfoField info_field,AddressField address_field);
    DataUnit10F90Up data_unit_up_;//!<上行数据
    DataUnit10F90Down data_unit_down_;//!<下行数据
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
#endif // AFN10F90_H
