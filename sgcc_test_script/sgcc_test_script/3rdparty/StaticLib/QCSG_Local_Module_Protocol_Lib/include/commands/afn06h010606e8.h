#ifndef AFN06H010606E8_H
#define AFN06H010606E8_H

#include <QObject>
#include "../QCSG_Local_Module_Protocol_Lib_global.h"
#include "../framelocalprotocolbase.h"
#include "../exceptions/decodeexception.h"

namespace  qcsg_local_module_protocol {

/**
 * @brief The Afn06H010606E8 请求集中器时间
 */
#ifdef UNIT_TEST
class Afn06H010606E8 : public FrameLocalProtocolBase
#else
class QCSG_LOCAL_MODULE_PROTOCOL_LIB_EXPORT Afn06H010606E8 : public FrameLocalProtocolBase
#endif
{
public:
    Afn06H010606E8();
    /**
     * @brief Afn06H010606E8
     * @param ctrl_field 控制域
     * @param address_field 地址域
     * @param seq 帧序列域
     */
    Afn06H010606E8(CtrlField ctrl_field,AddressField address_field,uchar seq);

    QString sec_;     //!< 当前时间—秒
    QString min_;     //!< 当前时间—分
    QString hour_;    //!< 当前时间—时
    QString day_;     //!< 当前时间—日
    QString month_;   //!< 当前时间—月
    QString year_;    //!< 当前时间—年（低字节）

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
#endif // AFN06H010606E8_H
