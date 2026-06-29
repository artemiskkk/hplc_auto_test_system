#ifndef AFNF0F4_H
#define AFNF0F4_H

#include <QObject>

#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"

namespace qgdw_3762_protocol {
/**
 * @brief The ZeroTestUnit 过零序列测试单元结构体
 */
struct ZeroTestUnit
{
    char response_state_;//!<应答状态
    char phase_;//!<相线
    uchar element_num_;//!<过零序列元素个数
    QList<uint> zero_list_;//!<过零序列QList
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const ZeroTestUnit &unit_)const
    {
        if(this->response_state_==unit_.response_state_
                &&this->phase_==unit_.phase_
                &&this->element_num_==unit_.element_num_
                &&this->zero_list_.size()==unit_.zero_list_.size())
        {
            for(int i=0;i<this->zero_list_.size();i++)
            {
                if(!(this->zero_list_.at(i)==unit_.zero_list_.at(i)))
                    return false;
            }
            return true;
        }
        else
            return false;
    }
};

#ifdef UNIT_TEST
class AfnF0F4 : public Frame3762Base
#else
/**
 * @brief The AfnF0F4 过零序列测试
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT AfnF0F4 : public Frame3762Base
#endif
{
public:
    /**
     * @brief AfnF0F4
     */
    AfnF0F4();
    /**
     * @brief AfnF0F4
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    AfnF0F4(CtrlField ctrl_field,InfoField info_field,AddressField address_field);

    ZeroTestUnit zero_test_unit_;//!<过零序列测试单元
    char phase_;//!<相线
    uchar element_num_;//!<过零序列元素个数
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

#endif // AFNF0F4_H
