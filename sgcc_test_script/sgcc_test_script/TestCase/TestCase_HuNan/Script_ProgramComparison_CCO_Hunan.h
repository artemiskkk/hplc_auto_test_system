#ifndef SCRIPT_PROGRAMCOMPARISON_CCO_HUNAN_H
#define SCRIPT_PROGRAMCOMPARISON_CCO_HUNAN_H

#include <QObject>
#include "TestCase/BuildNetwork_GW.h"
const ushort SEG_LEN_512 = 512;
const ushort SEG_LEN_2048 = 2048;

enum Script_ProgramComparison_CCO_Hunan_RunState
{
    ProgramComparisonInit,
    Wait_BuildNetFinish_Whole,
    Wait_QueryCCOVersionInfo_03F130,
    Wait_QueryCCOData_03F131_Finish,
    Wait_QueryCCOData_03F131_2_Finish,
    Wait_QueryCCOData_03F131_3_Finish,
//    Wait_00F1_For_05F3_Broadcast,
//    Wait_06F5_Report,
//    Wait_Finish_Broadcast,
    ScriptSuccess
};
//namespace Namespace_BroadcastTime_Hunan
//{
//    struct MeterInfoBroadcast_Struct
//    {
//        Address meterNo;
//        uchar protocolType;
//        bool readFlag;
//        uchar offsetFlag;
//        char dateTime[6];
//    };
//}
//using namespace Namespace_BroadcastTime_Hunan;
class Script_ProgramComparison_CCO_Hunan : public QObject, public AbstractScript,public DynamicCreate<Script_ProgramComparison_CCO_Hunan>
{
    Q_OBJECT
//    Q_PLUGIN_METADATA(IID "com.topscomm.AbstractScript")
//    Q_INTERFACES(AbstractScript)
public:
    explicit Script_ProgramComparison_CCO_Hunan(QObject *parent = nullptr);
    ~Script_ProgramComparison_CCO_Hunan();

    Script_ProgramComparison_CCO_Hunan_RunState emScriptRunState;
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
    shared_ptr<Afn03F130> p_QueryCCOVersionInfo_03F130;
    shared_ptr<Afn03F131> p_QueryCCOData_03F131;
//    shared_ptr<Afn05F3> p_Broadcast_05F3;
//    shared_ptr<Afn00F1> p_Confirm_00F1;

    shared_ptr<Frame645Helper> p_Frame645Helper;
    shared_ptr<RspsNormal_ReadAddr_0x93> p_MeterAddrResp_93;
    shared_ptr<FrameOOPHelper> p_FrameOOPHelper;
    shared_ptr<GetResponseNormal> p_GetResponseNormal_ReadAddr;
    //OOP校时

    int tryTimes=0;
    uchar msgSeq=0;
    ushort times=0;
    ushort index=0;
//    QString startTime;
//    QString endTime;
    const int maxWaitTime=60;
    double havePassedTimeLen=0.0;
    bool haveStartContinueTimer=false;

    /***可配置参数***/
    ushort timerForReachThresld=1200; //单位:s
    ushort timerAfterReachThresld=120; //单位:s

    double netSucRateThresld=1.0;
    bool needBuildNet=true;
    uchar addCntPerTime=5;
    uchar topoCntPerTime=5;
    bool needPowerOff=true;
    bool isStdPrcs=true;


    DataUnitUp03F130 softwareInfoData;
    QByteArray originProgram;
    struct ProgramCompareStruct
    {
        char startAddress[4];
        QByteArray programSegment;
        QByteArray compareData;
    };
    QList<ProgramCompareStruct> programCompareList;

//    QString vendor_code;//!< 厂商代码
//    QByteArray version_date;//!< 版本日期
//    QByteArray software_version;//!< 软件版本
//    QString mcu_type;//!<mcu型号

    uchar dstUpgradeDvc=1;
    int dataLen;    //升级包总长度，单位:字节
    ushort totalSegs_512;   //总段数
    ushort totalSegs_2048;
    QByteArray rawUpdateFile;
    ushort fileIndex=0;

//    uchar StartAddr_0[4] = {00,00,00,00};
//    uchar StartAddr_1[4] = {00,01,00,00};
//    uchar *StatAdd=StartAddr_0;
    //校时参数
//    QList<MeterInfoBroadcast_Struct> meterInfoList;
//    int checkTime=600;//超过5分钟
//    int delayTime=120;//s
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
    void LoadUpdateFile();
    void SoftwareInfo();
    void softwareInfoInit(int currentIndex,int segmentLen);

//    void meterInfoInit();
//    bool isMeterExist(Address address);
//    int getMeterInfo(Address address);
//    double calSuccessRate();
//    QString getFailMeterNo();

    void processMsgFromCCO(DvcType dvcType, int dvcId);
    void processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID);
    void processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID);
    void sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame);
    void sendSrcMsg(DvcType dvcType,int dvcId,QByteArray msg);
};

#endif // SCRIPT_PROGRAMCOMPARISON_CCO_HUNAN_H
