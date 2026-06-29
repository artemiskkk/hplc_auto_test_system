#ifndef SCRIPT_CURVEREAD_F1F100_HUNAN_H
#define SCRIPT_CURVEREAD_F1F100_HUNAN_H

#include <QObject>
#include "TestCase/BuildNetwork_GW.h"
enum Script_CurveRead_F1F100_Hunan_RunState
{
    ReadMeterInit,
    Wait_BuildNetFinish_Whole,
    Wait_00F1_for_12F2_Pause,
    Wait_00F1_for_12F3_Recover,
    Wait_Finish_Curve,
    ScriptSuccess
};
class Script_CurveRead_F1F100_Hunan : public QObject, public AbstractScript,public DynamicCreate<Script_CurveRead_F1F100_Hunan>
{
    Q_OBJECT
//    Q_PLUGIN_METADATA(IID "com.topscomm.AbstractScript")
//    Q_INTERFACES(AbstractScript)
public:
    explicit Script_CurveRead_F1F100_Hunan(QObject *parent = nullptr);
    ~Script_CurveRead_F1F100_Hunan();

    Script_CurveRead_F1F100_Hunan_RunState emScriptRunState;
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
    shared_ptr<Afn12F3> p_RouterRecover_12F3;
    shared_ptr<AfnF1F100> p_CurveReadMeter_F1F100;

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

    enum ReadMode
    {
        MeterNo,
        RoundIndex
    };
    ReadMode emReadMode;
    struct ReadModeFrame
    {
        Address meterNo;
        uchar protocolType;
        QList<QByteArray> frameList;
    };

    QList<ReadModeFrame> meterNoModeFrameList;
    QList<ReadModeFrame> roundIndexModeFrameList;
    //曲线参数
    struct RoundMark
    {
        QByteArray dateTime;
        QByteArray replyData;
        bool readFlag=false;
    };

    struct ReadDataUnit
    {
        DataIdLen dataIdLen;
        QList<RoundMark> roundList;

//        uchar readRound=0;
//        uchar successRound=0;
//        uchar failRound=0;
//        double successRate=0;
    };

    /**
     * @brief F1F1抄读信息
     */
    struct ReadInfo_Curve
    {
        Address meterNo;
        uchar protocolType;
        QList<ReadDataUnit> dataUnitList;
    };
    QList<ReadInfo_Curve> readInfoList;
    int maxFrameNum=3;//单帧最大报文数
    int maxParallelNum=5;//最大并行数
    int readPeriod=1;//5分钟曲线
    //int maxReadRound=10;//最大轮次

    int totalReadNum=96;//抄读总点数
    uchar singleReadNum=3;//单条报文抄读点数
    int waitedTime=0;//


    int parallelCount=0;//并行数
    int meterIndex=0;//电表序号
    int roundIndex=0;//当前轮次

    QList<DataIdLen> singleDataId_645_List;
    QList<DataIdLen> triDataId_645_List;
    QList<DataIdLen> singleDataId_OOP_List;
    QList<DataIdLen> triDataId_OOP_List;
    QString single645=QString("00010102 02,00010202 03,00010302 03,00ff0000 14,00ff0100 14,00ff0200 14,01008002 03");
    QString tri645=QString("00ff0102 06,00ff0202 09,00ff0302 0c,00ff0402 0c,00ff0602 08,00ff0000 14,00ff0100 14,00ff0200 14,"
                           "00000300 04,00000400 04,00000500 04,00000600 04,00000700 04,00000800 04,04008002 03,05008002 03,01008002 03");
    QString singleOOP=QString("20000200 0e,20010200 0f,20040200 0f,00000200 20,00100200 20,00200200 20,20010200 0f");
    QString triOOP=QString("20000200 12,20010200 15,20040200 18,20050200 18,200a0200 14,00000200 20,00100200 20,00200200 20,"
                           "00300200 10,00400200 10,00500200 10,00600200 10,00700200 10,00800200 10,10100200 0f,10300200 0f,20010200 0f");


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
    QList<DataIdLen> dataIdInit(QString dataId);
    void readInfoInit();
    void readInfoClear();
    void readFrameListInit();
//    bool isMeterExist(Address address);
    int getMeterNoIndex(Address address);
    bool isNotReplyEmpty(QByteArray data);
    void calSuccessRate();
    void displayResult();
//    QString calCostTime();
//    QString getFailMeterNo();
//    bool isAllReadThisMeter(int meterID);
//    bool isAllMeterReadThisRound();
//    void CalcAvrgConsumeTimeLen(uchar rdFlag);  //1:13F1; 2:F1F1; 3:14F1

    void processMsgFromCCO(DvcType dvcType, int dvcId);
    void processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID);
    void processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID);
    void sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame);
    void sendSrcMsg(DvcType dvcType,int dvcId,QByteArray msg);
};
#endif // SCRIPT_CURVEREAD_F1F100_HUNAN_H
