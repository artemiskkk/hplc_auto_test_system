#ifndef SCRIPT_TRANSPARENTTRANS_PROTOCOLTEST_H
#define SCRIPT_TRANSPARENTTRANS_PROTOCOLTEST_H

#include <QObject>
#include "TestCase/BuildNetwork_GW.h"

enum Script_TransparentTrans_ProtocolTest_RunState
{
    TestInit,
    Wait_BuildNetFinish_Whole,
    Wait_TransmitData02F1_1_Finish,
    Wait_TransmitData02F1_2_Finish,
    Wait_TransmitData02F1_3_Finish,
    Wait_TransmitData02F1_4_Finish,
    Wait_TransmitData02F1_5_Finish,

    ScriptSuccess
};
enum Set_Query_State
{
    Wait_SetDAC_Finish,
    Wait_QueryDAC_Finish,
    Wait_SetFrameDetect_Finish,
    Wait_QueryFrameDetect_Finish,
    Wait_SetSuppress_Finish,
    Wait_QuerySuppress_Finish,
    Wait_SetRelateThreshold_Finish,
    Wait_QueryRelateThreshold_Finish
};
enum ParaType
{
    DAC,
    FrameDetect,
    Suppress,
    RelateThreshold
};

/**
 * @brief The Script_TransparentTrans 透传功能-协议规范性测试【GW-CCO-F006-0003-V01】
 *
 */

class Script_TransparentTrans_ProtocolTest : public QObject, public AbstractScript,public DynamicCreate<Script_TransparentTrans_ProtocolTest>
{
    Q_OBJECT
//    Q_PLUGIN_METADATA(IID "com.topscomm.AbstractScript")
//    Q_INTERFACES(AbstractScript)
public:
    explicit Script_TransparentTrans_ProtocolTest(QObject *parent = nullptr);
    ~Script_TransparentTrans_ProtocolTest();
    Script_TransparentTrans_ProtocolTest_RunState emScriptRunState;
    Set_Query_State emSetQueryState;
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
    shared_ptr<Afn02F1> p_TransmitData_02F1;

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
    ushort timerAfterReachThresld=1800; //单位:s

    double netSucRateThresld=1.0,transSucRateThresld=1.0;
    bool needBuildNet=true;
    bool needPowerOff=true;
    uchar addCntPerTime=5;
    uchar topoCntPerTime=5;

    //设定参数
    char vol=0x04;
    char factor=0x00;
    QString multi="00020080";
    QString threshold="00001081";
    QString suppress="9001c830";
    uchar relateThreshold=80;

    int sucNum=0;
    QString failAddrs;
    bool report_02_f1_flag =false;

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
    QByteArray getSetFrame(ParaType type);
    QByteArray getQueryFrame(ParaType type);
    void processMsgFromCCO(DvcType dvcType, int dvcId);
    void processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID);
    void processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID);
    void sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame);
    void sendSrcMsg(DvcType dvcType,int dvcId,QByteArray msg);
};

#endif // SCRIPT_TRANSPARENTTRANS_PROTOCOLTEST_H
