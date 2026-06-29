#ifndef SCRIPT_CHIPIDMANAGE_STA_H
#define SCRIPT_CHIPIDMANAGE_STA_H

#include <QObject>
#include "TestCase/BuildNetwork_GW.h"

enum Script_ChipIdManage_STA_RunState
{
    TestInit,
    Wait_BuildNetFinish_Whole,
    Wait_SetStaChipId_Finish,
    Wait_QueryStaChipId_Finish,
    ScriptSuccess
};
class Script_ChipIdManage_STA : public QObject, public AbstractScript,public DynamicCreate<Script_ChipIdManage_STA>
{
    Q_OBJECT
//    Q_PLUGIN_METADATA(IID "com.topscomm.AbstractScript")
//    Q_INTERFACES(AbstractScript)
public:
    explicit Script_ChipIdManage_STA(QObject *parent = nullptr);
    ~Script_ChipIdManage_STA();
    Script_ChipIdManage_STA_RunState emScriptRunState;
    shared_ptr<BuildNetwork_GW> p_BuildNetwork_GW;
	//推送
    ushort concentratorCnt;
    bool resultFlag;
    shared_ptr<QList<shared_ptr<CtrInfo>>> p_CtrInfoList;

    shared_ptr<QTimer> p_timer;
    shared_ptr<QTimer> p_maxAllowTimer;
    shared_ptr<QTimer> p_delayTimer;
    AbstractScriptHost *p_AbstractScriptHost;

    QByteArray sendMsgOct;
    QString sendMsgLog;
    QString logMsgStr;
    uchar addr[6];

    shared_ptr<Frame3762Helper> p_Frame3762Helper;
    shared_ptr<Afn01F2> p_ParaInit_01F2;
    shared_ptr<Afn01F1> p_HardReset_01F1;
    shared_ptr<Afn10F9> p_QueryNetScale_10F9;//查询网络规模
    shared_ptr<Afn10F112> p_QueryStaChipID_10F112;
    shared_ptr<Afn10F40> p_QueryStaChipID_10F40;
    shared_ptr<Afn02F1> p_TransmitData_02F1;//抄控器

    shared_ptr<Frame645Helper> p_Frame645Helper;
    shared_ptr<RspsNormal_ReadAddr_0x93> p_MeterAddrResp_93;
    shared_ptr<Rqst_WriteData_0x14> p_WritechipId_14;
    shared_ptr<RspsNormal_WriteData_0x94> p_WritechipIdNormalResp_94;
    shared_ptr<RspsAbNormal_WriteData_0xD4> p_WritechipIdAbNormalResp_D4;

    shared_ptr<FrameOOPHelper> p_FrameOOPHelper;
    shared_ptr<GetResponseNormal> p_GetResponseNormal_ReadAddr;
    shared_ptr<SetRequestNormal> p_SetRequestNormalChipId;
    shared_ptr<GetResponseNormal> p_GetResponseNormalChipId;

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
    //参数
    QMap<QString, QString> m_paraDic;
    int m_cachedFreq = 0;
    bool flagFind=false;
    int chipIdIndex=0;
    QString dstChipId;
    QString chipIdHeader;
    QString chipId_0="0102030405060708070809101112131415161718192021222324";
    QString chipId_1="0506070809101112131415161718192021222324252627282930";
    QString chipId_2="";
    QStringList chipIdList;
    bool flag02F1=true;
    bool flagBuildNetOver;
    int currentMeterIndex;    // 当前正在设置的STA索引
    QString currentChipId;    // 当前正在设置的芯片ID
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
    QByteArray getSetFrame();
    QByteArray getQueryFrame();
    QByteArray buildWriteChipIdFrame(uchar* meterAddr, const QByteArray& chipIdData);
    void processMsgFromCCO(DvcType dvcType, int dvcId);
    void processMsgFromReadCtrlDvc(DvcType dvcType, int dvcId);
    void processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID);
    void processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID);
    void sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame);
    void sendSrcMsg(DvcType dvcType,int dvcId,QByteArray msg);
};
#endif // SCRIPT_CHIPIDMANAGE_STA_H
