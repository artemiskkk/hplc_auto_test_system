#ifndef AFNF0H03F003E8_H
#define AFNF0H03F003E8_H

#include <QObject>
#include "../QCSG_Local_Module_Protocol_Lib_global.h"
#include "../framelocalprotocolbase.h"
#include "../exceptions/decodeexception.h"

namespace  qcsg_local_module_protocol {

/**
 * @brief The AfnF0H03F003E8  厂家自定义：查询从节点信息
 */
#ifdef UNIT_TEST
class AfnF0H03F003E8 : public FrameLocalProtocolBase
#else
class QCSG_LOCAL_MODULE_PROTOCOL_LIB_EXPORT AfnF0H03F003E8 : public FrameLocalProtocolBase
#endif
{
public:
    AfnF0H03F003E8();
    /**
     * @brief AfnF0H03F003E8
     * @param ctrl_field 控制域
     * @param address_field 地址域
     * @param seq 帧序列域
     */
    AfnF0H03F003E8(CtrlField ctrl_field,AddressField address_field,uchar seq);

    ushort start_index_;   //!< 节点起始序号：表示查询的节点起始序号，其值从“00 00”开始，“00 00”为主节点信息
    uchar node_count_;     //!< 节点数量：表示查询的节点数量

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
#endif // AFNF0H03F003E8_H
