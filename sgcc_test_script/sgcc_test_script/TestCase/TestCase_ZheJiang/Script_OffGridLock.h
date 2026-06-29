#ifndef SCRIPT_OFFGRIDLOCK_H
#define SCRIPT_OFFGRIDLOCK_H

#include <QObject>
#include "TestCase/BuildNetwork_GW.h"

enum Script_OffGridLock_RunState
{
    TestInit,
    Wait_BuildNetFinish_Whole,
    Wait_SetStaOffGridLockTime_Finish,
    Wait_HardReset_Finish,
    Wait_QueryNetScale_Finish,
    Wait_AbnormalOffGridTime_Finish,
    Wait_AbnormalOffGridTime_Query_Finish,
    ScriptSuccess
};
class Script_OffGridLock : public QObject, public AbstractScript,public DynamicCreate<Script_OffGridLock>
{
    Q_OBJECT
public:
    explicit Script_OffGridLock(QObject *parent = nullptr);
    ~Script_OffGridLock();
    Script_OffGridLock_RunState emScriptRunState;
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
    shared_ptr<Afn01F1> p_HardReset_01F1;
    shared_ptr<Afn10F9> p_QueryNetScale_10F9;
    shared_ptr<Afn10F21> p_QueryTopoInfo_10F21;

    shared_ptr<Frame645Helper> p_Frame645Helper;
    shared_ptr<RspsNormal_ReadAddr_0x93> p_MeterAddrResp_93;
    shared_ptr<FrameOOPHelper> p_FrameOOPHelper;
    shared_ptr<GetResponseNormal> p_GetResponseNormal_ReadAddr;
    //**脚本需要的指针
    shared_ptr<Afn05F30_Zhejiang> p_SetStaOffGridLockTime_05F30;

    int tryTimes=0;
    uchar msgSeq=0;
    ushort times=0;
    //QString startTime;
    //QString endTime;
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
    ////////
    ushort netScale;
    const int routerPeriod=10;//单位:s

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
    //**处理单通槽位返回的376.2报文
    //void processMsgFromSingleSta(DvcType dvcType, int dvcId);
};

#endif // SCRIPT_OFFGRIDLOCK_H
