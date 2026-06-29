#ifndef SCRIPT_READMETER_F1F1_METERADDRINCONSISTENT_H
#define SCRIPT_READMETER_F1F1_METERADDRINCONSISTENT_H

#include <QObject>
#include "TestCase/BuildNetwork_GW.h"
enum Script_ReadMeter_F1F1_MeterAddrInconsistent_RunState
{
    ReadMeterInit,
    Wait_BuildNetFinish_Whole,
    Wait_00F1_for_12F2_Pause,
    Wait_F1F1_MeterRead97_Finish,
    Wait_F1F1_MeterRead07_Finish,
    ScriptSuccess
};
namespace Namespace_ReadMeter_F1F1
{
enum ReadFlag
{
    //NotRead,
    ReadSuccess,
    Reading
};

struct ReadDataUnit
{
    QByteArray dataID;
    bool notRead;
    double costTime;
};

/**
     * @brief F1F1抄读信息
     */
struct ReadInfo_F1F1
{
    Address meterNo;
    uchar protocolType;
    ReadFlag readFlag;
    QList<ReadDataUnit> dataUnitList;
};
enum ReadMeterProtocol
{
    RMPTran,
    RMP97,
    RMP645,
    RMP698,
    RMPOther,
    RMPEnd
};
}
using namespace Namespace_ReadMeter_F1F1;
class Script_ReadMeter_F1F1_MeterAddrInconsistent : public QObject, public AbstractScript,public DynamicCreate<Script_ReadMeter_F1F1_MeterAddrInconsistent>
{
    Q_OBJECT
    //    Q_PLUGIN_METADATA(IID "com.topscomm.AbstractScript")
    //    Q_INTERFACES(AbstractScript)
public:
    explicit Script_ReadMeter_F1F1_MeterAddrInconsistent(QObject *parent = nullptr);
    ~Script_ReadMeter_F1F1_MeterAddrInconsistent();

    Script_ReadMeter_F1F1_MeterAddrInconsistent_RunState emScriptRunState;
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
    shared_ptr<Afn10F4> p_QueryRouterState_10F4;
    shared_ptr<Afn12F3> p_RouterRecover_12F3;
    shared_ptr<AfnF1F1> p_ParallelReadMeter_F1F1;

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

    double netSucRateThresld=1.0,readMeterSucRateThresld=1.0;
    bool needBuildNet=true;
    bool needPowerOff=true;
    uchar addCntPerTime=5;
    uchar topoCntPerTime=5;
    QList<int> dataIdIndexList;
    QString userCaseIds;
    QList<ReadInfo_F1F1> readInfoList;
    //并发抄表参数
    int maxFrameNum=16;//单帧最大报文数
    int maxParallelNum=5;//最大并行数
    int parallelCount=0;//并行数
    int meterIndex=0;//电表序号
    ReadMeterProtocol protocol;
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
    void readInfoInit();
    bool isMeterExist(Address address);
    int getReadInfo(Address address);
    double calSuccessRate();
    QString calCostTime();
    QString getFailMeterNo();
    bool isAllRead(int meterID);
    bool isAllMeterReadSuccess();
    void CalcAvrgConsumeTimeLen(uchar rdFlag);  //1:13F1; 2:F1F1; 3:14F1
    bool extractAndProcess3762Frame(QByteArray& buf,QByteArray& completeFrame);
    void processMsgFromCCO(DvcType dvcType, int dvcId);
    void processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID);
    void processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID);
    void sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame);
    void sendSrcMsg(DvcType dvcType,int dvcId,QByteArray msg);
};

#endif // SCRIPT_READMETER_F1F1_METERADDRINCONSISTENT_H
