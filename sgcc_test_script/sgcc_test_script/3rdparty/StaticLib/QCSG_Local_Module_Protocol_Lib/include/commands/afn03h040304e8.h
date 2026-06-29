#ifndef AFN03H040304E8_H
#define AFN03H040304E8_H

#include <QObject>
#include "../QCSG_Local_Module_Protocol_Lib_global.h"
#include "../framelocalprotocolbase.h"
#include "../exceptions/decodeexception.h"

namespace  qcsg_local_module_protocol {

/**
 * @brief The Afn03H040304E8 返回查询通信延时时长
 */
#ifdef UNIT_TEST
class Afn03H040304E8 : public FrameLocalProtocolBase
#else
class QCSG_LOCAL_MODULE_PROTOCOL_LIB_EXPORT Afn03H040304E8 : public FrameLocalProtocolBase
#endif
{
public:
    Afn03H040304E8();
    /**
     * @brief Afn03H040304E8
     * @param ctrl_field 控制域
     * @param address_field 地址域
     * @param seq 帧序列域
     */
    Afn03H040304E8(CtrlField ctrl_field,AddressField address_field,uchar seq);

    QByteArray address_;    //!< 通信目的地址，小端模式
    ushort delayed_time_;   //!< 通信延时时长
    uchar frame_len_;      //!< 报文长度

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
#endif // AFN03H040304E8_H
