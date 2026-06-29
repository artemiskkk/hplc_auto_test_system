#ifndef AFN07H050704E8_H
#define AFN07H050704E8_H

#include <QObject>
#include "../QCSG_Local_Module_Protocol_Lib_global.h"
#include "../framelocalprotocolbase.h"
#include "../exceptions/decodeexception.h"

namespace  qcsg_local_module_protocol {

/**
 * @brief The Afn07H050704E8 返回查询文件传输失败节点
 */
#ifdef UNIT_TEST
class Afn07H050704E8 : public FrameLocalProtocolBase
#else
class QCSG_LOCAL_MODULE_PROTOCOL_LIB_EXPORT Afn07H050704E8 : public FrameLocalProtocolBase
#endif
{
public:
    Afn07H050704E8();
    /**
     * @brief Afn07H050704E8
     * @param ctrl_field 控制域
     * @param address_field 地址域
     * @param seq 帧序列域
     */
    Afn07H050704E8(CtrlField ctrl_field,AddressField address_field,uchar seq);

    ushort total_count_;               //!< 从节点总数量
    uchar answer_quantity_;            //!< 本次应答的从节点数量
    QList<QByteArray> address_list_;   //!< 从节点地址列表,小端模式

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
#endif // AFN07H050704E8_H
