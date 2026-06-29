#ifndef AFNF0F1_H
#define AFNF0F1_H

#include <QObject>

#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"

namespace qgdw_3762_protocol {
/**
 * @brief The ConfigRouterChipIdUnit 配置路由芯片ID数据单元结构体
 */
struct ConfigRouterChipIdUnit
{
    QByteArray id_content_;//!<ID数据
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const ConfigRouterChipIdUnit &unit_)const
    {
        if(this->id_content_==unit_.id_content_)
            return true;
        else
            return false;
    }
};

#ifdef UNIT_TEST
class AfnF0F1 : public Frame3762Base
#else
/**
 * @brief The AfnF0F1 芯片ID配置
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT AfnF0F1 : public Frame3762Base
#endif
{
public:
    /**
     * @brief AfnF0F1
     */
    AfnF0F1();
    /**
     * @brief AfnF0F1
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    AfnF0F1(CtrlField ctrl_field,InfoField info_field,AddressField address_field);

    ConfigRouterChipIdUnit config_chip_id_unit_;//!<配置芯片ID数据单元
public:
    /**
     * @brief 将16进制格式数据，解析为数据单元
     * @param data
     */
    void DecodeFrameDataField(QByteArray data) override;

    /**
     * @brief 将数据单元编码为16进制格式数据
     * @return 反回编码好的字节串
     */
    QByteArray EncodeFrameDataField() override;
private:
//    /**
//     * @brief 将数据单元编码为16进制格式数据
//     * @return 反回编码好的字节串
//     */
//    QByteArray EncodeFrameDataField() override;
};
}
#endif // AFNF0F1_H
