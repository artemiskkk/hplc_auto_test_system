#ifndef AFN05F133_H
#define AFN05F133_H

#include <QObject>

#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"
namespace qgdw_3762_protocol {
/**
 * @brief The RouterWorkModeUnit 工作模式数据单元结构体
 */
struct RouterWorkModeUnit
{
    uchar power_off_report_:1;//!<停电上报
    uchar power_on_report_:1;//!<上电上报
    uchar clock_syn_:1;//!<时钟同步
    uchar loop_impedance_:1;//!<回路阻抗
    uchar change_09_to_13_:1;//!<09转13
    uchar electricity_steal_prevention_:1;//!<防窃电
    uchar reverse_:2;//!<保留

    char reverse_1_;//!<保留
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const RouterWorkModeUnit &unit_)const
    {
        if(this->power_off_report_==unit_.power_off_report_
                &&this->power_on_report_==unit_.power_on_report_
                &&this->clock_syn_==unit_.clock_syn_
                &&this->loop_impedance_==unit_.loop_impedance_
                &&this->change_09_to_13_==unit_.change_09_to_13_
                &&this->electricity_steal_prevention_==unit_.electricity_steal_prevention_
                &&this->reverse_==unit_.reverse_
                &&this->reverse_1_==unit_.reverse_1_)
            return true;
        else
            return false;
    }

};

#ifdef UNIT_TEST
class Afn05F133 : public Frame3762Base
#else
/**
 * @brief The AFN05F133 路由工作模式设置
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT Afn05F133 : public Frame3762Base
#endif
{
public:
    /**
     * @brief Afn05F133
     */
    Afn05F133();
    /**
     * @brief Afn05F133
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    Afn05F133(CtrlField ctrl_field,InfoField info_field,AddressField address_field);

    RouterWorkModeUnit router_work_mode_unit_;//!<工作模式数据单元
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

#endif // AFN05F133_H
