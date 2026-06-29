#ifndef SCRIPT_READMETER_14F1_13F1_F1F1_H
#define SCRIPT_READMETER_14F1_13F1_F1F1_H

#include <QObject>
#include "TestCase/BuildNetwork_GW.h"
enum Script_ReadMeter_14F1_13F1_F1F1_RunState
{
    TestInit,
    Wait_BuildNetFinish_Whole,
    Wait_RouterRestart_Finish,
    Wait_ReadMeter_Finish,
    RouterPause,
    ScriptSuccess
};
namespace Namespace_ReadMeter_14F1
{
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
    //    uchar phase;
        bool requestFlag_14F1;
        ReadFlag readFlag_14F1;
        ReadFlag readFlag_13F1;
        ReadFlag readFlag_F1F1;
        QList<ReadDataUnit> dataUnitList;
    };
}
using namespace Namespace_ReadMeter_14F1;

class Script_ReadMeter_14F1_13F1_F1F1 : public QObject, public AbstractScript,public DynamicCreate<Script_ReadMeter_14F1_13F1_F1F1>
{
    Q_OBJECT
//    Q_PLUGIN_METADATA(IID "com.topscomm.AbstractScript")
//    Q_INTERFACES(AbstractScript)
public:
    explicit Script_ReadMeter_14F1_13F1_F1F1(QObject *parent = nullptr);
    ~Script_ReadMeter_14F1_13F1_F1F1();

    Script_ReadMeter_14F1_13F1_F1F1_RunState emScriptRunState;
    BuildNetwork_GW *p_BuildNetwork_GW;

    ushort concentratorCnt;
    bool resultFlag;
    shared_ptr<QList<shared_ptr<CtrInfo>>> p_CtrInfoList;

    QTimer *p_timer;
    QTimer *p_maxAllowTimer;
    QTimer *p_13F1Timer,*p_F1F1Timer,*p_14F1Timer;
    AbstractScriptHost *p_AbstractScriptHost;

    QByteArray sendMsgOct;
    QString sendMsgLog;
    QString logMsgStr;
    uchar addr[6];

    shared_ptr<Frame3762Helper> p_Frame3762Helper;
    shared_ptr<Afn00F1> p_Confirm_00F1;
    shared_ptr<Afn00F2> p_Deny_00F2;
    shared_ptr<Afn12F1> p_RouterRestart_12F1;
    shared_ptr<Afn12F2> p_RouterPause_12F2;
    shared_ptr<Afn12F3> p_RouterRecover_12F3;
    shared_ptr<Afn14F1> p_RouterRequestRead_14F1;
    shared_ptr<Afn13F1> p_MonitorSlaveNode_13F1;
    shared_ptr<AfnF1F1> p_ParallelReadMeter_F1F1;

    QList<ReadInfo> readInfoList;

    shared_ptr<Frame645Helper> p_Frame645Helper;
    shared_ptr<RspsNormal_ReadAddr_0x93> p_MeterAddrResp_93;
    shared_ptr<FrameOOPHelper> p_FrameOOPHelper;
    shared_ptr<GetResponseNormal> p_GetResponseNormal_ReadAddr;

    int tryTimes=0;
    uchar msgSeq=0;
    ushort times=0;
  //  ushort index=0;
 //   QString startTime;
    QString endTime;
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
    //参数
//    bool flagAllowRouterPause=true;
//    int waitTime=1;//min
//    int reportCount_14F1=0;

 //   int currentMeterIndex;
    int requestNum_14F1=0,reportSucNum_14F1=0,reportFailNum_14F1=0;
    int requestNum_13F1=0,responseSucNum_13F1=0,responseFailNum_13F1=0,timeoutNum_13F1=0,readNo_13F1=0;
    int requestNum_F1F1=0,responseSucNum_F1F1=0,responseFailNum_F1F1=0,timeoutNum_F1F1=0,readNo_F1F1=0;
    bool endFlag_13F1=false,endFlag_14F1=false,endFlag_F1F1=false;

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
    void timeoutProc_13F1();
    void timeoutProc_F1F1();
    void timeoutProc_14F1();
private:
    void readInfoInit();
    void resultCheck();
    bool isMeterExist(Address address);
    int getReadInfo(Address address);
    double calSuccessRate();
    QString calCostTime();
    QString getFailMeterNo();
    void CalcAvrgConsumeTimeLen(uchar rdFlag);  //1:13F1; 2:F1F1; 3:14F1

    void processMsgFromCCO(DvcType dvcType, int dvcId);
    void processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID);
    void processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID);
    //不加锁的版本（供processMsg内部调用）
    void processMsgFromMeter645_NoLock(DvcType dvcType, int dvcId, int mtrlID);
    void processMsgFromMeterOOP_NoLock(DvcType dvcType, int dvcId, int mtrlID);

    void sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame);
    void sendSrcMsg(DvcType dvcType,int dvcId,QByteArray msg);

    bool extractAndProcess3762Frame(QByteArray& buf,QByteArray& completeFrame);
    Address extractAddressFromAfn06F2(shared_ptr<Afn06F2> p_ReportReadData_Up);

    // 消息处理互斥锁（每个表一个锁，保护缓冲区访问）
    QMap<int, QMutex*> meterMutexMap;  // key: mtrlID

    // 响应限流（防止短时间内大量回复导致系统崩溃）
    QMap<int, qint64> lastResponseTimeMap;  // key: mtrlID, value: last response timestamp (ms)
    const int MIN_RESPONSE_INTERVAL_MS = 50;  // 最小响应间隔50ms

    // 消息处理标志（防止递归和重入）
    QMap<int, bool> meterProcessingMap;  // key: mtrlID, value: is currently processing

    // 初始化和清理互斥锁
    void initMeterMutex(int mtrlID);
    void cleanupMeterMutex();

    // 检查并等待响应间隔
    void waitForResponseInterval(int mtrlID);

    QMutex m_ccoProcessMutex;  // 保护processMsgFromCCO
    QAtomicInt m_msgSeqAtomic; // 原子msgSeq

};

#endif // SCRIPT_READMETER_14F1_13F1_F1F1_H
