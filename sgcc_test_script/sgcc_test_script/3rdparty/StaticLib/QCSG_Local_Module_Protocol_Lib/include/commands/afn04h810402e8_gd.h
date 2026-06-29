#ifndef AFN04H810402E8_GD_H
#define AFN04H810402E8_GD_H

#include <QObject>
#include "../QCSG_Local_Module_Protocol_Lib_global.h"
#include "../framelocalprotocolbase.h"
#include "../exceptions/decodeexception.h"

namespace  qcsg_local_module_protocol {

/**
 * @brief The Afn04H810402E8_GD 广东扩展：停止台区识别
 */
#ifdef UNIT_TEST
class Afn04H810402E8_GD : public FrameLocalProtocolBase
#else
class QCSG_LOCAL_MODULE_PROTOCOL_LIB_EXPORT Afn04H810402E8_GD : public FrameLocalProtocolBase
#endif
{
public:
    Afn04H810402E8_GD();
    /**
     * @brief Afn04H810402E8_GD
     * @param ctrl_field 控制域
     * @param address_field 地址域
     * @param seq 帧序列域
     */
    Afn04H810402E8_GD(CtrlField ctrl_field,AddressField address_field,uchar seq);

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
#endif // AFN04H810402E8_GD_H
