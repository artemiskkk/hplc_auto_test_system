#ifndef AFNF0H05F003E8_H
#define AFNF0H05F003E8_H

#include <QObject>
#include "../QCSG_Local_Module_Protocol_Lib_global.h"
#include "../framelocalprotocolbase.h"
#include "../exceptions/decodeexception.h"

namespace  qcsg_local_module_protocol {

/**
 * @brief The AfnF0H05F003E8 厂家自定义：查询宽带载波芯片信息
 */
#ifdef UNIT_TEST
class AfnF0H05F003E8 : public FrameLocalProtocolBase
#else
class QCSG_LOCAL_MODULE_PROTOCOL_LIB_EXPORT AfnF0H05F003E8 : public FrameLocalProtocolBase
#endif
{
public:
    AfnF0H05F003E8();
    /**
     * @brief AfnF0H05F003E8
     * @param ctrl_field 控制域
     * @param address_field 地址域
     * @param seq 帧序列域
     */
    AfnF0H05F003E8(CtrlField ctrl_field,AddressField address_field,uchar seq);

    ushort start_index_;   //!< 从节点起始序号 m： 表示在从节点列表中的第 m 个从节点， 节点序号从1开始，其中1为主节点，后续为从节点
    uchar node_count_;     //!< 从节点数量 n： 从节点起始序号为 m， 从节点数量为 n， 表示查询从节点列表中的第m,m+1， ……， m+n-1 个从节点，

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
#endif // AFNF0H05F003E8_H
