#ifndef AFN03H010300E8_H
#define AFN03H010300E8_H

#include <QObject>
#include "../QCSG_Local_Module_Protocol_Lib_global.h"
#include "../framelocalprotocolbase.h"
#include "../exceptions/decodeexception.h"

namespace  qcsg_local_module_protocol {

/**
 * @brief The Afn03H010300E8 查询厂商代码和版本信息
 */
#ifdef UNIT_TEST
class Afn03H010300E8 : public FrameLocalProtocolBase
#else
class QCSG_LOCAL_MODULE_PROTOCOL_LIB_EXPORT Afn03H010300E8 : public FrameLocalProtocolBase
#endif
{
public:
    Afn03H010300E8();
    /**
     * @brief Afn03H010300E8
     * @param ctrl_field 控制域
     * @param address_field 地址域
     * @param seq 帧序列域
     */
    Afn03H010300E8(CtrlField ctrl_field,AddressField address_field,uchar seq);

    QString vendor_code_;    //!< 厂商代码,例“TC"
    QString chip_code_;      //!< 芯片代码,例“R5"
    QString version_time_;   //!< 版本时间,年月日,例“200516"
    QString version_;        //!< 版本,例“0306"

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
#endif // AFN03H010300E8_H
