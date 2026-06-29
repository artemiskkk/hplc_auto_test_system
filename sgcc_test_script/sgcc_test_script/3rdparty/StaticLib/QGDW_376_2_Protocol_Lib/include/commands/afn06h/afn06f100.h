#ifndef AFN06F100_H
#define AFN06F100_H


#include <QObject>

#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"
namespace qgdw_3762_protocol {

#ifdef UNIT_TEST
class Afn06F100 : public Frame3762Base
#else
/**
 * @brief The AFN06F100 上报冻结数据
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT Afn06F100 : public Frame3762Base
#endif
{
public:
    /**
     * @brief Afn06F100
     */
    Afn06F100();
    /**
     * @brief Afn06F100
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    Afn06F100(CtrlField ctrl_field,InfoField info_field,AddressField address_field);

    char task_num_;//!<任务号
    QByteArray address_;//!<从节点地址
    char commun_type_;//!<通信协议类型
    QByteArray time_stamp_;//!<采集时间
    ushort frame_len_;//!<报文长度
    QByteArray frame_content_;//!<报文

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
#endif // AFN06F100_H
