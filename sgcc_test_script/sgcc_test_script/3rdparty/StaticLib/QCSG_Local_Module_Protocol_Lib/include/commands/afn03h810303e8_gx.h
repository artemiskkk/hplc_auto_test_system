#ifndef AFN03H810303E8_GX_H
#define AFN03H810303E8_GX_H

#include <QObject>
#include "../QCSG_Local_Module_Protocol_Lib_global.h"
#include "../framelocalprotocolbase.h"
#include "../exceptions/decodeexception.h"

namespace  qcsg_local_module_protocol {

/**
 * @brief The Afn03H810303E8_GX 广西扩展：查询节点版本信息
 */
#ifdef UNIT_TEST
class Afn03H810303E8_GX : public FrameLocalProtocolBase
#else
class QCSG_LOCAL_MODULE_PROTOCOL_LIB_EXPORT Afn03H810303E8_GX : public FrameLocalProtocolBase
#endif
{
public:
    Afn03H810303E8_GX();
    /**
     * @brief Afn03H810303E8_GX
     * @param ctrl_field 控制域
     * @param address_field 地址域
     * @param seq 帧序列域
     */
    Afn03H810303E8_GX(CtrlField ctrl_field,AddressField address_field,uchar seq);

    ushort start_index_;   //!< 从节点起始序号 m：表示在从节点列表中的第 m 个从节点，序号从 0 开始，0表示CCO，1表示第1个从节点，2表示第2个从节点，以此类推。每次查询的节点数量建议不超过15个
    uchar node_count_;     //!< 从节点数量 n：从节点起始序号为 m，从节点数量为 n，表示查询从节点列表中的第m,m+1，……，m+n-1 个从节点，n≥1

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
#endif // AFN03H810303E8_GX_H
