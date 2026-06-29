#ifndef AFNF0HFBF004E8_H
#define AFNF0HFBF004E8_H

#include <QObject>
#include "../QCSG_Local_Module_Protocol_Lib_global.h"
#include "../framelocalprotocolbase.h"
#include "../exceptions/decodeexception.h"

namespace  qcsg_local_module_protocol {

/**
 * @brief The AfnF0HFBF004E8 厂家自定义：返回功率参数
 */
#ifdef UNIT_TEST
class AfnF0HFBF004E8 : public FrameLocalProtocolBase
#else
class QCSG_LOCAL_MODULE_PROTOCOL_LIB_EXPORT AfnF0HFBF004E8 : public FrameLocalProtocolBase
#endif
{
public:
    AfnF0HFBF004E8();
    /**
     * @brief AfnF0HFBF004E8
     * @param ctrl_field 控制域
     * @param address_field 地址域
     * @param seq 帧序列域
     */
    AfnF0HFBF004E8(CtrlField ctrl_field,AddressField address_field,uchar seq);

    uchar dac_output_volt_;  //!< DAC输出电压：范围4~20
    uchar peak_clipping_divisor_; //!< 削峰因子：0，5~25

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
#endif // AFNF0HFBF004E8_H
