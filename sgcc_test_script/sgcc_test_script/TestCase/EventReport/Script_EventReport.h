#ifndef SCRIPT_EVENTREPORT_H
#define SCRIPT_EVENTREPORT_H

#include <QObject>
#include "TestCase/BuildNetwork_GW.h"
enum Script_EventReport_RunState
{
    TestInit,
    Wait_BuildNetFinish_Whole,
    Wait_HardwareInit_Finish,
    Wait_EventReport_Finish,
    ScriptSuccess
};
class Script_EventReport: public QObject, public AbstractScript,public DynamicCreate<Script_EventReport>
{
    Q_OBJECT
//    Q_PLUGIN_METADATA(IID "com.topscomm.AbstractScript")
//    Q_INTERFACES(AbstractScript)
public:
    explicit Script_EventReport(QObject *parent = nullptr);
    ~Script_EventReport();
    Script_EventReport_RunState emScriptRunState;
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
    shared_ptr<qgdw_3762_protocol::Afn01F1> p_HardwareInit_01F1;
    shared_ptr<Afn10F9> p_QueryNetScale_10F9;//查询网络规模

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
  //  bool receiveEventFlag;
    ushort recvEventNum=0;
    QList<double> sendParams;
    QList<int> idList;

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
    QList<int> getDvcIdList(DvcType);
};

#endif // SCRIPT_EVENTREPORT_H
