#ifndef AFN04H020402E8_H
#define AFN04H020402E8_H

#include <QObject>
#include "../QCSG_Local_Module_Protocol_Lib_global.h"
#include "../framelocalprotocolbase.h"
#include "../exceptions/decodeexception.h"

namespace  qcsg_local_module_protocol {

/**
 * @brief The Afn04H020402E8 添加从节点
 */
#ifdef UNIT_TEST
class Afn04H020402E8 : public FrameLocalProtocolBase
#else
class QCSG_LOCAL_MODULE_PROTOCOL_LIB_EXPORT Afn04H020402E8 : public FrameLocalProtocolBase
#endif
{
public:
    Afn04H020402E8();
    /**
     * @brief Afn04H020402E8
     * @param ctrl_field 控制域
     * @param address_field 地址域
     * @param seq 帧序列域
     */
    Afn04H020402E8(CtrlField ctrl_field,AddressField address_field,uchar seq);

    uchar node_count_;               //!< 从节点的数量
    QList<QByteArray> address_list_; //!< 从节点地址链表,小端模式

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
#endif // AFN04H020402E8_H
