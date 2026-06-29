#ifndef AFN07H020702E8_H
#define AFN07H020702E8_H

#include <QObject>
#include "../QCSG_Local_Module_Protocol_Lib_global.h"
#include "../framelocalprotocolbase.h"
#include "../exceptions/decodeexception.h"

namespace  qcsg_local_module_protocol {

/**
 * @brief The Afn07H020702E8 传输文件内容
 */
#ifdef UNIT_TEST
class Afn07H020702E8 : public FrameLocalProtocolBase
#else
class QCSG_LOCAL_MODULE_PROTOCOL_LIB_EXPORT Afn07H020702E8 : public FrameLocalProtocolBase
#endif
{
public:
    Afn07H020702E8();
    /**
     * @brief Afn07H020702E8
     * @param ctrl_field 控制域
     * @param address_field 地址域
     * @param seq 帧序列域
     */
    Afn07H020702E8(CtrlField ctrl_field,AddressField address_field,uchar seq);

    ushort segment_num_;           //!< 文件段号
    ushort segment_length_;        //!< 文件段长度 L
    QByteArray segment_data_;      //!< 文件段内容
    ushort segment_cs_;            //!< 文件段校验

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
#endif // AFN07H020702E8_H
