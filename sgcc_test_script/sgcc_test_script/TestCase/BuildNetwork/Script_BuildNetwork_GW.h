#ifndef SCRIPT_BUILDNETWORK_GW_H
#define SCRIPT_BUILDNETWORK_GW_H

#include <QObject>
#include "TestCase/BuildNetwork_GW.h"

enum Script_BuildNetwork_GW_RunState
{
      Script_BuildNetwork_Init,
      Wait_BuildNetFinish_Whole
};
class Script_BuildNetwork_GW: public QObject, public AbstractScript,public DynamicCreate<Script_BuildNetwork_GW>
{
    Q_OBJECT
//    Q_PLUGIN_METADATA(IID "com.topscomm.AbstractScript")
//    Q_INTERFACES(AbstractScript)
public:
    explicit Script_BuildNetwork_GW(QObject *parent = nullptr);
    ~Script_BuildNetwork_GW();
    Script_BuildNetwork_GW_RunState emScriptRunState;

    ushort concentratorCnt;
    bool resultFlag;
    shared_ptr<QList<shared_ptr<CtrInfo>>> p_CtrInfoList;

    shared_ptr<QTimer> p_timer;
    AbstractScriptHost *p_AbstractScriptHost;

    shared_ptr<BuildNetwork_GW> p_BuildNetwork_GW;


    /***可配置参数***/
    ushort timerForReachThresld=1800; //单位:s
    double netSucRateThresld=1.0;
    ushort timerAfterReachThresld=120; //单位:s
    bool needBuildNet=true;

    uchar addCntPerTime=5;
    uchar topoCntPerTime=5;
    bool needPowerOff=true;
public:
    void  execute();
    void  stop();
    void  addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq);
    void  setHost(AbstractScriptHost *host);
    bool  config(const QMap<QString,QString> *paraDic);
    void  processMsg(DvcType dvcType,int id,uchar* data,int datalen);
    void  processCtrlDvcRes(DvcType dvcType,QList<int> idList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params);

public slots:
    void slotBuildNetFinish();
    void timer_timeoutProc();
};

#endif // SCRIPT_BUILDNETWORK_GW_H
