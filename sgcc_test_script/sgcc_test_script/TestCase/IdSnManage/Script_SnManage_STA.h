#ifndef SCRIPT_SNMANAGE_STA_H
#define SCRIPT_SNMANAGE_STA_H

#include <QObject>
#include "TestCase/BuildNetwork_GW.h"

enum Script_SnManage_STA_RunState
{
    TestInit,
    Wait_BuildNetFinish_Whole,
    Wait_SetStaSn_Finish,
    Wait_ParaInit_Finish,
    Wait_QueryStaSn_Finish,
    ScriptSuccess
};
class Script_SnManage_STA : public QObject, public AbstractScript,public DynamicCreate<Script_SnManage_STA>
{
    Q_OBJECT
//    Q_PLUGIN_METADATA(IID "com.topscomm.AbstractScript")
//    Q_INTERFACES(AbstractScript)
public:
    explicit Script_SnManage_STA(QObject *parent = nullptr);
    ~Script_SnManage_STA();
    Script_SnManage_STA_RunState emScriptRunState;
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
    shared_ptr<AfnF0F41> p_QueryStaSn_F0F41;
//    shared_ptr<AfnF0F42> p_SetStaSn_F0F42;
    shared_ptr<Afn02F1> p_TransmitData_02F1;//抄控器

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
    int snIndex=0;
    QString dstSn;
    QString snHeader;
    QString sn_0="0708090A0B0C0D0E0F10111213";
    QString sn_1="0708090A0B0C0D0E0F1011121314";
    QString sn_2="";
    QStringList snList;
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
    void processMsgFromCCO(DvcType dvcType, int dvcId);
    void processMsgFromReadCtrlDvc(DvcType dvcType, int dvcId);
    void processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID);
    void processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID);
    void sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame);
    void sendSrcMsg(DvcType dvcType,int dvcId,QByteArray msg);
};
#endif // SCRIPT_SNMANAGE_STA_H
