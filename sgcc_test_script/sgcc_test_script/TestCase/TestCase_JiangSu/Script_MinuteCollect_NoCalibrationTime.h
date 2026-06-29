#ifndef SCRIPT_MINUTECOLLECT_NOCALIBRATIONTIME_H
#define SCRIPT_MINUTECOLLECT_NOCALIBRATIONTIME_H

#include <QObject>
#include "TestCase/BuildNetwork_GW.h"

enum Script_MinuteCollect_NoCalibrationTime_RunState
{
    ReadMeterInit,
    Wait_BuildNetFinish_Whole,
    //**Wait_RouterReqTime_14F2_Finish,
    Wait_00F1_for_12F2_Pause,
    Wait_QueryRouterState_Finish,
    Wait_00F1_for_12F3_Recover,
    Wait_Finish_F1F1,
    ScriptSuccess
};

namespace Namespace_ReadMeter_F1F1
{
    enum ReadFlag
    {
        //NotRead,
        ReadSuccess,
        Reading
    };

    struct ReadDataUnit
    {
        QByteArray dataID;
        bool notRead;
        double costTime;
    };

    /**
     * @brief F1F1抄读信息
     */
    struct ReadInfo_F1F1
    {
        Address meterNo;
        uchar protocolType;
        ReadFlag readFlag;
        QList<ReadDataUnit> dataUnitList;
    };
}
using namespace Namespace_ReadMeter_F1F1;

enum oopProtocolType
{
    pro_ActionRequest_RollCallRepor,
    pro_reportResponseRecordList,
    pro_minuteFroRecordList

};

class Script_MinuteCollect_NoCalibrationTime : public QObject, public AbstractScript,public DynamicCreate<Script_MinuteCollect_NoCalibrationTime>
{
    Q_OBJECT
public:
    explicit Script_MinuteCollect_NoCalibrationTime(QObject *parent = nullptr);
    ~Script_MinuteCollect_NoCalibrationTime();

    Script_MinuteCollect_NoCalibrationTime_RunState emScriptRunState;
    //**发送OOP报文类型
    oopProtocolType emSendOopProtype;
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
    shared_ptr<Afn12F2> p_RouterPause_12F2;
    shared_ptr<Afn10F4> p_QueryRouterState_10F4;
    shared_ptr<Afn12F3> p_RouterRecover_12F3;
    shared_ptr<AfnF1F1> p_ParallelReadMeter_F1F1;
    shared_ptr<Afn14F2> p_ReqCtrTime_14F2;

    shared_ptr<Frame645Helper> p_Frame645Helper;
    shared_ptr<RspsNormal_ReadAddr_0x93> p_MeterAddrResp_93;
    shared_ptr<FrameOOPHelper> p_FrameOOPHelper;
    shared_ptr<GetResponseNormal> p_GetResponseNormal_ReadAddr;
    shared_ptr<ReportNotificationRecordList> p_ReportNotificationRecordList;

    int tryTimes=0;
    uchar msgSeq=0;
    ushort times=0;
    ushort index=0;
    QString startTime;
    QString endTime;
    const int maxMonitorTime=70;
    double havePassedTimeLen=0.0;
    bool haveStartContinueTimer=false;
    QList<QByteArray> addrList;
    /***可配置参数***/
    ushort timerForReachThresld=1200; //单位:s
    ushort timerAfterReachThresld=120; //单位:s
    ushort timerMaxReachThresld=40;//**单位:min

    double netSucRateThresld=1.0;
    bool needBuildNet=true;
    bool needPowerOff=true;
    uchar addCntPerTime=5;
    uchar topoCntPerTime=5;
    QList<int> dataIdIndexList;

    QList<ReadInfo_F1F1> readInfoList;
    //并发抄表参数
    int maxFrameNum=16;//单帧最大报文数
    int maxParallelNum=5;//最大并行数
    int parallelCount=0;//并行数
    int meterIndex=0;//电表序号

    //**等待开启点名上报的延时时间
    int waitTime=10;//**单位:min

    QTime curTime;

    //**关键字
    bool oopFlag=true;
    bool minFlag=true;
    bool dayFlay=true;
    bool monFlay=true;
    int dvcNo;


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
    void readInfoInit();
    bool isMeterExist(Address address);
    int getReadInfo(Address address);
    double calSuccessRate();
    QString calCostTime();
    QString getFailMeterNo();
    bool isAllRead(int meterID);
    bool isAllMeterReadSuccess();
    void CalcAvrgConsumeTimeLen(uchar rdFlag);  //1:13F1; 2:F1F1; 3:14F1

    void processMsgFromCCO(DvcType dvcType, int dvcId);
    void processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID);
    void processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID);
    void sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame);
    void sendSrcMsg(DvcType dvcType,int dvcId,QByteArray msg);
};

#endif // SCRIPT_MINUTECOLLECT_NOCALIBRATIONTIME_H
