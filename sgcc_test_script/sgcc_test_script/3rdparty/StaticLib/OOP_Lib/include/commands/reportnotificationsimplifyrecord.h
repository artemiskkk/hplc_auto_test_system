#ifndef REPORTNOTIFICATIONSIMPLIFYRECORD_H
#define REPORTNOTIFICATIONSIMPLIFYRECORD_H

#include <QObject>
#include "../frameoopbase.h"
using namespace std;

namespace  object_oriented_electic_data_exchange_protocol {

/**
 * @brief 通知上报精简的记录型对象属性
 */
#ifdef UNIT_TEST
class ReportNotificationSimplifyRecord : public FrameOOPBase
#else
class  OOP_LIB_EXPORT ReportNotificationSimplifyRecord:public FrameOOPBase
 #endif
{
public:
    ReportNotificationSimplifyRecord();
    ~ReportNotificationSimplifyRecord() override;
    ReportNotificationSimplifyRecord(CtrlField ctrl_field,AddressField address_field);
    PIID_ACD piid_acd_;
    AResultSimplifyRecord aresult_simplify_record;
    FollowReportField followreport;

   QByteArray EncodeFrameDataField() override;
   void DecodeFrameDataField(QByteArray data) override;

};



}
#endif // REPORTNOTIFICATIONSIMPLIFYRECORD_H
