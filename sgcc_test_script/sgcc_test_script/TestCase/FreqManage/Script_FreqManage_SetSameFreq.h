#ifndef SCRIPT_FREQMANAGE_SETSAMEFREQ_H
#define SCRIPT_FREQMANAGE_SETSAMEFREQ_H

#include <QObject>
#include "TestCase/BuildNetwork_GW.h"

enum Script_FreqManage_SetSameFreq_RunState
{
    TestInit,
    Wait_BuildNetFinish_Whole,
//    Wait_SetInitFreq_Finish,
    Wait_QueryInitFreq_Finish,
    Wait_SetDstFreq_Finish,
    Wait_QueryDstFreq_Finish,
//    Wait_HardResetTest_Finish,
//    Wait_EraseFlash_Finish,
    ScriptSuccess
};
enum QueryInfo_State
{
    Wait_QueryFreq_03F16,
    Wait_QueryFreqDivide_F0F51,
};
class Script_FreqManage_SetSameFreq : public QObject, public AbstractScript,public DynamicCreate<Script_FreqManage_SetSameFreq>
{
    Q_OBJECT
//    Q_PLUGIN_METADATA(IID "com.topscomm.AbstractScript")
//    Q_INTERFACES(AbstractScript)
public:
    explicit Script_FreqManage_SetSameFreq(QObject *parent = nullptr);
    ~Script_FreqManage_SetSameFreq();

    Script_FreqManage_SetSameFreq_RunState emScriptRunState;
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
 //   shared_ptr<Afn01F1> p_HardReset_01F1;
 //   shared_ptr<AfnF0F12> p_EraseFlash_F0F12;
    shared_ptr<Afn03F16> p_QueryFreq_03F16;
 //   shared_ptr<AfnF0F51> p_QueryFreqDivide_F0F51;
    shared_ptr<Afn05F16> p_SetFreq_05F16;
 //   shared_ptr<AfnF0F50> p_SetFreqDivide_F0F50;

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

    double netSucRateThresld=1.0;
    bool needBuildNet=true;
    bool needPowerOff=true;
    uchar addCntPerTime=5;
    uchar topoCntPerTime=5;
    //
    int waitTime=65;//单位:s
    char initFreq;//初始频段
//    char initFreqDivide=0x01;//初始分频系数1
//    char dstFreq=0x00;//目的频段0
//    char dstFreqDivide=0x01;//目的分频系数1
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
    void processMsgFromCCO(DvcType dvcType, int dvcId);
    void processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID);
    void processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID);
    void sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame);
    void sendSrcMsg(DvcType dvcType,int dvcId,QByteArray msg);
};
#endif // SCRIPT_FREQMANAGE_SETSAMEFREQ_H
