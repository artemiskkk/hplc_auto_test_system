#ifndef AFN14F1_H
#define AFN14F1_H

#include <QObject>

#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"
namespace qgdw_3762_protocol {
/**
 * @brief The RouterRequestReadUnit 路由请求抄读内容数据单元结构体
 */
struct RouterRequestReadUnit
{
    char read_flag_;//!<抄读标志
    char delay_related_flag_;//!<通信延时相关性标志
    uchar frame_length_;//!<路由请求数据长度
    QByteArray frame_content_;//!<路由请求数据内容
    uchar subsidiary_node_num_;//!<附属从节点数量
    QList<Address> subsidiary_node_address_list_;//附属从节点地址QList
    /**
     * @brief operator ==
     * @param unit_
     * @return
     */
    bool operator==(const RouterRequestReadUnit &unit_)const
    {
        if(this->read_flag_==unit_.read_flag_
                &&this->delay_related_flag_==unit_.delay_related_flag_
                &&this->subsidiary_node_num_==unit_.subsidiary_node_num_
                &&this->frame_length_==unit_.frame_length_
                &&this->frame_content_==unit_.frame_content_
                &&this->subsidiary_node_address_list_.size()==unit_.subsidiary_node_address_list_.size())
        {
            for(int i=0;i<this->subsidiary_node_address_list_.size();i++)
            {
                if(!(this->subsidiary_node_address_list_.at(i)==unit_.subsidiary_node_address_list_.at(i)))
                    return false;
            }
            return true;
        }
        else
            return false;
    }
};

#ifdef UNIT_TEST
class Afn14F1 : public Frame3762Base
#else
/**
 * @brief The AFN14F1 路由请求抄读内容
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT Afn14F1 : public Frame3762Base
#endif
{
public:
    /**
     * @brief Afn14F1
     */
    Afn14F1();
    /**
     * @brief Afn14F1
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    Afn14F1(CtrlField ctrl_field,InfoField info_field,AddressField address_field);
    char phase_;//!<通信相位
    Address node_address_;//!<从节点地址
    ushort node_no_;//!<从节点序号

    RouterRequestReadUnit router_request_read_unit_;//!<路由请求抄读内容数据单元
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
#endif // AFN14F1_H
