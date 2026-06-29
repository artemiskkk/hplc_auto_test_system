#ifndef SCRIPT_MODULEINFOMANAGE_SHAANXI_H
#define SCRIPT_MODULEINFOMANAGE_SHAANXI_H

#include <QObject>
#include "TestCase/BuildNetwork_GW.h"
enum Script_ModuleInfoManage_Shaanxi_RunState
{
    TestInit,
    Wait_BuildNetFinish_Whole,
    Wait_QuerySoftwareVersion_Finish,
    Wait_QueryNodeChipInfo_Finish,
    Wait_QueryCcoMode_Finish,
    ScriptSuccess
};
class Script_ModuleInfoManage_Shaanxi: public QObject, public AbstractScript,public DynamicCreate<Script_ModuleInfoManage_Shaanxi>
{
    Q_OBJECT
//    Q_PLUGIN_METADATA(IID "com.topscomm.AbstractScript")
//    Q_INTERFACES(AbstractScript)
public:
    explicit Script_ModuleInfoManage_Shaanxi(QObject *parent = nullptr);
    ~Script_ModuleInfoManage_Shaanxi();
    Script_ModuleInfoManage_Shaanxi_RunState emScriptRunState;
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
    shared_ptr<Afn10F101> p_QuerySoftwareVersion_10F101;
    shared_ptr<Afn10F112> p_QueryNodeChipInfo_10F112;

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

    char ccoSoftwareVersion[3]={0x01,0x01,0x2B};
    char softwareVersion[3]={0x01,0x01,0x43};
    QList<NodeInfoGroup10F101> nodeInfoList_10F101;
    QByteArray nodeChipId=QByteArray::fromHex(QString("ffffffffffffffffffffffffffffffffffffffffffffffff").toLatin1());//////////chushihau
    QList<NodeChipInfo> nodeChipInfoList_10F112;
    ushort index=0;
    uchar num=5;//每次查询数量
    QString compareSoftwareResult;
    QString compareChipResult;
    QString softwareResult;
    QString chipResult;
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
    void getCompareSoftwareResult();
    void getCompareChipResult();
    void processMsgFromCCO(DvcType dvcType, int dvcId);
    void processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID);
    void processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID);
    void sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame);
    void sendSrcMsg(DvcType dvcType,int dvcId,QByteArray msg);
};
#endif // SCRIPT_MODULEINFOMANAGE_SHAANXI_H
