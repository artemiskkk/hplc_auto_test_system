#ifndef RECORDSELECTIONDESCRIPTOR_H
#define RECORDSELECTIONDESCRIPTOR_H

#include <QObject>
#include "datatypebasedef.h"
#include "enumerated.h"
#include "ms.h"
#include "datatype.h"
#include "../OOP_Lib_global.h"

#include "../exceptions/decodeexception.h"


namespace  object_oriented_electic_data_exchange_protocol {


/**
 * @brief 记录选择描述符RSD（Record Selection Descriptor）的数据类型定义
 */
#ifdef UNIT_TEST
class RSDParent
#else
class OOP_LIB_EXPORT RSDParent
#endif
{
public:
    RSDParent(RsdChoice choice);
    virtual ~RSDParent();
    RsdChoice choice_; //!< 选择方法类型
    virtual QByteArray EncodeFrame()=0;
    virtual void DecodeFrame(QByteArray *data)=0;
};

/**
 * @brief 不选择
 */
#ifdef UNIT_TEST
class RsdNoSelector : public RSDParent
#else
class OOP_LIB_EXPORT RsdNoSelector : public RSDParent
#endif
{
public:
    RsdNoSelector();
    ~RsdNoSelector() override;

    QByteArray EncodeFrame() override;
    void DecodeFrame(QByteArray *data) override;
};

/**
 * @brief 指定对象指定值
 */
#ifdef UNIT_TEST
class RsdSelector1 : public RSDParent
#else
class OOP_LIB_EXPORT RsdSelector1 : public RSDParent
#endif
{
public:
    RsdSelector1();
    ~RsdSelector1() override;

    QByteArray EncodeFrame() override;
    void DecodeFrame(QByteArray *data) override;

    OAD oad_;//!< 对象属性描述符
    std::shared_ptr<DataParent> value_ptr_ = nullptr;//!< 数据值
};

/**
 * @brief 指定对象区间内连续间隔值
 */
#ifdef UNIT_TEST
class RsdSelector2 : public RSDParent
#else
class OOP_LIB_EXPORT RsdSelector2 : public RSDParent
#endif
{
public:
    RsdSelector2();
    ~RsdSelector2() override;

    QByteArray EncodeFrame() override;
    void DecodeFrame(QByteArray *data) override;

    OAD oad_;//!< 对象属性描述符
    std::shared_ptr<DataParent> start_value_ptr_ = nullptr;//!< 起始数据值
    std::shared_ptr<DataParent> end_value_ptr_ = nullptr;//!< 结束数据值
    std::shared_ptr<DataParent> interval_value_ptr_ = nullptr;//!< 数据间隔数据值
};

/**
 * @brief 组合筛选，即若干个指定对象连续值
 */
#ifdef UNIT_TEST
class RsdSelector3 : public RSDParent
#else
class OOP_LIB_EXPORT RsdSelector3 : public RSDParent
#endif
{
public:
    RsdSelector3();
    ~RsdSelector3() override;

    QByteArray EncodeFrame() override;
    void DecodeFrame(QByteArray *data) override;
    uchar GetSelector2Size();

    QList<std::shared_ptr<RsdSelector2>> list_selector2_;//!< 若干个指定对象连续值
private:
    uchar Selector2_size_;//!< 指定对象区间的个数
};

/**
 * @brief 指定电能表集合、指定采集启动时间
 */
#ifdef UNIT_TEST
class RsdSelector4 : public RSDParent
#else
class OOP_LIB_EXPORT RsdSelector4 : public RSDParent
#endif
{
public:
    RsdSelector4();
    ~RsdSelector4() override;

    QByteArray EncodeFrame() override;
    void DecodeFrame(QByteArray *data) override;

    DateTimeS collect_start_time_;//!< 采集启动时间
    std::shared_ptr<MSParent> meter_set_ptr_ = nullptr;//!< 电能表集合

};

/**
 * @brief 指定电能表集合、指定采集存储时间
 */
#ifdef UNIT_TEST
class RsdSelector5 : public RSDParent
#else
class OOP_LIB_EXPORT RsdSelector5 : public RSDParent
#endif
{
public:
    RsdSelector5();
    ~RsdSelector5() override;

    QByteArray EncodeFrame() override;
    void DecodeFrame(QByteArray *data) override;

    DateTimeS collect_store_time_;//!< 采集存储时间
    std::shared_ptr<MSParent> meter_set_ptr_ = nullptr;//!< 电能表集合
};

/**
 * @brief 指定电能表集合、指定采集启动时间区间内连续间隔值
 */
#ifdef UNIT_TEST
class RsdSelector6 : public RSDParent
#else
class OOP_LIB_EXPORT RsdSelector6 : public RSDParent
#endif
{
public:
    RsdSelector6();
    ~RsdSelector6() override;

    QByteArray EncodeFrame() override;
    void DecodeFrame(QByteArray *data) override;

    DateTimeS collect_start_time_init_;//!< 采集启动时间起始值
    DateTimeS collect_start_time_end_;//!< 采集启动时间结束值
    TI time_interval_;//!< 时间间隔
    std::shared_ptr<MSParent> meter_set_ptr_ = nullptr;//!< 电能表集合
};

/**
 * @brief 指定电能表集合、指定采集存储时间区间内连续间隔值
 */
#ifdef UNIT_TEST
class RsdSelector7 : public RSDParent
#else
class OOP_LIB_EXPORT RsdSelector7 : public RSDParent
#endif
{
public:
    RsdSelector7();
    ~RsdSelector7() override;

    QByteArray EncodeFrame() override;
    void DecodeFrame(QByteArray *data) override;

    DateTimeS collect_store_time_init_;//!< 采集存储时间起始值
    DateTimeS collect_store_time_end_;//!< 采集存储时间结束值
    TI time_interval_;//!< 时间间隔
    std::shared_ptr<MSParent> meter_set_ptr_ = nullptr;//!< 电能表集合

};

/**
 * @brief 指定电能表集合、指定采集到时间区间内连续间隔值
 */
#ifdef UNIT_TEST
class RsdSelector8 : public RSDParent
#else
class OOP_LIB_EXPORT RsdSelector8 : public RSDParent
#endif
{
public:
    RsdSelector8();
    ~RsdSelector8() override;

    QByteArray EncodeFrame() override;
    void DecodeFrame(QByteArray *data) override;

    DateTimeS collect_success_time_init_;//!< 采集成功时间起始值
    DateTimeS collect_success_time_end_;//!< 采集成功时间结束值
    TI time_interval_;//!< 时间间隔
    std::shared_ptr<MSParent> meter_set_ptr_ = nullptr;//!< 电能表集合

};

/**
 * @brief 指定选取上第n次记录
 */
#ifdef UNIT_TEST
class RsdSelector9 : public RSDParent
#else
class OOP_LIB_EXPORT RsdSelector9 : public RSDParent
#endif
{
public:
    RsdSelector9();
    ~RsdSelector9() override;

    QByteArray EncodeFrame() override;
    void DecodeFrame(QByteArray *data) override;

    uchar last_n_th_times_; //!< 上第n次记录

};

/**
 * @brief 指定选取最新的n条记录
 */
#ifdef UNIT_TEST
class RsdSelector10 : public RSDParent
#else
class OOP_LIB_EXPORT RsdSelector10 : public RSDParent
#endif
{
public:
    RsdSelector10();
    ~RsdSelector10() override;

    QByteArray EncodeFrame() override;
    void DecodeFrame(QByteArray *data) override;

    uchar last_n_times_; //!< 上n次记录
    std::shared_ptr<MSParent> meter_set_ptr_ = nullptr;//!< 电能表集合

};

/**
 * @brief RSD工厂类
 */
#ifdef UNIT_TEST
class RsdFactory
#else
class OOP_LIB_EXPORT RsdFactory
#endif
{
public:
    RsdFactory(){}
    ~RsdFactory(){}

    std::shared_ptr<RSDParent> CreateRSD(uchar choice);
};



}
#endif // RECORDSELECTIONDESCRIPTOR_H
