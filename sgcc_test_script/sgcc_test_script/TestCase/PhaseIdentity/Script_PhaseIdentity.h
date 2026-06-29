#ifndef SCRIPT_PHASEIDENTITY_H
#define SCRIPT_PHASEIDENTITY_H

#include <QObject>
#include "TestCase/BuildNetwork_GW.h"
enum Script_PhaseIdentity_RunState
{
    TestInit,
    Wait_BuildNetFinish_Whole,
    Wait_00F1_for_12F2_Pause,
    Wait_PhaseIdentity_10F2_Finish,
    Wait_PhaseIdentity_10F31_Finish,
    Wait_PhaseIdentity_13F1_Finish,
    ScriptSuccess
};
class Script_PhaseIdentity: public QObject, public AbstractScript,public DynamicCreate<Script_PhaseIdentity>
{
    Q_OBJECT
//    Q_PLUGIN_METADATA(IID "com.topscomm.AbstractScript")
//    Q_INTERFACES(AbstractScript)
public:
    explicit Script_PhaseIdentity(QObject *parent = nullptr);
    ~Script_PhaseIdentity();
    Script_PhaseIdentity_RunState emScriptRunState;
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
    shared_ptr<Afn12F2> p_RouterPause_12F2;
    shared_ptr<Afn10F2> p_QueryNodeInfo_10F2;
    shared_ptr<Afn10F31> p_QueryPhaseInfo_10F31;
    shared_ptr<Afn13F1> p_MonitorSlaveNode_13F1;
    shared_ptr<Afn11F1> p_AddSlaveNode_11F1;
    shared_ptr<Afn01F1> p_HardReset_01F1;
    shared_ptr<Afn10F9> p_QueryNetScale_10F9;//查询网络规模
    shared_ptr<Afn10F21> p_QueryNetTopoInfo_10F21;//查询网络拓扑信息

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
    int querySpan=60;
    int maxQueryIndex=30;
    //
    enum Phase
    {
        A=1,
        B=2,
        AB=3,
        C=4,
        AC=5,
        BC=6,
        ABC=7,
        ACB=8,
        BAC=9,
        BCA=10,
        CAB=11,
        CBA=12,
        Error=0
    };
    enum QueryType
    {
        Query_13F1,
        Query_10F2,
        Query_10F31
    };
    enum CommandType
    {
        NormalCmd,
        AddNode,
        BuildNet,
        HardReset,
    };
    CommandType emCmdType=NormalCmd;
    struct NodeInfoStruct
    {
        Address nodeAddress;
        Phase nodePhase;
        Phase threePhase;
        bool phaseIsRight=true;
        Phase initPhase;
    };
    QList<NodeInfo10F31> nodeInfo10F31List_10F31;
    QList<NodeInfoGroup> nodeInfoGroupList_10F2;

    QList<NodeInfoStruct> nodeInfoList_13F1;
    QList<NodeInfoStruct> nodeInfoList_10F31;
    QList<NodeInfoStruct> nodeInfoList_10F2;
    ushort index=0;
    ushort nodeIndex=1;
    int currentQueryIndex=0;
    int identityFailTimes=0;
    QString identifyErrorInfo;
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
    void generateStandardList(QueryType emQueryType);
    void getFailList(QueryType emQueryType);
    Phase getNodePhase(QueryType emQueryType,uchar phase);
    Phase getThreePhase_10F31(uchar phase_sequence);
    Phase getThreePhase_config(uchar config);
    QString getPhaseInfo(Phase phase);
    void processMsgFromCCO(DvcType dvcType, int dvcId);
    void processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID);
    void processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID);
    void sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame);
    void sendSrcMsg(DvcType dvcType,int dvcId,QByteArray msg);
};

#endif // SCRIPT_PHASEIDENTITY_H
