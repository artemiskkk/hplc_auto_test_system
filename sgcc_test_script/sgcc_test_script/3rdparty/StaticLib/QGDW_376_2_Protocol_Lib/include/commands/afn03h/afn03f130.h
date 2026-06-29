#ifndef AFN03F130_H
#define AFN03F130_H

#include <QObject>
#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"

namespace qgdw_3762_protocol {

/**
 * @brief The DataUnitUp03F130 struct
 */
struct DataUnitUp03F130
{
    char cpu_no_;//!< CPU编号
    char vendor_code_[2];//!< 厂商代码
    char version_date_[3];//!< 版本日期
    char hardware_version_[2];//!< 硬件版本
    char software_version_[2];//!< 软件版本
    uchar mcu_type_len_;//!<mcu型号长度
    QByteArray mcu_type_;//!<mcu型号
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const DataUnitUp03F130 &unit_)const
    {
        if(this->cpu_no_==unit_.cpu_no_
        &&memcmp(this->vendor_code_,unit_.vendor_code_,2)==0
        &&memcmp(this->version_date_,unit_.version_date_,3)==0
        &&memcmp(this->hardware_version_,unit_.hardware_version_,2)==0
        &&memcmp(this->software_version_,unit_.software_version_,2)==0
        &&this->mcu_type_len_==unit_.mcu_type_len_
        &&this->mcu_type_==unit_.mcu_type_)
        {
            return true;
        }
        else
            return false;
    }
};
#ifdef UNIT_TEST
class Afn03F130 : public Frame3762Base
#else
/**
 * @brief The Afn03F130
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT Afn03F130 : public Frame3762Base
#endif
{
public:
    /**
     * @brief Afn03F130
     */
    Afn03F130();
    /**
     * @brief Afn03F130
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    Afn03F130(CtrlField ctrl_field,InfoField info_field,AddressField address_field);

    DataUnitUp03F130 data_unit_up_;//!< 数据单元上行
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

#endif // AFN03F130_H
