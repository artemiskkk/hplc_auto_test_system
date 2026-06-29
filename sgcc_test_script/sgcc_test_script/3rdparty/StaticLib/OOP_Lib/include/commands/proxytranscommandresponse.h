#ifndef PROXYTRANSCOMMANDRESPONSE_H
#define PROXYTRANSCOMMANDRESPONSE_H

#include <QObject>
#include "../frameoopbase.h"

using namespace std;

namespace  object_oriented_electic_data_exchange_protocol {

#ifdef UNIT_TEST
class ProxyTransCommandResponse : public FrameOOPBase
#else
class OOP_LIB_EXPORT ProxyTransCommandResponse:public FrameOOPBase
#endif
{
public:
    ProxyTransCommandResponse();
    ~ProxyTransCommandResponse();
    ProxyTransCommandResponse(CtrlField ctrfield,AddressField addressfield);

    PIID_ACD piid_acd_;
    OAD oad_;
    shared_ptr<TransResultParent> trans_result_ptr_=nullptr;
    FollowReportField follow_report_field_;
    // FrameOOPBase interface
public:
    virtual void DecodeFrameDataField(QByteArray data) override;

protected:
    virtual QByteArray EncodeFrameDataField() override;
};

}
#endif // PROXYTRANSCOMMANDRESPONSE_H
