#ifndef SCRIPT_IDMANAGE_CCO_H
#define SCRIPT_IDMANAGE_CCO_H

#include <QObject>
#include "TestCase/BuildNetwork_GW.h"

enum Script_IdManage_CCO_RunState
{
    TestInit,
    Wait_BuildNetFinish_Whole,
    Wait_SetRouterID_Finish,
    Wait_ParaInit_Finish,
    Wait_QueryRouterID_Finish,
    ScriptSuccess
};
class Script_IdManage_CCO : public QObject, public AbstractScript,public DynamicCreate<Script_IdManage_CCO>
{
    Q_OBJECT
//    Q_PLUGIN_METADATA(IID "com.topscomm.AbstractScript")
//    Q_INTERFACES(AbstractScript)
    
    // 常量定义，避免魔数
private:
    static const uchar FRAME_START_CHAR = 0x68;
    static const uchar FRAME_END_CHAR = 0x16;
    static const uchar CONTROL_FIELD_VALUE = 0x43;
    static const uchar AFN_F0 = 0xF0;
    static const uchar DT1_02 = 0x02;
    static const uchar DT2_00 = 0x00;
    static const int FRAME_TOTAL_LENGTH = 26;
    static const int MODULE_ID_LENGTH = 11;
    static const int EXPECTED_ID_STRING_LENGTH = 22;
    static const int MAC_ADDRESS_LENGTH = 6;
    
public:
    explicit Script_IdManage_CCO(QObject *parent = nullptr);
    ~Script_IdManage_CCO();
    Script_IdManage_CCO_RunState emScriptRunState;

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
//    shared_ptr<AfnF0F40> p_QueryRouterID_F0F40;
    shared_ptr<AfnF0F2> p_SetRouterID_F0F2;
    shared_ptr<Afn03F12> p_QueryRouterID_03F12;
    shared_ptr<Afn10F40> p_QueryRouterID_10F40;

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
    bool needBuildNet=false;
    bool needPowerOff=true;
    uchar addCntPerTime=5;
    uchar topoCntPerTime=5;
    //参数
    QMap<QString, QString> m_paraDic;
    int m_cachedFreq = 0;
    int idIndex=0;
    QString dstId;
    QString idHeader;
    QString id_0="0708091011121314151617181920212223242526272829303132333435363738394041424344454647484950";
    QString id_1="050607080910111213141516171819202122232425262728293031323334353637383940414243444546474849515253545556575859";
    QString id_2="";
    QStringList idList;
    QString chipType="GY";
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
    QByteArray buildFrameManually(const QByteArray& IdData);
    void validateFrame(const QByteArray& frame);
    QByteArray buildFrameManually();
    void sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame);
    void sendSrcMsg(DvcType dvcType,int dvcId,QByteArray msg);
};

#endif // SCRIPT_IDMANAGE_CCO_H
