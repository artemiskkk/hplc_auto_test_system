#ifndef AFNF0F91_H
#define AFNF0F91_H


class AfnF0F91
{
public:
    AfnF0F91();
};
#include <QObject>

#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"

namespace qgdw_3762_protocol {


#ifdef UNIT_TEST
class AfnF0F91 : public Frame3762Base
#else
/**
 * @brief The AfnF0F91 退出连接状态
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT AfnF0F91 : public Frame3762Base
#endif
{
public:
    /**
     * @brief AfnF0F91
     */
    AfnF0F91();
    /**
     * @brief AfnF0F91
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    AfnF0F91(CtrlField ctrl_field,InfoField info_field,AddressField address_field);


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
#endif // AFNF0F91_H
