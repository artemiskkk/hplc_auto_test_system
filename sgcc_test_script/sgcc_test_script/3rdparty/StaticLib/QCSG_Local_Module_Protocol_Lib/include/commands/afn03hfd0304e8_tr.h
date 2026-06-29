#ifndef AFN03HFD0304E8_TR_H
#define AFN03HFD0304E8_TR_H

#include <QObject>
#include "../QCSG_Local_Module_Protocol_Lib_global.h"
#include "../framelocalprotocolbase.h"
#include "../exceptions/decodeexception.h"

namespace  qcsg_local_module_protocol {

/**
 * @brief The Afn03HFD0304E8_TR 返回查询从节点其他信息
 */
#ifdef UNIT_TEST
class Afn03HFD0304E8_TR : public FrameLocalProtocolBase
#else
class QCSG_LOCAL_MODULE_PROTOCOL_LIB_EXPORT Afn03HFD0304E8_TR : public FrameLocalProtocolBase
#endif
{
public:
    Afn03HFD0304E8_TR();
    /**
     * @brief Afn03HFD0304E8_TR
     * @param ctrl_field 控制域
     * @param address_field 地址域
     * @param seq 帧序列域
     */
    Afn03HFD0304E8_TR(CtrlField ctrl_field,AddressField address_field,uchar seq);

    uchar answer_quantity_;                       //!< 本次应答的从节点数量
    QList<NodeOtherInfo_TR> node_other_info_list_;      //!< 从节点相位信息链表(广东修订)，定义详见<datatypedef.h>


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
#endif // AFN03HFD0304E8_TR_H
