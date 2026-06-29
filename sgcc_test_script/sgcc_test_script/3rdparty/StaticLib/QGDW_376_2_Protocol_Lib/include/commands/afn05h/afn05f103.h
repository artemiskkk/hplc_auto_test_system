#ifndef AFN05F103_H
#define AFN05F103_H

#include <QObject>

#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"
#include "../afn03h/afn03f102.h"
namespace qgdw_3762_protocol {
/**
 * @brief The CurveDataUnitDown05F103 曲线数据单元
 */
struct CurveDataUnitDown05F103
{
    char set_flag_;//启停标志
    uchar read_period_;//采集周期
    char protocol;//!<协议
    char single_meter_=0x00;//!<单相表
    uchar single_data_id_num_;//!<单相数据项数量n
    uchar single_reply_data_len_;//!<预计回复总长度
    QList<DataIdLen> single_data_id_len_list_;//!<单相数据项内容list
    char tri_meter_=0x01;//!<三相表
    uchar tri_data_id_num_;//!<三相数据项数量n
    uchar tri_reply_data_len_;//!<预计回复总长度
    QList<DataIdLen> tri_data_id_len_list_;//!<三相数据项内容list
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const CurveDataUnitDown05F103 &unit_)const
    {
        if(this->set_flag_==unit_.set_flag_
            &&this->read_period_==unit_.read_period_
            &&this->protocol==unit_.protocol
            &&this->single_data_id_num_==unit_.single_data_id_num_
            &&this->single_reply_data_len_==unit_.single_reply_data_len_
            &&this->tri_data_id_num_==unit_.tri_data_id_num_
            &&this->tri_reply_data_len_==unit_.tri_reply_data_len_
            &&this->single_data_id_len_list_.size()==unit_.single_data_id_len_list_.size()
            &&this->tri_data_id_len_list_.size()==unit_.tri_data_id_len_list_.size())
        {
            for(int i=0;i<single_data_id_len_list_.size();i++)
            {
                if(!unit_.single_data_id_len_list_.contains(single_data_id_len_list_.at(i)))
                    return false;
            }
            for(int i=0;i<tri_data_id_len_list_.size();i++)
            {
                if(!unit_.tri_data_id_len_list_.contains(tri_data_id_len_list_.at(i)))
                    return false;
            }
            return true;
        }
        else
            return false;
    }
    CurveDataUnitDown05F103& operator=(const CurveDataUnitDown05F103 &unit_)
    {
        this->set_flag_=unit_.set_flag_;
        this->read_period_=unit_.read_period_;
        this->protocol=unit_.protocol;
        this->single_data_id_num_=unit_.single_data_id_num_;
        this->single_reply_data_len_=unit_.single_reply_data_len_;
        this->tri_data_id_num_=unit_.tri_data_id_num_;
        this->tri_reply_data_len_=unit_.tri_reply_data_len_;
        this->single_data_id_len_list_.clear();
        this->single_data_id_len_list_.append(unit_.single_data_id_len_list_);
        this->tri_data_id_len_list_.clear();
        this->tri_data_id_len_list_.append(unit_.tri_data_id_len_list_);

        return *this;
    }
};
#ifdef UNIT_TEST
class Afn05F103 : public Frame3762Base
#else
/**
 * @brief The Afn05F103 设置STA模块曲线配置
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT Afn05F103 : public Frame3762Base
#endif
{
public:
    Afn05F103();
    /**
     * @brief Afn05F103
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */

    Afn05F103(CtrlField ctrl_field,InfoField info_field,AddressField address_field);
    CurveDataUnitDown05F103 curve_config_data_;//!<曲线配置数据
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
#endif // AFN05F103_H
