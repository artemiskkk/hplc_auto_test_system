#ifndef SCRIPT_PARAMETERMANAGE_REPEATPARAMETER_V3_H
#define SCRIPT_PARAMETERMANAGE_REPEATPARAMETER_V3_H

#include <QObject>
#include "TestCase/BuildNetwork_GW.h"
enum Script_ParameterManage_RepeatParameter_V3_RunState
{
    TestInit,
    Wait_BuildNetFinish_Whole,
    Wait_ParaInit_Finish,
    Wait_QueryNodeNum_Finish,
    Wait_AddSlaveNode_Finish,
    Wait_AddSlaveNode_Repeat_Finish,
    Wait_QueryNodeInfo_Finish,
    ScriptSuccess
};
class Script_ParameterManage_RepeatParameter_V3: public QObject, public AbstractScript,public DynamicCreate<Script_ParameterManage_RepeatParameter_V3>
{
    Q_OBJECT
//    Q_PLUGIN_METADATA(IID "com.topscomm.AbstractScript")
//    Q_INTERFACES(AbstractScript)
public:
    explicit Script_ParameterManage_RepeatParameter_V3(QObject *parent = nullptr);
    ~Script_ParameterManage_RepeatParameter_V3();
    Script_ParameterManage_RepeatParameter_V3_RunState emScriptRunState;
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
    shared_ptr<Afn01F2> p_ParaInit_01F2;
    shared_ptr<Afn10F1> p_QueryNodeNum_10F1;
    shared_ptr<Afn10F2> p_QueryNodeInfo_10F2;
    shared_ptr<Afn11F1> p_AddSlaveNode_11F1;
    shared_ptr<Afn11F2> p_DeleteSlaveNode_11F2;

    shared_ptr<Frame645Helper> p_Frame645Helper;
    shared_ptr<RspsNormal_ReadAddr_0x93> p_MeterAddrResp_93;
    shared_ptr<FrameOOPHelper> p_FrameOOPHelper;
    shared_ptr<GetResponseNormal> p_GetResponseNormal_ReadAddr;

    int tryTimes=0;
    uchar msgSeq=0;
    ushort times=0;
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

    QList<NodeParameter> parameterList;
    QList<NodeInfoGroup> nodeInfoList_10F2;
    ushort index=0;
    uchar num=5;//每次查询数量
    ushort maxNodeNum=500;//从节点数量
    uchar maxParaInitTimes=2;//最大参数初始化次数

    QList<NodeParameter> repeatParameterList;
    ushort repeatNodeNum=12;//重复从节点数量
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
    void generateParameterList();
    void generateRepeatParameterList();
    bool isAllNodeInfoRight();
    void processMsgFromCCO(DvcType dvcType, int dvcId);
    void processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID);
    void processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID);
    void sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame);
    void sendSrcMsg(DvcType dvcType,int dvcId,QByteArray msg);
};
#endif // SCRIPT_PARAMETERMANAGE_REPEATPARAMETER_V3_H
