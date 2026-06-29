#ifndef SCRIPT_CLOCKOVERUNDERREPORT_HUNAN_H
#define SCRIPT_CLOCKOVERUNDERREPORT_HUNAN_H

#include <QObject>
#include "TestCase/BuildNetwork_GW.h"
enum Script_BroadcastTime_Hunan_RunState
{
    BroadcastInit,
    Wait_BuildNetFinish_Whole,
    Wait_00F1_For_05F3_Broadcast,//校时时间大于电表事间5min以上
    Wait_06F5_Report,

    Wait_00F1_For_05F3_01_Broadcast,//校时时间小于电表时间5min以上
    Wait_06F5_2_Report,
    Wait_00F1_For_05F3_00_Broadcast,//校时时间与电表事间差在5min之内
    Wait_06F5_3_Report,

    Wait_Finish_Broadcast,
    ScriptSuccess
};
namespace Namespace_BroadcastTime_Hunan
{
    struct MeterInfoBroadcast_Struct
    {
        Address meterNo;
        uchar protocolType;
        bool readFlag;
        uchar offsetFlag;
        char dateTime[6];
    };
}
using namespace Namespace_BroadcastTime_Hunan;
class Script_ClockOverUnderReport_Hunan : public QObject, public AbstractScript,public DynamicCreate<Script_ClockOverUnderReport_Hunan>
{
    Q_OBJECT
//    Q_PLUGIN_METADATA(IID "com.topscomm.AbstractScript")
//    Q_INTERFACES(AbstractScript)
public:
    explicit Script_ClockOverUnderReport_Hunan(QObject *parent = nullptr);
    ~Script_ClockOverUnderReport_Hunan();

    Script_BroadcastTime_Hunan_RunState emScriptRunState;
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
    const int maxWaitTime=60;
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
    int checkTime=600;//超过5分钟
    int delayTime=120;//s
    //是否收到校时报文
    bool timingMsgFlag=false;
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


#endif // SCRIPT_CLOCKOVERUNDERREPORT_HUNAN_H
