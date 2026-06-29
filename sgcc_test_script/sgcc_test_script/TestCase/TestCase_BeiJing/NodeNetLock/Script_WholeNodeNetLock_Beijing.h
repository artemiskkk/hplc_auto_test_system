#ifndef SCRIPT_WHOLENODENETLOCK_BEIJING_H
#define SCRIPT_WHOLENODENETLOCK_BEIJING_H

#include <QObject>
#include "TestCase/BuildNetwork_GW.h"

enum Script_WholeNodeNetLock_Beijing_RunState
{
    TestInit,
    Wait_BuildNetFinish_Whole,
    Wait_ParameterInit_Finish,
    Wait_SetWhiteListState_05F131_Finish,
    Wait_SetWholeNodeNetLock_11F126_Finish,
    Wait_SetMasterAddr_05F1_Finish,
    Wait_QueryNetScale_10F9_Finish,
    ScriptSuccess
};
class Script_WholeNodeNetLock_Beijing : public QObject, public AbstractScript,public DynamicCreate<Script_WholeNodeNetLock_Beijing>
{
    Q_OBJECT
//    Q_PLUGIN_METADATA(IID "com.topscomm.AbstractScript")
//    Q_INTERFACES(AbstractScript)
public:
    explicit Script_WholeNodeNetLock_Beijing(QObject *parent = nullptr);
    ~Script_WholeNodeNetLock_Beijing();
    Script_WholeNodeNetLock_Beijing_RunState emScriptRunState;

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
    shared_ptr<Afn05F131_Beijing> p_SetWhiteListState_05F131;
    shared_ptr<Afn10F9> p_QueryNetScale_10F9;
    shared_ptr<Afn11F126_Beijing> p_SetNodeNetLock_11F126;
    shared_ptr<Afn05F1> p_SetMasterAddr_05F1;

    shared_ptr<Frame645Helper> p_Frame645Helper;
    shared_ptr<RspsNormal_ReadAddr_0x93> p_MeterAddrResp_93;
    shared_ptr<FrameOOPHelper> p_FrameOOPHelper;
    shared_ptr<GetResponseNormal> p_GetResponseNormal_ReadAddr;

    int tryTimes=0;
    uchar msgSeq=0;
    ushort times=0;
    ushort index=0;
    QString startTime;
    QString endTime;
    const int maxMonitorTime=60;
    double havePassedTimeLen=0.0;
    bool haveStartContinueTimer=false;
    QList<QByteArray> addrList;
    /***可配置参数***/
    ushort timerForReachThresld=1200; //单位:s
    ushort timerAfterReachThresld=120; //单位:s

    double netSucRateThresld=1.0;
    uchar addCntPerTime=5;
    uchar topoCntPerTime=5;
    bool needBuildNet=true;
    bool needPowerOff=true;
    //参数
    const char stateLock=0x00;
    const char stateUnlock=0x01;

    const char stateSingle=0x00;
    const char stateWhole=0x01;

    const char stateEnable=0x01;
    const char stateDisable=0x00;
    int waitTime=60;
    int queryPeriod=5;
    int waitBuildNetTime=600;//s
    char masterAddr[6]={0x11,0x11,0x11,0x22,0x22,0x22};
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
#endif // SCRIPT_SINGLENODENETLOCK_BEIJING_H
