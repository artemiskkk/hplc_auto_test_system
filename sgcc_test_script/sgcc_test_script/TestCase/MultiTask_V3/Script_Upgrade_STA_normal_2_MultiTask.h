#ifndef SCRIPT_UPGRADE_STA_NORMAL_2_MULTITASK_H
#define SCRIPT_UPGRADE_STA_NORMAL_2_MULTITASK_H

#include "TestCase/BuildNetwork_GW.h"

const ushort SEG_LEN_STA = 512;
enum Script_Upgrade_STA_RunState
{
    UpgradeInit,
    Wait_BuildNetFinish_Whole,
    Wait_00F1_for_12F2_UpgrdSta,
    Wait_00F1_for_11F6_UpgrdSta,
    Wait_15F1_for_15F1_BeforeUpgrdSta,
    Wait_Res_for_10F4_BeforeUpgrdSta,
    Wait_FileTransferFinish,
    Wait_Res_for_03F10_WaitTimeLen,
    Wait_StaUpgradeFinish,
    Wait_QueryOutVrsnInfo10F104Finish,
    Wait_QueryInVrsnInfo02F1Finish,
    ScriptSuccess
};
enum ReadFlag
{
    ReadFail,
    ReadSuccess,
    WaitRead,
};

struct ReadDataUnit
{
    QByteArray dataID;
    bool notRead;
    double costTime;
};

struct ReadInfo
{
    Address meterNo;
    uchar protocolType;
    bool requestFlag_14F1;
    ReadFlag readFlag_14F1;
//    ReadFlag readFlag_13F1;
//    ReadFlag readFlag_F1F1;
    QList<ReadDataUnit> dataUnitList;
};

class Script_Upgrade_STA_normal_2_MultiTask : public QObject, public AbstractScript, public DynamicCreate<Script_Upgrade_STA_normal_2_MultiTask>
{
    Q_OBJECT
  //  Q_PLUGIN_METADATA(IID "com.topscomm.AbstractScript")
   // Q_INTERFACES(AbstractScript)
public:
    explicit Script_Upgrade_STA_normal_2_MultiTask(QObject *parent = nullptr);
    ~Script_Upgrade_STA_normal_2_MultiTask();

    Script_Upgrade_STA_RunState emScriptRunState;

    ushort concentratorCnt;
    bool resultFlag;
    shared_ptr<QList<shared_ptr<CtrInfo>>> p_CtrInfoList;

    shared_ptr<QTimer> p_timer;
    shared_ptr<QTimer> p_maxAllowTimer;
  //  shared_ptr<QTimer> p_send10F4Timer;
    shared_ptr<QTimer> p_delayTimer;
    shared_ptr<QTimer> p_13F1Timer,p_F1F1Timer,p_14F1Timer;
    AbstractScriptHost *p_AbstractScriptHost;

    shared_ptr<BuildNetwork_GW> p_BuildNetwork;
    shared_ptr<qgdw_3762_protocol::Frame3762Helper> p_MsgBase_1376_2;
    shared_ptr<qgdw_3762_protocol::Afn15F1> p_FileTransfer_15F1_Down;
    shared_ptr<qgdw_3762_protocol::Afn10F4> p_CcoRunStateInfo_10F4_Down;
    shared_ptr<qgdw_3762_protocol::Afn03F10> p_CcoRunModeInfo_03F10_Down;
    shared_ptr<qgdw_3762_protocol::Afn12F2> p_CcoCtrlPause_12F2;
    shared_ptr<qgdw_3762_protocol::Afn11F6> p_StopSlaveNodeReg_11F6;
    shared_ptr<qgdw_3762_protocol::Afn02F1> p_ChkStaInVrsnInfo_02F1_Down;
    shared_ptr<qgdw_3762_protocol::Afn10F104> p_ChkStaOutVrsnInfo_10F104_Down;
    shared_ptr<Afn11F5> p_ActiveRegister_11F5;
    shared_ptr<Afn00F1> p_Confirm_00F1;
    shared_ptr<Afn12F1> p_RouterRestart_12F1;
    shared_ptr<Afn14F1> p_RouterRequestRead_14F1;
    shared_ptr<Afn13F1> p_MonitorSlaveNode_13F1;
    shared_ptr<AfnF1F1> p_ParallelReadMeter_F1F1;
    QByteArray sendMsgOct;
    QString sendMsgLog;

    shared_ptr<dlt_645_Protocol::Frame645Helper> p_MsgBase_645;
    shared_ptr<dlt_645_Protocol::RspsNormal_ReadAddr_0x93> p_MeterAddrResp_93;
    shared_ptr<object_oriented_electic_data_exchange_protocol::FrameOOPHelper> p_MsgBase_698_45;
    shared_ptr<GetResponseNormal> p_GetResponseNormal_ReadAddr;

    uchar addr[6];
    QList<ReadInfo> readInfoList;

    int dataLen;    //升级包长度，单位:字节
    ushort totalSegs;   //总段数
    QByteArray rawUpdateFile;
    QList<bool> transResList;
    ushort cfgCntPerTime;
    ushort fileIndex,meterIndex_02F1,meterIndex_10F104;
    uchar msgSeq;
    ushort sendTimes;

    double havePassedTimeLen;
    bool haveStartContinueTimer;
    /***可配置参数***/
    ushort timerForReachThresld; //单位:s
    double netSucRateThresld;
    ushort timerAfterReachThresld; //单位:s

    ushort timerForReachThresld_Upgrade; //单位:s
    double sucRateThresld_Upgrade;
    ushort timerAfterTransferFinished; //单位:s

    ushort timerForReachThresld_QueryNodeVrsnInfo02F1; //单位:s
    double sucRateThresld_QueryNodeVrsnInfo02F1;
    ushort timerAfterReachThresld_QueryNodeVrsnInfo02F1; //单位:s

    bool needBuildNet;
    uchar dstUpgradeDvc;
    bool isStdPrcs=true;

    QString staOutVrsn,staInVrsn;
    QString staVendorChipCode;
    QStringList failAddr_OutVrsn;
    ushort failCnt_InVrsn;
    ushort activeTime=2;//搜表时间
 //   bool reportFlag=false;
    ushort readNo_13F1=0,readNum_13F1=2,sucNum_13F1=0;
    ushort requestNo_14F1=0,requestMeterNum_14F1=2,sucNum_14F1=0;
    ushort readNo_F1F1=0,readNum_F1F1=2,sucNum_F1F1=0;
    ushort ackNo=0,searchNum=0;
    bool start14F1Flag=false,startSearchMeterFlag=false;
    bool end13F1Flag=false,end14F1Flag=false,endF1F1Flag=false,endSearchMeterFlag=false;

public:
    void  execute();
    void  stop();
    void  addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq);
    void  setHost(AbstractScriptHost *host);
    bool  config(const QMap<QString,QString> *paraDic);
    void  processMsg(DvcType dvcType,int id,uchar* data,int datalen);
    void  processCtrlDvcRes(DvcType dvcType,QList<int> idList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params);

private:
    void readInfoInit();
    int getReadInfo(Address address);
    ushort getCurrentRequestMeterNum_14F1();
    void LoadUpdateFile();
    void Refresh_TestResult_15F1(shared_ptr<Afn15F1> p_FileTransfer_15F1_Up);
    ushort Refresh_SuccessCnt_15F1();
    void Check_StaOutVrsnInfo(shared_ptr<Afn10F104> p_ChkStaOutVrsnInfo_10F104_Up);
    void Refresh_TestResult_02F1(shared_ptr<Afn02F1> p_ChkNodeVrsnInfo_02F1_Up);
    void Refresh_SuccessCnt_02F1();
    void checkResult();

    void processMsgFromCco(int id);
    void processMsgFromMeter645(DvcType dvcType,int id, int mtrlID);
    void processMsgFromMeter698(DvcType dvcType,int id, int mtrlID);
    void sendMsg(DvcType dvcType,int id,int meterID, shared_ptr<void> frame);
    void sendMsg(DvcType dvcType,int id,QByteArray msg);

    void RetransmitFailedSegments();
    Address extractAddressFromAfn06F2(shared_ptr<Afn06F2> p_ReportReadData_Up);
    bool extractAndProcess3762Frame(QByteArray& buf,QByteArray& completeFrame);
    QMap<ushort, ushort> segmentRetryCount;
    QList<ushort> failedSegments;
    const ushort MAX_RETRY_TIMES = 3;
    bool isRetransmitting = false;

    QMutex m_meterMutex;              // 电表互斥锁
    QSet<QString> m_busyMeters;       // 正在抄读的表集合
    QMap<QString, qint64> m_meterLockTime;  // 锁定时间

    bool tryLockMeter(char* meterAddr, const QString& taskName);
    void unlockMeter(char* meterAddr);
    void tryStart13F1();   // 尝试启动13F1抄表，失败时自动重试
    void tryStartF1F1();   // 尝试启动F1F1抄表，失败时自动重试

public slots:
    void timer_timeoutProc();
  //  void timer_send10F4TimeoutProc();
    void delayTimer_timeoutProc();
    void maxAllowTimer_timeoutProc();
    void timeoutProc_14F1();
    void timeoutProc_13F1();
    void timeoutProc_F1F1();
};

#endif // SCRIPT_UPGRADE_STA_NORMAL_2_MULTITASK_H
