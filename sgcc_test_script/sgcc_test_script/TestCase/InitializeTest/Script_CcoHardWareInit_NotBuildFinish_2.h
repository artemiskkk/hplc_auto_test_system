#ifndef SCRIPT_CCOHARDWAREINIT_NOTBUILDFINISH_2_H
#define SCRIPT_CCOHARDWAREINIT_NOTBUILDFINISH_2_H


#include <QObject>
#include "TestCase/BuildNetwork_GW.h"
#define SENCE2

enum Script_CcoHardWareInit_NetworkingFinish_2_RunState
{
    ScriptInit,
    Wait_BuildNetFinish_Whole,
    Wait_QueryInitialState_Finish,
    Wait_SetFreq_05F16,
    Wait_SetEventReport_05F2,
    Wait_SetAreaIdentification_05F6,
    Wait_QueryHardWareInit_Finish,
    Wait_QueryCheckState_Finish,
    ScriptSuccess
};

enum QueryInfo_State
{
    Wait_QueryMasterAddr_03F4,
    Wait_QueryBulidChanle_F0F38,
    Wait_QueryNodeNum_10F1,
    Wait_QueryNodeInfo_10F2,
    Wait_QueryNetNum_10F21,
    Wait_QueryModuleInfor_10F7,
    Wait_QueryRouterID_F0F40,
    Wait_QueryChipID_F0F40,
    Wait_QueryRouterSN_F0F41,
    Wait_QueryFreq_03F16,
    Wait_QueryWorkSwitch_10F4,
    Wait_SetBulidChanle_F0F37,
    Wait_SetFreq_05F16_1,
};


class Script_CcoHardWareInit_NotBuildFinish_2 : public QObject, public AbstractScript,public DynamicCreate<Script_CcoHardWareInit_NotBuildFinish_2>
{
    Q_OBJECT
public:
    explicit Script_CcoHardWareInit_NotBuildFinish_2(QObject *parent = nullptr);
    ~Script_CcoHardWareInit_NotBuildFinish_2();

private:
    Script_CcoHardWareInit_NetworkingFinish_2_RunState emScriptRunState = ScriptInit;
    QueryInfo_State emQueryState;

    BuildNetwork_GW *p_BuildNetwork_GW;//组网通用脚本

    ushort concentratorCnt;
    bool resultFlag;
    shared_ptr<QList<shared_ptr<CtrInfo>>> p_CtrInfoList;//一般我们只用p_CtrInfoList->at(0)。CtrInfo代表一个集中器的完整信息，当前工装只有一个CCO(原先设计是针对台体多个CCO)

    QTimer *p_timer;//命令发送超时定时器
    QTimer *p_maxAllowTimer;//脚本最大运行时间定时器
    QTimer *p_delayTimer;//命令延迟定时器
    AbstractScriptHost *p_AbstractScriptHost;//脚本抽象类，定义与测试系统之间的交互接口

    QByteArray sendMsgOct;//发送
    QString sendMsgLog;
    QString logMsgStr;
    uchar addr[6]={0x00};
    int tryTimes=0;//命令重试次数
    uchar msgSeq=0;//帧序号
    int index=0;//一般用于索引表号

    //定义帧格式处理
    shared_ptr<Frame3762Helper> p_Frame3762Helper;
    shared_ptr<Afn03F4> p_QueryMasterAddr_03F4;
    shared_ptr<Afn10F1> p_QueryNodeNum_10F1;
    shared_ptr<Afn10F2> p_QueryNodeInfo_10F2;
    shared_ptr<Afn10F9> p_QueryNetSize_10F9;//新增
    shared_ptr<Afn10F21> p_QueryNetNum_10F21;
    shared_ptr<Afn10F7> p_QueryModuleInfor_10F7;
    shared_ptr<AfnF0F40> p_QueryRouterID_F0F40;
    shared_ptr<AfnF0F40> p_QueryChipID_F0F40;
    shared_ptr<AfnF0F41> p_QueryRouterSN_F0F41;
    shared_ptr<Afn03F16> p_QueryFreq_03F16;
    shared_ptr<Afn10F4> p_QueryWorkSwitch_10F4;
    shared_ptr<Afn12F1> p_ResetRouter_12F1;
    shared_ptr<Afn01F1> p_HardReset_01F1;
    shared_ptr<Afn14F1> p_RouterRequestRead_14F1;
    shared_ptr<Afn14F2> p_RouterRequestTime_14F2;
    shared_ptr<AfnF0F38> p_QueryBulidChanle;
    shared_ptr<AfnF0F37> p_SetChannel_F0F37;
    shared_ptr<Afn05F2> p_SetEventReport_05F2;
    shared_ptr<Afn05F6> p_SetAreaIdentification_05F6;
    shared_ptr<Afn05F16> p_SetFreq_05F16;



    shared_ptr<Afn01F2> p_ParameterInit_01F2;
    shared_ptr<Afn13F1> p_MonitorSlaveNode_13F1;
    shared_ptr<Afn00F1> p_confirm_00F1;



    shared_ptr<Frame645Helper> p_Frame645Helper;
    shared_ptr<RspsNormal_ReadAddr_0x93> p_MeterAddrResp_93;
    shared_ptr<FrameOOPHelper> p_FrameOOPHelper;
    shared_ptr<GetResponseNormal> p_GetResponseNormal_ReadAddr;

    /***可配置参数***///此处只列举每个脚本必备的参数，不同脚本可根据实际情况配置特有参数
    ushort timerForReachThresld=1200; //单位:s,一般作为组网时间
    ushort timerAfterReachThresld=1800; //单位:s,一般作为组网完成后具体测试业务的执行最大时间
    double netSucRateThresld=1.0;//组网成功率
    bool needBuildNet=true;//需要组网标志
    bool needPowerOff=true;//需要断电标志
    uchar addCntPerTime=5;//每次添加从节点个数
    uchar topoCntPerTime=5;//每次添加

    //脚本参
    ushort times=0;
    QString startTime;
    QString endTime;
    const int maxMonitorTime=70;
    double havePassedTimeLen=0.0;
    bool haveStartContinueTimer=false;
    QList<QByteArray> addrList;

    //自定义参数
    uchar runFreq;
    uchar defaultWorkSwitch;
    bool hardResetFlag=true;
    QString master_addr_03F4_ ="";
    int  node_num_10F1_ =0;
    QString node_info_10F2_ ="";
    int net_size_10F9_=0 ;
    QString routerID_F0F40_="";
    QString chipID_F0F40_="";
    QString router_sn_F0F41_="";
    uchar event_report_flag_10F4_;
    uchar area_difference_flag_10F4_;
    QStringList query_id_10F7_;
    uchar set_freq_ =0x00;
    uchar channel_type_ =0x01;
    QString test_name_="【GW-CCO-F014-0014-V01】硬件初始化-组网未完成2";


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
    bool meterIsExist(Address meterAddr);

private slots:
    void timer_timeoutProc();//命令发送超时定时器槽函数
    void maxAllowTimer_timeoutProc();//脚本最大运行时间定时器槽函数
    void delayTimer_timeoutProc();//命令延迟定时器槽函数
};
#endif // SCRIPT_CCOHARDWAREINIT_NOTBUILDFINISH_2_H
