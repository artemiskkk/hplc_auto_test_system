#ifndef INTEROPERATEBASE_GW_H
#define INTEROPERATEBASE_GW_H

#include <QObject>
#include "InteroperateInit_GW.h"
class InteroperateBase_GW : public QObject
{
    Q_OBJECT
public:
    explicit InteroperateBase_GW(QObject *parent = nullptr);
    ~InteroperateBase_GW();
    shared_ptr<InteroperateInit_GW> p_InteroperateInit_GW=nullptr;
    shared_ptr<QList<shared_ptr<CtrInfo>>> p_CtrInfoList;//一般我们只用p_CtrInfoList->at(0)。CtrInfo代表一个集中器的完整信息，当前工装只有一个CCO(原先设计是针对台体多个CCO)
    QTimer *p_timer;//命令发送超时定时器
    QTimer *p_maxAllowTimer;//脚本最大运行时间定时器
    QTimer *p_delayTimer;//命令延迟定时器
    AbstractScriptHost *p_AbstractScriptHost;//脚本抽象类，定义与测试系统之间的交互接口

    shared_ptr<Frame3762Helper> p_Frame3762Helper;
    shared_ptr<Frame645Helper> p_Frame645Helper;
    shared_ptr<RspsNormal_ReadAddr_0x93> p_MeterAddrResp_93;
    shared_ptr<FrameOOPHelper> p_FrameOOPHelper;
    shared_ptr<GetResponseNormal> p_GetResponseNormal_ReadAddr;

    uchar addr[6]={0x00};
    int tryTimes=0;//命令重试次数
    uchar msgSeq=0;//帧序号
    int index=0;//一般用于索引表号

    /***可配置参数***/
    ushort timerForReachThresld=1800; //单位:s
    double netSucRateThresld=1.0;
    ushort timerAfterReachThresld=120; //单位:s
    bool needBuildNet=true;

    uchar addCntPerTime=5;
    uchar topoCntPerTime=5;
    bool needPowerOff=true;

    void sendSrcMsg(DvcType dvcType, int dvcId, QByteArray msg);
    void sendSrcMsg(DvcType dvcType, int dvcId, QString msgStr);
signals:

protected slots:
    virtual void slotStartFollowTest();
    virtual void timer_timeout()=0;//命令发送超时定时器槽函数
    virtual void maxAllowTimer_timeout()=0;//脚本最大运行时间定时器槽函数
    virtual void delayTimer_timeout()=0;//命令延迟定时器槽函数
};

#endif // INTEROPERATEBASE_GW_H
