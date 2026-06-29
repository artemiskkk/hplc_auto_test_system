#ifndef FOLLOWREPORTFIELD_H
#define FOLLOWREPORTFIELD_H

#include <QObject>
#include "followreportparent.h"
#include "../OOP_Lib_global.h"
#include "../exceptions/decodeexception.h"

namespace  object_oriented_electic_data_exchange_protocol {


/**
 * @brief 跟随上报信息域
 */
#ifdef UNIT_TEST
class FollowReportField
#else
class OOP_LIB_EXPORT FollowReportField
#endif
{
public:
    FollowReportField();
    FollowReportField(uchar optional);

    uchar optional_;//!< 可选，0：表示无跟随上报信息域，1：表示有跟随上报信息域
    std::shared_ptr<FollowReportParent> follow_report_;//!< 跟随上报信息

    QByteArray EncodeFrame();
    void DecodeFrame(QByteArray *data);
};

}

#endif // FOLLOWREPORTFIELD_H
