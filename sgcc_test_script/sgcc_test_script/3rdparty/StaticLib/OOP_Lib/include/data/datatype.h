#ifndef DATATYPE_H
#define DATATYPE_H

#include <QObject>
#include <memory>
#include "datatypebasedef.h"
#include "enumerated.h"
#include "../OOP_Lib_global.h"

#include "../exceptions/decodeexception.h"

namespace  object_oriented_electic_data_exchange_protocol {
/**
 * @brief Data数据类型基类定义
 */
#ifdef UNIT_TEST
class DataParent
#else
class OOP_LIB_EXPORT DataParent
#endif
{
public:
    DataParent();
    virtual ~DataParent();
    virtual QByteArray EncodeFrame()=0;
    virtual void DecodeFrame(QByteArray *data)=0;
    DataType type_;//!< 数据类型

};


/**
 * @brief Data数据类型基本类型子类定义
 */
#ifdef UNIT_TEST
class DataBasic : public DataParent
#else
class OOP_LIB_EXPORT DataBasic : public DataParent
#endif
{
public:
    DataBasic();
    ~DataBasic() override;
    QByteArray data_;//!< 数据

    QByteArray EncodeFrame() override;
    void DecodeFrame(QByteArray *data) override;
    uchar GetDataContentLength(DataType type);
};


/**
 * @brief Data数据类型字符串类型子类定义
 */
#ifdef UNIT_TEST
class DataString : public DataParent
#else
class OOP_LIB_EXPORT DataString : public DataParent
#endif
{
public:
    DataString();
    ~DataString() override;

    QByteArray data_;//!< 数据
    uchar GetDataContentLength();

    QByteArray EncodeFrame() override;
    void DecodeFrame(QByteArray *data) override;
//private:
//    uchar length_;//!< 数据长度
};


/**
 * @brief Data数据类型数组或结构体子类定义
 */
#ifdef UNIT_TEST
class DataList : public DataParent
#else
class OOP_LIB_EXPORT DataList : public DataParent
#endif
{
public:
    DataList();
    ~DataList() override;

    QList<std::shared_ptr<DataParent>> list_data_member_;//!< 数据成员
    uchar GetDataMemberSize();

    QByteArray EncodeFrame() override;
    void DecodeFrame(QByteArray *data) override;
private:
    uchar size_;//!< 成员个数
};


/**
 * @brief Data数据类型对象创建工厂类定义
 */
class DataFactory
{
public:
    DataFactory(){}
    ~DataFactory(){}
    std::shared_ptr<DataParent> CreateData(DataType type);
};











}

#endif // DATATYPE_H
