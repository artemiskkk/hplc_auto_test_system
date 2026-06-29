#ifndef SCRIPT_STAHIGHPOWERCONFIG_NORMALPARACONFIG_F0F21_H
#define SCRIPT_STAHIGHPOWERCONFIG_NORMALPARACONFIG_F0F21_H

#include <QObject>
#include "TestCase/BuildNetwork_GW.h"

enum Script_STAHighPowerConfig_NormalParaConfig_F0F21_RunState
{
    TestInit,
    Wait_BuildNetFinish_Whole,

    Wait_QueryNetScale_10F9_Finish,
    Wait_TransparentTransmit_02F1_Finish,

    Wait_SetSuperpowerPara_1010_Finish,
    Wait_QuerySuperpowerPara_1010_Finish,
    Wait_QuerySuperpowerPara_F0F22_1010_Finish,

    ScriptSuccess
};

enum SetConfigSuperpowerParaType_F0F21_State
{
    Wait_Config_1010_Finish
};
enum SetConfigSuperpowerParaType_02F1_State
{
    Wait_Config_0B0B_Finish
};

enum ParaType
{
    DAC,
    FrameDetect,
    Suppress,
    RelateThreshold
};
enum Set_Query_02F1_State
{
    Query_02F1,
    Set_02F1
};

class Script_STAHighPowerConfig_NormalParaConfig_F0F21 : public QObject, public AbstractScript,public DynamicCreate<Script_STAHighPowerConfig_NormalParaConfig_F0F21>
{
    Q_OBJECT
public:
    explicit Script_STAHighPowerConfig_NormalParaConfig_F0F21(QObject *parent = nullptr);
    ~Script_STAHighPowerConfig_NormalParaConfig_F0F21();

    Script_STAHighPowerConfig_NormalParaConfig_F0F21_RunState emScriptRunState;
    SetConfigSuperpowerParaType_F0F21_State emConfigSuperpowerParaState_F0F21;
    SetConfigSuperpowerParaType_02F1_State emConfigSuperpowerParaState_02F1;
    Set_Query_02F1_State em02F1_State;
//    SetTable_type emSetTable;
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
//    shared_ptr<Afn01F1> p_HardReset_01F1;

    shared_ptr<Afn10F9> p_QueryNetScale_10F9;
    shared_ptr<Afn02F1> p_TransparentTransmit_02F1;
//    shared_ptr<Afn00F2> p_Afn00F2;
    shared_ptr<AfnF0F21> p_ConfigSuperpowerPara_F0F21;
//    shared_ptr<AfnF0F21> p_ConfigSuperpowerPara_F0F21;
    shared_ptr<AfnF0F22> p_QueryConfigSuperpowerPara_F0F22;

    shared_ptr<Frame645Helper> p_Frame645Helper;
    shared_ptr<RspsNormal_ReadAddr_0x93> p_MeterAddrResp_93;
    shared_ptr<FrameOOPHelper> p_FrameOOPHelper;
    shared_ptr<GetResponseNormal> p_GetResponseNormal_ReadAddr;

    int tryTimes=0;
    uchar msgSeq=0;
    ushort times=0;
    ushort index=0;

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

    struct NodeInfo_Struct
    {
        Address nodeAddress;
//        char protocol;
        char deviceType;
        bool isSuccess=false;

        char dac_voltage;
        char peak_clipping_factor;
    };
    QList<NodeInfo_Struct> nodeInfoList;
    QList<NodeInfo_Struct> nodeParaList;

    QString test_name_="【GW-STA-F021-0002-V01】大功率配置-常规测试（F0F21）";
    //设定参数
    char vol=0x04;
    char factor=0x00;
    QString multi="00020080";
    QString threshold="00001081";
    QString suppress="9001c830";
    uchar relateThreshold=80;
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
    //bool isAllExist();
    QByteArray getSetFrame(ParaType type);
    QByteArray getQueryFrame(ParaType type);
    void processMsgFromCCO(DvcType dvcType, int dvcId);
    void processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID);
    void processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID);
    void sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame);
    void sendSrcMsg(DvcType dvcType,int dvcId,QByteArray msg);

};

#endif // SCRIPT_STAHIGHPOWERCONFIG_NORMALPARACONFIG_F0F21_H
