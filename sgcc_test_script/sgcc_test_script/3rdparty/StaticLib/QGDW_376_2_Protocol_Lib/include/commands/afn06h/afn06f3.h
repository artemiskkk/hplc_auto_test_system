#ifndef AFN06F3_H
#define AFN06F3_H

#include <QObject>

#include "../../frame3762base.h"
#include "../../exceptions/decodeexception.h"
namespace qgdw_3762_protocol {

#ifdef UNIT_TEST
class Afn06F3 : public Frame3762Base
#else
/**
 * @brief The AFN06F3 上报路由工况变动信息
 */
class QGDW_376_2_PROTOCOL_LIB_EXPORT Afn06F3 : public Frame3762Base
#endif
{
public:
    /**
     * @brief Afn06F3
     */
    Afn06F3();
    /**
     * @brief Afn06F3
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     */
    Afn06F3(CtrlField ctrl_field,InfoField info_field,AddressField address_field);

    char router_work_task_change_;//!<路由工作任务变动类型
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

#endif // AFN06F3_H
