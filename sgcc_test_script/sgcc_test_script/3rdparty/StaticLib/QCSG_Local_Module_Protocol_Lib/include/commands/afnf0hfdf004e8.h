#ifndef AFNF0HFDF004E8_H
#define AFNF0HFDF004E8_H

#include <QObject>
#include "../QCSG_Local_Module_Protocol_Lib_global.h"
#include "../framelocalprotocolbase.h"
#include "../exceptions/decodeexception.h"

namespace  qcsg_local_module_protocol {

/**
 * @brief The AfnF0HFDF004E8 厂家自定义：返回台区判别阈值
 */
#ifdef UNIT_TEST
class AfnF0HFDF004E8 : public FrameLocalProtocolBase
#else
class QCSG_LOCAL_MODULE_PROTOCOL_LIB_EXPORT AfnF0HFDF004E8 : public FrameLocalProtocolBase
#endif
{
public:
    AfnF0HFDF004E8();
    /**
     * @brief AfnF0HFDF004E8
     * @param ctrl_field 控制域
     * @param address_field 地址域
     * @param seq 帧序列域
     */
    AfnF0HFDF004E8(CtrlField ctrl_field,AddressField address_field,uchar seq);

    uchar threshold_value_;//!< 台区判别阈值,阈值范围0~255

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
#endif // AFNF0HFDF004E8_H
