#ifndef ARESULTSIMPLIFYRECORD_H
#define ARESULTSIMPLIFYRECORD_H
#include <QObject>
#include "datatypebasedef.h"
#include "csd.h"
#include "rsd.h"
#include "recordresponsedataparent.h"
#include "resultsimplifyrecorddataparent.h"


namespace  object_oriented_electic_data_exchange_protocol {

/**
 * @brief 一个记录型对象属性及其结果
 */
#ifdef UNIT_TEST
class AResultSimplifyRecord
#else
class  OOP_LIB_EXPORT AResultSimplifyRecord
#endif
{
public:
    AResultSimplifyRecord();
    ~AResultSimplifyRecord();

    OAD oad;
    std::shared_ptr<RSDParent>  rsd_ptr=nullptr;
    ROAD road;
    std::shared_ptr<ResultSimplifyRecordDataParent> resultsimplifyrecord_ptr=nullptr;
     QByteArray EncodeFrame();
     void DecodeFrame(QByteArray *data);

};




















}
#endif // ARESULTSIMPLIFYRECORD_H
