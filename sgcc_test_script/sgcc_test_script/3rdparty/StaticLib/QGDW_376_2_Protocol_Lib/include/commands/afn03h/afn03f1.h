#ifndef AFN03F1_H
#define AFN03F1_H

#include <QObject>
#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"
namespace qgdw_3762_protocol {


#ifdef UNIT_TEST
class Afn03F1 : public Frame3762Base
#else
/**
 * @brief The AFN03F1 厂商代码和版本信息
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT Afn03F1 : public Frame3762Base
#endif
{
public:
    /**
     * @brief Afn03F1
     */
    Afn03F1();
    /**
     * @brief Afn03F1
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    Afn03F1(CtrlField ctrl_field,InfoField info_field,AddressField address_field);
    //~Afn03F1() override;

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
#endif // AFN03F1_H
