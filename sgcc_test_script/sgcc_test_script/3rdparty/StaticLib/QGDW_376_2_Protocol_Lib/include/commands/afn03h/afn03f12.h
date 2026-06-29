#ifndef AFN03F12_H
#define AFN03F12_H

#include <QObject>
#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"

namespace qgdw_3762_protocol {
/**
 * @brief The LocalModuleIDDataUnit 本地通信模块ID信息数据单元
 */
struct LocalModuleIdDataUnit
{
    QString module_vendor_code_;    //!< 模块厂商代码,例“TC"
    uchar module_id_length;//!< 模块ID长度
    char module_id_format;//!< 模块ID格式
    QByteArray module_id_content;//!< 模块ID
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const LocalModuleIdDataUnit &unit_)const
    {
        if(this->module_vendor_code_==unit_.module_vendor_code_
                &&this->module_id_length==unit_.module_id_length
                &&this->module_id_format==unit_.module_id_format
                &&this->module_id_content==unit_.module_id_content
                )
        {
            return true;
        }
        else
            return false;
    }
};

#ifdef UNIT_TEST
class Afn03F12 : public Frame3762Base
#else
/**
 * @brief The Afn03F12
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT Afn03F12 : public Frame3762Base
#endif
{
public:
    /**
     * @brief Afn03F12
     */
    Afn03F12();
    /**
     * @brief Afn03F12
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    Afn03F12(CtrlField ctrl_field,InfoField info_field,AddressField address_field);

    LocalModuleIdDataUnit local_module_id_data_unit_; //!< 模块ID信息数据单元
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
#endif // AFN03F12_H
