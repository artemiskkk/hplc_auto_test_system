#ifndef AFN02H010202E8_H
#define AFN02H010202E8_H

#include <QObject>
#include "../QCSG_Local_Module_Protocol_Lib_global.h"
#include "../framelocalprotocolbase.h"
#include "../exceptions/decodeexception.h"

namespace  qcsg_local_module_protocol {

/**
 * @brief The Afn02H010202E8 添加任务
 */
#ifdef UNIT_TEST
class Afn02H010202E8 : public FrameLocalProtocolBase
#else
class QCSG_LOCAL_MODULE_PROTOCOL_LIB_EXPORT Afn02H010202E8 : public FrameLocalProtocolBase
#endif
{
public:
    Afn02H010202E8();
    /**
     * @brief Afn02H010202E8
     * @param ctrl_field 控制域
     * @param address_field 地址域
     * @param seq 帧序列域
     */
    Afn02H010202E8(CtrlField ctrl_field,AddressField address_field,uchar seq);

    ushort task_id_;              //!< 任务ID
    uchar reserve_:3;             //!< 任务模式字：保留
    uchar response_identifier_:1;  //!< 任务模式字：任务响应标识
    uchar task_priority_:4;        //!< 任务模式字：任务优先级
    ushort time_out_;             //!< 超时时间
    uchar frame_len_;           //!< 报文长度
    QByteArray frame_;            //!< 报文内容

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
#endif // AFN02H010202E8_H
