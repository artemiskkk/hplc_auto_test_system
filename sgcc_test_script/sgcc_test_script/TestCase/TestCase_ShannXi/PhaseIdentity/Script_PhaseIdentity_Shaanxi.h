#ifndef SCRIPT_PHASEIDENTITY_SHAANXI_H
#define SCRIPT_PHASEIDENTITY_SHAANXI_H

#include <QObject>
#include "TestCase/BuildNetwork_GW.h"
enum Script_PhaseIdentity_Shaanxi_RunState
{
    TestInit,
    Wait_BuildNetFinish_Whole,
    Wait_PhaseIdentity_10F102_Finish,
    ScriptSuccess
};
class Script_PhaseIdentity_Shaanxi: public QObject, public AbstractScript,public DynamicCreate<Script_PhaseIdentity_Shaanxi>
{
    Q_OBJECT
//    Q_PLUGIN_METADATA(IID "com.topscomm.AbstractScript")
//    Q_INTERFACES(AbstractScript)
public:
    explicit Script_PhaseIdentity_Shaanxi(QObject *parent = nullptr);
    ~Script_PhaseIdentity_Shaanxi();
    Script_PhaseIdentity_Shaanxi_RunState emScriptRunState;
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
    shared_ptr<Afn10F102_Shaanxi> p_QueryPhaseInfo_10F102;

    shared_ptr<Frame645Helper> p_Frame645Helper;
    shared_ptr<RspsNormal_ReadAddr_0x93> p_MeterAddrResp_93;
    shared_ptr<FrameOOPHelper> p_FrameOOPHelper;
    shared_ptr<GetResponseNormal> p_GetResponseNormal_ReadAddr;

    int tryTimes=0;
    uchar msgSeq=0;
    ushort times=0;
    //QString startTime;
    //QString endTime;
    const int maxMonitorTime=60;
    double havePassedTimeLen=0.0;
    bool haveStartContinueTimer=false;
    QList<QByteArray> addrList;
    /***可配置参数***/
    ushort timerForReachThresld=1200; //单位:s
    ushort timerAfterReachThresld=120; //单位:s

    double netSucRateThresld=1.0;
    bool needBuildNet=true;
    bool needPowerOff=true;
    uchar addCntPerTime=5;
    uchar topoCntPerTime=5;
    //
    enum Phase
    {
        A=1,
        B=2,
        C=4,
        ABC=7,
        Error
    };
    struct NodeInfoStruct
    {
        Address nodeAddress;
        uchar nodePhase;
        bool phaseIsRight=false;
        uchar realPhase;
    };
    QList<NodeInfo10F102_Shaanxi> nodeInfo_Shaanxi_List;
    QList<NodeInfoStruct> nodeInfoList_10F102;
    ushort index=0;
    uchar num=5;//每次查询数量
    int delayTime=90;//90s
    QString identityFailList_10F102;
    QString identitySuccessList_10F102;
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
    void generateStandardList();
    void getIndentityList();
    QString getPhaseInfo(uchar phase);
    void processMsgFromCCO(DvcType dvcType, int dvcId);
    void processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID);
    void processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID);
    void sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame);
    void sendSrcMsg(DvcType dvcType,int dvcId,QByteArray msg);
};
#endif // SCRIPT_PHASEIDENTITY_SHAANXI_H
