#ifndef AFN05F3_H
#define AFN05F3_H

#include <QObject>

#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"
namespace qgdw_3762_protocol {
/**
 * @brief The BroadcastDataUnit 启动广播数据单元结构体
 */
struct BroadcastDataUnit
{
    char ctrl_word_;//!<控制字
    uchar frame_length_;//!<报文长度L
    QByteArray frame_content_;//!<报文内容
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const BroadcastDataUnit &unit_)const
    {
        if(this->ctrl_word_==unit_.ctrl_word_
                &&this->frame_length_==unit_.frame_length_
                &&this->frame_content_==unit_.frame_content_)
            return true;
        else
            return false;
    }
};
#ifdef UNIT_TEST
class Afn05F3 : public Frame3762Base
#else
/**
 * @brief The AFN05F3 启动广播
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT Afn05F3 : public Frame3762Base
#endif
{
public:
    /**
     * @brief Afn05F3
     */
    Afn05F3();
    /**
     * @brief Afn05F3
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    Afn05F3(CtrlField ctrl_field,InfoField info_field,AddressField address_field);

    BroadcastDataUnit broadcast_data_unit_;//!<启动广播数据单元结构体
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
#endif // AFN05F3_H
