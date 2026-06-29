#ifndef AFN05F30_ZHEJIANG_H
#define AFN05F30_ZHEJIANG_H

#include <QObject>
#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"
namespace qgdw_3762_protocol {

#ifdef UNIT_TEST
class Afn05F30_Zhejiang : public Frame3762Base
#else
/**
 * @brief The Afn05F30_Zhejiang
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT Afn05F30_Zhejiang : public Frame3762Base
#endif
{
public:
    /**
     * @brief Afn05F30_Zhejiang
     */
    Afn05F30_Zhejiang();
    /**
     * @brief Afn05F30_Zhejiang
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    Afn05F30_Zhejiang(CtrlField ctrl_field,InfoField info_field,AddressField address_field);
    ushort online_lock_time_;//!<在网锁定时间
    ushort leave_lock_time_;//!<离网锁定时间
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
#endif // AFN05F30_ZHEJIANG_H
