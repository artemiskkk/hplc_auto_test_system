#ifndef SCRIPT_SUPERPOWERPARAMETERCONFIG_CCO_H
#define SCRIPT_SUPERPOWERPARAMETERCONFIG_CCO_H
#include <QObject>
#include "TestCase/BuildNetwork_GW.h"

enum Script_SuperpowerParameterConfig_CCO_RunState
{
    TestInit,
    Wait_BuildNetFinish_Whole,

    Wait_QueryPowerPara_F0F7_Finish,
    Wait_ConfigPowerPara_F0F8_Finish,
    Wait_QueryPowerPara_F0F7_2_Finish,
    Wait_ConfigPowerPara_F0F8_2_Finish,
    Wait_QueryPowerPara_F0F7_3_Finish,

    ScriptSuccess
};

enum SetPowerParameter_F0F8_State
{
    Normal_F0F8,
    Superpower_F0F8
};

class Script_SuperpowerParameterConfig_CCO : public QObject, public AbstractScript,public DynamicCreate<Script_SuperpowerParameterConfig_CCO>
{
    Q_OBJECT
public:
    explicit Script_SuperpowerParameterConfig_CCO(QObject *parent = nullptr);
    ~Script_SuperpowerParameterConfig_CCO();

    Script_SuperpowerParameterConfig_CCO_RunState emScriptRunState;
    SetPowerParameter_F0F8_State emSetPowerParameter_F0F8_State;
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
    shared_ptr<Afn01F1> p_HardReset_01F1;

    shared_ptr<AfnF0F7> p_QueryPowerPara_F0F7;
    shared_ptr<AfnF0F8> p_ConfigPowerPara_F0F8;

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
    ushort timerForReachThresld=1800; //单位:s
    ushort timerAfterReachThresld=120; //单位:s
    double netSucRateThresld=1.0;
    bool needBuildNet=true;
    bool needPowerOff=true;

    uchar addCntPerTime=5;
    uchar topoCntPerTime=5;

public:
    void  execute();
    void  stop();
    void  addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq);
    void  setHost(AbstractScriptHost *host);
    bool  config(const QMap<QString,QString> *paraDic);
    void  processMsg(DvcType dvcType,int id,uchar* data,int datalen);
    void  processCtrlDvcRes(DvcType dvcType,QList<int> idList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params);

signals:

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

#endif // SCRIPT_SUPERPOWERPARAMETERCONFIG_CCO_H
