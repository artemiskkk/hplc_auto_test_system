#ifndef SCRIPT_READMETER_SEARCHMETER_UPGRADE_MULTITASK_H
#define SCRIPT_READMETER_SEARCHMETER_UPGRADE_MULTITASK_H


#include <QObject>
#include "TestCase/BuildNetwork_GW.h"
#define SENCE2
#define DIRECT_CCO_MODE

/**
 * @brief The Script_ReadMeter_SearchMeter_Upgrade_MultiTask_RunState 运行状态机
 */

const ushort SEG_LEN_STA = 512;
enum Script_ReadMeter_SearchMeter_Upgrade_MultiTask_RunState
{
    ScriptInit,
    Wait_BuildNetFinish_Whole,
    Wait_CcoHardInit_Finish,
    Wait_SetAeraIdentification_05F6_Finish,
    Wait_SetBandRate_Finish,
    Wait_00F1_for_12F2_UpgrdSta,
    Wait_StartUpdata_15F1_Finish_1,
    Wait_StartUpdata_15F1_Finish_2,
    Wait_StaUpgradeFinish,


    Wait_SetStaActiveResgister_11F5_Finish,

    Wait_SendResetRoute_12F1_Finish,
    Wait_ReadMeter_Finish,

    Wait_QueryRouteRunStateCycles_10F4_Finish,
    Wait_QueryStaVersion_02F1_Finish,


    Wait_EventReport_06F5_Finish,
    ScriptSuccess

};

namespace Namespace_ReadMeter_14F1
{
    enum ReadFlag
    {
        ReadFail,
        ReadSuccess,
        WaitRead,
    };

    struct ReadDataUnit
    {
        QByteArray dataID;
        bool notRead;
        double costTime;
    };

    struct ReadInfo
    {
        Address meterNo;
        uchar protocolType;
    //    uchar phase;
        bool requestFlag_14F1;
        ReadFlag readFlag_14F1;
        ReadFlag readFlag_13F1;
        ReadFlag readFlag_F1F1;
        QList<ReadDataUnit> dataUnitList;
    };
}
using namespace Namespace_ReadMeter_14F1;

/**
 * @brief The Script_ReadMeter_SearchMeter_Upgrade_MultiTask 多任务并行-抄表+搜表+升级-未组网【GW-CCO-F022-0004-V01】
 *
 */

class Script_ReadMeter_SearchMeter_Upgrade_MultiTask :  public QObject, public AbstractScript, public DynamicCreate<Script_ReadMeter_SearchMeter_Upgrade_MultiTask>
{
    Q_OBJECT
public:
    explicit Script_ReadMeter_SearchMeter_Upgrade_MultiTask(QObject *parent = nullptr);
    ~Script_ReadMeter_SearchMeter_Upgrade_MultiTask();
private:
    Script_ReadMeter_SearchMeter_Upgrade_MultiTask_RunState emScriptRunState = ScriptInit;
    BuildNetwork_GW *p_BuildNetwork_GW;//组网通用脚本
    shared_ptr<QList<shared_ptr<CtrInfo>>> p_CtrInfoList;//一般我们只用p_CtrInfoList->at(0)。CtrInfo代表一个集中器的完整信息，当前工装只有一个CCO(原先设计是针对台体多个CCO)
    QTimer *p_timer;//命令发送超时定时器
    QTimer *p_maxAllowTimer;//脚本最大运行时间定时器
    QTimer *p_delayTimer;//命令延迟定时器
    QTimer *p_meterSearchTimer;//搜表
    QTimer *p_upgardeTimer;

    AbstractScriptHost *p_AbstractScriptHost;//脚本抽象类，定义与测试系统之间的交互接口
    bool resultFlag;

    QByteArray sendMsgOct;//发送
    QString sendMsgLog;
    QString logMsgStr;
    uchar addr[6]={0x00};
    char cs_null;

    QByteArray  ccoNewAddr;
    int tryTimes=0;//命令重试次数
    uchar msgSeq=0;//帧序号
    int index=0;//一般用于索引表号
    char totalReportNum = 0;
    bool report_flag_06F5 = false;

    //定义帧格式处理
    shared_ptr<Frame3762Helper> p_Frame3762Helper;
    shared_ptr<Afn01F1> p_HardReset_01F1;
    shared_ptr<Afn05F6> p_SetAreaIdentityFlag_05F6;
    shared_ptr<qgdw_3762_protocol::Afn15F1> p_FileTransfer_15F1_Down;

    shared_ptr<Afn11F5> p_ActiveRegister_11F5;
    shared_ptr<Afn00F1> p_Confirm_00F1;
    shared_ptr<Afn00F2> p_Deny_00F2;

    shared_ptr<Afn11F4> p_SetBaudRate_11F4;
    shared_ptr<Afn12F1> p_RouterRestart_12F1;
    shared_ptr<Afn12F2> p_RouterPause_12F2;
    shared_ptr<Afn12F3> p_RouterRecover_12F3;
    shared_ptr<Afn14F1> p_RouterRequestRead_14F1;
    shared_ptr<Afn13F1> p_MonitorSlaveNode_13F1;
    shared_ptr<AfnF1F1> p_ParallelReadMeter_F1F1;
    shared_ptr<qgdw_3762_protocol::Afn10F4> p_CcoRunStateInfo_10F4_Down;
    shared_ptr<qgdw_3762_protocol::Afn02F1> p_ChkStaInVrsnInfo_02F1_Down;

    //**工装参数
    DvcType emDvcType;
    QList<double> sendParams;
    QList<int> idList;

    //搜表
    ushort search_meter_duration = 30;
    struct NodeInfo_Struct
    {
        Address nodeAddress;
        char protocol;
        char deviceType;
        bool isExist=false;
    };
    QList<NodeInfo_Struct> nodeInfoList;
    int num_reportOutOfRecord;   // 上报档案外的表数目

    //抄表
    QList<ReadInfo> readInfoList;
    QTimer *p_13F1Timer,*p_F1F1Timer,*p_14F1Timer;
    int requestNum_14F1=0,reportSucNum_14F1=0,reportFailNum_14F1=0,timeoutNum_14F1=0;
    int requestNum_13F1=0,responseSucNum_13F1=0,responseFailNum_13F1=0,timeoutNum_13F1=0,readNo_13F1=0;
    int requestNum_F1F1=0,responseSucNum_F1F1=0,responseFailNum_F1F1=0,timeoutNum_F1F1=0,readNo_F1F1=0;
    bool endFlag_13F1=false,endFlag_14F1=false,endFlag_F1F1=false;

    // 升级
    uchar dstUpgradeDvc = 2;
    ushort timerForReachThresld_QueryNodeVrsnInfo02F1 = 1800; //单位:s
    ushort meterIndex_02F1 = 0;
    // QStringList failAddr_OutVrsn;
    ushort failCnt_InVrsn;
    QString staVendorChipCode;
    QString staOutVrsn,staInVrsn;
    ushort fileIndex;
    int dataLen;    //升级包长度，单位:字节
    ushort totalSegs;   //总段数
    QByteArray rawUpdateFile;
    QList<bool> transResList;

    ushort recvEventNum=0;

    void RetransmitFailedSegments();
    QMap<ushort, ushort> segmentRetryCount;
    QList<ushort> failedSegments;
    const ushort MAX_RETRY_TIMES = 3;
    bool isRetransmitting = false;

    shared_ptr<Frame645Helper> p_Frame645Helper;
    shared_ptr<RspsNormal_ReadAddr_0x93> p_MeterAddrResp_93;

    shared_ptr<FrameOOPHelper> p_FrameOOPHelper;
    shared_ptr<GetResponseNormal> p_GetResponseNormal_ReadAddr;

    /***可配置参数***///此处只列举每个脚本必备的参数，不同脚本可根据实际情况配置特有参数
    ushort timerForReachThresld=1200; //单位:s,一般作为组网时间
    ushort timerAfterReachThresld=1800; //单位:s,一般作为组网完成后具体测试业务的执行最大时间
    double netSucRateThresld=1.0;       //组网成功率
    bool needBuildNet=false;            //需要组网标志

    bool needPowerOff=false;//需要断电标志
    uchar addCntPerTime=50;//每次添加从节点个数
    uchar topoCntPerTime=5;//每次添加


private:
    //脚本接口函数，测试系统通过这些接口函数与脚本进行交互
    void  addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq);//添加档案信息
    void  setHost(AbstractScriptHost *host);//设置脚本抽象类，定义与测试系统之间的交互接口，脚本发送消息给测试系统
    bool  config(const QMap<QString,QString> *paraDic);//加载脚本配置参数
    void  execute();//脚本启动执行
    void  stop();//脚本停止
    void  processMsg(DvcType dvcType,int id,uchar* data,int datalen);//处理从测试系统传入的五种槽位的数据信息
    void  processCtrlDvcRes(DvcType dvcType,QList<int> idList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params);//处理从测试系统传入的关于工装控制的回复信息
    //
    void processMsgFromCCO(DvcType dvcType, int dvcId);//处理来自CCO槽位的报文
    void processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID);//处理来自槽位的645报文
    void processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID);//处理来自槽位的OOP报文
    void sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame);//发送报文到测试系统，主要处理状态机内的报文发送
    void sendSrcMsg(DvcType dvcType,int dvcId,QByteArray msg);//处理其他报文的发送，如其它事件上报时，处理之后的报文通过此处发送
    QList<int> getDvcIdList(DvcType dvcType);
    void LoadUpdateFile();
    void Refresh_TestResult_15F1(shared_ptr<Afn15F1> p_FileTransfer_15F1_Up);
    ushort Refresh_SuccessCnt_15F1();
    bool isAllExist();
    void readInfoInit();
    int getReadInfo(Address address);
    void resultCheck();
    void Refresh_TestResult_02F1(shared_ptr<Afn02F1> p_ChkNodeVrsnInfo_02F1_Up);
    void Refresh_SuccessCnt_02F1();
    void getVendorChipCode();
    bool search_flag = false;
    bool upgrade_flag = false;

    Address extractAddressFromAfn06F2(shared_ptr<Afn06F2> p_ReportReadData_Up);
    bool extractAndProcess3762Frame(QByteArray& buf,QByteArray& completeFrame);

    QMutex m_meterMutex;              // 电表互斥锁
    QSet<QString> m_busyMeters;       // 正在抄读的表集合
    QMap<QString, qint64> m_meterLockTime;  // 锁定时间

    bool tryLockMeter(char* meterAddr, const QString& taskName);
    void unlockMeter(char* meterAddr);

private slots:
    void timer_timeoutProc();//命令发送超时定时器槽函数
    void maxAllowTimer_timeoutProc();//脚本最大运行时间定时器槽函数
    void delayTimer_timeoutProc();//命令延迟定时器槽函数
    void timeoutProc_13F1();
    void timeoutProc_F1F1();
    void timeoutProc_14F1();
    void timeoutProc_meterSearch();
    void timeoutProc_upgarde();
};

#endif // SCRIPT_READMETER_SEARCHMETER_UPGRADE_MULTITASK_H
