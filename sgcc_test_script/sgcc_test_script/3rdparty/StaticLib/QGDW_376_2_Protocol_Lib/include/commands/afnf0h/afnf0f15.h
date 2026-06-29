#ifndef AFNF0F15_H
#define AFNF0F15_H

#include <QObject>

#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"
namespace qgdw_3762_protocol {


#ifdef UNIT_TEST
class AfnF0F15 : public Frame3762Base
#else
/**
 * @brief The AFNF0F15 路由版本信息查询
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT AfnF0F15 : public Frame3762Base
#endif
{
public:
    /**
     * @brief AfnF0F15
     */
    AfnF0F15();
    /**
     * @brief AfnF0F15
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    AfnF0F15(CtrlField ctrl_field,InfoField info_field,AddressField address_field);

    QString vendor_code_;    //!< 厂商代码,例“TC"
    QString chip_code_;      //!< 芯片代码,例“R5"
    QString version_time_;   //!< 版本时间,年月日,例“200516"
    QString version_;        //!< 版本,例“0388"
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
#endif // AFNF0F15_H
