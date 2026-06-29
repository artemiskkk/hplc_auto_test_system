#ifndef AFN03H690304E8_GD_H
#define AFN03H690304E8_GD_H

#include <QObject>
#include "../QCSG_Local_Module_Protocol_Lib_global.h"
#include "../framelocalprotocolbase.h"
#include "../exceptions/decodeexception.h"

namespace  qcsg_local_module_protocol {

/**
 * @brief The Afn03H690304E8_GD 广东扩展：返回应用层报文信息
 */
#ifdef UNIT_TEST
class Afn03H690304E8_GD : public FrameLocalProtocolBase
#else
class QCSG_LOCAL_MODULE_PROTOCOL_LIB_EXPORT Afn03H690304E8_GD : public FrameLocalProtocolBase
#endif
{
public:
    Afn03H690304E8_GD();
    /**
     * @brief Afn03H690304E8_GD
     * @param ctrl_field 控制域
     * @param address_field 地址域
     * @param seq 帧序列域
     */
    Afn03H690304E8_GD(CtrlField ctrl_field,AddressField address_field,uchar seq);

    ushort total_count_;               //!< 报文总数量
    uchar answer_quantity_;           //!< 本次上报的报文数量
    QList<AppMsgInfo_GD> msg_info_list_;      //!< 报文信息链表，定义详见<datatypedef.h>

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
#endif // AFN03H690304E8_GD_H
