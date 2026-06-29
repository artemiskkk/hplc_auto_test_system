#ifndef SCRIPT_ROUTERSTATE_READMETER_BEIJING_H
#define SCRIPT_ROUTERSTATE_READMETER_BEIJING_H

#include <QObject>
#include "TestCase/BuildNetwork_GW.h"
enum Script_RouterState_ReadMeter_Beijing_RunState
{
    TestInit,
    Wait_BuildNetFinish_Whole,
    Wait_QueryRouterStateReserve_Finish,
    Wait_QueryRouterStateReadMeter_Finish,
    Wait_QueryRouterStateAfterReadMeter_Finish,
    ScriptSuccess
};
class Script_RouterState_ReadMeter_Beijing : public QObject, public AbstractScript,public DynamicCreate<Script_RouterState_ReadMeter_Beijing>
{
    Q_OBJECT
//    Q_PLUGIN_METADATA(IID "com.topscomm.AbstractScript")
//    Q_INTERFACES(AbstractScript)
public:
    explicit Script_RouterState_ReadMeter_Beijing(QObject *parent = nullptr);
    ~Script_RouterState_ReadMeter_Beijing();
    Script_RouterState_ReadMeter_Beijing_RunState emScriptRunState;

    BuildNetwork_GW *p_BuildNetwork_GW;

    ushort concentratorCnt;
    bool resultFlag;
    shared_ptr<QList<shared_ptr<CtrInfo>>> p_CtrInfoList;

    QTimer *p_timer;
    QTimer *p_maxAllowTimer;
    QTimer *p_delayTimer;
    AbstractScriptHost *p_AbstractScriptHost;

    QByteArray sendMsgOct;
    QString sendMsgLog;
    QString logMsgStr;
    uchar addr[6];

    shared_ptr<Frame3762Helper> p_Frame3762Helper;
    shared_ptr<Afn00F1> p_Confirm_00F1;
    shared_ptr<Afn14F1> p_RouterRequestRead_14F1;
    shared_ptr<Afn12F1> p_RestartRouter_12F1;
    shared_ptr<Afn05F130_Beijing> p_SetAreaIdentityFlag_05F130;
    shared_ptr<Afn10F4_Beijing> p_QueryRouterState_10F4;

    shared_ptr<Frame645Helper> p_Frame645Helper;
    shared_ptr<RspsNormal_ReadAddr_0x93> p_MeterAddrResp_93;
    shared_ptr<FrameOOPHelper> p_FrameOOPHelper;
    shared_ptr<GetResponseNormal> p_GetResponseNormal_ReadAddr;

    int tryTimes=0;
    uchar msgSeq=0;
    ushort times=0;
    ushort index=0;
    const int maxMonitorTime=61;
    double havePassedTimeLen=0.0;
    bool haveStartContinueTimer=false;
    QList<QByteArray> addrList;
    /***可配置参数***/
    ushort timerForReachThresld=1200; //单位:s
    ushort timerAfterReachThresld=120; //单位:s

    double netSucRateThresld=1.0;
    bool needBuildNet=true;
    uchar addCntPerTime=5;
    uchar topoCntPerTime=5;
    bool needPowerOff=true;
    //参数
    Address thisRequestNodeAddr;
public:
    void  execute();
    void  stop();
    void  addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq);
    void  setHost(AbstractScriptHost *host);
    bool  config(const QMap<QString,QString> *paraDic);
    void  processMsg(DvcType dvcType,int id,uchar* data,int datalen);
    void  processCtrlDvcRes(DvcType dvcType,QList<int> idList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params);

public slots:
    void timer_timeoutProc();
    void maxAllowTimer_timeoutProc();
    void delayTimer_timeoutProc();
private:
    void processMsgFromCCO(DvcType dvcType, int dvcId);
    void processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID);
    void processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID);
    void sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame);
    void sendSrcMsg(DvcType dvcType,int dvcId,QByteArray msg);
};
#endif // SCRIPT_ROUTERSTATE_READMETER_BEIJING_H
