#ifndef AFNF0F3_H
#define AFNF0F3_H

#include <QObject>

#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"

namespace qgdw_3762_protocol {
/**
 * @brief The MemorySettingUnit 存储器设置单元结构体
 */
struct MemorySettingUnitF0F3
{
    char memory_type_;//!<存储器类型
    uint start_address_;//!<起始地址
    uchar data_length_;//!<数据长度
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const MemorySettingUnitF0F3 &unit_)const
    {
        if(this->memory_type_==unit_.memory_type_
                &&this->start_address_==unit_.start_address_
                &&this->data_length_==unit_.data_length_)
            return true;
        else
            return false;
    }
};

#ifdef UNIT_TEST
class AfnF0F3 : public Frame3762Base
#else
/**
 * @brief The AfnF0F3 存储器设置
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT AfnF0F3 : public Frame3762Base
#endif
{
public:
    /**
     * @brief AfnF0F3
     */
    AfnF0F3();
    /**
     * @brief AfnF0F3
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    AfnF0F3(CtrlField ctrl_field,InfoField info_field,AddressField address_field);

    MemorySettingUnitF0F3 memory_setting_unit_;//!<存储器设置单元
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
#endif // AFNF0F3_H
