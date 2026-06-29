#ifndef SCRIPT_FREQMANAGE_FREQANDFREQDIVIDESWITCH_H
#define SCRIPT_FREQMANAGE_FREQANDFREQDIVIDESWITCH_H

#include <QObject>
#include "TestCase/BuildNetwork_GW.h"

enum Script_FreqManage_FreqDivideSwitch_RunState
{
    TestInit,
    Wait_BuildNetFinish_Whole,
    Wait_SetInitFreq_Finish,
    Wait_SetInitFreqDivide_Finish,
    Wait_QueryInitFreq_Finish,
    Wait_QueryInitFreqDivide_Finish,
    Wait_SetDstFreq0_Finish,
    Wait_SetDstFreq1_Finish,
    Wait_SetDstFreq2_Finish, //**连接初始化频段与分频系数
    Wait_SetDstFreq2_ab_Finish, //**处理回复否认为异常
    //Wait_SetDstFreq3_Finish,
    Wait_SetDstFreqDivide1_Finish,
    Wait_SetDstFreqDivide2_Finish, //**处理回复确认为异常
    Wait_QueryDstFreq0_Finish,
    Wait_QueryDstFreq1_Finish,
    Wait_QueryDstFreq2_Finish,
    //Wait_QueryDstFreq3_Finish,
    Wait_QueryDstFreqDivide1_Finish,
    Wait_QueryDstFreqDivide2_Finish,
    Wait_HardResetTest_Finish,
    Wait_EraseFlash_Finish,
    ScriptSuccess
};
class Script_FreqManage_FreqAndFreqDivideSwitch : public QObject,public AbstractScript,public DynamicCreate<Script_FreqManage_FreqAndFreqDivideSwitch>
{
    Q_OBJECT
public:
    explicit Script_FreqManage_FreqAndFreqDivideSwitch(QObject *parent = nullptr);
    ~Script_FreqManage_FreqAndFreqDivideSwitch();
    Script_FreqManage_FreqDivideSwitch_RunState emScriptRunState;
    BuildNetwork_GW *p_BuildNetwork_GW;

    ushort concentratorCnt;
    bool resultFlag=true;
    bool transFlag=true;
    bool adFlag=true;
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
    shared_ptr<AfnF0F12> p_EraseFlash_F0F12;
    shared_ptr<Afn03F16> p_QueryFreq_03F16;
    shared_ptr<AfnF0F51> p_QueryFreqDivide_F0F51;
    shared_ptr<Afn05F16> p_SetFreq_05F16;
    shared_ptr<AfnF0F50> p_SetFreqDivide_F0F50;

    shared_ptr<Frame645Helper> p_Frame645Helper;
    shared_ptr<RspsNormal_ReadAddr_0x93> p_MeterAddrResp_93;
    shared_ptr<FrameOOPHelper> p_FrameOOPHelper;
    shared_ptr<GetResponseNormal> p_GetResponseNormal_ReadAddr;
    QList<QByteArray> addrList;

    uchar msgSeq=0;
    ushort index=0;
    /***可配置参数***/
    ushort timerForReachThresld=1200; //单位:s
    ushort timerAfterReachThresld=130; //单位:s

    double netSucRateThresld=1.0;
    bool needBuildNet=true;
    bool needPowerOff=true;
    uchar addCntPerTime=5;
    uchar topoCntPerTime=5;

    int waitTime=120;//单位:s
    //**F2D1
    char initFreq=0x02;//初始频段
    char initFreqDivide=0x01;//初始分频系数

    struct FreqAndDivideStruct
    {
        char dstFreq;
        char dstFreqDivide;
    };
    QList<FreqAndDivideStruct> freqAndDivideList;

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
    void sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame);
    void sendSrcMsg(DvcType dvcType,int dvcId,QByteArray msg);

};

#endif // SCRIPT_FREQMANAGE_FREQANDFREQDIVIDESWITCH_H
