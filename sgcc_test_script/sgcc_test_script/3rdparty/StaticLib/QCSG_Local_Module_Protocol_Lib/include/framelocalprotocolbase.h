#ifndef FRAMELOCALPROTOCOLBASE_H
#define FRAMELOCALPROTOCOLBASE_H

#include <QObject>

#include "QCSG_Local_Module_Protocol_Lib_global.h"
#include "datatypedef.h"

namespace  qcsg_local_module_protocol {

/**
 * @brief 南网本地通信模块接口协议帧格式基类
 */
#ifdef UNIT_TEST
class FrameLocalProtocolBase
#else
class QCSG_LOCAL_MODULE_PROTOCOL_LIB_EXPORT FrameLocalProtocolBase
#endif
{
public:
    FrameLocalProtocolBase();

    /**
     * @brief FrameLocalProtocolBase
     * @param ctrl_field 控制域
     * @param address_field 地址域
     * @param afn  应用功能码AFN
     * @param seq  帧序列域SEQ
     * @param di   数据标识编码DI
     */
    FrameLocalProtocolBase(CtrlField ctrl_field,AddressField address_field,uchar afn,uchar seq,QString di);

    /**
     * @brief 析构函数
     */
    virtual ~FrameLocalProtocolBase();

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
    CtrlField ctrl_field_;          //!< 控制域,定义见<datatypedef.h>
    AddressField address_field_;    //!< 地址域,定义见<datatypedef.h>
    uchar afn_;                      //!< 应用功能码 AFN
    uchar seq_;                      //!< 帧序列域SEQ
    QString di_;                    //!< 数据标识编码DI
    //QByteArray data_field_;         //!< 数据单元
    uchar cs_;                       //!< 校验

private:

    const char start_flag_ = 0x68;  //!< 起始位固定为 0x68
    const char end_flag_ = 0x16;    //!< 终止位固定为 0x16

//    static const char kDirDown= 0;  //!< 传输方向位：由集中器、采集器发出的下行报文
//    static const char kDirUp  = 1;  //!< 传输方向位：通信模块发出的上行报文

//    static const char kNoAddr  = 0; //!< 地址域标识位：不带地址域
//    static const char kHasAddr = 1; //!< 地址域标识位：带地址域

protected:
    /**
     * @brief 将数据单元编码为16进制格式数据，由子类实现
     * @return 反回编码好的字节串
     */
    virtual QByteArray EncodeFrameDataField()=0;
};

}

#endif // FRAMELOCALPROTOCOLBASE_H
