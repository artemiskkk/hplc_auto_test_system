#ifndef REGION_H
#define REGION_H

#include <QObject>
#include "datatypebasedef.h"
#include "enumerated.h"
#include "datatype.h"
#include "tsa.h"
#include "../OOP_Lib_global.h"

namespace  object_oriented_electic_data_exchange_protocol {


/**
 * @brief 区间类型Region定义-基类
 */
#ifdef UNIT_TEST
class RegionParent
#else
class OOP_LIB_EXPORT RegionParent
#endif
{
public:
    RegionParent();
    RegionParent(DataType type);
    virtual ~RegionParent();

    RegionChoice choice_;//!< 区间类型种类选择
    virtual QByteArray EncodeFrame()=0;
    virtual void DecodeFrame(QByteArray *data)=0;
//protected:
    DataType type_;//!< 数据类型
};

/**
 * @brief 区间类型Region定义--Data子类
 */
#ifdef UNIT_TEST
class RegionData :public RegionParent
#else
class OOP_LIB_EXPORT RegionData :public RegionParent
#endif
{
public:
    RegionData();
    ~RegionData() override;

    std::shared_ptr<DataParent> start_value_ptr_ = nullptr;//!< 起始值
    std::shared_ptr<DataParent> end_value_ptr_ = nullptr;//!< 结束值
    QByteArray EncodeFrame() override;
    void DecodeFrame(QByteArray *data) override;
};

/**
 * @brief 区间类型Region定义--Unsigned子类
 */
#ifdef UNIT_TEST
class RegionUnsigned :public RegionParent
#else
class OOP_LIB_EXPORT RegionUnsigned :public RegionParent
#endif
{
public:
    RegionUnsigned();
    ~RegionUnsigned() override;

    uchar start_value_;//!< 起始值
    uchar end_value_;//!< 结束值
    QByteArray EncodeFrame() override;
    void DecodeFrame(QByteArray *data) override;
};

/**
 * @brief 区间类型Region定义--LongUnsigned子类
 */
#ifdef UNIT_TEST
class RegionLongUnsigned :public RegionParent
#else
class OOP_LIB_EXPORT RegionLongUnsigned :public RegionParent
#endif
{
public:
    RegionLongUnsigned();
    ~RegionLongUnsigned() override;

    ushort start_value_;//!< 起始值
    ushort end_value_;//!< 结束值
    QByteArray EncodeFrame() override;
    void DecodeFrame(QByteArray *data) override;
};

/**
 * @brief 区间类型Region定义--TSA子类
 */
#ifdef UNIT_TEST
class RegionTsa :public RegionParent
#else
class OOP_LIB_EXPORT RegionTsa :public RegionParent
#endif
{
public:
    RegionTsa();
    ~RegionTsa() override;

    std::shared_ptr<TSA> start_value_ptr_ = nullptr;//!< 起始值
    std::shared_ptr<TSA> end_value_ptr_ = nullptr;//!< 结束值
    QByteArray EncodeFrame() override;
    void DecodeFrame(QByteArray *data) override;
};

}

#endif // REGION_H
