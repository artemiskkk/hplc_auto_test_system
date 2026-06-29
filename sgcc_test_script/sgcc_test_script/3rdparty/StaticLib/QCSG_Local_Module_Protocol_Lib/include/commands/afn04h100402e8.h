#ifndef AFN04H100402E8_H
#define AFN04H100402E8_H

#include <QObject>
#include "../QCSG_Local_Module_Protocol_Lib_global.h"
#include "../framelocalprotocolbase.h"
#include "../exceptions/decodeexception.h"

namespace  qcsg_local_module_protocol {

/**
 * @brief The Afn04H100402E8  厂家自定义：设置频段
 */
#ifdef UNIT_TEST
class Afn04H100402E8 : public FrameLocalProtocolBase
#else
class QCSG_LOCAL_MODULE_PROTOCOL_LIB_EXPORT Afn04H100402E8 : public FrameLocalProtocolBase
#endif
{
public:
    Afn04H100402E8();
    /**
     * @brief Afn04H100402E8
     * @param ctrl_field 控制域
     * @param address_field 地址域
     * @param seq 帧序列域
     */
    Afn04H100402E8(CtrlField ctrl_field,AddressField address_field,uchar seq);

    uchar freq_band_; //!< 载波频段,频段： 频段：0：2~12M，1：2~6M，2：0.7~3M，3：1.7~3M，其它：保留

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
#endif // AFN04H100402E8_H
