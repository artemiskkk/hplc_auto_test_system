#ifndef AFN07H040700E8_H
#define AFN07H040700E8_H

#include <QObject>
#include "../QCSG_Local_Module_Protocol_Lib_global.h"
#include "../framelocalprotocolbase.h"
#include "../exceptions/decodeexception.h"

namespace  qcsg_local_module_protocol {

/**
 * @brief The Afn07H040700E8 查询文件处理进度
 */
#ifdef UNIT_TEST
class Afn07H040700E8 : public FrameLocalProtocolBase
#else
class QCSG_LOCAL_MODULE_PROTOCOL_LIB_EXPORT Afn07H040700E8 : public FrameLocalProtocolBase
#endif
{
public:
    Afn07H040700E8();
    /**
     * @brief Afn07H040700E8
     * @param ctrl_field 控制域
     * @param address_field 地址域
     * @param seq 帧序列域
     */
    Afn07H040700E8(CtrlField ctrl_field,AddressField address_field,uchar seq);

    uchar file_processing_status_;  //!< 文件处理进度
    uchar unfinished_file_id_;      //!< 处理未完成的文件ID
    ushort failed_count_;           //!< 失败的节点数量

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
#endif // AFN07H040700E8_H
