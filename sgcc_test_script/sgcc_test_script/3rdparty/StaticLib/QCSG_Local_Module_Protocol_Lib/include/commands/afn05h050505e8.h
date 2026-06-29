#ifndef AFN05H050505E8_H
#define AFN05H050505E8_H

#include <QObject>
#include "../QCSG_Local_Module_Protocol_Lib_global.h"
#include "../framelocalprotocolbase.h"
#include "../exceptions/decodeexception.h"

namespace  qcsg_local_module_protocol {

/**
 * @brief The Afn05H050505E8 上报任务状态
 */
#ifdef UNIT_TEST
class Afn05H050505E8 : public FrameLocalProtocolBase
#else
class QCSG_LOCAL_MODULE_PROTOCOL_LIB_EXPORT Afn05H050505E8 : public FrameLocalProtocolBase
#endif
{
public:
    Afn05H050505E8();
    /**
     * @brief Afn05H050505E8
     * @param ctrl_field 控制域
     * @param address_field 地址域
     * @param seq 帧序列域
     */
    Afn05H050505E8(CtrlField ctrl_field,AddressField address_field,uchar seq);

    ushort task_id_;        //!< 任务ID
    QByteArray address_;    //!< 从节点地址,小端模式
    uchar task_status_;     //!< 任务状态

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
#endif // AFN05H050505E8_H
