#ifndef SCRIPT_NETWORKOPTIMIZE_ZHEJIANG_H
#define SCRIPT_NETWORKOPTIMIZE_ZHEJIANG_H

#include <QObject>
#include "TestCase/TestCase_ZheJiang/BuildNetwork/BuildNetwork_Zhejiang.h"
enum Script_NetworkOptimize_Zhejiang_RunState
{
    TestInit,
    Wait_BuildNetFinish_Whole,
    Wait_SetNetPara_05F30_Finish,
    Wait_QueryNetPara_03F30_Finish,
    Wait_EraseFlash_F0F12_Finish,
    Wait_QueryDefaultNetPara_03F30_Finish,
    ScriptSuccess
};
class Script_NetworkOptimize_Zhejiang : public QObject, public AbstractScript,public DynamicCreate<Script_NetworkOptimize_Zhejiang>
{
    Q_OBJECT
//    Q_PLUGIN_METADATA(IID "com.topscomm.AbstractScript")
//    Q_INTERFACES(AbstractScript)
public:
    explicit Script_NetworkOptimize_Zhejiang(QObject *parent = nullptr);
    ~Script_NetworkOptimize_Zhejiang();
    Script_NetworkOptimize_Zhejiang_RunState emScriptRunState;
    shared_ptr<BuildNetwork_Zhejiang> p_BuildNetwork_Zhejiang;

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
    shared_ptr<Afn03F30_Zhejiang> p_QueryNetPara_03F30;
    shared_ptr<Afn05F30_Zhejiang> p_SetNetPara_05F30;
    shared_ptr<Afn10F21> p_QueryNetTopo_10F21;
    shared_ptr<AfnF0F12> p_EraseFlash_F0F12;
    shared_ptr<Afn00F1> p_Confirm_00F1;

    shared_ptr<Frame645Helper> p_Frame645Helper;
    shared_ptr<RspsNormal_ReadAddr_0x93> p_MeterAddrResp_93;
    shared_ptr<FrameOOPHelper> p_FrameOOPHelper;
    shared_ptr<GetResponseNormal> p_GetResponseNormal_ReadAddr;

    int tryTimes=0;
    uchar msgSeq=0;
    ushort times=0;
    ushort index=0;
    //QString startTime;
    //QString endTime;
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
    ushort onlineLockTime=5;//min
    ushort leaveLockTime=2;//min
    ushort defaultOnlineLockTime=360;//min
    ushort defaultLeaveLockTime=30;//min
    int waitTime=1;//min
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
    void statisticsNode();
    void processMsgFromCCO(DvcType dvcType, int dvcId);
    void processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID);
    void processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID);
    void sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame);
    void sendSrcMsg(DvcType dvcType,int dvcId,QByteArray msg);

};
#endif // SCRIPT_NETWORKOPTIMIZE_ZHEJIANG_H
