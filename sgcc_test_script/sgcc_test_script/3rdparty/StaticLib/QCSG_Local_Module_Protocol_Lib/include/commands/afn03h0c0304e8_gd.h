#ifndef AFN03H0C0304E8_GD_H
#define AFN03H0C0304E8_GD_H

#include <QObject>
#include "../QCSG_Local_Module_Protocol_Lib_global.h"
#include "../framelocalprotocolbase.h"
#include "../exceptions/decodeexception.h"

namespace  qcsg_local_module_protocol {

/**
 * @brief The Afn03H0C0304E8_GD 广东修订：返回查询从节点相位信息
 */
#ifdef UNIT_TEST
class Afn03H0C0304E8_GD : public FrameLocalProtocolBase
#else
class QCSG_LOCAL_MODULE_PROTOCOL_LIB_EXPORT Afn03H0C0304E8_GD : public FrameLocalProtocolBase
#endif
{
public:
    Afn03H0C0304E8_GD();
    /**
     * @brief Afn03H0C0304E8_GD
     * @param ctrl_field 控制域
     * @param address_field 地址域
     * @param seq 帧序列域
     */
    Afn03H0C0304E8_GD(CtrlField ctrl_field,AddressField address_field,uchar seq);

    uchar answer_quantity_;                       //!< 本次应答的从节点数量
    QList<NodePhaseInfo_GD> node_phase_info_list_;      //!< 从节点相位信息链表(广东修订)，定义详见<datatypedef.h>


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
     * @return 返回编码好的字节串
     */
    QByteArray EncodeFrameDataField() override;
};

}
#endif // AFN03H0C0304E8_GD_H
