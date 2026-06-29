#ifndef AFNF0F92_H
#define AFNF0F92_H

#include <QObject>

#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"

namespace qgdw_3762_protocol {


#ifdef UNIT_TEST
class AfnF0F92 : public Frame3762Base
#else
/**
 * @brief The AfnF0F92 启动搜索帧
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT AfnF0F92 : public Frame3762Base
#endif
{
public:
    /**
     * @brief AfnF0F92
     */
    AfnF0F92();
    /**
     * @brief AfnF0F92
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    AfnF0F92(CtrlField ctrl_field,InfoField info_field,AddressField address_field);

 QByteArray address_;//连接对象地址
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
#endif // AFNF0F92_H
