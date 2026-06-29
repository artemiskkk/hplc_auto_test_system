#ifndef SCRIPT_ROUTERUNNINGSTATUS_GANSU_H
#define SCRIPT_ROUTERUNNINGSTATUS_GANSU_H

#include <QObject>
#include "TestCase/BuildNetwork_GW.h"
const ushort SEG_LEN_STA = 512;
enum Script_RouteRunningStatus_Gansu_RunState
{
    TestInit,
    Wait_BuildNetFinish_Whole,

//    Wait_HardReset_Finish,
//    Wait_ParaInit_Finish,
//    Wait_AddSlaveNode_11F1_Finish,
//    Wait_QueryNetTopoInfo_10F21_Finish,
    Wait_QueryRouteRunStatus_10F4_Finish,
    Wait_StationIdentiSwitch_05F6_Open_Finish,
    Wait_QueryRouteRunStatus_10F4_2_Finish,
    Wait_StationIdentiSwitch_05F6_Close_Finish,
    Wait_QueryRouteRunStatus_10F4_3_Finish,
    Wait_StartMeterSearch_11F5_Finish,
    Wait_QueryRouteRunStatus_10F4_4_Finish,
    Wait_StopMeterSearch_11F6_Finish,
    Wait_QueryRouteRunStatus_10F4_5_Finish,
    Wait__15F1_BeforeUpgrdCco_Finish,
    Wait_FileTransfer_15F1_Down_Finish,
    Wait_QueryRouteRunStatus_10F4_6_Finish,
    Wait_QueryRouteRunStatus_10F4_7_Finish,

    ScriptSuccess
};

enum Areadifference_State
{
    OpenIdenti,
    CloseIdenti
};

class Script_RouteRunningStatus_Gansu : public QObject, public AbstractScript,public DynamicCreate<Script_RouteRunningStatus_Gansu>
{
    Q_OBJECT
public:
    explicit Script_RouteRunningStatus_Gansu(QObject *parent = nullptr);
    ~Script_RouteRunningStatus_Gansu();

    Script_RouteRunningStatus_Gansu_RunState emScriptRunState;
    Areadifference_State emAreadifference_State;
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
//    shared_ptr<Afn01F1> p_HardReset_01F1;
//    shared_ptr<Afn01F2> p_ParaInit_01F2;
//    shared_ptr<Afn11F1> p_AddSlaveNode_11F1;
//    shared_ptr<Afn10F21> p_QueryNetTopoInfo_10F21;
    shared_ptr<Afn10F4_Beijing> p_QueryRouteRunStatus_10F4;
    shared_ptr<Afn05F6> p_StationIdentiSwitch_05F6;
    shared_ptr<Afn11F5> p_StartMeterSearch_11F5;
    shared_ptr<Afn11F6> p_StopMeterSearch_11F6;
    shared_ptr<Afn15F1> p_FileTransfer_15F1_Down;

//    shared_ptr<Afn10F1>  p_QueryNodeNum_10F1;
//    shared_ptr<Afn06F10> p_ChangeNodeState_06F10;

//    shared_ptr<AfnF0F100> p_QueryBasicNetworkInfo_F0F100;
//    shared_ptr<Afn00F1> p_Confirm_00F1;

    shared_ptr<Frame645Helper> p_Frame645Helper;
    shared_ptr<RspsNormal_ReadAddr_0x93> p_MeterAddrResp_93;
    shared_ptr<FrameOOPHelper> p_FrameOOPHelper;
    shared_ptr<GetResponseNormal> p_GetResponseNormal_ReadAddr;

    int tryTimes=0;
    uchar msgSeq=0;
    ushort times=0;
//    ushort index=0;
    //QString startTime;
    //QString endTime;
    const int maxMonitorTime=60;
    double havePassedTimeLen=0.0;
    bool haveStartContinueTimer=false;
    QList<QByteArray> addrList;

    /***可配置参数***/
    ushort timerForReachThresld=1800; //单位:s
    ushort timerAfterReachThresld=120; //单位:s
    double netSucRateThresld=1.0;
    bool needBuildNet=true;
    bool needPowerOff=true;

    uchar addCntPerTime=5;
    uchar topoCntPerTime=5;

    //参数
    int dataLen;    //升级包长度，单位:字节
    ushort totalSegs;   //总段数
    ushort fileIndex=0;
    QByteArray rawUpdateFile;
    QList<bool> transResList;
    uchar dstUpgradeDvc=1;
    ushort activeTime=2;
//    int report_node_state_num = 0;//上报的节点状态数
    ushort netScale;
//    const int routerPeriod=10;//单位:s
    int index=0;
    int num=5;

//    struct PowerEvent_Struct
//    {
//        char eventType;
//        Address reportNodeAddress;
//    };
//    QList<PowerEvent_Struct> powerOnReportList;
//    QList<PowerEvent_Struct> powerOffReportList;
//    QList<PowerEvent_Struct> otherEventReportList;
//    bool firstPowerOnReport=true;
//    const int maxWaitReportTime=35*60;
public:
    void  execute();
    void  stop();
    void  addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq);
    void  setHost(AbstractScriptHost *host);
    bool  config(const QMap<QString,QString> *paraDic);
    void  processMsg(DvcType dvcType,int id,uchar* data,int datalen);
    void  processCtrlDvcRes(DvcType dvcType,QList<int> idList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params);

signals:

public slots:
    void timer_timeoutProc();
    void maxAllowTimer_timeoutProc();
    void delayTimer_timeoutProc();

private:
    void LoadUpdateFile();
    void Refresh_TestResult_15F1(shared_ptr<Afn15F1> p_FileTransfer_15F1_Up);
    ushort Refresh_SuccessCnt_15F1();
//    QString GetBit( uchar x, int y);

    void processMsgFromCCO(DvcType dvcType, int dvcId);
    void processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID);
    void processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID);
    void sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame);
    void sendSrcMsg(DvcType dvcType,int dvcId,QByteArray msg);

};
#endif // SCRIPT_ROUTERUNNINGSTATUS_GANSU_H
