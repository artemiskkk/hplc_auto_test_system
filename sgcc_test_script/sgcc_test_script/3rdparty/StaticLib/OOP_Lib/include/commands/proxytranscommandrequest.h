#ifndef PROXYTRANSCOMMANDREQUEST_H
#define PROXYTRANSCOMMANDREQUEST_H
#include <QObject>
#include"QDebug"
#include "../frameoopbase.h"

using namespace std;

namespace  object_oriented_electic_data_exchange_protocol {

#ifdef UNIT_TEST
class ProxyTransCommandRequest : public FrameOOPBase
#else
class OOP_LIB_EXPORT ProxyTransCommandRequest:public FrameOOPBase
#endif
{
public:
    ProxyTransCommandRequest();
    ~ProxyTransCommandRequest();
    ProxyTransCommandRequest(CtrlField ctrl_field,AddressField address_field);

    PIID piid_;
    OAD oad_;
    ComDcb comdcb;
    ushort msg_timeout_;
    ushort byte_timeout_;
    uchar msgtype;
    QByteArray msg;

    // FrameOOPBase interface
public:
    virtual void DecodeFrameDataField(QByteArray data) override;

protected:
    virtual QByteArray EncodeFrameDataField() override;
};


}
#endif // PROXYTRANSCOMMANDREQUEST_H
