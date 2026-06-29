#ifndef AFN03F2_H
#define AFN03F2_H

#include <QObject>
#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"

namespace qgdw_3762_protocol {

#ifdef UNIT_TEST
class Afn03F2 : public Frame3762Base
#else
/**
 * @brief The Afn03F2
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT Afn03F2 : public Frame3762Base
#endif
{
public:
    /**
     * @brief Afn03F2
     */
    Afn03F2();
    /**
     * @brief Afn03F2
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    Afn03F2(CtrlField ctrl_field,InfoField info_field,AddressField address_field);
    /**
     * @brief The NoiseValue 噪声值结构体
     */
    struct NoiseValue
    {
        uchar noise_intensity_:4;
        uchar reserve_:4;
        /**
         * @brief operator ==
         * @param noise_field
         * @return
         */
        bool operator==(const NoiseValue &noise_field)const
        {
            if(this->reserve_==noise_field.reserve_&&this->noise_intensity_==noise_field.noise_intensity_)
                return true;
            else
                return false;
        }
    };
    NoiseValue noise_value_; //!< 噪声值

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
#endif // AFN03F2_H
