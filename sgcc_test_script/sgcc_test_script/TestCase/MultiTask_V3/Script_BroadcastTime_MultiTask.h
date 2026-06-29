#ifndef SCRIPT_BROADCASTTIME_MULTITASK_H
#define SCRIPT_BROADCASTTIME_MULTITASK_H

#include <QObject>
#include "TestCase/BuildNetwork_GW.h"
enum Script_BroadcastTime_MultiTask_RunState
{
    BroadcastInit,
    Wait_BuildNetFinish_Whole,
    Wait_00F1_For_05F3_Broadcast,
    Wait_00F1_for_12F1_Restart,
    Wait_Finish_Broadcast,
    ScriptSuccess
};
namespace Namespace_BroadcastTime
{
    struct MeterInfoBroadcast_Struct
    {
        Address meterNo;
        uchar protocolType;
        bool readFlag;
    };

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
     //   uchar phase;
        ReadFlag readFlag;
        QList<ReadDataUnit> dataUnitList;
    };
}
using namespace Namespace_BroadcastTime;
class Script_BroadcastTime_MultiTask : public QObject, public AbstractScript,public DynamicCreate<Script_BroadcastTime_MultiTask>
{
    Q_OBJECT
//    Q_PLUGIN_METADATA(IID "com.topscomm.AbstractScript")
//    Q_INTERFACES(AbstractScript)
public:
    explicit Script_BroadcastTime_MultiTask(QObject *parent = nullptr);
    ~Script_BroadcastTime_MultiTask();

    Script_BroadcastTime_MultiTask_RunState emScriptRunState;
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
    shared_ptr<Afn05F3> p_Broadcast_05F3;
    shared_ptr<Afn00F1> p_Confirm_00F1;
    shared_ptr<Afn12F1> p_RouterRestart_12F1;
    shared_ptr<Afn12F2> p_RouterPause_12F2;
    shared_ptr<Afn14F1> p_RouterRequestRead_14F1;
    shared_ptr<Afn13F1> p_MonitorSlaveNode_13F1;
    shared_ptr<AfnF1F1> p_ParallelReadMeter_F1F1;

    QList<ReadInfo_14F1> readInfoList;

    shared_ptr<Frame645Helper> p_Frame645Helper;
    shared_ptr<RspsNormal_ReadAddr_0x93> p_MeterAddrResp_93;
    shared_ptr<FrameOOPHelper> p_FrameOOPHelper;
    shared_ptr<GetResponseNormal> p_GetResponseNormal_ReadAddr;
    //OOP校时

    int tryTimes=0;
    uchar msgSeq=0;
 //   ushort times=0;
 //   ushort index=0;
    QString startTime;
    QString endTime;
    const int maxBroadcastWaitTime=60;
    double havePassedTimeLen=0.0;
    bool haveStartContinueTimer=false;
    /***可配置参数***/
    ushort timerForReachThresld=1200; //单位:s
    ushort timerAfterReachThresld=120; //单位:s

    double netSucRateThresld=1.0;
    bool needBuildNet=true;
    uchar addCntPerTime=5;
    uchar topoCntPerTime=5;
    bool needPowerOff=true;
    //校时参数
    QList<MeterInfoBroadcast_Struct> meterInfoList;
    int checkTime=0;//秒
  //  int currentMeterIndex;
    int No_14F1;
    bool flag_13F1,flag_14F1,flag_F1F1,flag_broadcast;
    bool flag_pause;
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
    void meterInfoInit();
    void readInfoInit();
    int getReadInfo(Address address);
  //  bool isMeterExist(Address address);
    int getMeterInfo(Address address);
    double calSuccessRate();
    QString getFailMeterNo();

    void processMsgFromCCO(DvcType dvcType, int dvcId);
    void processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID);
    void processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID);
    void sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame);
    void sendSrcMsg(DvcType dvcType,int dvcId,QByteArray msg);
};

#endif // SCRIPT_BROADCASTTIME_MULTITASK_H
