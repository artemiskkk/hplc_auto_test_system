#ifndef SCRIPT_BAUDRATEQUERYANDSETTING_H
#define SCRIPT_BAUDRATEQUERYANDSETTING_H

#include <QObject>
#include "TestCase/BuildNetwork_GW.h"
#include "PublicDataStruct/commdatatype.h"

enum Script_BaudRateQueryAndSetting_RunState
{
    TestInit,
    Wait_BuildNetFinish_Whole,
    Wait_HardReset_Finish,
    Wait_QueryNetScale_Finish,
    Wait_QueryLocalStaRunModelInfo_03F10_Finish,
    Wait_SetBaudRate_11F4_Finish,
    ScriptSuccess
};

class Script_BaudRateQueryAndSetting : public QObject, public AbstractScript, public DynamicCreate<Script_BaudRateQueryAndSetting>
{
    Q_OBJECT
public:
    explicit Script_BaudRateQueryAndSetting(QObject *parent = nullptr);
    ~Script_BaudRateQueryAndSetting();
    Script_BaudRateQueryAndSetting_RunState emScriptRunState;
    BuildNetwork_GW *p_BuildNetwork_GW;

    //**工装参数
    DvcType emDvcType;
    QList<double> sendParams;
    QList<int> idList;


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
    shared_ptr<Afn10F9> p_QueryNetScale_10F9;
    shared_ptr<Afn03F10> p_QureyLocalStaRunModelInfo_03F10;
    shared_ptr<Afn11F4> p_SetBaudRate_11F4;

    shared_ptr<Frame645Helper> p_Frame645Helper;
    shared_ptr<RspsNormal_ReadAddr_0x93> p_MeterAddrResp_93;
    shared_ptr<FrameOOPHelper> p_FrameOOPHelper;
    shared_ptr<GetResponseNormal> p_GetResponseNormal_ReadAddr;

    int tryTimes=0;
    uchar msgSeq=0;
    ushort times=0;
    //QString startTime;
    //QString endTime;
    double havePassedTimeLen=0.0;
    bool haveStartContinueTimer=false;
    QList<QByteArray> addrList;
    /***可配置参数***/
    ushort timerForReachThresld=1200; //单位:s
    ushort timerAfterReachThresld=120; //单位:s

    double netSucRateThresld=1.0;
    bool needBuildNet=true;
    uchar addCntPerTime=5;
    uchar topoCntPerTime=5;
    bool needPowerOff=true;

    ushort netScale;
    const int routerPeriod=10;//单位:s4

    bool bufFlag=true;
    bool queFlag;

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
    QList<int> getDvcIdList(DvcType dvcType);
};

#endif // SCRIPT_BAUDRATEQUERYANDSETTING_H
