#ifndef SCRIPT_INITIALIZETEST_NOTBUILDNET_H
#define SCRIPT_INITIALIZETEST_NOTBUILDNET_H

#include <QObject>
#include "TestCase/BuildNetwork_GW.h"

enum Script_InitializeTest_NotBuildNet_RunState
{
    TestInit,
    Wait_BuildNetFinish_Whole,
    Wait_QueryDefaultWorkSwitch_Finish,
    Wait_HardResetTest_Finish,
    Wait_ParaInitTest_Finish,
    ScriptSuccess
};
enum QueryInfo_State
{
    Wait_QueryMasterAddr_03F4,
    Wait_QueryNodeNum_10F1,
    Wait_QueryNodeInfo_10F2,
    Wait_QueryFreq_03F16,
    Wait_QueryRouterID_10F40,
    Wait_QueryChipID_10F40,
    Wait_QueryRouterSN_F0F41,
    Wait_QueryWorkSwitch_10F4,
};

class Script_InitializeTest_NotBuildNet : public QObject, public AbstractScript,public DynamicCreate<Script_InitializeTest_NotBuildNet>
{
    Q_OBJECT
//    Q_PLUGIN_METADATA(IID "com.topscomm.AbstractScript")
//    Q_INTERFACES(AbstractScript)
public:
    explicit Script_InitializeTest_NotBuildNet(QObject *parent = nullptr);
    ~Script_InitializeTest_NotBuildNet();
    Script_InitializeTest_NotBuildNet_RunState emScriptRunState;
    QueryInfo_State emQueryState;

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
    shared_ptr<Afn01F2> p_ParameterInit_01F2;
    shared_ptr<Afn03F4> p_QueryMasterAddr_03F4;
    shared_ptr<Afn10F1> p_QueryNodeNum_10F1;
    shared_ptr<Afn10F2> p_QueryNodeInfo_10F2;
    shared_ptr<Afn03F16> p_QueryFreq_03F16;
    shared_ptr<Afn10F40> p_QueryRouterID_10F40;
    shared_ptr<Afn10F40> p_QueryRouterChipID_10F40;
    shared_ptr<AfnF0F41> p_QueryRouterSN_F0F41;
    shared_ptr<Afn10F4> p_QueryWorkSwitch_10F4;

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
    const int maxMonitorTime=70;
    double havePassedTimeLen=0.0;
    bool haveStartContinueTimer=false;
    QList<QByteArray> addrList;
    /***可配置参数***/
    ushort timerForReachThresld=1200; //单位:s
    ushort timerAfterReachThresld=120; //单位:s

    QString routerID_10F40_="";
    QString chipID_10F40_="";

    double netSucRateThresld=1.0;
    bool needBuildNet=true;
    bool needPowerOff=true;
    uchar addCntPerTime=5;
    uchar topoCntPerTime=5;
    uchar runFreq;
    uchar current_state_;
    uchar event_report_flag_10F4_;
    uchar area_difference_flag_10F4_;
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
    bool meterIsExist(Address meterAddr);
    void processMsgFromCCO(DvcType dvcType, int dvcId);
    void processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID);
    void processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID);
    void sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame);
    void sendSrcMsg(DvcType dvcType,int dvcId,QByteArray msg);

};
#endif // SCRIPT_INITIALIZETEST_NOTBUILDNET_H
