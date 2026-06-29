#ifndef AFNF0H0BF004E8_H
#define AFNF0H0BF004E8_H

#include <QObject>
#include "../QCSG_Local_Module_Protocol_Lib_global.h"
#include "../framelocalprotocolbase.h"
#include "../exceptions/decodeexception.h"

namespace  qcsg_local_module_protocol {

/**
 * @brief The AfnF0H0BF004E8 厂家自定义：返回STA版本信息
 */
#ifdef UNIT_TEST
class AfnF0H0BF004E8 : public FrameLocalProtocolBase
#else
class QCSG_LOCAL_MODULE_PROTOCOL_LIB_EXPORT AfnF0H0BF004E8 : public FrameLocalProtocolBase
#endif
{
public:
    AfnF0H0BF004E8();
    /**
     * @brief AfnF0H0BF004E8
     * @param ctrl_field 控制域
     * @param address_field 地址域
     * @param seq 帧序列域
     */
    AfnF0H0BF004E8(CtrlField ctrl_field,AddressField address_field,uchar seq);

    uchar protocol_type_; //!< 协议类型
    uchar frame_len_;     //!< 报文长度
    QByteArray frame_;    //!< 报文内容

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
#endif // AFNF0H0BF004E8_H
