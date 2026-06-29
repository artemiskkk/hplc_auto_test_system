#ifndef FRAME645BASE_H
#define FRAME645BASE_H

#include <QObject>
#include <QDebug>

#include "datatypedef.h"
#include "DLT_645_Protocol_Lib_global.h"

namespace dlt_645_Protocol {


/**
 * @brief 645帧格式基类
 */
#ifdef UNIT_TEST
class Frame645Base
#else
class DLT_645_PROTOCOL_LIB_EXPORT Frame645Base
#endif
{
public:
    Frame645Base(uchar *addr, uchar ctrlCode, uchar dataLen);
    virtual ~Frame645Base();


    uchar addr_[6];
    uchar ctrlCode_;
    uchar dataLen_;
    QByteArray data_field_;
    uchar cs_;



    QByteArray EncodeFrame();

    /**
     * @brief 将16进制格式数据，解析为数据单元，由子类实现
     * @param data
     */
    virtual void DecodeFrameDataField(QByteArray data)=0;


private:
    const uchar start_flag1_ = 0x68;  //!< 起始位固定为 0x68
    const uchar start_flag2_ = 0x68;  //!< 第2个0x68
    const uchar end_flag_ = 0x16;    //!< 终止位固定为 0x16


protected:
    /**
     * @brief 将数据单元编码为16进制格式数据，由子类实现
     * @return 反回编码后的字节串
     */
    virtual QByteArray EncodeFrameDataField()=0;


};



}


#endif // FRAME645BASE_H
