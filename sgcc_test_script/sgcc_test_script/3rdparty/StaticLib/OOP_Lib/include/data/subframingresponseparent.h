#ifndef SUBFRAMINGRESPONSEPARENT_H
#define SUBFRAMINGRESPONSEPARENT_H

#include <QObject>
#include "enumerated.h"
#include "aresultnormal.h"
#include "aresultrecord.h"
#include "../OOP_Lib_global.h"

namespace  object_oriented_electic_data_exchange_protocol {

/**
 * @brief 分帧响应数据-基类
 */
#ifdef UNIT_TEST
class SubFramingResponseParent
#else
class OOP_LIB_EXPORT SubFramingResponseParent
#endif
{
public:
    SubFramingResponseParent(SubFramingResponseChoice choice);
    virtual ~SubFramingResponseParent();

    SubFramingResponseChoice choice_;//!< 区间类型种类选择
    virtual uchar GetDataSize()=0;
    virtual QByteArray EncodeFrame()=0;
    virtual void DecodeFrame(QByteArray *data)=0;
protected:
    uchar size_;
};

/**
 * @brief 分帧响应数据--错误信息子类
 */
#ifdef UNIT_TEST
class SubFramingResponseDAR : public SubFramingResponseParent
#else
class OOP_LIB_EXPORT SubFramingResponseDAR : public SubFramingResponseParent
#endif
{
public:
    SubFramingResponseDAR();
    ~SubFramingResponseDAR() override;

    DAR dar_;
    uchar GetDataSize() override;
    QByteArray EncodeFrame() override;
    void DecodeFrame(QByteArray *data) override;

};

/**
 * @brief 分帧响应数据--若干个对象属性及其数据子类
 */
#ifdef UNIT_TEST
class SubFramingResponseNormal :public SubFramingResponseParent
#else
class OOP_LIB_EXPORT SubFramingResponseNormal :public SubFramingResponseParent
#endif
{
public:
    SubFramingResponseNormal();
    ~SubFramingResponseNormal() override;

    QList<AResultNormal> list_result_normal_;

    uchar GetDataSize() override;
    QByteArray EncodeFrame() override;
    void DecodeFrame(QByteArray *data) override;
};


/**
 * @brief 分帧响应数据--若干个记录型对象属性及其数据子类
 */
#ifdef UNIT_TEST
class SubFramingResponseRecord :public SubFramingResponseParent
#else
class OOP_LIB_EXPORT SubFramingResponseRecord :public SubFramingResponseParent
#endif
{
public:
    SubFramingResponseRecord();
    ~SubFramingResponseRecord() override;

    QList<AResultRecord> list_result_record_;

    uchar GetDataSize() override;
    QByteArray EncodeFrame() override;
    void DecodeFrame(QByteArray *data) override;
};

/**
 * @brief 分帧响应数据工厂类
 */
#ifdef UNIT_TEST
class SubFramingResponseFactory
#else
class OOP_LIB_EXPORT SubFramingResponseFactory
#endif
{
public:
    SubFramingResponseFactory(){}
    ~SubFramingResponseFactory(){}

    std::shared_ptr<SubFramingResponseParent> CreateSubFramingResponse(uchar choice);
};

}

#endif // SUBFRAMINGRESPONSEPARENT_H
