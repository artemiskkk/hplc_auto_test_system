#ifndef FRAME3762BASE_H
#define FRAME3762BASE_H

#include <QObject>
#include <QDebug>

#include "datatypedef.h"
#include "QGDW_376_2_Protocol_Lib_global.h"

namespace  qgdw_3762_protocol {
/**
 * @brief 376.2帧格式基类
 */
#ifdef UNIT_TEST
class Frame3762Base
#else
class QGDW_376_2_PROTOCOL_LIB_EXPORT Frame3762Base
#endif
{
public:
    Frame3762Base();
    /**
     * @brief Frame3762Base
     * @param ctrl_field 控制域
     * @param info_field 信息域
     * @param address_field 地址域
     * @param afn AFN
     * @param dt1 DT1
     * @param dt2 DT2
     */
    Frame3762Base(CtrlField ctrl_field,InfoField info_field,AddressField address_field,char afn,char dt1,char dt2);

    /**
     * @brief 析构函数
     */
    virtual ~Frame3762Base();

    /**
     * @brief 将该帧编码为16进制格式数据
     * @return 反回编码好的字节串
     */
    QByteArray EncodeFrame();

    /**
     * @brief 将16进制格式数据，解析为数据单元，由子类实现
     * @param data
     */
    virtual void DecodeFrameDataField(QByteArray data)=0;

    ushort len_;                    //!< 帧长度域
    CtrlField ctrl_field_;          //!< 控制域
    InfoField info_field_;          //!< 信息域
    AddressField address_field_;    //!< 地址域
    char afn_;                      //!< 应用功能码 AFN
    char dt1_;                      //!< 数据单元标识 DT1
    char dt2_;                      //!< 数据单元标识 DT2
    QByteArray data_field_;         //!< 数据单元
    char cs_;                       //!< 校验

private:

    const char start_flag_ = 0x68;  //!< 起始位固定为 0x68
    const char end_flag_ = 0x16;    //!< 终止位固定为 0x16


protected:
    /**
     * @brief 将数据单元编码为16进制格式数据，由子类实现
     * @return 反回编码好的字节串
     */
    virtual QByteArray EncodeFrameDataField()=0;


};

}

#endif // FRAME3762BASE_H
