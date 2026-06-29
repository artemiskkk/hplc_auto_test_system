#ifndef RECORDRESPONSEDATAPARENT_H
#define RECORDRESPONSEDATAPARENT_H

#include <QObject>
#include "datatypebasedef.h"
#include "enumerated.h"
#include "datatype.h"
#include "recordrow.h"
#include "../OOP_Lib_global.h"
#include "../exceptions/decodeexception.h"

namespace  object_oriented_electic_data_exchange_protocol {

/**
 * @brief 一个记录型对象属性的响应数据的基类
 */
#ifdef UNIT_TEST
class RecordResponseDataParent
#else
class OOP_LIB_EXPORT RecordResponseDataParent
#endif
{
public:
    RecordResponseDataParent(RecordResponseChoice choice);
    virtual ~RecordResponseDataParent();
    RecordResponseChoice choice_;
    uchar data_column_number_;

    virtual QByteArray EncodeFrame()=0;
    virtual void DecodeFrame(QByteArray *data)=0;
};

/**
 * @brief 一个对象属性的响应的数据的结果--错误信息
 */
#ifdef UNIT_TEST
class RecordResponseDAR : public RecordResponseDataParent
#else
class OOP_LIB_EXPORT RecordResponseDAR : public RecordResponseDataParent
#endif
{
public:
    RecordResponseDAR();
    ~RecordResponseDAR() override;

    DAR dar_;
    QByteArray EncodeFrame() override;
    void DecodeFrame(QByteArray *data) override;

};

/**
 * @brief 一个对象属性的响应的数据的结果--数据
 */
#ifdef UNIT_TEST
class RecordResponseData : public RecordResponseDataParent
#else
class OOP_LIB_EXPORT RecordResponseData : public RecordResponseDataParent
#endif
{
public:
    RecordResponseData();
    ~RecordResponseData() override;



    //std::shared_ptr<DataParent> value_ptr_ = nullptr;//!< 数据值
    QList<RecordRow> list_record_row_;

    QByteArray EncodeFrame() override;
    void DecodeFrame(QByteArray *data) override;
    uchar GetRecordRowSize();
private:
    uchar size_;
};

/**
 * @brief RecordResponse工厂类
 */
#ifdef UNIT_TEST
class RecordResponseFactory
#else
class OOP_LIB_EXPORT RecordResponseFactory
#endif
{
public:
    RecordResponseFactory(){}
    ~RecordResponseFactory(){}

    std::shared_ptr<RecordResponseDataParent> CreateRecordResponse(uchar choice);
};

}

#endif // RECORDRESPONSEDATAPARENT_H
