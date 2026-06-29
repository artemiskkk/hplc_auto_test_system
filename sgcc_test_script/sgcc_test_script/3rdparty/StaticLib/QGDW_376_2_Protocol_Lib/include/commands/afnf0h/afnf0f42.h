#ifndef AFNF0F42_H
#define AFNF0F42_H

#include <QObject>

#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"
namespace qgdw_3762_protocol {


#ifdef UNIT_TEST
class AfnF0F42 : public Frame3762Base
#else
/**
 * @brief The AFNF0F42 设置生产序列号
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT AfnF0F42 : public Frame3762Base
#endif
{
public:
    /**
     * @brief AfnF0F42
     */
    AfnF0F42();
    /**
     * @brief AfnF0F42
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    AfnF0F42(CtrlField ctrl_field,InfoField info_field,AddressField address_field);
    char device_type_;//!<设备类型
    Address mac_address_;//!<mac地址
    QByteArray sn_content_;//!<SN数据
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


#endif // AFNF0F42_H
