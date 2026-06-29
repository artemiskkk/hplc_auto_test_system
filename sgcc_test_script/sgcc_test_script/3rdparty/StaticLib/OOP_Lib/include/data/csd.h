#ifndef COLUMNSELECTIONDESCRIPTOR_H
#define COLUMNSELECTIONDESCRIPTOR_H

#include <QObject>
#include "datatypebasedef.h"
#include "enumerated.h"
#include "../OOP_Lib_global.h"
#include "../exceptions/decodeexception.h"

namespace  object_oriented_electic_data_exchange_protocol {



/**
 * @brief 列选择描述符CSD（Column Selection Descriptor）的数据类型定义 基类
 */
#ifdef UNIT_TEST
class CSDParent
#else
class OOP_LIB_EXPORT CSDParent
#endif
{
public:
    CSDParent(CsdChoice choice);
    virtual ~CSDParent();
    CsdChoice choice_; //!< 选择方法类型
    virtual QByteArray EncodeFrame()=0;
    virtual void DecodeFrame(QByteArray *data)=0;
};

/**
 * @brief 子CSD类：对象属性描述符
 */
#ifdef UNIT_TEST
class CsdOad : public CSDParent
#else
class OOP_LIB_EXPORT CsdOad : public CSDParent
#endif
{
public:
    CsdOad();
    ~CsdOad() override;
    OAD oad_;

    QByteArray EncodeFrame() override;
    void DecodeFrame(QByteArray *data) override;
};

/**
 * @brief CSD子类：记录型对象属性描述符
 */
#ifdef UNIT_TEST
class CsdROad : public CSDParent
#else
class OOP_LIB_EXPORT CsdROad : public CSDParent
#endif
{
public:
    CsdROad();
    ~CsdROad() override;
    ROAD road_;

    QByteArray EncodeFrame() override;
    void DecodeFrame(QByteArray *data) override;
};

/**
 * @brief RSD工厂类
 */
#ifdef UNIT_TEST
class CsdFactory
#else
class OOP_LIB_EXPORT CsdFactory
#endif
{
public:
    CsdFactory(){}
    ~CsdFactory(){}

    std::shared_ptr<CSDParent> CreateCSD(uchar choice);
};

/**
 * @brief 记录列选择描述符RCSD（Record Column Selection Descriptor）的数据类型定义
 */
#ifdef UNIT_TEST
class RCSD
#else
class OOP_LIB_EXPORT RCSD
#endif
{
public:
    RCSD();
    ~RCSD();
    QList<std::shared_ptr<CSDParent>> list_csd_;
    uchar GetCsdSize();

    QByteArray EncodeFrame();
    void DecodeFrame(QByteArray *data);
private:
    uchar size_;
};

}
#endif // COLUMNSELECTIONDESCRIPTOR_H
