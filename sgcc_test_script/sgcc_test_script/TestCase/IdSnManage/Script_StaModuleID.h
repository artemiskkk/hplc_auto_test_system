#ifndef SCRIPT_STAMODULEID_H
#define SCRIPT_STAMODULEID_H

#include <QObject>
#include "TestCase/BuildNetwork_GW.h"
enum Script_StaModuleID_RunState
{
    ScriptInit,
    Wait_BuildNetFinish_Whole,
    Wait_QueryModuleIDInit_F0F40_Finish,
    Wait_SetModuleID_50Bytes_F0F39_Finish,
    Wait_QueryModuleID_50Bytes_F0F40_Finish,
    Wait_SetModuleID_51Bytes_F0F39_Finish,
    Wait_QueryModuleID_51Bytes_F0F40_Finish,
    Wait_SetModuleID_49Bytes_F0F39_Finish,
    Wait_QueryModuleID_49Bytes_F0F40_Finish,
    Wait_SetModuleID_0Bytes_F0F39_Finish,
    Wait_QueryModuleID_0Bytes_F0F40_Finish,
    Wait_QueryModuleIDInit_02F1_Finish,
    Wait_SetModuleID_02F1_Finish,
    Wait_HardReset_01F1_Finish,
    Wait_QueryModuleID_10F40_Finish,
    ScriptSuccess
};

enum Module_Id_Length
{
    len_50Bytes,
    len_51Bytes,
    len_49Bytes,
    len_0Bytes
};
class Script_StaModuleID : public QObject , public AbstractScript, public DynamicCreate<Script_StaModuleID>
{
    Q_OBJECT
    //    Q_PLUGIN_METADATA(IID "com.topscomm.AbstractScript")
    //    Q_INTERFACES(AbstractScript)
public:
    explicit Script_StaModuleID(QObject *parent = nullptr);
    ~Script_StaModuleID();
    Script_StaModuleID_RunState emScriptRunState;
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
    shared_ptr<Afn10F9> p_QueryNetScale_10F9;//查询网络规模
    shared_ptr<Afn10F7> p_QueryStaModuleID_10F7;
    shared_ptr<Afn10F40> p_QueryStaModuleID_10F40;
    shared_ptr<Afn02F1> p_TransmitData_02F1;//抄控器
    shared_ptr<AfnF0F39> p_SetModuleID_F0F39;
    shared_ptr<AfnF0F40> p_QueryModuleID_F0F40;




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

    //新参数
    bool flagFind=false;
    QString moduleId_50Bytes_="";
    QString moduleId_51Bytes_="";
    QString moduleId_49Bytes_="";
    QString moduleId_0Bytes_="";
    QString moduleId_02F1Send_="";
    QString test_name_ = "【GW-STA-F010-0001-V01】STA模块ID管理-常规;【GW-STA-F010-0002-V01】STA模块ID管理-ID长度测试;【GW-STA-F010-0003-V01】STA模块ID管理-透传（组网）";
    QString test_result = "状态，结果，目标值，实际值，【用例编号1】【用例编号n】";
    bool is_moduleid_10F40_ =true;
    bool is_hardReset_01F1_ =true;
    Module_Id_Length id_length;
    bool flagBuildNetOver;

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
#endif // SCRIPT_STAMODULEID_H
