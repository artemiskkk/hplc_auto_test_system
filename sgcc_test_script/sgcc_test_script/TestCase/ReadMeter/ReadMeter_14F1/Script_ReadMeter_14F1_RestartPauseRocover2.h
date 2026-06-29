#ifndef SCRIPT_READMETER_14F1_RESTARTPAUSEROCOVER2_H
#define SCRIPT_READMETER_14F1_RESTARTPAUSEROCOVER2_H

#include <QObject>
#include "TestCase/BuildNetwork_GW.h"
enum Script_ReadMeter_14F1_RestartPauseRocover2_RunState
{
    ReadMeterInit,
    Wait_BuildNetFinish_Whole,
    Wait_00F1_for_12F1_Restart,
    Wait_00F1_for_12F2_Pause,
    Wait_00F1_for_12F3_Recover,
    Wait_Finish_14F1,
    ScriptSuccess
};
namespace Namespace_ReadMeter_14F1
{
    enum ReadFlag
    {
        ReadFail,
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
     * @brief 14F1抄读信息
     */
    struct ReadInfo_14F1
    {
        Address meterNo;
        uchar protocolType;
        uchar phase;
        ReadFlag readFlag;
        QList<ReadDataUnit> dataUnitList;
    };
}
using namespace Namespace_ReadMeter_14F1;
class Script_ReadMeter_14F1_RestartPauseRocover2 : public QObject, public AbstractScript,public DynamicCreate<Script_ReadMeter_14F1_RestartPauseRocover2>
{
    Q_OBJECT
//    Q_PLUGIN_METADATA(IID "com.topscomm.AbstractScript")
//    Q_INTERFACES(AbstractScript)
public:
    explicit Script_ReadMeter_14F1_RestartPauseRocover2(QObject *parent = nullptr);
    ~Script_ReadMeter_14F1_RestartPauseRocover2();

    Script_ReadMeter_14F1_RestartPauseRocover2_RunState emScriptRunState;
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
    shared_ptr<Afn00F1> p_Confirm_00F1;
    shared_ptr<Afn00F2> p_Deny_00F2;
    shared_ptr<Afn12F1> p_RouterRestart_12F1;
    shared_ptr<Afn12F2> p_RouterPause_12F2;
    shared_ptr<Afn12F3> p_RouterRecover_12F3;
    shared_ptr<Afn14F1> p_RouterRequestRead_14F1;

    QList<ReadInfo_14F1> readInfoList;

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

    double netSucRateThresld=1.0,readMeterSucRateThresld=1.0;
    bool needBuildNet=true;
    bool needPowerOff=true;
    uchar addCntPerTime=5;
    uchar topoCntPerTime=5;
    ////
    int runStep=0;
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
    void CalcAvrgConsumeTimeLen(uchar rdFlag);  //1:13F1; 2:F1F1; 3:14F1

    Address extractAddressFromAfn06F2(shared_ptr<Afn06F2> p_ReportReadData_Up);
    bool extractAndProcess3762Frame(QByteArray& buf,QByteArray& completeFrame);
    void processMsgFromCCO(DvcType dvcType, int dvcId);
    void processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID);
    void processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID);
    void sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame);
    void sendSrcMsg(DvcType dvcType,int dvcId,QByteArray msg);
};

#endif // SCRIPT_READMETER_14F1_RESTARTPAUSEROCOVER2_H
