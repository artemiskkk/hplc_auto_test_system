#ifndef AFN06F55_H
#define AFN06F55_H


#include <QObject>

#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"
namespace qgdw_3762_protocol {
/**
 * @brief The ReportEventUnit 上报从节点事件数据单元结构体
 */
struct ReportEventUnitF55
{
    char node_device_type_;//!<从节点设备类型
    char node_protocol_type_;//!<通信协议类型
    ushort frame_length_;//!<报文长度L
    QByteArray frame_content_;//!<报文内容
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const ReportEventUnitF55 &unit_)const
    {
        if(this->node_device_type_==unit_.node_device_type_
                &&this->node_protocol_type_==unit_.node_protocol_type_
                 &&this->frame_length_==unit_.frame_length_
                 &&this->frame_content_==unit_.frame_content_)
            return true;
        else
            return false;
    }
};

#ifdef UNIT_TEST
class Afn06F55 : public Frame3762Base
#else
/**
 * @brief The AFN06F55 上报从节点事件
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT Afn06F55 : public Frame3762Base
#endif
{
public:
    /**
     * @brief Afn06F55
     */
    Afn06F55();
    /**
     * @brief Afn06F55
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    Afn06F55(CtrlField ctrl_field,InfoField info_field,AddressField address_field);

    ReportEventUnitF55 report_event_unit_;//!<上报从节点事件数据单元
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
#endif // AFN06F55_H
