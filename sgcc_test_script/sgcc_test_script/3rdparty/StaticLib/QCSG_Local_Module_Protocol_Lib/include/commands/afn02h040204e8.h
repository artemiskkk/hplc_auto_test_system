#ifndef AFN02H040204E8_H
#define AFN02H040204E8_H

#include <QObject>
#include "../QCSG_Local_Module_Protocol_Lib_global.h"
#include "../framelocalprotocolbase.h"
#include "../exceptions/decodeexception.h"

namespace  qcsg_local_module_protocol {

/**
 * @brief The Afn02H040204E8 返回查询未完成任务列表
 */
#ifdef UNIT_TEST
class Afn02H040204E8 : public FrameLocalProtocolBase
#else
class QCSG_LOCAL_MODULE_PROTOCOL_LIB_EXPORT Afn02H040204E8 : public FrameLocalProtocolBase
#endif
{
public:
    Afn02H040204E8();
    /**
     * @brief Afn02H040204E8
     * @param ctrl_field 控制域
     * @param address_field 地址域
     * @param seq 帧序列域
     */
    Afn02H040204E8(CtrlField ctrl_field,AddressField address_field,uchar seq);

    ushort task_count_;          //!< 本次上报的任务数量
    QList<ushort> task_id_list_; //!< 上报的任务ID列表

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
#endif // AFN02H040204E8_H
