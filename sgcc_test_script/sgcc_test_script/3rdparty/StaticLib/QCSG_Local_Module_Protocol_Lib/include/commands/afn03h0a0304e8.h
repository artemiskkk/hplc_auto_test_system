#ifndef AFN03H0A0304E8_H
#define AFN03H0A0304E8_H

#include <QObject>
#include "../QCSG_Local_Module_Protocol_Lib_global.h"
#include "../framelocalprotocolbase.h"
#include "../exceptions/decodeexception.h"

namespace  qcsg_local_module_protocol {

/**
 * @brief The Afn03H0A0304E8 返回查询从节点通信地址映射表
 */
#ifdef UNIT_TEST
class Afn03H0A0304E8 : public FrameLocalProtocolBase
#else
class QCSG_LOCAL_MODULE_PROTOCOL_LIB_EXPORT Afn03H0A0304E8 : public FrameLocalProtocolBase
#endif
{
public:
    Afn03H0A0304E8();
    /**
     * @brief Afn03H0A0304E8
     * @param ctrl_field 控制域
     * @param address_field 地址域
     * @param seq 帧序列域
     */
    Afn03H0A0304E8(CtrlField ctrl_field,AddressField address_field,uchar seq);

    ushort mapping_table_count_;             //!< 映射表记录节点数量
    uchar answer_quantity_;               //!< 本次应答的映射表记录数量
    QList<QByteArray> commu_address_list_;   //!< 从节点通信地址列表,小端模式,通信地址是6字节
    QList<QByteArray> meter_address_list_;   //!< 从节点表计地址列表,小端模式，表计地址是12字节

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
#endif // AFN03H0A0304E8_H
