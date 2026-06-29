#ifndef AFNF0F88_H
#define AFNF0F88_H

#include <QObject>

#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"

namespace qgdw_3762_protocol {
#ifdef UNIT_TEST
class AfnF0F88 : public Frame3762Base
#else
/**
 * @brief The AfnF0F88 抄控器扩展-南网测试命令-透明转发模式
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT AfnF0F88 : public Frame3762Base
#endif
{
public:
    /**
     * @brief AfnF0F88
     */
    AfnF0F88();
    /**
     * @brief AfnF0F88
     * @param ctrl_field
     * @param info_field
     * @param address_field
     */
    AfnF0F88(CtrlField ctrl_field,InfoField info_field,AddressField address_field);

    // Frame3762Base interface
    uchar freq_; //<! 测试频段
public:
    void DecodeFrameDataField(QByteArray data) override;

protected:
    QByteArray EncodeFrameDataField() override;
};
}
#endif // AFNF0F88_H
