#ifndef AFN05H800505E8_GD_H
#define AFN05H800505E8_GD_H

#include <QObject>
#include "../QCSG_Local_Module_Protocol_Lib_global.h"
#include "../framelocalprotocolbase.h"
#include "../exceptions/decodeexception.h"

namespace  qcsg_local_module_protocol {

/**
 * @brief The Afn05H800505E8_GD 广东扩展：上报非本台区从节点信息
 */
#ifdef UNIT_TEST
class Afn05H800505E8_GD : public FrameLocalProtocolBase
#else
class QCSG_LOCAL_MODULE_PROTOCOL_LIB_EXPORT Afn05H800505E8_GD : public FrameLocalProtocolBase
#endif
{
public:
    Afn05H800505E8_GD();
    /**
     * @brief Afn05H800505E8_GD
     * @param ctrl_field 控制域
     * @param address_field 地址域
     * @param seq 帧序列域
     */
    Afn05H800505E8_GD(CtrlField ctrl_field,AddressField address_field,uchar seq);

    QByteArray node_address_;           //!< 从节点地址： 当从节点设备类型为 01H 电能表时， 表示电表地址； 当从节点设备类型为 00H 采集器时， 表示采集器地址,小端模式
    uchar device_type_;                 //!< 从节点设备类型： 00H 采集器， 01H 电能表
    uchar link_meter_count_;            //!< 非本台区采集器下接电能表数量
    QByteArray cco_address_;            //!< 应属台区主节点地址， 表示该从节点应当归属的台区主节点地址。 若无法确定台区归属（ 但明确不属于当前台区） ， 填 6 个字节 FFH,小端模式
    ushort reserve_;                    //!< 保留
    QList<QByteArray> link_meter_list_; //!< 报文内容： 当非本台区从节点设备类型为 00H 采集器时有效， 长度为 6 *采集器下接电能表数量（ 字节）

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
#endif // AFN05H800505E8_GD_H
