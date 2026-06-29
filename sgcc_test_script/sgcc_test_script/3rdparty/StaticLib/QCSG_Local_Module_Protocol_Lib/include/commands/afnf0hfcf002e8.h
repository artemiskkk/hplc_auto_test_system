#ifndef AFNF0HFCF002E8_H
#define AFNF0HFCF002E8_H

#include <QObject>
#include "../QCSG_Local_Module_Protocol_Lib_global.h"
#include "../framelocalprotocolbase.h"
#include "../exceptions/decodeexception.h"

namespace  qcsg_local_module_protocol {

/**
 * @brief The AfnF0HFCF002E8 厂家自定义：设置频段兼容参数
 */
#ifdef UNIT_TEST
class AfnF0HFCF002E8 : public FrameLocalProtocolBase
#else
class QCSG_LOCAL_MODULE_PROTOCOL_LIB_EXPORT AfnF0HFCF002E8 : public FrameLocalProtocolBase
#endif
{
public:
    AfnF0HFCF002E8();
    /**
     * @brief AfnF0HFCF002E8
     * @param ctrl_field 控制域
     * @param address_field 地址域
     * @param seq 帧序列域
     */
    AfnF0HFCF002E8(CtrlField ctrl_field,AddressField address_field,uchar seq);

    ushort exe_cycle_;   //!< 执行周期取值范围0~65535，单位：分钟，表示每隔多长时间执行一次。取值为0时，表示只进行一次，不再周期执行
    uchar max_exe_time_; //!< 最大执行时间取值范围0~255，单位：分钟，取值为0时，无效

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
#endif // AFNF0HFCF002E8_H
