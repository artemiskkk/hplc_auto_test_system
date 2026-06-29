#ifndef AFN07H050703E8_H
#define AFN07H050703E8_H

#include <QObject>
#include "../QCSG_Local_Module_Protocol_Lib_global.h"
#include "../framelocalprotocolbase.h"
#include "../exceptions/decodeexception.h"

namespace  qcsg_local_module_protocol {

/**
 * @brief The Afn07H050703E8 查询文件传输失败节点
 */
#ifdef UNIT_TEST
class Afn07H050703E8 : public FrameLocalProtocolBase
#else
class QCSG_LOCAL_MODULE_PROTOCOL_LIB_EXPORT Afn07H050703E8 : public FrameLocalProtocolBase
#endif
{
public:
    Afn07H050703E8();
    /**
     * @brief Afn07H050703E8
     * @param ctrl_field 控制域
     * @param address_field 地址域
     * @param seq 帧序列域
     */
    Afn07H050703E8(CtrlField ctrl_field,AddressField address_field,uchar seq);

    ushort start_index_;   //!< 节点起始序号
    uchar node_count_;     //!< 本次查询的节点数量

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
#endif // AFN07H050703E8_H
