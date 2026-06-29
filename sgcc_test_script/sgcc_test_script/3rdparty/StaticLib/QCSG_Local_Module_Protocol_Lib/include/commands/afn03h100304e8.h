#ifndef AFN03H100304E8_H
#define AFN03H100304E8_H

#include <QObject>
#include "../QCSG_Local_Module_Protocol_Lib_global.h"
#include "../framelocalprotocolbase.h"
#include "../exceptions/decodeexception.h"

namespace  qcsg_local_module_protocol {

/**
 * @brief The Afn03H100304E8 厂家自定义：返回查询频段
 */
#ifdef UNIT_TEST
class Afn03H100304E8 : public FrameLocalProtocolBase
#else
class QCSG_LOCAL_MODULE_PROTOCOL_LIB_EXPORT Afn03H100304E8 : public FrameLocalProtocolBase
#endif
{
public:
    Afn03H100304E8();
    /**
     * @brief Afn03H100304E8
     * @param ctrl_field 控制域
     * @param address_field 地址域
     * @param seq 帧序列域
     */
    Afn03H100304E8(CtrlField ctrl_field,AddressField address_field,uchar seq);

    uchar freq_band_;   //!< 频段：0：2~12M，1：2~6M，2：0.7~3M，3：1.7~3M，其它：保留

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
#endif // AFN03H100304E8_H
