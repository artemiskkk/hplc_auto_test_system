#ifndef AFNF0F39_H
#define AFNF0F39_H

#include <QObject>

#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"
namespace qgdw_3762_protocol {
/**
 * @brief The ConfigChipIdUnit 配置芯片ID数据单元结构体
 */
struct ConfigChipIdUnit
{
    Address mac_address_;//!<mac地址
    char id_type_;//!<ID类型
    char id_format_;//!<模块ID格式
    uchar id_length_;//!<ID长度
    QByteArray id_content_;//!<ID数据
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const ConfigChipIdUnit &unit_)const
    {
        if(this->mac_address_==unit_.mac_address_
                &&this->id_type_==unit_.id_type_
                &&this->id_format_==unit_.id_format_
                &&this->id_length_==unit_.id_length_
                &&this->id_content_==unit_.id_content_)
            return true;
        else
            return false;
    }
};

#ifdef UNIT_TEST
class AfnF0F39 : public Frame3762Base
#else
/**
 * @brief The AFNF0F39 芯片ID配置
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT AfnF0F39 : public Frame3762Base
#endif
{
public:
    /**
     * @brief AfnF0F39
     */
    AfnF0F39();
    /**
     * @brief AfnF0F39
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    AfnF0F39(CtrlField ctrl_field,InfoField info_field,AddressField address_field);

    ConfigChipIdUnit config_chip_id_unit_;//!<配置芯片ID数据单元
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
#endif // AFNF0F39_H
