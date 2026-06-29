#ifndef AFNF0H06F004E8_H
#define AFNF0H06F004E8_H

#include <QObject>
#include "../QCSG_Local_Module_Protocol_Lib_global.h"
#include "../framelocalprotocolbase.h"
#include "../exceptions/decodeexception.h"

namespace  qcsg_local_module_protocol {

/**
 * @brief The AfnF0H06F004E8 厂家自定义：返回查询网络拓扑信息
 */
#ifdef UNIT_TEST
class AfnF0H06F004E8 : public FrameLocalProtocolBase
#else
class QCSG_LOCAL_MODULE_PROTOCOL_LIB_EXPORT AfnF0H06F004E8 : public FrameLocalProtocolBase
#endif
{
public:
    AfnF0H06F004E8();
    /**
     * @brief AfnF0H06F004E8
     * @param ctrl_field 控制域
     * @param address_field 地址域
     * @param seq 帧序列域
     */
    AfnF0H06F004E8(CtrlField ctrl_field,AddressField address_field,uchar seq);

    ushort total_count_;                              //!< 节点总数量
    ushort start_index_;                             //!< 节点起始序号
    uchar answer_quantity_;                       //!< 本次应答的节点数量
    QList<NodeTopologyInfo> node_topology_info_list_;      //!< 节点网络拓扑信息链表，定义详见<datatypedef.h>

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
#endif // AFNF0H06F004E8_H
