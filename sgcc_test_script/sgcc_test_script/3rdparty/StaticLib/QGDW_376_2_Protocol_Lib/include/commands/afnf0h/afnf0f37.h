#ifndef AFNF0F37_H
#define AFNF0F37_H

#include <QObject>

#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"

namespace qgdw_3762_protocol {


#ifdef UNIT_TEST
class AfnF0F37 : public Frame3762Base
#else
/**
 * @brief The AfnF0F37 设置频段
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT AfnF0F37 : public Frame3762Base
#endif
{
public:
    /**
     * @brief AfnF0F37
     */
    AfnF0F37();
    /**
     * @brief AfnF0F37
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    AfnF0F37(CtrlField ctrl_field,InfoField info_field,AddressField address_field);

    uchar freq_;//!<频段 宽带载波频段：0~31对应32个频段， 32~255表示保留需要支持设置0：2-12M；1：2-6M；2：0.7-3；3：1.7-3；4：1-3M；5：1-6M。
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
#endif // AFNF0F37_H
