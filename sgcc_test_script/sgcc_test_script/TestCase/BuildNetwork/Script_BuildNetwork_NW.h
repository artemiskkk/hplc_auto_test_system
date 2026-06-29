#ifndef SCRIPT_BUILDNETWORK_NW_H
#define SCRIPT_BUILDNETWORK_NW_H

#include "TestCase/BuildNetwork_NW.h"


enum Script_BuildNetwork_NW_RunState
{
      Script_BuildNetwork_Init,
      Wait_BuildNetFinish_Whole
};

class Script_BuildNetwork_NW : public QObject, public AbstractScript
{
    Q_OBJECT
//    Q_PLUGIN_METADATA(IID "com.topscomm.AbstractScript")
//    Q_INTERFACES(AbstractScript)
public:
    explicit Script_BuildNetwork_NW(QObject *parent = nullptr);
    ~Script_BuildNetwork_NW();

    Script_BuildNetwork_NW_RunState emScriptRunState;

    ushort concentratorCnt;
    bool resultFlag;
//    QList<CtrInfo*> *p_CtrInfoList;
    shared_ptr<QList<shared_ptr<CtrInfo>>> p_CtrInfoList;

    shared_ptr<QTimer> p_timer;
    AbstractScriptHost *p_AbstractScriptHost;


    shared_ptr<BuildNetwork_NW> p_BuildNetwork_NW;


    /***可配置参数***/
    ushort timerForReachThresld=1800; //单位:s
    double netSucRateThresld=1.0;
    ushort timerAfterReachThresld=120; //单位:s

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

#endif // SCRIPT_BUILDNETWORK_NW_H
