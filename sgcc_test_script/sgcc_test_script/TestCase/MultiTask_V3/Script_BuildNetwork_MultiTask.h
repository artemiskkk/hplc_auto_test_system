#ifndef SCRIPT_BUILDNETWORK_MULTITASK_H
#define SCRIPT_BUILDNETWORK_MULTITASK_H

#include <QObject>
#include "TestCase/BuildNetwork_GW.h"
enum Script_BuildNetwork_MultiTask_RunState
{
    TestInit,
    Wait_BuildNetFinish_Whole,
    Wait_MultiTask_Finish,
    ScriptSuccess
};
enum ReadFlag
{
    ReadFail,
    ReadSuccess,
    Reading
};
struct ReadDataUnit
{
    QByteArray dataID;
    bool notRead;
    double costTime;
};
struct ReadInfo_13F1
{
    Address meterNo;
    uchar protocolType;
 //   uchar phase;
    ReadFlag readFlag;
    QList<ReadDataUnit> dataUnitList;
};
class Script_BuildNetwork_MultiTask : public QObject, public AbstractScript,public DynamicCreate<Script_BuildNetwork_MultiTask>
{
    Q_OBJECT
//    Q_PLUGIN_METADATA(IID "com.topscomm.AbstractScript")
//    Q_INTERFACES(AbstractScript)
public:
    explicit Script_BuildNetwork_MultiTask(QObject *parent = nullptr);
    ~Script_BuildNetwork_MultiTask();
    Script_BuildNetwork_MultiTask_RunState emScriptRunState;
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
    shared_ptr<Afn11F5> p_ActiveRegister_11F5;
    shared_ptr<Afn00F1> p_Confirm_00F1;
    shared_ptr<Afn13F1> p_MonitorSlaveNode_13F1;
    shared_ptr<Afn10F9> p_QueryNetScale_10F9;

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
    ushort activeTime=2;
 //   bool activeFlag=false;
 //   bool reportFlag=false;
//    struct NodeInfo_Struct
//    {
//        Address nodeAddress;
//        char protocol;
//        char deviceType;
//        bool isExist=false;
//    };
//    QList<NodeInfo_Struct> nodeInfoList;
//    int num_reportOutOfRecord;
      QList<ReadInfo_13F1> readInfoList;
      ushort readNo_13F1,readNum_13F1=2;
      ushort searchNum=0;
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
    void readInfoInit();
    void processMsgFromCCO(DvcType dvcType, int dvcId);
    void processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID);
    void processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID);
    void sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame);
    void sendSrcMsg(DvcType dvcType,int dvcId,QByteArray msg);

};

#endif // SCRIPT_BUILDNETWORK_MULTITASK_H
