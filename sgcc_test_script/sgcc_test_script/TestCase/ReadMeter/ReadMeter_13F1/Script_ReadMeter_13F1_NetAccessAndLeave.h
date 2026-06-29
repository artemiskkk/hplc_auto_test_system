#ifndef SCRIPT_READMETER_13F1_NETACCESSANDLEAVE_H
#define SCRIPT_READMETER_13F1_NETACCESSANDLEAVE_H

#include <QObject>
#include "TestCase/BuildNetwork_GW.h"
enum Script_ReadMeter_13F1_RunState
{
    ReadMeterInit,
    Wait_BuildNetFinish_Whole,
    Wait_12F12_RouterPause_Finish,
    Wait_11F2_DeleteArchives_Finish,
    Wait_13F1_MeterReadDelete_Finish,
    Wait_13F1_MeterReadDeleteContinue_Finish,
    ScriptSuccess
};

class Script_ReadMeter_13F1_NetAccessAndLeave : public QObject, public AbstractScript,public DynamicCreate<Script_ReadMeter_13F1_NetAccessAndLeave>
{
    Q_OBJECT
public:
    explicit Script_ReadMeter_13F1_NetAccessAndLeave(QObject *parent = nullptr);
    ~Script_ReadMeter_13F1_NetAccessAndLeave();

    Script_ReadMeter_13F1_RunState emScriptRunState;
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

    shared_ptr<Frame3762Helper> p_MsgBase_3762;
    shared_ptr<Afn12F2> p_RouterPause_12F2;
    shared_ptr<Afn12F3> p_RouterRecover_12F3;
    shared_ptr<Afn13F1> p_MonitorSlaveNode_13F1;
    shared_ptr<Afn11F2> p_DeleteArchives_11F2;

    QList<ParallelReadMeter> parallelReadMeterList;

    shared_ptr<Frame645Helper> p_Frame645Helper;
    shared_ptr<RspsNormal_ReadAddr_0x93> p_MeterAddrResp_93;
    shared_ptr<FrameOOPHelper> p_FrameOOPHelper;
    shared_ptr<GetResponseNormal> p_GetResponseNormal_ReadAddr;
    //    MsgBase_645 *p_MsgBase_645;
    //    MstrStnNrmlRqst *p_MstrStnNrmlRqst;
    //    MeterAddrResp_93 *p_MeterAddrResp_93;
    //    SlaveNodeNormalResp *p_SlaveNodeNormalResp;
    //    MsgBase_698_45 *p_MsgBase_698_45;

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
    double netSucRateThresld=1.0,readMeterSucRateThresld=1.0;
    bool needBuildNet=true;
    bool needPowerOff=true;
    uchar addCntPerTime=5;
    uchar topoCntPerTime=5;

    bool flagNeedRead=false;

    Address CtrInfoList_00_addr;
    uchar tmp_645_addr[6];
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
    void statisticResult();
    void CalcAvrgConsumeTimeLen(uchar rdFlag);  //1:13F1; 2:F1F1; 3:14F1

    void processMsgFromCCO(DvcType dvcType, int dvcId);
    void processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID);
    void processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID);
    void sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame);
    void sendSrcMsg(DvcType dvcType,int dvcId,QByteArray msg);
};

#endif // SCRIPT_READMETER_13F1_NETACCESSANDLEAVE_H
