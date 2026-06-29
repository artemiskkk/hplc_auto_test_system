#ifndef FRAMEOOPBASE_H
#define FRAMEOOPBASE_H

#include <QObject>
#include <QMap>
#include <memory>
#include "OOP_Lib_global.h"
#include "data/datatypedef.h"
#include "data/aresultnormal.h"
#include "data/aresultrecord.h"
#include "data/ms.h"
#include "data/oadtodatatype.h"
#include "data/enumerated.h"
#include "data/connectmechanism.h"
#include "data/timetagfield.h"
#include "data/followreportfield.h"
#include "data/subframingresponseparent.h"
#include "data/md5resultparent.h"
#include "exceptions/decodeexception.h"
#include "data/aresultsimplifyrecord.h"
#include"data/transresultparent.h"

namespace  object_oriented_electic_data_exchange_protocol {

/**
 * @brief 面向对象的用电信息数据交换协议帧格式基类
 */
#ifdef UNIT_TEST
class FrameOOPBase
#else
class OOP_LIB_EXPORT FrameOOPBase
#endif
{
public:
    FrameOOPBase();

    //FrameOOPBase(CtrlField ctrl_field,AddressField address_field,FrameFormatField  framing_field,uchar service_type,uchar service_sub_type = 0xff);
    FrameOOPBase(CtrlField ctrl_field,AddressField address_field,uchar service_type,uchar service_sub_type = 0xff);

    /**
     * @brief 析构函数
     */
    virtual ~FrameOOPBase();

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

    LengthField len_field_;        //!< 帧长度域
    CtrlField ctrl_field_;         //!< 控制域,定义见<datatypedef.h>
    AddressField address_field_;   //!< 地址域,定义见<datatypedef.h>
    TimeTagField time_tag_field_;  //!< 时间标签域,定义见<datatypedef.h>

    uchar service_type_;           //!< 服务类型
    uchar service_sub_type_;       //!< 服务子类型
    ushort hcs_;                   //!< 帧头校验
    ushort fcs_;                   //!< 帧校验

    FrameFormatField  framing_field_;  //!< 分帧格式域,定义见<datatypedef.h>
    static QByteArray apdu_fragment_;  //!< APDU片段

    static ushort Crc16Check(QByteArray data);

private:

    const char start_flag_ = 0x68;  //!< 起始位固定为 0x68
    const char end_flag_ = 0x16;    //!< 终止位固定为 0x16

//    static const char kDirDown= 0;  //!< 传输方向位：由集中器、采集器发出的下行报文
//    static const char kDirUp  = 1;  //!< 传输方向位：通信模块发出的上行报文

protected:
    /**
     * @brief 将数据单元编码为16进制格式数据，由子类实现
     * @return 返回编码好的字节串
     */
    virtual QByteArray EncodeFrameDataField()=0;
};

}
#endif // FRAMEOOPBASE_H
