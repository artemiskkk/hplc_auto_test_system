#ifndef AFNF0H04F004E8_H
#define AFNF0H04F004E8_H

#include <QObject>
#include "../QCSG_Local_Module_Protocol_Lib_global.h"
#include "../framelocalprotocolbase.h"
#include "../exceptions/decodeexception.h"

namespace  qcsg_local_module_protocol {

/**
 * @brief The AfnF0H04F004E8 厂家自定义：返回查询多网络信息
 */
#ifdef UNIT_TEST
class AfnF0H04F004E8 : public FrameLocalProtocolBase
#else
class QCSG_LOCAL_MODULE_PROTOCOL_LIB_EXPORT AfnF0H04F004E8 : public FrameLocalProtocolBase
#endif
{
public:
    AfnF0H04F004E8();
    /**
     * @brief AfnF0H04F004E8
     * @param ctrl_field 控制域
     * @param address_field 地址域
     * @param seq 帧序列域
     */
    AfnF0H04F004E8(CtrlField ctrl_field,AddressField address_field,uchar seq);

    uchar total_count_;                   //!< 多网络节点总数量
    uint this_node_snid_;                //!< 本节点网络标识号,网络标识号（SNID） , 是用于标识一个载波通信网络的唯一身份识别号。 NID：3字节，有效取值范围为1- 16777215
    QByteArray address_;                //!< 主节点地址，小端模式
    QList<uint> neighbor_node_snid_list_;//!< 邻居节点网络标识号链表

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
#endif // AFNF0H04F004E8_H
