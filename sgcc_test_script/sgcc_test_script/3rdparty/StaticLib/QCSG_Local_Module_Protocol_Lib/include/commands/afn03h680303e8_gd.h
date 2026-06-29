#ifndef AFN03H680303E8_GD_H
#define AFN03H680303E8_GD_H

#include <QObject>
#include "../QCSG_Local_Module_Protocol_Lib_global.h"
#include "../framelocalprotocolbase.h"
#include "../exceptions/decodeexception.h"

namespace  qcsg_local_module_protocol {

/**
 * @brief The Afn03H680303E8_GD 广东扩展：查询节点信道信息
 */
#ifdef UNIT_TEST
class Afn03H680303E8_GD : public FrameLocalProtocolBase
#else
class QCSG_LOCAL_MODULE_PROTOCOL_LIB_EXPORT Afn03H680303E8_GD : public FrameLocalProtocolBase
#endif
{
public:
    Afn03H680303E8_GD();
    /**
     * @brief Afn03H680303E8_GD
     * @param ctrl_field 控制域
     * @param address_field 地址域
     * @param seq 帧序列域
     */
    Afn03H680303E8_GD(CtrlField ctrl_field,AddressField address_field,uchar seq);

    QByteArray address_;    //!< 节点地址： 主节点地址或表地址。 CCO 是主节点地址， STA 是入网 MAC 地址;小端模式
    ushort start_index_;   //!< 周边节点起始序号 m： 表示在周边节点列 表中的第 m 个从节点， 序号从 0 开始。
    uchar node_count_;      //!< 周边节点数量 n： 周边节点起始序号为 m， 节点数量为 n， 表示查询周边节点列表中的第m,m+1， ……， m+n-1 个从节点， 如果查询的节点是主节点， n≥1， 如果查询的节点是从节点，6≥n≥1。 （对象是 CCO， n≤20； 对象是 STA， n≤6）

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
#endif // AFN03H680303E8_GD_H
