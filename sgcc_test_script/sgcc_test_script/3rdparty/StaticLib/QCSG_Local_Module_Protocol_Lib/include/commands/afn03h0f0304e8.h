#ifndef AFN03H0F0304E8_H
#define AFN03H0F0304E8_H

#include <QObject>
#include "../QCSG_Local_Module_Protocol_Lib_global.h"
#include "../framelocalprotocolbase.h"
#include "../exceptions/decodeexception.h"

namespace  qcsg_local_module_protocol {

/**
 * @brief The Afn03H0F0304E8 返回查询节点的台区识别结果
 * * @verbatim
 * 标准南网：返回查询多余节点的台区识别结果
 * 广东修订：返回查询非本台区节点的台区识别结果
 * @endverbatim
 */
#ifdef UNIT_TEST
class Afn03H0F0304E8 : public FrameLocalProtocolBase
#else
class QCSG_LOCAL_MODULE_PROTOCOL_LIB_EXPORT Afn03H0F0304E8 : public FrameLocalProtocolBase
#endif
{
public:
    Afn03H0F0304E8();
    /**
     * @brief Afn03H0F0304E8
     * @param ctrl_field 控制域
     * @param address_field 地址域
     * @param seq 帧序列域
     */
    Afn03H0F0304E8(CtrlField ctrl_field,AddressField address_field,uchar seq);

    ushort total_count_;                              //!< 标准南网：台区识别结果从节点总数量； 广东修订：非本台区从节点总数量
    uchar answer_quantity_;                       //!< 标准南网：本次应答的节点总数量； 广东修订：本次应答的非本台区节点数量
    QList<PlatIdentAttr> plat_ident_attr_list_;      //!< 标准南网：节点及节点属性链表； 广东修订：节点及节点属性链表；  定义详见<datatypedef.h>

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
#endif // AFN03H0F0304E8_H
