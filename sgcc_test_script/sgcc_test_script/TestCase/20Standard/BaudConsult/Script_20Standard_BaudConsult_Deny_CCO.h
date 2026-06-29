#ifndef SCRIPT_20STANDARD_BAUDCONSULT_DENY_CCO_H
#define SCRIPT_20STANDARD_BAUDCONSULT_DENY_CCO_H

#include <QObject>
#include "BaudConsult.h"
#include "BaudConsultPrepare.h"
#define SENCE2
class Script_20Standard_BaudConsult_Deny_CCO : public QObject, public AbstractScript,public DynamicCreate<Script_20Standard_BaudConsult_Deny_CCO>
{
    Q_OBJECT
public:
    explicit Script_20Standard_BaudConsult_Deny_CCO(QObject *parent = nullptr);
    ~Script_20Standard_BaudConsult_Deny_CCO();
    enum Script_20Standard_BaudConsult_Deny_CCO_RunState
    {
        ScriptInit,
        Wait_BuildNetFinish_Whole,
        Wait_BaudConsultPrepare_Finish,
        Wait_BaudConsult_115200bps_Finish=115200,
        Wait_BaudConsult_19200bps_Finish=19200,
        Wait_BaudConsult_9600bps_Finish=9600,
        Wait_BaudConsult_4800bps_Finish=4800,
        Wait_BaudConsult_2400bps_Finish=2400,
        Wait_BaudConsult_1200bps_Finish=1200,
        ScriptSuccess
    };
private:
    Q_ENUM(Script_20Standard_BaudConsult_Deny_CCO_RunState)
    Script_20Standard_BaudConsult_Deny_CCO_RunState emScriptRunState=ScriptInit;
    QMetaEnum metaEnum=QMetaEnum::fromType<Script_20Standard_BaudConsult_Deny_CCO_RunState>();
    BuildNetwork_GW *p_BuildNetwork_GW;//组网通用脚本
    BaudConsult *p_BaudConsult=nullptr;//波特率协商
    BaudConsultPrepare *p_BaudConsultPrepare=nullptr;//波特率协商准备
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
    //
    QString testCase="【GW-STA-F028-0002-V01】";
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
    void slotNoticeBaudConsultState(ProcessState,const QString& state);
    void slotNoticeBaudConsultPrepareState(ProcessState,const QString& state);
};
#endif // SCRIPT_20STANDARD_BAUDCONSULT_DENY_CCO_H
