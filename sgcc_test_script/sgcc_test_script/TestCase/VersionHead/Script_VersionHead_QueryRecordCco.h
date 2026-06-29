#ifndef SCRIPT_VERSIONHEAD_QUERYRECORDCCO_H
#define SCRIPT_VERSIONHEAD_QUERYRECORDCCO_H

#include <QObject>
#include "TestCase/BuildNetwork_GW.h"
enum Script_VersionHead_QueryRecordCco_RunState
{
    TestInit,
    Wait_BuildNetFinish_Whole,
    ReadRecordBefore,
    ReadRecordAfterParainit,
    ReadRecordAfterHardinit,
    ScriptSuccess
};
class Script_VersionHead_QueryRecordCco: public QObject, public AbstractScript,public DynamicCreate<Script_VersionHead_QueryRecordCco>
{
    Q_OBJECT
//    Q_PLUGIN_METADATA(IID "com.topscomm.AbstractScript")
//    Q_INTERFACES(AbstractScript)
public:
    explicit Script_VersionHead_QueryRecordCco(QObject *parent = nullptr);
    ~Script_VersionHead_QueryRecordCco();
    Script_VersionHead_QueryRecordCco_RunState emScriptRunState;
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
    QString eventMsgList;

    shared_ptr<Frame3762Helper> p_Frame3762Helper;
    shared_ptr<Afn00F1> p_Confirm_00F1;

    shared_ptr<AfnF0F7> p_QueryRecordF0F7_Down;
    shared_ptr<Afn01F1> p_HardInit_01F1_Down;
    shared_ptr<Afn01F2> p_ParaInit_01F2_Down;

    shared_ptr<Frame645Helper> p_Frame645Helper;
    shared_ptr<RspsNormal_ReadAddr_0x93> p_MeterAddrResp_93;
    shared_ptr<FrameOOPHelper> p_FrameOOPHelper;
    shared_ptr<GetResponseNormal> p_GetResponseNormal_ReadAddr;

    uchar dtValue3762;
    QByteArray ccoRecord_init;
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
    bool receiveEventFlag;

    struct PowerEvent_Struct
    {
        char eventType;
        Address reportNodeAddress;
    };
    QList<PowerEvent_Struct> powerOnReportList;
    QList<PowerEvent_Struct> powerOffReportList;
    QList<PowerEvent_Struct> otherEventReportList;
    bool firstPowerOnReport=true;
    const int maxWaitReportTime=120;
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
    void calPowerOnReportRate();
    void processMsgFromCCO(DvcType dvcType, int dvcId);
    void processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID);
    void processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID);
    void sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame);
    void sendSrcMsg(DvcType dvcType,int dvcId,QByteArray msg);
};

#endif // SCRIPT_VERSIONHEAD_QUERYRECORDCCO_H
