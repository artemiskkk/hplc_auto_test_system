#ifndef RECORDROW_H
#define RECORDROW_H

#include <QObject>
#include "datatypebasedef.h"
#include "datatype.h"
#include "../OOP_Lib_global.h"

namespace  object_oriented_electic_data_exchange_protocol {

/**
 * @brief 一行记录型响应数据一行记录
 */
#ifdef UNIT_TEST
class RecordRow
#else
class OOP_LIB_EXPORT RecordRow
#endif
{
public:
    RecordRow();
    ~RecordRow();

    QList<std::shared_ptr<DataParent>> list_data;
    uchar GetColumnNumber();

    QByteArray EncodeFrame();
    void DecodeFrame(QByteArray *data,uchar column_num);
};

}

#endif // RECORDROW_H
