#ifndef SCRIPT_SOFTWARERECORD_CCO_HUNAN_H
#define SCRIPT_SOFTWARERECORD_CCO_HUNAN_H

#include <QObject>
#include <QCryptographicHash>
#include "TestCase/BuildNetwork_GW.h"

enum Script_SoftwareRecord_CCO_Hunan_RunState
{
    TestInit,
    Wait_BuildNetFinish_Whole,
    Wait_QuerySoftWareInfo_03F130_Finish,
    Wait_QuerySoftWareCompare_03F131_Finish,
    ScriptSuccess
};
class Script_SoftwareRecord_CCO_Hunan : public QObject, public AbstractScript,public DynamicCreate<Script_SoftwareRecord_CCO_Hunan>
{
    Q_OBJECT
//    Q_PLUGIN_METADATA(IID "com.topscomm.AbstractScript")
//    Q_INTERFACES(AbstractScript)
public:
    explicit Script_SoftwareRecord_CCO_Hunan(QObject *parent = nullptr);
    ~Script_SoftwareRecord_CCO_Hunan();

    Script_SoftwareRecord_CCO_Hunan_RunState emScriptRunState;

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
    shared_ptr<Afn03F130> p_QuerySoftWareInfo_03F130;
    shared_ptr<Afn03F131> p_QuerySoftWareCompare_03F131;

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
    DataUnitUp03F130 softwareInfoData;
    struct ProgramCompareStruct
    {
        char startAddress[4];
        QByteArray programSegment;
        QByteArray compareData;
    };
    QList<ProgramCompareStruct> programCompareList;
    QByteArray originProgram;
    int segmentLen=512;
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
    void softwareInfoInit();
    void processMsgFromCCO(DvcType dvcType, int dvcId);
    void processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID);
    void processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID);
    void sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame);
    void sendSrcMsg(DvcType dvcType,int dvcId,QByteArray msg);
};

#endif // SCRIPT_SOFTWARERECORD_CCO_HUNAN_H
