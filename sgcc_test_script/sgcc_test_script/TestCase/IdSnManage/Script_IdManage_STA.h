#ifndef SCRIPT_IDMANAGE_STA_H
#define SCRIPT_IDMANAGE_STA_H

#include <QObject>
#include "TestCase/BuildNetwork_GW.h"

enum Script_IdManage_STA_RunState
{
    TestInit,
    Wait_BuildNetFinish_Whole,
    Wait_SetStaId_Finish,
    Wait_QueryStaId_Finish,
    ScriptSuccess
};
class Script_IdManage_STA : public QObject, public AbstractScript,public DynamicCreate<Script_IdManage_STA>
{
    Q_OBJECT
//    Q_PLUGIN_METADATA(IID "com.topscomm.AbstractScript")
//    Q_INTERFACES(AbstractScript)
public:
    explicit Script_IdManage_STA(QObject *parent = nullptr);
    ~Script_IdManage_STA();
    Script_IdManage_STA_RunState emScriptRunState;
    shared_ptr<BuildNetwork_GW> p_BuildNetwork_GW;

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
    shared_ptr<Afn10F7> p_QueryStaID_10F7;
    shared_ptr<Afn10F40> p_QueryStaID_10F40;
    shared_ptr<Afn02F1> p_TransmitData_02F1;//抄控器

    shared_ptr<Frame645Helper> p_Frame645Helper;
    shared_ptr<RspsNormal_ReadAddr_0x93> p_MeterAddrResp_93;
    shared_ptr<Rqst_WriteData_0x14> p_WriteId_14;
    shared_ptr<RspsNormal_WriteData_0x94> p_WriteIdNormalResp_94;
    shared_ptr<RspsAbNormal_WriteData_0xD4> p_WriteIdAbNormalResp_D4;

    shared_ptr<FrameOOPHelper> p_FrameOOPHelper;
    shared_ptr<GetResponseNormal> p_GetResponseNormal_ReadAddr;
    shared_ptr<SetRequestNormal> p_SetRequestNormalId;
    shared_ptr<GetResponseNormal> p_GetResponseNormalId;

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
    int idIndex=0;
    QString dstId;
    QString idHeader;
    QString id_0="0708091011121314151617181920212223242526272829303132333435363738394041424344454647484950";
    QString id_1="30313233343536373839404142434445464748495051525354555657585960";
    QString id_2="";
    QString idType="02";
    QStringList idList;
    bool flag02F1=true;
    int currentMeterIndex;    // 当前正在设置的STA索引
    QString currentId;    // 当前正在设置的芯片ID
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
    QByteArray buildWriteIdFrame(uchar* meterAddr, const QByteArray& IdData);
    void processMsgFromCCO(DvcType dvcType, int dvcId);
    void processMsgFromReadCtrlDvc(DvcType dvcType, int dvcId);
    void processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID);
    void processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID);
    void sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame);
    void sendSrcMsg(DvcType dvcType,int dvcId,QByteArray msg);
};
#endif // SCRIPT_IDMANAGE_STA_H
