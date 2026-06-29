#ifndef SCRIPT_QUERYNETWORKNODEINFO_F0F103_SHANXI_H
#define SCRIPT_QUERYNETWORKNODEINFO_F0F103_SHANXI_H

#include <QObject>
#include "TestCase/BuildNetwork_GW.h"

enum Script_QueryNetworkNodeInfo_F0F102_RunState
{
    TestInit,
    Wait_BuildNetFinish_Whole,

    Wait_HardReset_Finish,
    Wait_ParaInit_Finish,
    Wait_QueryNetScale_10F9_Finish,
    Wait_QueryNetScale_2_10F9_Finish,

    Wait_QueryNetworkNodeInfo_F0F103_Finish,
    Wait_QueryNetworkNodeInfo_F0F103_20_Finish,
    Wait_QueryNetworkNodeInfo_F0F103_40_Finish,
    Wait_QueryNetworkNodeInfo_F0F103_64_Finish,

    ScriptSuccess
};

enum SetNodeStartNo_State
{
    NodeStartNo_1,
    NodeStartNo_0,
    NodeStartNo_Not_0_or_1
};

class Script_QueryNetworkNodeInfo_F0F103_Shanxi : public QObject, public AbstractScript,public DynamicCreate<Script_QueryNetworkNodeInfo_F0F103_Shanxi>
{
    Q_OBJECT
public:
    explicit Script_QueryNetworkNodeInfo_F0F103_Shanxi(QObject *parent = nullptr);
    ~Script_QueryNetworkNodeInfo_F0F103_Shanxi();

    Script_QueryNetworkNodeInfo_F0F102_RunState emScriptRunState;
    SetNodeStartNo_State emSetNodeStartNo_State;
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

    shared_ptr<Afn01F2> p_ParaInit_01F2;
    shared_ptr<Afn10F9> p_QueryNetScale_10F9;
    shared_ptr<Afn10F1>  p_QueryNodeNum_10F1;
//    shared_ptr<Afn06F10> p_ChangeNodeState_06F10;
    shared_ptr<Afn11F1> p_AddSlaveNode_11F1;
    shared_ptr<AfnF0F103> p_QueryNetworkNodeInfo_F0F103;
    shared_ptr<Afn00F1> p_Confirm_00F1;

    shared_ptr<Frame645Helper> p_Frame645Helper;
    shared_ptr<RspsNormal_ReadAddr_0x93> p_MeterAddrResp_93;
    shared_ptr<FrameOOPHelper> p_FrameOOPHelper;
    shared_ptr<GetResponseNormal> p_GetResponseNormal_ReadAddr;

    int tryTimes=0;
    uchar msgSeq=0;
    ushort times=0;
//    ushort index=0;
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

    //参数
    int report_node_state_num = 0;//上报的节点状态数
    ushort netScale;
    const int routerPeriod=10;//单位:s
    int index=0;
    int num=5;

    uchar everytime_query_network_node_info_num = 10; //每次查询网络节点信息个数
    ushort node_start_number = 1;   //节点起始序号
//    int success_report_network_node_info_num = 0;   //成功上报网络节点信息个数
//    ushort first_query_num = 1;    //首次查询时的节点起始号：0或1
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
#endif // SCRIPT_QUERYNETWORKNODEINFO_F0F103_SHANXI_H
