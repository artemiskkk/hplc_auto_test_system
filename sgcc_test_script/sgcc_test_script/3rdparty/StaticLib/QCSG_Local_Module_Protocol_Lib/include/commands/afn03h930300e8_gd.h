#ifndef AFN03H930300E8_GD_H
#define AFN03H930300E8_GD_H

#include <QObject>
#include "../QCSG_Local_Module_Protocol_Lib_global.h"
#include "../framelocalprotocolbase.h"
#include "../exceptions/decodeexception.h"

namespace  qcsg_local_module_protocol {

/**
 * @brief The Afn03H930300E8_GD 广东扩展：查询白名单生效信息
 */
#ifdef UNIT_TEST
class Afn03H930300E8_GD : public FrameLocalProtocolBase
#else
class QCSG_LOCAL_MODULE_PROTOCOL_LIB_EXPORT Afn03H930300E8_GD : public FrameLocalProtocolBase
#endif
{
public:
    Afn03H930300E8_GD();
    /**
     * @brief Afn03H930300E8_GD
     * @param ctrl_field 控制域
     * @param address_field 地址域
     * @param seq 帧序列域
     */
    Afn03H930300E8_GD(CtrlField ctrl_field,AddressField address_field,uchar seq);

    uchar white_list_switch_;  //!< 白名单开关： 0： 关闭； 1： 打开
    uchar white_list_range_;   //!< 白名单生效范围： 0： 表档案； 1： 厂家自定义； 2： 表档案和厂家自定义的合集； 3～255： 保留

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
#endif // AFN03H930300E8_GD_H
