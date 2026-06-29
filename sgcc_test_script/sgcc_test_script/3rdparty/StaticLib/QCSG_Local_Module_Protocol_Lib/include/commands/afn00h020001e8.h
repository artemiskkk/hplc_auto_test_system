#ifndef AFN00H020001E8_H
#define AFN00H020001E8_H

#include <QObject>
#include "../QCSG_Local_Module_Protocol_Lib_global.h"
#include "../framelocalprotocolbase.h"
#include "../exceptions/decodeexception.h"

namespace  qcsg_local_module_protocol {

/**
 * @brief The Afn00H020001E8 否认
 */
#ifdef UNIT_TEST
class Afn00H020001E8 : public FrameLocalProtocolBase
#else
class QCSG_LOCAL_MODULE_PROTOCOL_LIB_EXPORT Afn00H020001E8 : public FrameLocalProtocolBase
#endif
{
public:
    Afn00H020001E8();
    /**
     * @brief Afn00H020001E8
     * @param ctrl_field 控制域
     * @param address_field 地址域
     * @param seq 帧序列域
     */
    Afn00H020001E8(CtrlField ctrl_field,AddressField address_field,uchar seq);

    uchar error_code_; //!< 错误状态字

    // FrameLocalProtocolBase interface
public:
    /**
     * @brief 将16进制格式数据，解析为数据单元
     * @param data
     */
    void DecodeFrameDataField(QByteArray data) override;

protected:
    /**
     * @brief 将数据单元编码为16进制格式数据
     * @return 反回编码好的字节串
     */
    QByteArray EncodeFrameDataField() override;
};

}

#endif // AFN00H020001E8_H
