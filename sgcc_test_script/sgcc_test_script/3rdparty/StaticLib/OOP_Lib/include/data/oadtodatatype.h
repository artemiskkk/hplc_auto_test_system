#ifndef OADTODATATYPE_H
#define OADTODATATYPE_H

#include <QObject>
#include <QHash>
#include "datatypebasedef.h"
#include "../OOP_Lib_global.h"

namespace  object_oriented_electic_data_exchange_protocol {

/**
 * @brief OAD到数据类型Data关联的定义
 */
#ifdef UNIT_TEST
class OadToDataType
#else
class OOP_LIB_EXPORT OadToDataType
#endif
{
private:
    static QHash<QString,DataType> oad_data_hash;//!< OAD和Data数据类型关联的hash图
    /**
     * @brief 对静态成员变量进行初始化
     * @return
     */
    static QHash<QString,DataType> CreateQhash();
public:
    /**
     * @brief 根据OAD查找hash图中对应的Data数据类型
     * @param oad 对象属性描述符OAD
     * @return Data数据类型
     */
    static DataType GetDataType(QString oad);

};


//改成单例模式

}
#endif // OADTODATATYPE_H
