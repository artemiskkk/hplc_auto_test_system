#ifndef SCRIPT_BROADCASTTIME_ZEROTIME_H
#define SCRIPT_BROADCASTTIME_ZEROTIME_H

#include <QObject>
#include "TestCase/BuildNetwork_GW.h"
enum Script_BroadcastTime_ZeroTime_RunState
{
    BroadcastInit,
    Wait_BuildNetFinish_Whole,
    Wait_00F1_For_05F3_Broadcast,
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
}
using namespace Namespace_BroadcastTime;
class Script_BroadcastTime_ZeroTime : public QObject, public AbstractScript,public DynamicCreate<Script_BroadcastTime_ZeroTime>
{
    Q_OBJECT
//    Q_PLUGIN_METADATA(IID "com.topscomm.AbstractScript")
//    Q_INTERFACES(AbstractScript)
public:
    explicit Script_BroadcastTime_ZeroTime(QObject *parent = nullptr);
    ~Script_BroadcastTime_ZeroTime();

    Script_BroadcastTime_ZeroTime_RunState emScriptRunState;
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

    shared_ptr<Frame645Helper> p_Frame645Helper;
    shared_ptr<RspsNormal_ReadAddr_0x93> p_MeterAddrResp_93;
    shared_ptr<FrameOOPHelper> p_FrameOOPHelper;
    shared_ptr<GetResponseNormal> p_GetResponseNormal_ReadAddr;
    //OOP校时

    int tryTimes=0;
    uchar msgSeq=0;
    ushort times=0;
    ushort index=0;
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
    int checkTime=0;//秒，时间无偏移
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
    bool isMeterExist(Address address);
    int getMeterInfo(Address address);
    double calSuccessRate();
    QString getFailMeterNo();

    void processMsgFromCCO(DvcType dvcType, int dvcId);
    void processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID);
    void processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID);
    void sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame);
    void sendSrcMsg(DvcType dvcType,int dvcId,QByteArray msg);
};

#endif // SCRIPT_BROADCASTTIME_ZEROTIME_H
