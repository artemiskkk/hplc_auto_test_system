#ifndef SCRIPT_CHIPIDMANAGE_CCO_H
#define SCRIPT_CHIPIDMANAGE_CCO_H

#include <QObject>
#include "TestCase/BuildNetwork_GW.h"

enum Script_ChipIdManage_CCO_RunState
{
    TestInit,
    Wait_BuildNetFinish_Whole,
    Wait_SetRouterChipID_Finish,
    Wait_ParaInit_Finish,
    Wait_QueryRouterChipID_Finish,
    ScriptSuccess
};
class Script_ChipIdManage_CCO : public QObject, public AbstractScript,public DynamicCreate<Script_ChipIdManage_CCO>
{
    Q_OBJECT
//    Q_PLUGIN_METADATA(IID "com.topscomm.AbstractScript")
//    Q_INTERFACES(AbstractScript)
public:
    explicit Script_ChipIdManage_CCO(QObject *parent = nullptr);
    ~Script_ChipIdManage_CCO();
    Script_ChipIdManage_CCO_RunState emScriptRunState;

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
    shared_ptr<AfnF0F1> p_SetRouterChipID_F0F1;
    shared_ptr<Afn10F112> p_QueryRouterChipID_10F112;
    shared_ptr<Afn10F40> p_QueryRouterChipID_10F40;

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
    //参数
    QMap<QString, QString> m_paraDic;
    int m_cachedFreq = 0;
    int chipIdIndex=0;
    QString dstChipId;
    QString chipIdHeader;
    QString chipId_0="070809101112131415161718192021222324";
    QString chipId_1="05060708091011121314151617181920212223242526272829";
    QString chipId_2="";
    QStringList chipIdList;
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
    void processMsgFromCCO(DvcType dvcType, int dvcId);
    void processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID);
    void processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID);
    QByteArray buildFrameManually(const QByteArray& chipIdData);
    void validateFrame(const QByteArray& frame);
    QByteArray buildFrameManually();
    void sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame);
    void sendSrcMsg(DvcType dvcType,int dvcId,QByteArray msg);
};

#endif // SCRIPT_CHIPIDMANAGE_CCO_H
