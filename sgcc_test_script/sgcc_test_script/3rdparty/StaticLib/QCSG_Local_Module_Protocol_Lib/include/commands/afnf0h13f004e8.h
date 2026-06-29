#ifndef AFNF0H13F004E8_H
#define AFNF0H13F004E8_H

#include <QObject>
#include "../QCSG_Local_Module_Protocol_Lib_global.h"
#include "../framelocalprotocolbase.h"
#include "../exceptions/decodeexception.h"

namespace  qcsg_local_module_protocol {

/**
 * @brief The AfnF0H13F004E8 厂家自定义：返回过零序列
 */
#ifdef UNIT_TEST
class AfnF0H13F004E8 : public FrameLocalProtocolBase
#else
class QCSG_LOCAL_MODULE_PROTOCOL_LIB_EXPORT AfnF0H13F004E8 : public FrameLocalProtocolBase
#endif
{
public:
    AfnF0H13F004E8();
    /**
     * @brief AfnF0H13F004E8
     * @param ctrl_field 控制域
     * @param address_field 地址域
     * @param seq 帧序列域
     */
    AfnF0H13F004E8(CtrlField ctrl_field,AddressField address_field,uchar seq);

    uchar answer_state_;//!< 应答状态：0：正常应答；1：下发参数错误；2：应答数据异常
    uchar phase_;  //!< 相限： 1~3: A~C相限；0:全相限
    uchar zero_seq_count_;  //!< 过零序列元素个数：需要上报过零时间点NTB的个数
    QList<uint> zero_list_; //!< 过零序列：过零点ntb

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
#endif // AFNF0H13F004E8_H
