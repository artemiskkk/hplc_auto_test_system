#ifndef AFN04H900402E8_GD_H
#define AFN04H900402E8_GD_H

#include <QObject>
#include "../QCSG_Local_Module_Protocol_Lib_global.h"
#include "../framelocalprotocolbase.h"
#include "../exceptions/decodeexception.h"

namespace  qcsg_local_module_protocol {

/**
 * @brief The Afn04H900402E8_GD 广东扩展：设置宽带载波频段
 */
#ifdef UNIT_TEST
class Afn04H900402E8_GD : public FrameLocalProtocolBase
#else
class QCSG_LOCAL_MODULE_PROTOCOL_LIB_EXPORT Afn04H900402E8_GD : public FrameLocalProtocolBase
#endif
{
public:
    Afn04H900402E8_GD();
    /**
     * @brief Afn04H900402E8_GD
     * @param ctrl_field 控制域
     * @param address_field 地址域
     * @param seq 帧序列域
     */
    Afn04H900402E8_GD(CtrlField ctrl_field,AddressField address_field,uchar seq);

    uchar hplc_freq_band_; //!< 宽带载波频段,频段： 0： 1.953～11.96MHz； 1： 2.441～5.615 MHz； 2： 0.781～2.930 MHz； 3～255表示保留

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
#endif // AFN04H900402E8_GD_H
