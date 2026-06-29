#ifndef SCRIPT_AREAIDENTITY_BEIJING_H
#define SCRIPT_AREAIDENTITY_BEIJING_H

#include <QObject>
#include "TestCase/BuildNetwork_GW.h"
enum Script_AreaIdentity_Beijing_RunState
{
    TestInit,
    Wait_BuildNetFinish_Whole,
    Wait_EraseFlash_Finish,
    Wait_AddSlaveNode_Finish,
    Wait_QueryRouterStateDisable_Finish,
    Wait_SetAreaIdentityFlagEnable_Finish,
    Wait_ParameterInit_Finish,
    Wait_HardReset_Finish,
    Wait_QueryRouterStateEnable_Finish,
    Wait_SetAreaIdentityFlagDisable_Finish,
    Wait_ParameterInit_After_Finish,
    Wait_HardReset_After_Finish,
    Wait_QueryRouterStateDisable_After_Finish,
    ScriptSuccess
};
class Script_AreaIdentity_Beijing : public QObject, public AbstractScript,public DynamicCreate<Script_AreaIdentity_Beijing>
{
    Q_OBJECT
//    Q_PLUGIN_METADATA(IID "com.topscomm.AbstractScript")
//    Q_INTERFACES(AbstractScript)
public:
    explicit Script_AreaIdentity_Beijing(QObject *parent = nullptr);
    ~Script_AreaIdentity_Beijing();
    Script_AreaIdentity_Beijing_RunState emScriptRunState;

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
    shared_ptr<Afn01F2> p_ParameterInit_01F2;
    shared_ptr<Afn01F1> p_HardReset_01F1;
    shared_ptr<Afn05F130_Beijing> p_SetAreaIdentityFlag_05F130;
    shared_ptr<AfnF0F12> p_EraseFlash_F0F12;
    shared_ptr<Afn11F1> p_AddSlaveNode_11F1;
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
    struct MeterInfo_Struct
    {
        Address nodeAddress;
        char protocol;
    };
    QList<MeterInfo_Struct> meterList;
    const char stateEnable=0x01;
    const char stateDisable=0x00;
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
    void getMeterList();
    void processMsgFromCCO(DvcType dvcType, int dvcId);
    void processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID);
    void processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID);
    void sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame);
    void sendSrcMsg(DvcType dvcType,int dvcId,QByteArray msg);
};

#endif // SCRIPT_AREAIDENTITY_BEIJING_H
