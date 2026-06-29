#ifndef AFN03H0B0300E8_H
#define AFN03H0B0300E8_H

#include <QObject>
#include "../QCSG_Local_Module_Protocol_Lib_global.h"
#include "../framelocalprotocolbase.h"
#include "../exceptions/decodeexception.h"

namespace  qcsg_local_module_protocol {

/**
 * @brief The Afn03H0B0300E8 查询任务建议超时时间
 */
#ifdef UNIT_TEST
class Afn03H0B0300E8 : public FrameLocalProtocolBase
#else
class QCSG_LOCAL_MODULE_PROTOCOL_LIB_EXPORT Afn03H0B0300E8 : public FrameLocalProtocolBase
#endif
{
public:
    Afn03H0B0300E8();
    /**
     * @brief Afn03H0B0300E8
     * @param ctrl_field 控制域
     * @param address_field 地址域
     * @param seq 帧序列域
     */
    Afn03H0B0300E8(CtrlField ctrl_field,AddressField address_field,uchar seq);

    ushort priority_0_timeout_;  //!< 优先级 0 的任务建议超时时间，单位 s
    ushort priority_1_timeout_;  //!< 优先级 1 的任务建议超时时间，单位 s
    ushort priority_2_timeout_;  //!< 优先级 2 的任务建议超时时间，单位 s
    ushort priority_3_timeout_;  //!< 优先级 3 的任务建议超时时间，单位 s

    //FrameLocalProtocolBase interface
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
#endif // AFN03H0B0300E8_H
