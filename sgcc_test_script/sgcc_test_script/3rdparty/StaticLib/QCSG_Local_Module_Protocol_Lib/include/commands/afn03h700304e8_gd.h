#ifndef AFN03H700304E8_GD_H
#define AFN03H700304E8_GD_H

#include <QObject>
#include "../QCSG_Local_Module_Protocol_Lib_global.h"
#include "../framelocalprotocolbase.h"
#include "../exceptions/decodeexception.h"

namespace  qcsg_local_module_protocol {

/**
 * @brief The Afn03H700304E8_GD 广东扩展：返回节点自检信息
 */
#ifdef UNIT_TEST
class Afn03H700304E8_GD : public FrameLocalProtocolBase
#else
class QCSG_LOCAL_MODULE_PROTOCOL_LIB_EXPORT Afn03H700304E8_GD : public FrameLocalProtocolBase
#endif
{
public:
    Afn03H700304E8_GD();
    /**
     * @brief Afn03H700304E8_GD
     * @param ctrl_field 控制域
     * @param address_field 地址域
     * @param seq 帧序列域
     */
    Afn03H700304E8_GD(CtrlField ctrl_field,AddressField address_field,uchar seq);

    uchar zero_self_test_result_;        //!< 过零自检结果： 当节点为主节点或三相表模块时， 0： 未知； 1： 三相相序为ABC； 2： 三相相序错误； 3： 存在断相； 4： 存在相同相位； 当节点为单相表模块时， 0： 不支持过零； 1： 支持过零
    uchar serial_blicked_reason_;        //!< 串口/485不通状态： 0： 正常； 1： 历史上出现过不通现象； 2： 目前不通
    uchar last_off_net_reason_;          //!< 上次离网原因： 0： 未离网； 1： 组网序列号变化； 2： 2个路由周期收不到信标帧； 3： 与代理节点连续四个路由周期的通信成功率都是0； 4： 站点所在层级超过15级； 5： 收到离线指示； 0x80-0xEF:厂家自定义； 0xF0-0xFF:保留。
    uchar reset_reason_;                 //!< 复位原因： 0： 掉电复位； 1： 复位引脚复位； 2： 升级完成复位； 3： CCO 控制从节点重启； 0x80-0xEF:厂家自定义； 0xF0-0xFF： 保留

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
#endif // AFN03H700304E8_GD_H
