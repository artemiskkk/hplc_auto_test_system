#ifndef SCRIPT_CCOHARDWAREINIT_STAUPDATE_H
#define SCRIPT_CCOHARDWAREINIT_STAUPDATE_H

#include <QObject>
#include "TestCase/BuildNetwork_GW.h"

const ushort SEG_LEN_STA = 512;
enum Script_Upgrade_STA_RunState
{
    ScriptInit,
    Wait_BuildNetFinish_Whole,
    Wait_QueryRouterRunState_10F4,
    Wait_BeforeQueryInVrsnInfo02F1Finish,
    Wait_BeforeQueryOutVrsnInfo10F104Finish,
    Wait_00F1_for_11F2_DeleteNode,//删除不需要升级的节点，当前主要是其它厂商STA
    Wait_00F1_for_12F2_UpgrdSta,
    Wait_00F1_for_11F6_UpgrdSta,
    Wait_15F1_for_15F1_BeforeUpgrdSta,
    Wait_Res_for_10F4_BeforeUpgrdSta,
    Wait_FileTransferFinish,
    Wait_Res_for_03F10_WaitTimeLen,
    Wait_StaUpgradeFinish,
    Wait_CcoHardWareInit_01F1_Finish,
    Wait_QueryNetSize_10F9_Finish,
    Wait_QueryOutVrsnInfo10F104Finish,
    Wait_QueryInVrsnInfo02F1Finish,
    ScriptSuccess
};

class Script_CcoHardWareInit_StaUpdate : public QObject, public AbstractScript, public DynamicCreate<Script_CcoHardWareInit_StaUpdate>
{
    Q_OBJECT
public:
    explicit Script_CcoHardWareInit_StaUpdate(QObject *parent = nullptr);
    ~Script_CcoHardWareInit_StaUpdate();
    Script_Upgrade_STA_RunState emScriptRunState;

    ushort concentratorCnt;
    shared_ptr<QList<shared_ptr<CtrInfo>>> p_CtrInfoList;

    shared_ptr<QTimer> p_timer;
    shared_ptr<QTimer> p_maxAllowTimer;
    shared_ptr<QTimer> p_send10F4Timer;
    AbstractScriptHost *p_AbstractScriptHost;
    QTimer *p_delayTimer;//命令延迟定时器
    QStringList in_version_record;
    bool first_query_inversion = true ;
    QStringList out_version_record;
    bool first_query_outversion = true ;


    shared_ptr<BuildNetwork_GW> p_BuildNetwork;
    shared_ptr<qgdw_3762_protocol::Frame3762Helper> p_MsgBase_1376_2;
    shared_ptr<qgdw_3762_protocol::Afn11F2> p_DeleteNode_11F2_Down;
    shared_ptr<qgdw_3762_protocol::Afn15F1> p_FileTransfer_15F1_Down;
    shared_ptr<qgdw_3762_protocol::Afn10F4> p_CcoRunStateInfo_10F4_Down;
    shared_ptr<qgdw_3762_protocol::Afn03F10> p_CcoRunModeInfo_03F10_Down;
    shared_ptr<qgdw_3762_protocol::Afn12F2> p_CcoCtrlPause_12F2;
    shared_ptr<qgdw_3762_protocol::Afn11F6> p_StopSlaveNodeReg_11F6;
    shared_ptr<qgdw_3762_protocol::Afn02F1> p_ChkStaInVrsnInfo_02F1_Down;
    shared_ptr<qgdw_3762_protocol::Afn10F104> p_ChkStaOutVrsnInfo_10F104_Down;
    shared_ptr<Afn01F1> p_HardReset_01F1;
    shared_ptr<Afn10F9> p_QueryNetSize_10F9;
    QByteArray sendMsgOct;

    shared_ptr<dlt_645_Protocol::Frame645Helper> p_MsgBase_645;
    shared_ptr<dlt_645_Protocol::RspsNormal_ReadAddr_0x93> p_MeterAddrResp_93;
    shared_ptr<object_oriented_electic_data_exchange_protocol::FrameOOPHelper> p_MsgBase_698_45;
    shared_ptr<GetResponseNormal> p_GetResponseNormal_ReadAddr;

    uchar addr[6];

    int dataLen;    //升级包长度，单位:字节
    ushort totalSegs;   //总段数
    QByteArray rawUpdateFile;
    QList<bool> transResList;
    ushort cfgCntPerTime;
    ushort fileIndex,meterIndex_02F1,meterIndex_10F104;
    uchar msgSeq;
    ushort sendTimes;

    double havePassedTimeLen;
    //   bool haveStartContinueTimer;
    /***可配置参数***/
    ushort timerForReachThresld; //单位:s
    double netSucRateThresld;
    ushort timerAfterReachThresld; //单位:s

    ushort timerForReachThresld_Upgrade; //单位:s
    double sucRateThresld_Upgrade;
    ushort timerAfterTransferFinished; //单位:s

    ushort timerForReachThresld_QueryNodeVrsnInfo02F1; //单位:s
    double sucRateThresld_QueryNodeVrsnInfo02F1;
    ushort timerAfterReachThresld_QueryNodeVrsnInfo02F1; //单位:s

    bool needBuildNet;
    uchar dstUpgradeDvc;
    bool isStdPrcs=true;
    QString staOutVrsn,staInVrsn;
    QString staVendorChipCode;

    QStringList failList_OutVrsn;
    QStringList deleteNodeList;
    QString failAddrs_OutVrsn,failAddrs_InVrsn;
    ushort failCnt_InVrsn;
    QStringList versionsV2=QStringList()<<"0202"<<"0002"<<"0205"<<"0207"<<"0209"<<"0210"
                                           <<"4302"<<"0800"<<"2205"<<"2111"<<"2714"<<"2101"<<"1100"<<"0610";//湖南、湖北、陕西、陕西、重庆、山东、黑龙江、河南
    QStringList versionsV3=QStringList()<<"0320"<<"4303"<<"2301"<<"0810"<<"2721"<<"2731"<<"2302"<<"0002";
    QStringList versionsV3B=QStringList()<<"0360"<<"2302";
    QStringList topscommChipCode=QStringList()<<"TC"<<"DX";
    QString test_name_="【GW-CCO-F014-0007-V01】硬件初始化-在线升级sta";

    // 重传机制相关变量
    QMap<ushort, ushort> segmentRetryCount;  // 记录每段重传次数
    QList<ushort> failedSegments;             // 记录失败的段
    const ushort MAX_RETRY_TIMES = 5;         // 最大重传次数
    bool isRetransmitting = false;            // 标记是否正在重传

public:
    void  execute();
    void  stop();
    void  addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq);
    void  setHost(AbstractScriptHost *host);
    bool  config(const QMap<QString,QString> *paraDic);
    void  processMsg(DvcType dvcType,int id,uchar* data,int datalen);
    void  processCtrlDvcRes(DvcType dvcType,QList<int> idList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params);

private:
    void LoadUpdateFile();
    void getVendorChipCode();
    void Refresh_TestResult_15F1(shared_ptr<Afn15F1> p_FileTransfer_15F1_Up);
    ushort Refresh_SuccessCnt_15F1();
    void Check_StaOutVrsnInfo(shared_ptr<Afn10F104> p_ChkStaOutVrsnInfo_10F104_Up);
    void Refresh_TestResult_02F1(shared_ptr<Afn02F1> p_ChkNodeVrsnInfo_02F1_Up);
    void Refresh_SuccessCnt_02F1();
    void RetransmitFailedSegments();

    void processMsgFromCco(int id);
    void processMsgFromMeter645(DvcType dvcType,int id, int mtrlID);
    void processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID);
    void sendMsg(DvcType dvcType,int id,int meterID, shared_ptr<void> frame);
    void sendMsg(DvcType dvcType,int id,QByteArray msg);

public slots:
    void timer_timeoutProc();
    void timer_send10F4TimeoutProc();
    void maxAllowTimer_timeoutProc();
    void delayTimer_timeoutProc();//命令延迟定时器槽函数
};

#endif // SCRIPT_CCOHARDWAREINIT_STAUPDATE_H
