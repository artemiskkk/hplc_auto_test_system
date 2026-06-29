#ifndef SCRIPT_BROADCASTTIMEFLOW_H
#define SCRIPT_BROADCASTTIMEFLOW_H

#include "../InteroperateBase_GW.h"
enum Script_BroadcastTimeFlow_RunState
{
    ScriptInit,
    Wait_InteroperateInit_Finish,
    Wait_PauseRouter_Finish,
    Wait_BroadcastTime_Finish,
    Wait_RecoverRouter_Finish,
    ScriptFinish
};
class Script_BroadcastTimeFlow : public InteroperateBase_GW, public AbstractScript,public DynamicCreate<Script_BroadcastTimeFlow>
{
    Q_OBJECT
public:
    explicit Script_BroadcastTimeFlow(QObject *parent = nullptr);
    ~Script_BroadcastTimeFlow();
private:
    Script_BroadcastTimeFlow_RunState emScriptRunState=ScriptInit;
    uchar protocol=0x00;
private:
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

private slots:
    virtual void timer_timeout();//命令发送超时定时器槽函数
    virtual void maxAllowTimer_timeout();//脚本最大运行时间定时器槽函数
    virtual void delayTimer_timeout();//命令延迟定时器槽函数
    virtual void slotStartFollowTest();
};

#endif // SCRIPT_BROADCASTTIMEFLOW_H
