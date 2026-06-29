#ifndef FOLLOWREPORTPARENT_H
#define FOLLOWREPORTPARENT_H

#include <QObject>
#include "enumerated.h"
#include "datatypedef.h"
#include "aresultnormal.h"
#include "aresultrecord.h"
#include "../OOP_Lib_global.h"

namespace  object_oriented_electic_data_exchange_protocol {


/**
 * @brief 跟随上报-基类
 */
#ifdef UNIT_TEST
class FollowReportParent
#else
class OOP_LIB_EXPORT FollowReportParent
#endif
{
public:
    FollowReportParent(FollowReportChoice choice);
    virtual ~FollowReportParent();

    FollowReportChoice choice_;
    virtual uchar GetDataSize()=0;
    virtual QByteArray EncodeFrame()=0;
    virtual void DecodeFrame(QByteArray *data)=0;
protected:
    uchar size_;
};


/**
 * @brief 跟随上报信息--若干个对象属性及其数据子类
 */
#ifdef UNIT_TEST
class FollowReportNormal :public FollowReportParent
#else
class OOP_LIB_EXPORT FollowReportNormal :public FollowReportParent
#endif
{
public:
    FollowReportNormal();
    ~FollowReportNormal() override;

    QList<AResultNormal> list_result_normal_;

    uchar GetDataSize() override;
    QByteArray EncodeFrame() override;
    void DecodeFrame(QByteArray *data) override;
};


/**
 * @brief 跟随上报信息--若干个记录型对象属性及其数据子类
 */
#ifdef UNIT_TEST
class FollowReportRecord :public FollowReportParent
#else
class OOP_LIB_EXPORT FollowReportRecord :public FollowReportParent
#endif
{
public:
    FollowReportRecord();
    ~FollowReportRecord() override;

    QList<AResultRecord> list_result_record_;

    uchar GetDataSize() override;
    QByteArray EncodeFrame() override;
    void DecodeFrame(QByteArray *data) override;
};

/**
 * @brief 跟随上报信息工厂类
 */
#ifdef UNIT_TEST
class FollowReportFactory
#else
class OOP_LIB_EXPORT FollowReportFactory
#endif
{
public:
    FollowReportFactory(){}
    ~FollowReportFactory(){}

    std::shared_ptr<FollowReportParent> CreateFollowReport(uchar choice);
};

}

#endif // FOLLOWREPORTPARENT_H
