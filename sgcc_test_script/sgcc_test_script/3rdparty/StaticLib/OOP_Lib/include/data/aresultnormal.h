#ifndef ARESULTNORMAL_H
#define ARESULTNORMAL_H

#include <QObject>
#include "datatypebasedef.h"
#include "getresultparent.h"

namespace  object_oriented_electic_data_exchange_protocol {

/**
 * @brief 一个对象属性及其结果
 */
#ifdef UNIT_TEST
class AResultNormal
#else
class OOP_LIB_EXPORT AResultNormal
#endif
{
public:
    AResultNormal();
    ~AResultNormal();

    OAD oad;
    std::shared_ptr<GetResultParent> get_result_ptr = nullptr;

    QByteArray EncodeFrame();
    void DecodeFrame(QByteArray *data);
};

}

#endif // ARESULTNORMAL_H
