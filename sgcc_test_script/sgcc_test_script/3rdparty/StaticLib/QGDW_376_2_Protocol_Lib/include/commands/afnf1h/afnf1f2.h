#ifndef AFNF1F2_H
#define AFNF1F2_H

#include <QObject>

#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"
namespace qgdw_3762_protocol {


#ifdef UNIT_TEST
class AfnF1F2 : public Frame3762Base
#else
/**
 * @brief The AFNF1F2 集中器主动汇聚抄表
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT AfnF1F2 : public Frame3762Base
#endif
{
public:
    /**
     * @brief AfnF1F2
     */
    AfnF1F2();
    /**
     * @brief AfnF1F2
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    AfnF1F2(CtrlField ctrl_field,InfoField info_field,AddressField address_field);
    char protocol_type_;//!<规约类型
    ushort frame_length_;//!<报文长度
    QByteArray frame_content_;//!<报文内容
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
#endif // AFNF1F2_H
