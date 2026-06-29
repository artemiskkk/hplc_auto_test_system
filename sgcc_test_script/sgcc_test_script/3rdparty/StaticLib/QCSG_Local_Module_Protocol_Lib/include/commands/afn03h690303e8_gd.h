#ifndef AFN03H690303E8_GD_H
#define AFN03H690303E8_GD_H

#include <QObject>
#include "../QCSG_Local_Module_Protocol_Lib_global.h"
#include "../framelocalprotocolbase.h"
#include "../exceptions/decodeexception.h"

namespace  qcsg_local_module_protocol {

/**
 * @brief The Afn03H690303E8_GD 广东扩展：查询应用层报文信息
 */
#ifdef UNIT_TEST
class Afn03H690303E8_GD : public FrameLocalProtocolBase
#else
class QCSG_LOCAL_MODULE_PROTOCOL_LIB_EXPORT Afn03H690303E8_GD : public FrameLocalProtocolBase
#endif
{
public:
    Afn03H690303E8_GD();
    /**
     * @brief Afn03H690303E8_GD
     * @param ctrl_field 控制域
     * @param address_field 地址域
     * @param seq 帧序列域
     */
    Afn03H690303E8_GD(CtrlField ctrl_field,AddressField address_field,uchar seq);

    uchar msg_type;        //!< 报文类型： 0： 宽带载波应用层报文； 1:本地接口协议报文
    ushort start_index_;   //!< 起始报文序号m： 表示查询应用层存储的的第 m 帧报文， 序号从 0 开始
    uchar msg_count_;     //!< 查询报文数量n： 起始报文序号为 m， 报文数量为 n， 表示查询应用层存储的第m,m+1， ……， m+n-1 帧报文， n≥1

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
#endif // AFN03H690303E8_GD_H
