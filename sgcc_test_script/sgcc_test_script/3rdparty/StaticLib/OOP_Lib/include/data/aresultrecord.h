#ifndef ARESULTRECORD_H
#define ARESULTRECORD_H

#include <QObject>
#include "datatypebasedef.h"
#include "csd.h"
#include "recordresponsedataparent.h"

namespace  object_oriented_electic_data_exchange_protocol {

/**
 * @brief 一个记录型对象属性及其结果
 */
#ifdef UNIT_TEST
class AResultRecord
#else
class OOP_LIB_EXPORT AResultRecord
#endif
{
public:
    AResultRecord();
    ~AResultRecord();

    OAD oad;
    std::shared_ptr<RCSD> rcsd_ptr = nullptr;
    std::shared_ptr<RecordResponseDataParent> response_data_ptr = nullptr;


    QByteArray EncodeFrame();
    void DecodeFrame(QByteArray *data);
};

}

#endif // ARESULTRECORD_H
