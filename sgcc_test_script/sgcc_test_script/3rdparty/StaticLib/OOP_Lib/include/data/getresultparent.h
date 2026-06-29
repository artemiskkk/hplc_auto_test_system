#ifndef GETRESULTPARENT_H
#define GETRESULTPARENT_H

#include <QObject>
#include "datatypebasedef.h"
#include "enumerated.h"
#include "datatype.h"
#include "../OOP_Lib_global.h"

namespace  object_oriented_electic_data_exchange_protocol {


/**
 * @brief 一个对象属性的响应的数据的结果基类
 */
#ifdef UNIT_TEST
class GetResultParent
#else
class OOP_LIB_EXPORT GetResultParent
#endif
{
public:
    GetResultParent(GetResultChoice choice);
    virtual ~GetResultParent();
    GetResultChoice choice_;

    virtual QByteArray EncodeFrame()=0;
    virtual void DecodeFrame(QByteArray *data)=0;

};

/**
 * @brief 一个对象属性的响应的数据的结果--错误信息
 */
#ifdef UNIT_TEST
class GetResultDAR : public GetResultParent
#else
class OOP_LIB_EXPORT GetResultDAR : public GetResultParent
#endif
{
public:
    GetResultDAR();
    ~GetResultDAR() override;

    DAR dar_;
    QByteArray EncodeFrame() override;
    void DecodeFrame(QByteArray *data) override;

};

/**
 * @brief 一个对象属性的响应的数据的结果--数据
 */
#ifdef UNIT_TEST
class GetResultData : public GetResultParent
#else
class OOP_LIB_EXPORT GetResultData : public GetResultParent
#endif
{
public:
    GetResultData();
    ~GetResultData() override;

    std::shared_ptr<DataParent> value_ptr_ = nullptr;//!< 数据值
    QByteArray EncodeFrame() override;
    void DecodeFrame(QByteArray *data) override;

};


/**
 * @brief GetResult工厂类
 */
#ifdef UNIT_TEST
class GetResultFactory
#else
class OOP_LIB_EXPORT GetResultFactory
#endif
{
public:
    GetResultFactory(){}
    ~GetResultFactory(){}

    std::shared_ptr<GetResultParent> CreateGetResult(uchar choice);
};

}

#endif // GETRESULTPARENT_H
