#ifndef SCRIPT_CCOPARAINIT_STAUPDATE_H
#define SCRIPT_CCOPARAINIT_STAUPDATE_H

#include <QObject>
#include "TestCase/BuildNetwork_GW.h"

const ushort SEG_LEN_STA = 512;
enum Script_Upgrade_STA_RunState
{
    ScriptInit,
    Wait_BuildNetFinish_Whole,
    Wait_QueryRouterRunState_10F4,
    Wait_00F1_for_12F2_UpgrdSta,
    Wait_00F1_for_11F6_UpgrdSta,
    Wait_15F1_for_15F1_BeforeUpgrdSta,
    Wait_Res_for_10F4_BeforeUpgrdSta,
    Wait_FileTransferFinish,
    Wait_Res_for_03F10_WaitTimeLen,
    Wait_StaUpgradeFinish,
    Wait_CcoParaInit_01F2_Finish,
    ScriptSuccess
};

class Script_CcoParaInit_StaUpdate : public QObject, public AbstractScript, public DynamicCreate<Script_CcoParaInit_StaUpdate>
{
    Q_OBJECT
public:
    explicit Script_CcoParaInit_StaUpdate(QObject *parent = nullptr);
    ~Script_CcoParaInit_StaUpdate();

    Script_Upgrade_STA_RunState emScriptRunState;

    ushort concentratorCnt;
    shared_ptr<QList<shared_ptr<CtrInfo>>> p_CtrInfoList;

    shared_ptr<QTimer> p_timer;
    shared_ptr<QTimer> p_maxAllowTimer;
    shared_ptr<QTimer> p_send10F4Timer;
    AbstractScriptHost *p_AbstractScriptHost;
    QTimer *p_delayTimer;//命令延迟定时器

    shared_ptr<BuildNetwork_GW> p_BuildNetwork;
    shared_ptr<qgdw_3762_protocol::Frame3762Helper> p_MsgBase_1376_2;
    shared_ptr<qgdw_3762_protocol::Afn15F1> p_FileTransfer_15F1_Down;
    shared_ptr<qgdw_3762_protocol::Afn10F4> p_CcoRunStateInfo_10F4_Down;
    shared_ptr<qgdw_3762_protocol::Afn03F10> p_CcoRunModeInfo_03F10_Down;
    shared_ptr<qgdw_3762_protocol::Afn12F2> p_CcoCtrlPause_12F2;
    shared_ptr<qgdw_3762_protocol::Afn11F6> p_StopSlaveNodeReg_11F6;
    shared_ptr<Afn01F2> p_ParameterInit_01F2;
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
    ushort fileIndex;
    uchar msgSeq;
    ushort sendTimes;


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
    QString test_name_="【GW-CCO-F014-0013-V01】参数初始化-在线升级sta";

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

#endif // SCRIPT_CCOPARAINIT_STAUPDATE_H
