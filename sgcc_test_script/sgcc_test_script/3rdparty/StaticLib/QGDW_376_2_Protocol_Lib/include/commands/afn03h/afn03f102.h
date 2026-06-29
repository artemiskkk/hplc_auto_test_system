#ifndef AFN03F102_H
#define AFN03F102_H

#include <QObject>
#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"
namespace qgdw_3762_protocol {
/**
 * @brief The DataIdLen struct
 */
struct DataIdLen
{
    char data_id_[4];//!<数据项
    uchar reply_data_len_;//!<回复长度
    bool operator==(const DataIdLen &unit_)const
    {
        if(memcmp(this->data_id_,unit_.data_id_,4)==0
                &&this->reply_data_len_==unit_.reply_data_len_)
            return true;
        else
            return false;
    }
    DataIdLen& operator=(const DataIdLen &unit_)
    {
        memcpy(this->data_id_,unit_.data_id_,4);
        this->reply_data_len_=unit_.reply_data_len_;

        return *this;
    }
};

/**
 * @brief The CurveDataUnitUp03F102 曲线数据单元
 */
struct CurveDataUnitUp03F102
{
    char enable_flag;
    uchar sample_cycle;
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
    bool operator==(const CurveDataUnitUp03F102 &unit_)const
    {
        if(this->protocol==unit_.protocol
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
};
#ifdef UNIT_TEST
class Afn03F102 : public Frame3762Base
#else
/**
 * @brief The Afn03F102 查询CCO模块曲线数据
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT Afn03F102 : public Frame3762Base
#endif
{
public:
    Afn03F102();
    /**
     * @brief Afn03F102
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */

    Afn03F102(CtrlField ctrl_field,InfoField info_field,AddressField address_field);
    char protocol_;//!<协议
    CurveDataUnitUp03F102 data_unit_up_;//!<数据单元上行
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

#endif // AFN03F102_H
