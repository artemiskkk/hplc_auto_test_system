#ifndef SCRIPT_COMMADDRREQUEST_1
#define SCRIPT_COMMADDRREQUEST_1

#include "TestCase/BuildNetwork_GW.h"

enum Script_CommAddrRequest_1_RunState
{
  //  Init_PowerOffSta,
    Init_SetBaudRate,
    Init_PowerOnSta,
    Init_ResetSta,
    Wait_StaReadAddr_645,
    Wait_StaReadAddr_OOP,
    ScriptComplete
};

class Script_CommAddrRequest_1 : public QObject, public AbstractScript, public DynamicCreate<Script_CommAddrRequest_1>
{
    Q_OBJECT
   // Q_PLUGIN_METADATA(IID "com.topscomm.AbstractScript")
   // Q_INTERFACES(AbstractScript)
public:
    explicit Script_CommAddrRequest_1(QObject *parent = nullptr);
    ~Script_CommAddrRequest_1();

    Script_CommAddrRequest_1_RunState emScriptRunState;

  //  ushort concentratorCnt;
 //   bool resultFlag;
    shared_ptr<QList<shared_ptr<CtrInfo>>> p_CtrInfoList;

    shared_ptr<QTimer> p_timer;
    shared_ptr<QTimer> p_maxAllowTimer;
    AbstractScriptHost *p_AbstractScriptHost;

  //  shared_ptr<BuildNetwork_GW> p_BuildNetwork;
    QByteArray sendMsgOct;

    shared_ptr<dlt_645_Protocol::Frame645Helper> p_MsgBase_645;
//    shared_ptr<dlt_645_Protocol::RspsNormal_ReadAddr_0x93> p_MeterAddrResp_93;
    shared_ptr<object_oriented_electic_data_exchange_protocol::FrameOOPHelper> p_MsgBase_698_45;
//    shared_ptr<GetResponseNormal> p_GetResponseNormal_ReadAddr;

  //  uchar addr[6];

    /***可配置参数***/
    ushort maxAllowTime=1800; //单位:s
    int staBaudRate=2400;

    int tryTimes=0;
    QList<double> sendParams;
    QList<int> idList;

  //  ushort staReadAddrFlag;

public:
    void  execute();
    void  stop();
    void  addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq);
    void  setHost(AbstractScriptHost *host);
    bool  config(const QMap<QString,QString> *paraDic);
    void  processMsg(DvcType dvcType,int id,uchar* data,int datalen);
    void  processCtrlDvcRes(DvcType dvcType,QList<int> idList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params);

private:
  //  void processMsgFromCco(int id);
    void processMsgFromMeter645(int id);
    void processMsgFromMeter698(int id);
  //  void sendMsg(DvcType dvcType,int id,shared_ptr<void> frame);
//    void sendMsg(DvcType dvcType,int id,QByteArray msg);
    QList<int> getDvcIdList(DvcType dvcType);

public slots:
    void timer_timeoutProc();
    void maxAllowTimer_timeoutProc();
};

#endif // SCRIPT_COMMADDRREQUEST_1
