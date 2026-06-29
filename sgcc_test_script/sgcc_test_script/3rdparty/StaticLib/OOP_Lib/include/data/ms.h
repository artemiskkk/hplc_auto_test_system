#ifndef MS_H
#define MS_H

#include <QObject>
#include "datatypebasedef.h"
#include "enumerated.h"
#include "region.h"
#include "tsa.h"
#include "../OOP_Lib_global.h"

#include "../exceptions/decodeexception.h"

namespace  object_oriented_electic_data_exchange_protocol {



/**
 * @brief 电能表集合MS（Meter Set）的数据类型定义
 */
#ifdef UNIT_TEST
class MSParent
#else
class OOP_LIB_EXPORT MSParent
#endif
{
public:
    MSParent(MSChoice choice);
    virtual ~MSParent();
    MSChoice choice_;//!< 电能表集合MS种类选择
    virtual QByteArray EncodeFrame()=0;
    virtual void DecodeFrame(QByteArray *data)=0;

};

/**
 * @brief 电能表集合MS（Meter Set）的无电能表子类定义
 */
#ifdef UNIT_TEST
class MSNoMeter : public MSParent
#else
class OOP_LIB_EXPORT MSNoMeter : public MSParent
#endif
{
public:
    MSNoMeter();
    ~MSNoMeter() override;

    QByteArray EncodeFrame() override;
    void DecodeFrame(QByteArray *data) override;

};

/**
 * @brief 电能表集合MS（Meter Set）的无电能表子类定义
 */
#ifdef UNIT_TEST
class MSAllUserAddr : public MSParent
#else
class OOP_LIB_EXPORT MSAllUserAddr : public MSParent
#endif
{
public:
    MSAllUserAddr();
    ~MSAllUserAddr() override;

    QByteArray EncodeFrame() override;
    void DecodeFrame(QByteArray *data) override;
};

/**
 * @brief 电能表集合MS（Meter Set）的一组用户类型子类定义
 */
#ifdef UNIT_TEST
class MSUserType : public MSParent
#else
class OOP_LIB_EXPORT MSUserType : public MSParent
#endif
{
public:
    MSUserType();
    ~MSUserType() override;

    QList<uchar> list_type_;//!< 用户类型列表
    uchar GetUserTypeSize();

    QByteArray EncodeFrame() override;
    void DecodeFrame(QByteArray *data) override;
private:
    uchar size_;//!< 用户类型个数
};

/**
 * @brief 电能表集合MS（Meter Set）的一组用户地址子类定义
 */
#ifdef UNIT_TEST
class MSUserAddr : public MSParent
#else
class OOP_LIB_EXPORT MSUserAddr : public MSParent
#endif
{
public:
    MSUserAddr();
    ~MSUserAddr() override;

    QList<TSA> list_tsa_;//!< TSA列表
    uchar GetUserAddrSize();

    QByteArray EncodeFrame() override;
    void DecodeFrame(QByteArray *data) override;
private:
    uchar size_;//!< TSA个数
};

/**
 * @brief 电能表集合MS（Meter Set）的一组配置序号子类定义
 */
#ifdef UNIT_TEST
class MSConfigSeq : public MSParent
#else
class OOP_LIB_EXPORT MSConfigSeq : public MSParent
#endif
{
public:
    MSConfigSeq();
    ~MSConfigSeq() override;

    QList<ushort> list_config_seq_;//!< 配置序号列表
    uchar GetConfigSeqSize();

    QByteArray EncodeFrame() override;
    void DecodeFrame(QByteArray *data) override;
private:
    uchar size_;//!< 配置序号个数
};


/**
 * @brief 电能表集合MS（Meter Set）的一组用户类型区间子类定义
 */
#ifdef UNIT_TEST
class MSUserTypeRegion : public MSParent
#else
class OOP_LIB_EXPORT MSUserTypeRegion : public MSParent
#endif
{
public:
    MSUserTypeRegion();
    ~MSUserTypeRegion() override;

    QList<std::shared_ptr<RegionUnsigned>> list_region_;//!< 区间列表，区间分为：一组用户类型区间、一组用户地址区间、一组配置序号区间
    uchar GetRegionSize();

    QByteArray EncodeFrame() override;
    void DecodeFrame(QByteArray *data) override;
private:
    uchar size_;//!< 区间个数

};

/**
 * @brief 电能表集合MS（Meter Set）的一组用户地址区间子类定义
 */
#ifdef UNIT_TEST
class MSUserAddrRegion : public MSParent
#else
class OOP_LIB_EXPORT MSUserAddrRegion : public MSParent
#endif
{
public:
    MSUserAddrRegion();
    ~MSUserAddrRegion() override;

    QList<std::shared_ptr<RegionTsa>> list_region_;//!< 区间列表，区间分为：一组用户类型区间、一组用户地址区间、一组配置序号区间
    uchar GetRegionSize();

    QByteArray EncodeFrame() override;
    void DecodeFrame(QByteArray *data) override;
private:
    uchar size_;//!< 区间个数
};

/**
 * @brief 电能表集合MS（Meter Set）的一组配置序号区间子类定义
 */
#ifdef UNIT_TEST
class MSConfigSeqRegion : public MSParent
#else
class OOP_LIB_EXPORT MSConfigSeqRegion : public MSParent
#endif
{
public:
    MSConfigSeqRegion();
    ~MSConfigSeqRegion() override;

    QList<std::shared_ptr<RegionLongUnsigned>> list_region_;//!< 区间列表，区间分为：一组用户类型区间、一组用户地址区间、一组配置序号区间
    uchar GetRegionSize();

    QByteArray EncodeFrame() override;
    void DecodeFrame(QByteArray *data) override;
private:
    uchar size_;//!< 区间个数
};

/**
 * @brief MS工厂类
 */
#ifdef UNIT_TEST
class MsFactory
#else
class OOP_LIB_EXPORT MsFactory
#endif
{
public:
    MsFactory(){}
    ~MsFactory(){}

    std::shared_ptr<MSParent> CreateMS(uchar choice);
};

}

#endif // MS_H
