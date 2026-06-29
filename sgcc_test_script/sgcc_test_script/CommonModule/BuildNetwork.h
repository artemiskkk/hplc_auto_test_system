#ifndef BUILDNETWORK_H
#define BUILDNETWORK_H

#include <QObject>
#include "TestCase/CommonData/CommonDataType_TestCase.h"
class BuildNetwork : public QObject
{
    Q_OBJECT
    enum BuildNetwork_State
    {
        Init,
        Wait_00F1_for_01F2_ParamInit,
        Wait_AddMetersFinish,
        Wait_BuildNetFinish,
        BuildNetFinish
    };
    shared_ptr<qgdw_3762_protocol::Frame3762Helper> p_MsgBase_3762;
    shared_ptr<qgdw_3762_protocol::Afn01F2> p_ParamInit_01F2;//参数初始化
    shared_ptr<qgdw_3762_protocol::Afn11F1> p_AddSlaveNode_11F1;//添加从节点
    shared_ptr<qgdw_3762_protocol::Afn10F21> p_QueryNetTopoInfo_10F21;//查询网络拓扑信息

    shared_ptr<dlt_645_Protocol::Frame645Helper> p_MsgBase_645;
    shared_ptr<dlt_645_Protocol::RspsNormal_ReadAddr_0x93> p_MeterAddrResp_93;

    void processMsgFromCco(DvcType dvcType,int dvcId);
    void processMsgFromMeter645(DvcType dvcType,int dvcId,int mtrlID);
    void processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID);
    void processMsgFromCJQ(DvcType dvcType,int dvcId);
    //void sendMsg(DvcType dvcType,int dvcId,int meterID,void *frame);
    void sendMsg(DvcType dvcType,int dvcId,int meterID,shared_ptr<void> frame);
    void sendSrcMsg(DvcType dvcType,int dvcId,QByteArray msg);
public:
    explicit BuildNetwork(QObject *parent = nullptr);
//    ~BuildNetwork();

    void  execute();
    void  stop();
    void  addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq);
    void  setHost(AbstractScriptHost *host);
    bool  config(const QMap<QString,QString> *paraDic);
    void  processMsg(DvcType dvcType,int id,uchar* data,int datalen);
    void  processCtrlDvcRes(DvcType dvcType,QList<int> idList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params);

signals:
public slots:
//    void timer_timeoutProc();
//    void maxAllowTimer_timeoutProc();
//    void delayTimer_timeoutProc();
};

#endif // BUILDNETWORK_H
