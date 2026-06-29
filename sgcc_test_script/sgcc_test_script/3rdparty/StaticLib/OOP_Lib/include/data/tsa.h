#ifndef TSA_H
#define TSA_H

#include <QObject>
#include "datatypebasedef.h"
#include "../OOP_Lib_global.h"

#include "../exceptions/decodeexception.h"


namespace  object_oriented_electic_data_exchange_protocol {

/**
 * @brief 目标服务器地址TSA（Target Server Address）的数据类型定义
 */
#ifdef UNIT_TEST
class TSA
#else
class OOP_LIB_EXPORT TSA
#endif
{
public:
    TSA();

    uchar size_;//!< 字节数
    ServerAddress tsa_;//!< 目标服务器地址T

    QByteArray EncodeFrame();
    void DecodeFrame(QByteArray *data);

    /**
     * @brief operator ==  重载运算符“==”
     * @param tsa
     * @return 返回比较结果
     */
    bool operator==(const TSA &tsa) const
    {
        if(this->size_ == tsa.size_
                && this->tsa_ == tsa.tsa_
                )
            return true;
        else
            return false;
    }
};

}

#endif // TSA_H
