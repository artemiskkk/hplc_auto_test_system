#ifndef AFNF0F51_H
#define AFNF0F51_H

#include <QObject>

#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"
namespace qgdw_3762_protocol {

#ifdef UNIT_TEST
class AfnF0F51 : public Frame3762Base
#else
/**
 * @brief The AfnF0F51
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT AfnF0F51 : public Frame3762Base
#endif
{
public:
    /**
     * @brief AfnF0F51
     */
    AfnF0F51();
    /**
     * @brief AfnF0F51
     * @param ctrl_field
     * @param info_field
     * @param address_field
     */
    AfnF0F51(CtrlField ctrl_field,InfoField info_field,AddressField address_field);
    char frequence_division_;//!<分频系数
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
#endif // AFNF0F51_H
