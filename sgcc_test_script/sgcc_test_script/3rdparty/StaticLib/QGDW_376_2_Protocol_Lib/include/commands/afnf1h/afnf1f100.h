#ifndef AFNF1F100_H
#define AFNF1F100_H

#include <QObject>

#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"

namespace qgdw_3762_protocol {
/**
 * @brief The ConcurrentReadMeterUnitDownF1F100 并发抄表数据单元下行
 */
struct ConcurrentReadMeterUnitDownF1F100
{
    char protocol_type_;//!<规约类型
    char preserve_;//!<保留
    ushort frame_length_;//!<报文长度
    QByteArray frame_content_;//!<报文内容
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const ConcurrentReadMeterUnitDownF1F100 &unit_)const
    {
        if(this->protocol_type_==unit_.protocol_type_
                &&this->preserve_==unit_.preserve_
                &&this->frame_length_==unit_.frame_length_
                &&this->frame_content_==unit_.frame_content_)
            return true;
        else
            return false;
    }
};
/**
 * @brief The ConcurrentReadMeterUnitUpF1F100 并发抄表数据单元上行
 */
struct ConcurrentReadMeterUnitUpF1F100
{
    //char read_state_;//!<抄读结果状态
    char protocol_type_;//!<规约类型
    ushort frame_length_;//!<报文长度
    QByteArray frame_content_;//!<报文内容
    /**
    * @brief operator ==
    * @param unit_
    * @return
    */
    bool operator==(const ConcurrentReadMeterUnitUpF1F100 &unit_)const
    {
       if(this->protocol_type_==unit_.protocol_type_
               //&&this->read_state_==unit_.read_state_
               &&this->frame_length_==unit_.frame_length_
               &&this->frame_content_==unit_.frame_content_)
           return true;

       else
           return false;
    }
};

#ifdef UNIT_TEST
class AfnF1F100 : public Frame3762Base
#else
/**
 * @brief The AfnF1F100 集中器主动并发
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT AfnF1F100 : public Frame3762Base
#endif
{
public:
    /**
     * @brief AfnF1F100
     */
    AfnF1F100();
    /**
     * @brief AfnF1F100
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    AfnF1F100(CtrlField ctrl_field,InfoField info_field,AddressField address_field);

    ConcurrentReadMeterUnitDownF1F100 unit_down_;//!<并发抄表数据单元

    ConcurrentReadMeterUnitUpF1F100 unit_up_;//!<并发抄表数据单元
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


#endif // AFNF1F100_H
