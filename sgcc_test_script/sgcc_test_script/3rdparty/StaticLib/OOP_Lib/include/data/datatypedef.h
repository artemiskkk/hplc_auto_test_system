#ifndef DATATYPEDEF_H
#define DATATYPEDEF_H

#include <QObject>
#include "datatypebasedef.h"
#include "rsd.h"
#include "csd.h"
//#include "getresultparent.h"
//#include "recordresponsedataparent.h"
#include "datatype.h"
#include "aresultnormal.h"

namespace  object_oriented_electic_data_exchange_protocol {


struct GetRecord
{
    OAD oad;
    std::shared_ptr<RSDParent> rsd_ptr = nullptr;
    std::shared_ptr<RCSD> rcsd_ptr = nullptr;
};


//struct ResultNormal
//{
//    OAD oad;
//    std::shared_ptr<GetResultParent> get_result_ptr = nullptr;
//};




//struct ResultRecord
//{
//    OAD oad;
//    std::shared_ptr<RCSD> rcsd_ptr = nullptr;
//    std::shared_ptr<RecordResponseDataParent> response_data_ptr = nullptr;

//};



struct ASetNormal
{
    OAD oad;//!< 一个对象属性描述符
    std::shared_ptr<DataParent> data_ptr = nullptr;//!< 数据指针
};


struct ASetNormalResult
{
    OAD oad;//!< 一个对象属性描述符
    DAR dar;//!< 设置执行结果
};

struct ASetThenGetNormal
{
    OAD oad;//!< 一个对象属性描述符
    std::shared_ptr<DataParent> data_ptr = nullptr;//!< 数据指针
    OAD oad_get;//!< 一个读取的对象属性
    uchar delay_time;//!< 延时读取时间
};

struct ASetThenGetNormalResult
{
    OAD oad;//!< 一个对象属性描述符
    DAR dar;//!< 设置执行结果
    AResultNormal a_result_normal;//!<  一个对象属性及其结果
};


struct AAction
{
    OMD omd;//!< 一个对象方法描述符
    std::shared_ptr<DataParent> data_ptr = nullptr;//!< 数据指针
};


struct AActionResult
{
    OMD omd;//!< 一个对象方法描述符
    DAR dar;//!< 设置执行结果
    uchar optional;//!< 可选，0：表示没有，1：表示有
    std::shared_ptr<DataParent> data_ptr = nullptr;//!< 数据指针
};

struct AActionhenGetNormal
{
    OMD omd;//!< 一个对象方法描述符
    std::shared_ptr<DataParent> data_ptr = nullptr;//!< 数据指针
    OAD oad_get;//!< 一个读取的对象属性
    uchar delay_time;//!< 延时读取时间
};

struct AActionThenGetNormalResult
{
    OMD omd;//!< 一个对象方法描述符
    DAR dar;//!< 设置执行结果
    uchar optional;//!< 可选，0：表示没有，1：表示有
    std::shared_ptr<DataParent> data_ptr = nullptr;//!< 数据指针
    AResultNormal a_result_normal;//!<  一个对象属性及其结果
};















}

#endif // DATATYPEDEF_H
