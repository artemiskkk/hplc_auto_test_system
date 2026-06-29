#ifndef SCRIPT_CCO_ADDRMANAGE_NORMAL_H
#define SCRIPT_CCO_ADDRMANAGE_NORMAL_H

#include <QObject>
#include "TestCase/BuildNetwork_GW.h"
enum Script_CCO_AddrManage_RunState
{
    TestInit,
    Wait_BuildNetFinish_Whole,
    Init_SetCcoBaudRate,
    Init_PowerOnCco,
    CcoPowerOnInit,
    Wait_QueryCcoInitAddr_Finish,
    Wait_SetCcoAddr_Finish,
    Wait_CcoReset,
    Wait_QueryCcoNewAddr_Finish,
    Wait_CcoPoweroff,
    Wait_CcoPoweron,
    Wait_QueryCcoAddrAfterPoweroff_Finish,
    Wait_HardInit_Ack,
    Wait_CcoHardInit,
    Wait_QueryCcoAddrAfterHardInit_Finish,
    Wait_ParaInit_Ack,
    Wait_CcoParaInit,
    Wait_QueryCcoAddrAfterParaInit_Finish,
    ScriptComplete
};
class Script_CCO_AddrManage_normal : public QObject, public AbstractScript, public DynamicCreate<Script_CCO_AddrManage_normal>
{
    Q_OBJECT
  //  Q_PLUGIN_METADATA(IID "com.topscomm.AbstractScript")
 //   Q_INTERFACES(AbstractScript)
public:
    explicit Script_CCO_AddrManage_normal(QObject *parent = nullptr);
    ~Script_CCO_AddrManage_normal();

    Script_CCO_AddrManage_RunState emScriptRunState;

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
    shared_ptr<Afn03F4> p_QueryCcoAddr_03F4_Down;
    shared_ptr<Afn05F1> p_SetCcoAddr_05F1_Down;
    shared_ptr<Afn01F1> p_HardInit_01F1_Down;
    shared_ptr<Afn01F2> p_ParaInit_01F2_Down;

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
    QByteArray ccoNewAddr;
    QList<int> idList;
    QList<double> sendParams;
    QByteArray ccoAddrNow;
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

#endif // SCRIPT_CCO_ADDRMANAGE_NORMAL_H
