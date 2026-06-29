#ifndef ASIMPLIFYRECORD_H
#define ASIMPLIFYRECORD_H

#include <QObject>
#include "datatypebasedef.h"
#include "datatype.h"
#include "../OOP_Lib_global.h"

namespace  object_oriented_electic_data_exchange_protocol {

/**
 * @brief 一行精简的记录型响应数据
 */
#ifdef UNIT_TEST
class AsimplifyRecord
#else
class OOP_LIB_EXPORT  AsimplifyRecord
#endif
{
public:
    AsimplifyRecord();
    ~AsimplifyRecord();
    QList<uchar> simplifyrecordrowdatatype;
    QList<QList<QByteArray>>  simplifyrecordrowlist;

    uchar GetDataContentLength(object_oriented_electic_data_exchange_protocol::DataType datatype);
    uchar GetRecordRowsize();

     QByteArray EncodeFrame();
     void DecodeFrame(QByteArray *data ,uchar column_num);
};




}
#endif // ASIMPLIFYRECORD_H
