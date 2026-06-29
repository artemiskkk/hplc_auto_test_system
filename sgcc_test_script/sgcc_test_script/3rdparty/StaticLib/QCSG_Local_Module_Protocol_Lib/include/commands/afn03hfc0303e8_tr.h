#ifndef AFN03HFC0303E8_TR_H
#define AFN03HFC0303E8_TR_H

#include <QObject>
#include "../QCSG_Local_Module_Protocol_Lib_global.h"
#include "../framelocalprotocolbase.h"
#include "../exceptions/decodeexception.h"

namespace  qcsg_local_module_protocol {

/**
 * @brief The Afn03HFC0303E8_TR 批量查询从节点其他信息
 */
#ifdef UNIT_TEST
class Afn03HFC0303E8_TR : public FrameLocalProtocolBase
#else
class QCSG_LOCAL_MODULE_PROTOCOL_LIB_EXPORT Afn03HFC0303E8_TR : public FrameLocalProtocolBase
#endif
{
public:
    Afn03HFC0303E8_TR();
    /**
     * @brief Afn03HFC0303E8_TR
     * @param ctrl_field 控制域
     * @param address_field 地址域
     * @param seq 帧序列域
     */
    Afn03HFC0303E8_TR(CtrlField ctrl_field,AddressField address_field,uchar seq);

    ushort start_index_;   //!< 从节点起始序号m：表示在从节点列表中的第m个从节点，序号从0开始
    uchar node_count_;     //!< 从节点数量n：从节点起始序号为m，从节点数量为n，表示查询从节点列表中的第m,m+1,....,m+n-1个从节点；0<n≤16


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
#endif // AFN03HFC0303E8_TR_H
