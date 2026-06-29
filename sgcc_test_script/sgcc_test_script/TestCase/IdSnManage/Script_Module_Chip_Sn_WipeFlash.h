#ifndef SCRIPT_MODULE_CHIP_SN_WIPEFLASH_H
#define SCRIPT_MODULE_CHIP_SN_WIPEFLASH_H

#include <QObject>
#include "TestCase/BuildNetwork_GW.h"

enum Script_Module_Chip_Sn_WipeFlash_RunState
{
    ScriptInit,
    Wait_BuildNetFinish_Whole,
    Wait_InitialQueryChipID_Finish,
    Wait_InitialQueryRouterID_Finish,
    Wait_InitialQueryChipSN_Finish,
    Wait_SetChipID_Finish,
    Wait_SetRouterID_Finish,
    Wait_SetSN_Finish,
    Wait_WipeFlash_Finish,
    Wait_QueryChipID_Finish,
    Wait_QueryRouterID_Finish,
    Wait_QueryQuerySN_Finish,
    ScriptSuccess
};

class Script_Module_Chip_Sn_WipeFlash  : public QObject, public AbstractScript,public DynamicCreate<Script_Module_Chip_Sn_WipeFlash>
{
    Q_OBJECT
public:
    explicit Script_Module_Chip_Sn_WipeFlash(QObject *parent = nullptr);
    ~Script_Module_Chip_Sn_WipeFlash();

    Script_Module_Chip_Sn_WipeFlash_RunState emScriptRunState;
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
    shared_ptr<AfnF0F39> p_SetChipID_F0F39;
    shared_ptr<AfnF0F39> p_SetRouterID_F0F39;
    shared_ptr<AfnF0F42> p_SetSn_F0F42;
    shared_ptr<AfnF0F40> p_QueryChipID_F0F40;
    shared_ptr<AfnF0F40> p_QueryRouterID_F0F40;
    shared_ptr<AfnF0F41> p_QuerySn_F0F41;
    shared_ptr<AfnF0F12> p_EraseFlash_F0F12;

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
    int index = 0;
    QString test_name_ ="【GW-CCO-F010-0004-V01】路由ID管理-擦除flash 【GW-CCO-F010-0013-V01】路由芯片ID管理-擦除flash 【GW-CCO-F010-0017-V01】路由SN管理-擦除flash";

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

#endif // SCRIPT_MODULE_CHIP_SN_WIPEFLASH_H
