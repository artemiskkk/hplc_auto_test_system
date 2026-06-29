#ifndef BUILDNETWORKDETECT_H
#define BUILDNETWORKDETECT_H

#include <QObject>
#include "TestCase/CommonData/CommonDataType_TestCase.h"
class BuildNetworkDetect : public QObject
{
    Q_OBJECT
private:
    enum BuildNetworkDetect_State
    {
        Init=0,
        Wait_QueryNetTopo_10F21
    };
    shared_ptr<qgdw_3762_protocol::Frame3762Helper> p_MsgBase_3762;
    shared_ptr<qgdw_3762_protocol::Afn01F2> p_ParamInit_01F2;//参数初始化
    shared_ptr<qgdw_3762_protocol::Afn11F1> p_AddSlaveNode_11F1;//添加从节点
    shared_ptr<qgdw_3762_protocol::Afn10F21> p_QueryNetTopoInfo_10F21;//查询网络拓扑信息

    shared_ptr<dlt_645_Protocol::Frame645Helper> p_MsgBase_645;
    shared_ptr<dlt_645_Protocol::RspsNormal_ReadAddr_0x93> p_MeterAddrResp_93;
    uchar addr[6];
    shared_ptr<GetResponseNormal> p_GetResponseNormal_ReadAddr;

    shared_ptr<QTimer> p_timer;
    shared_ptr<QTimer> p_maxAllowTimer;
    shared_ptr<QTimer> p_delayTimer;
    BuildNetworkDetect_State emScriptRunState=Init;

    shared_ptr<CtrInfo> p_CtrInfo=nullptr;
    shared_ptr<QList<shared_ptr<CtrInfo>>> p_CtrInfoList=nullptr;
    QList<Address> meterParameterList;//CCO和电表档案
    AbstractScriptHost *p_AbstractScriptHost;

    int index=0;
    char msgSeq=0;
    int tryTimes=0;
    bool isNeedDetect=true;
    /***可配置参数***/
    uchar addCntPerTime=20;
    uchar topoCntPerTime=20;

    ushort timerForReachThresld=1200; //单位:s
    double netSucRateThresld=1.0;
    ushort timerAfterReachThresld=120; //单位:s


    void processMsgFromCco(DvcType dvcType,int dvcId);
    void processMsgFromMeter645(DvcType dvcType,int dvcId,int mtrlID);
    void processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID);
    void processMsgFromCJQ(DvcType dvcType,int dvcId);
    //void sendMsg(DvcType dvcType,int dvcId,int meterID,void *frame);
    void sendMsg(DvcType dvcType,int dvcId,int meterID,shared_ptr<void> frame);
    void sendSrcMsg(DvcType dvcType,int dvcId,QByteArray msg);

    bool judgeTopoNodeBelongCtrInfo(const QList<NetworkTypelogyInfo10F21> list);
public:
    explicit BuildNetworkDetect(QObject *parent = nullptr);
    ~BuildNetworkDetect();
    void  execute();
    void  stop();
    void  addAddrsInfo(shared_ptr<CtrInfo> p_CtrInfo);
    void  setHost(AbstractScriptHost *host);
    bool  config(const QMap<QString,QString> *paraDic);
    void  processMsg(DvcType dvcType,int id,uchar* data,int datalen);
    void  processCtrlDvcRes(DvcType dvcType,QList<int> idList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params);

    bool getIsNeedDetect() const;

signals:
    void signalSendRebuildNetFlag(bool isNeedRebuild);
public slots:
    void timer_timeoutProc();
    void maxAllowTimer_timeoutProc();
    void delayTimer_timeoutProc();
};

#endif // BUILDNETWORKDETECT_H
