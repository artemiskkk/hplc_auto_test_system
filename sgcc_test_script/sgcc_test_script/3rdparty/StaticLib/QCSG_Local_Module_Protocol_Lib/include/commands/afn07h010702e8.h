#ifndef AFN07H010702E8_H
#define AFN07H010702E8_H

#include <QObject>
#include "../QCSG_Local_Module_Protocol_Lib_global.h"
#include "../framelocalprotocolbase.h"
#include "../exceptions/decodeexception.h"

namespace  qcsg_local_module_protocol {

/**
 * @brief The Afn07H010702E8 启动文件传输
 */
#ifdef UNIT_TEST
class Afn07H010702E8 : public FrameLocalProtocolBase
#else
class QCSG_LOCAL_MODULE_PROTOCOL_LIB_EXPORT Afn07H010702E8 : public FrameLocalProtocolBase
#endif
{
public:
    Afn07H010702E8();
    /**
     * @brief Afn07H010702E8
     * @param ctrl_field 控制域
     * @param address_field 地址域
     * @param seq 帧序列域
     */
    Afn07H010702E8(CtrlField ctrl_field,AddressField address_field,uchar seq);

    uchar file_type_;        //!< 文件性质
    uchar file_id_;          //!< 文件 ID
    QByteArray address_;     //!< 目的地址,小端模式
    ushort segment_count_;   //!< 文件总段数
    uint file_size_;         //!< 文件大小
    ushort file_check_sum_;  //!< 文件总校验
    uchar trans_timeout_;    //!< 文件传输超时时间

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
#endif // AFN07H010702E8_H
