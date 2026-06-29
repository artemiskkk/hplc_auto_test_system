#ifndef SCRIPT_STAIDPRESSURTEST_MODULE_CHIP_SN_H
#define SCRIPT_STAIDPRESSURTEST_MODULE_CHIP_SN_H

#include <QObject>
#include "TestCase/BuildNetwork_GW.h"

enum Script_StaIDPressurTest_Module_Chip_Sn_RunState
{
    ScriptInit,
    Wait_BuildNetFinish_Whole,
    SetAndQueryChipID,
    SetAndQueryStaID,
    SetAndQuerySN,
    ScriptSuccess
};

class Script_StaIDPressurTest_Module_Chip_Sn : public QObject, public AbstractScript,public DynamicCreate<Script_StaIDPressurTest_Module_Chip_Sn>
{
    Q_OBJECT
    //    Q_PLUGIN_METADATA(IID "com.topscomm.AbstractScript")
    //    Q_INTERFACES(AbstractScript)
public:
    explicit Script_StaIDPressurTest_Module_Chip_Sn(QObject *parent = nullptr);
    ~Script_StaIDPressurTest_Module_Chip_Sn();
    Script_StaIDPressurTest_Module_Chip_Sn_RunState emScriptRunState;
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
    shared_ptr<Afn10F9> p_QueryNetScale_10F9;//查询网络规模
    shared_ptr<Afn10F40> p_QueryChipID_10F40;
    shared_ptr<Afn10F40> p_QueryStaID_10F40;

    shared_ptr<AfnF0F39> p_SetChipID_F0F39;
    shared_ptr<AfnF0F40> p_QueryChipID_F0F40;
    shared_ptr<AfnF0F39> p_SetStaID_F0F39;
    shared_ptr<AfnF0F40> p_QueryStaID_F0F40;
    shared_ptr<AfnF0F42> p_SetSn_F0F42;
    shared_ptr<AfnF0F41> p_QuerySn_F0F41;

    shared_ptr<Frame645Helper> p_Frame645Helper;
    shared_ptr<RspsNormal_ReadAddr_0x93> p_MeterAddrResp_93;
    shared_ptr<FrameOOPHelper> p_FrameOOPHelper;
    shared_ptr<GetResponseNormal> p_GetResponseNormal_ReadAddr;

    int tryTimes=0;
    uchar msgSeq=0;
    /***可配置参数***/
    ushort timerForReachThresld=1200; //单位:s
    ushort timerAfterReachThresld=120; //单位:s
    double netSucRateThresld=1.0;
    bool needBuildNet=true;
    bool needPowerOff=true;
    uchar addCntPerTime=5;
    uchar topoCntPerTime=5;

    //参数
    QString header;
    QString chipId;
    QString staId;
    QString sn;
    bool flagBuildNetOver=false;
    bool flag_10F40=false;
    uint execute_num_ =1;
    int index = 0;
    QString test_name_ ="【GW-STA-F010-0004-V01】STA模块ID、SN管理-压力测试";
    bool is_query_chipid_ =false;
    bool is_query_staid_ =false;
    bool is_query_sn_ =false;

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
    QByteArray getQueryFrame();
    void processMsgFromCCO(DvcType dvcType, int dvcId);
    void processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID);
    void processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID);
    void sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame);
    void sendSrcMsg(DvcType dvcType,int dvcId,QByteArray msg);
};
#endif // SCRIPT_STAIDPRESSURTEST_MODULE_CHIP_SN_H
