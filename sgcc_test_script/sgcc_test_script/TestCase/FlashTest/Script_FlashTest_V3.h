#ifndef SCRIPT_FLASHTEST_V3_H
#define SCRIPT_FLASHTEST_V3_H

#include <QObject>
#include "TestCase/BuildNetwork_GW.h"

enum Script_FlashTest_V3_RunState
{
    TestInit,
    Wait_BuildNetFinish_Whole,
    Wait_QueryInitFreq_Finish,
    Wait_EraseFlash_Finish,
    ScriptSuccess
};
enum QueryInfo_State
{
    Wait_QueryMasterAddr_03F4,
    Wait_QueryNodeNum_10F1,
    Wait_QueryNodeInfo_10F2,
    Wait_QueryFreq_03F16,
    Wait_QueryRouterID_F0F40,
    Wait_QueryChipID_F0F40,
    Wait_QueryRouterSN_F0F41,
    Wait_QueryWorkSwitch_10F4,
};
class Script_FlashTest_V3 : public QObject, public AbstractScript,public DynamicCreate<Script_FlashTest_V3>
{
    Q_OBJECT
//    Q_PLUGIN_METADATA(IID "com.topscomm.AbstractScript")
//    Q_INTERFACES(AbstractScript)
public:
    explicit Script_FlashTest_V3(QObject *parent = nullptr);
    ~Script_FlashTest_V3();
    Script_FlashTest_V3_RunState emScriptRunState;
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
    shared_ptr<AfnF0F12> p_EraseFlash_F0F12;
    shared_ptr<Afn03F4> p_QueryMasterAddr_03F4;
    shared_ptr<Afn10F1> p_QueryNodeNum_10F1;
    shared_ptr<Afn10F2> p_QueryNodeInfo_10F2;
    shared_ptr<Afn03F16> p_QueryFreq_03F16;
    shared_ptr<AfnF0F40> p_QueryRouterID_F0F40;
    shared_ptr<AfnF0F40> p_QueryChipID_F0F40;
    shared_ptr<AfnF0F41> p_QueryRouterSN_F0F41;
    shared_ptr<Afn10F4> p_QueryWorkSwitch_10F4;

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
    uchar addCntPerTime=5;
    uchar topoCntPerTime=5;
    bool needBuildNet=true;
    bool needPowerOff=true;
    //参数
    uchar initFreq;
    const uchar defaultWorkSwitch=0xC5;//待确认
    char ccoAddr[6];
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
    bool meterIsExist(Address meterAddr);
    void processMsgFromCCO(DvcType dvcType, int dvcId);
    void processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID);
    void processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID);
    void sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame);
    void sendSrcMsg(DvcType dvcType,int dvcId,QByteArray msg);
};

#endif // SCRIPT_FLASHTEST_V3_H
