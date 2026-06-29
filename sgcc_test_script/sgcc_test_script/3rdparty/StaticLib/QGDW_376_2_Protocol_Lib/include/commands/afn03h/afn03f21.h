#ifndef AFN03F21_H
#define AFN03F21_H

#include <QObject>
#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"

namespace qgdw_3762_protocol {
/**
 * @brief The ConcurrentParameterUnit 模块并发参数数据单元结构体
 */
struct ConcurrentParameterUnit
{
    uchar module_support_max_node_num_; //!< 模块支持并发最大节点数量
    uchar module_support_max_frame_num_; //!< 模块支持并发最大报文条数
    ushort module_support_max_timeout; //!< 模块支持并发最大超时时间
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const ConcurrentParameterUnit &unit_)const
    {
        if(this->module_support_max_node_num_==unit_.module_support_max_node_num_
                &&this->module_support_max_frame_num_==unit_.module_support_max_frame_num_
                &&this->module_support_max_timeout==unit_.module_support_max_timeout
                )
        {
            return true;
        }
        else
            return false;
    }
};

#ifdef UNIT_TEST
class Afn03F21 : public Frame3762Base
#else
/**
 * @brief The Afn03F21
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT Afn03F21 : public Frame3762Base
#endif
{
public:
    /**
     * @brief Afn03F21
     */
    Afn03F21();
    /**
     * @brief Afn03F21
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    Afn03F21(CtrlField ctrl_field,InfoField info_field,AddressField address_field);

    ConcurrentParameterUnit concurrent_parameter_unit_;//!< 并发参数
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
#endif // AFN03F21_H
