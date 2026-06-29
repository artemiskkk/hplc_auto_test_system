#ifndef AFN03F131_H
#define AFN03F131_H

#include <QObject>
#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"

namespace qgdw_3762_protocol {

/**
 * @brief The DataUnitDown03F131 struct
 */
struct DataUnitDown03F131
{
    char cpu_no_;//!< CPU编号
    char compare_start_address_[4];//!< 比对数据起始地址
    ushort compare_data_len_;//!< 比较数据长度
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const DataUnitDown03F131 &unit_)const
    {
        if(this->cpu_no_==unit_.cpu_no_
        &&memcmp(this->compare_start_address_,unit_.compare_start_address_,4)==0
        &&this->compare_data_len_==unit_.compare_data_len_)
        {
            return true;
        }
        else
            return false;
    }
};
#ifdef UNIT_TEST
class Afn03F131 : public Frame3762Base
#else
/**
 * @brief The Afn03F131
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT Afn03F131 : public Frame3762Base
#endif
{
public:
    /**
     * @brief Afn03F131
     */
    Afn03F131();
    /**
     * @brief Afn03F131
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    Afn03F131(CtrlField ctrl_field,InfoField info_field,AddressField address_field);

    DataUnitDown03F131 data_unit_down_;//!< 数据单元下行
    QByteArray compare_data_;//!< 返回比对数据
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

#endif // AFN03F131_H
