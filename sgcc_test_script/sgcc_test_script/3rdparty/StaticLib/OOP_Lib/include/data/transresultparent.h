#ifndef TRANSRESULTPARENT_H
#define TRANSRESULTPARENT_H
#include <QObject>
#include "datatypebasedef.h"
#include "enumerated.h"
#include "datatype.h"
#include "../OOP_Lib_global.h"
using namespace std;
namespace  object_oriented_electic_data_exchange_protocol {


/**
 * @brief 响应代理操作透明转发命令:转发命令返回结果基类
 */
#ifdef UNIT_TEST
class TransResultParent
#else
class OOP_LIB_EXPORT TransResultParent
#endif
{
public:
    TransResultParent(TransResultChoice choice_);
    virtual ~TransResultParent();

    TransResultChoice choice_;
    virtual QByteArray EncodeFrame()=0;
    virtual void DecodeFrame(QByteArray *data)=0;
};


/**
 * @brief 响应代理操作透明转发命令:转发命令返回结果错误信息类
 */
#ifdef UNIT_TEST
class TransResultDar
#else
class OOP_LIB_EXPORT TransResultDar:public TransResultParent
#endif
{
public:
    TransResultDar();
    virtual ~TransResultDar();

    DAR dar_;
    // TransResultParent interface
public:
    virtual QByteArray EncodeFrame() override;
    virtual void DecodeFrame(QByteArray *data) override;
};

/**
 * @brief 响应代理操作透明转发命令:转发命令返回结果类
 */
#ifdef UNIT_TEST
class TransResultDar
#else
class OOP_LIB_EXPORT TransResultResult:public TransResultParent
#endif
{
public:
    TransResultResult();
    virtual ~TransResultResult();

    uchar msgtype;
    QByteArray msg;
    // TransResultParent interface
public:
    virtual QByteArray EncodeFrame() override;
    virtual void DecodeFrame(QByteArray *data) override;
};

/**
 * @brief 响应代理操作透明转发命令:工厂类
 */
#ifdef UNIT_TEST
class creatFactory
#else
class OOP_LIB_EXPORT GetTransResultFactory
#endif
{
public:
    GetTransResultFactory();
    virtual ~GetTransResultFactory();

 std::shared_ptr<TransResultParent> GetTransResult(TransResultChoice choice);
};

}
#endif // TRANSRESULTPARENT_H
