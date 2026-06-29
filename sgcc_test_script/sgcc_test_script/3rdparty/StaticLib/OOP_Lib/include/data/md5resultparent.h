#ifndef MD5RESULTPARENT_H
#define MD5RESULTPARENT_H

#include <QObject>
#include "enumerated.h"
#include "datatype.h"
#include "../OOP_Lib_global.h"

namespace  object_oriented_electic_data_exchange_protocol {

/**
 * @brief MD5值结果-基类
 */
#ifdef UNIT_TEST
class MD5ResultParent
#else
class OOP_LIB_EXPORT MD5ResultParent
#endif
{
public:
    MD5ResultParent(MD5ResultChoice choice);
    virtual ~MD5ResultParent();

    MD5ResultChoice choice_;
    virtual QByteArray EncodeFrame()=0;
    virtual void DecodeFrame(QByteArray *data)=0;
};

/**
 * @brief MD5值结果--错误信息子类
 */
#ifdef UNIT_TEST
class MD5ResultDAR : public MD5ResultParent
#else
class OOP_LIB_EXPORT MD5ResultDAR : public MD5ResultParent
#endif
{
public:
    MD5ResultDAR();
    ~MD5ResultDAR() override;

    DAR dar_;
    QByteArray EncodeFrame() override;
    void DecodeFrame(QByteArray *data) override;

};

/**
 * @brief MD5值结果--MD5值子类
 */
#ifdef UNIT_TEST
class MD5ResultValue : public MD5ResultParent
#else
class OOP_LIB_EXPORT MD5ResultValue : public MD5ResultParent
#endif
{
public:
    MD5ResultValue();
    ~MD5ResultValue() override;

    QByteArray md5_value_;//!< MD5值
    uchar GetMD5ValueLength();

    QByteArray EncodeFrame() override;
    void DecodeFrame(QByteArray *data) override;
private:
    uchar length_;//!< 数据长度
};

/**
 * @brief MD5值工厂类
 */
#ifdef UNIT_TEST
class MD5ResultFactory
#else
class OOP_LIB_EXPORT MD5ResultFactory
#endif
{
public:
    MD5ResultFactory(){}
    ~MD5ResultFactory(){}

    std::shared_ptr<MD5ResultParent> CreateMD5Result(uchar choice);
};

}

#endif // MD5RESULTPARENT_H
