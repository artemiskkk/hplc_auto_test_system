#ifndef AFNF0HFEF003E8_H
#define AFNF0HFEF003E8_H

#include <QObject>
#include "../QCSG_Local_Module_Protocol_Lib_global.h"
#include "../framelocalprotocolbase.h"
#include "../exceptions/decodeexception.h"

namespace  qcsg_local_module_protocol {

/**
 * @brief The AfnF0HFEF003E8 厂家自定义：查询路由ID
 */
#ifdef UNIT_TEST
class AfnF0HFEF003E8 : public FrameLocalProtocolBase
#else
class QCSG_LOCAL_MODULE_PROTOCOL_LIB_EXPORT AfnF0HFEF003E8 : public FrameLocalProtocolBase
#endif
{
public:
    AfnF0HFEF003E8();
    /**
     * @brief AfnF0HFEF003E8
     * @param ctrl_field 控制域
     * @param address_field 地址域
     * @param seq 帧序列域
     */
    AfnF0HFEF003E8(CtrlField ctrl_field,AddressField address_field,uchar seq);

    QByteArray mac_address_;  //!< MAC地址：对象设备MAC地址,小端模式
    uchar id_type_;           //!< ID类型：1. 芯片ID；2.模块ID；其他：保留
    uchar id_format_;         //!< 模块ID格式：00H为组合格式；01H为BCD; 02H为BIN; 03H为ASCII
    uchar id_len_;           //!< ID长度：芯片ID长度固定为24字节；模块ID长度最大50字节

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
#endif // AFNF0HFEF003E8_H
