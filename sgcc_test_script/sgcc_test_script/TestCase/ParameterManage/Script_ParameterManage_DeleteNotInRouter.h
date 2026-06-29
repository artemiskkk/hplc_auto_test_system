#ifndef SCRIPT_PARAMETERMANAGE_DELETENOTINROUTER_H
#define SCRIPT_PARAMETERMANAGE_DELETENOTINROUTER_H

#include <QObject>
#include "TestCase/BuildNetwork_GW.h"
#define SENCE2

class Script_ParameterManage_DeleteNotInRouter : public QObject, public AbstractScript,public DynamicCreate<Script_ParameterManage_DeleteNotInRouter>
{
    Q_OBJECT
public:
    explicit Script_ParameterManage_DeleteNotInRouter(QObject *parent = nullptr);
    ~Script_ParameterManage_DeleteNotInRouter();
    /**
     * @brief The Script_ParameterManage_DeleteNotInRouter_RunState 运行状态机
     */
    enum Script_ParameterManage_DeleteNotInRouter_RunState
    {
        ScriptInit,
        Wait_BuildNetFinish_Whole,
        Wait_DeleteNode_11F2_Finish,
        Wait_QueryNodeInfo_10F2_Finish,
        Wait_ParameterInit_01F2_Finish,
        Wait_QueryNodeNum_10F1_Finish,
        Wait_DeleteNode_11F2_1_Finish,
        Wait_QueryNodeNum_10F1_1_Finish,
        ScriptSuccess
    };
private:    
    Q_ENUM(Script_ParameterManage_DeleteNotInRouter_RunState)
    QMetaEnum runStateMetaEnum=QMetaEnum::fromType<Script_ParameterManage_DeleteNotInRouter_RunState>();
    Script_ParameterManage_DeleteNotInRouter_RunState emScriptRunState=ScriptInit;
    BuildNetwork_GW *p_BuildNetwork_GW;//组网通用脚本
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

    shared_ptr<void> currentFrame;//记录
    QStringList allMeters;//所有电表
    QStringList deleteMeters=QStringList()<<"200001010000"//删除的电表
                                          <<"200001010001"
                                          <<"200001010002"
                                          <<"200001010003"
                                          <<"200001010004"
                                          <<"200001010005"
                                          <<"200001010006"
                                          <<"200001010007"
                                          <<"200001010008"
                                          <<"200001010009";
    QStringList queryInfoMeters;
    QString chipType="GY";
    //定义帧格式处理
    shared_ptr<Frame3762Helper> p_Frame3762Helper;
    shared_ptr<Afn11F2> p_Afn11F2;
    shared_ptr<Afn10F1> p_Afn10F1;
    shared_ptr<Afn10F2> p_Afn10F2;
    shared_ptr<Afn01F2> p_Afn01F2;
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
    QString testCase="【GW-CCO-F016-0009-V01】";
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

private slots:
    void timer_timeoutProc();//命令发送超时定时器槽函数
    void maxAllowTimer_timeoutProc();//脚本最大运行时间定时器槽函数
    void delayTimer_timeoutProc();//命令延迟定时器槽函数
};

#endif // SCRIPT_PARAMETERMANAGE_DELETENOTINROUTER_H
