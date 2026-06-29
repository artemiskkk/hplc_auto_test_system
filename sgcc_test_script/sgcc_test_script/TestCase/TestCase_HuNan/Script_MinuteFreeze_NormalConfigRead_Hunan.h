#ifndef SCRIPT_MINUTEFREEZE_NORMALCONFIGREAD_HUNAN_H
#define SCRIPT_MINUTEFREEZE_NORMALCONFIGREAD_HUNAN_H

#include <QObject>
#include "TestCase/BuildNetwork_GW.h"
enum Script_MinuteFreeze_NormalConfigRead_Hunan_RunState
{
    TestInit,
    Wait_BuildNetFinish_Whole,
    Wait_00F1_For_05F102_Finish,

    Wait_SetCurveConfig_STA_05F103_Finish,
    Wait_Delay_Finish,
    Wait_QueryCurveData_CCO_03F102_Finish,
    Wait_SetCurveConfig_STA_05F103_2_Finish,
    Wait_Delay_2_Finish,
    Wait_QueryCurveData_CCO_03F102_2_Finish,
    Wait_QueryConfigStatus_STA_10f103_Finish,
    Wait_Delay_3_Finish,
    Wait_Delay_4_Finish,
    Wait_ReadCurveData_STA_F1F100_Finish,
    Wait_SetCurveConfig_STA_05F103_3_Finish,
    Wait_SetCurveConfig_STA_05F103_4_Finish,
    Wait_Delay_5_Finish,
    Wait_ReadCurveData_STA_F1F100_2_Finish,

    ScriptSuccess,
    Wait_QueryTaskConfig_STA_10F103_Finish
};
class Script_MinuteFreeze_NormalConfigRead_Hunan: public QObject, public AbstractScript,public DynamicCreate<Script_MinuteFreeze_NormalConfigRead_Hunan>
{
    Q_OBJECT
//    Q_PLUGIN_METADATA(IID "com.topscomm.AbstractScript")
//    Q_INTERFACES(AbstractScript)
public:
    explicit Script_MinuteFreeze_NormalConfigRead_Hunan(QObject *parent = nullptr);
    ~Script_MinuteFreeze_NormalConfigRead_Hunan();
    Script_MinuteFreeze_NormalConfigRead_Hunan_RunState emScriptRunState;

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
    shared_ptr<Afn05F102> p_SetRouterTimePeriod_05F102;
    shared_ptr<Afn05F103> p_SetCurveConfig_STA_05F103;
    shared_ptr<Afn03F102> p_QueryCurveData_CCO_03F102;
    shared_ptr<Afn10F103_Hunan> p_QueryConfigStatus_STA_10f103;
    shared_ptr<AfnF1F100> p_ReadCurveData_STA_F1F100;

    shared_ptr<Frame645Helper> p_Frame645Helper;
    shared_ptr<RspsNormal_ReadAddr_0x93> p_MeterAddrResp_93;
    shared_ptr<FrameOOPHelper> p_FrameOOPHelper;
    shared_ptr<GetResponseNormal> p_GetResponseNormal_ReadAddr;

    int tryTimes=0;
    uchar msgSeq=0;
    ushort times=0;
    QString startTime;
    QString endTime;
    const int maxMonitorTime=60;
    double havePassedTimeLen=0.0;
    bool haveStartContinueTimer=false;
    QList<QByteArray> addrList;
    /***可配置参数***/
    ushort timerForReachThresld=30*60; //单位:s
    ushort timerAfterReachThresld=120; //单位:s
    double netSucRateThresld=1.0;
    bool needBuildNet=true;
    bool needPowerOff=true;
    uchar addCntPerTime=5;
    uchar topoCntPerTime=5;

    DataIdLen dataID;
    ushort index=0;
    uchar node_num=10;
    QList<ModuleConfig10F103_Hunan> moduleConfig10F103_Hunan;
    uchar delayTimer=15;//环境运行时间15分钟
//    QString curTime;//数据开始冻结时间
    QByteArray curTime;
    QByteArray curTime_start_oop;
    QByteArray curTime_end_oop;


    //F1F100相关数据内容
    struct RoundMark
    {
        QByteArray dateTime;
        QByteArray replyData;
        bool readFlag=false;
    };
    struct ReadDataUnit
    {
        DataIdLen dataIdLen;        //数据项id，数据项长度
        QList<RoundMark> roundList; //对应数据项时间，数据，是否正确标志
    };
    struct ReadModeFrame
    {
        Address meterNo;            //表地址
        uchar protocolType;         //协议
        QList<ReadDataUnit> dataUnitList;//数据项

        int dataIDNumIndex=0;       //抄读数据项的索引
    };
    QList<ReadModeFrame> singleSTA_645List;
    QList<ReadModeFrame> threeSTA_645List;
    QList<ReadModeFrame> singleSTA_oopList;
    QList<ReadModeFrame> threeSTA_oopList;
    QList<ReadModeFrame> allMeterList;

    int parallelCount=0; //并行数
    int maxParallelNum=5;//最大并行数

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
    uchar realPeriod=1; //下行05F103的采集周期，单位min
    int dataIDNum=2;    //冻结数据项的个数，单次
    int pointNum=2;     //F1F100下行数据点数，单次


    int MaxDataIDNum=5;       //最大抄读冻结数据项的个数
    int MaxPointNum=12;       //最大抄读数据点数
//    int dataIDNumIndex=0;   //单个表抄读数据项的索引
//    int ParallelNumIndex=0;   //并行抄表冻结项全部完成表数的索引
    int pointNumIndex=0;        //已读数据点数

    QDateTime startDateTime;    //开始时间
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
    void readInfoClear();
    bool isNotReplyEmpty(QByteArray data);
    int getMeterNoIndex(Address address);
    void meterInfoListInit();
    bool isSuccess(QList<ModuleConfig10F103_Hunan> moduleConfig10F103_Hunan);
    QString getFailMeterNo();
    void processMsgFromCCO(DvcType dvcType, int dvcId);
    void processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID);
    void processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID);
    void sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame);
    void sendSrcMsg(DvcType dvcType,int dvcId,QByteArray msg);
};

#endif // SCRIPT_MINUTEFREEZE_NORMALCONFIGREAD_HUNAN_H
