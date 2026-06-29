#ifndef BUILDNETWORK_ZHEJIANG_H
#define BUILDNETWORK_ZHEJIANG_H

#include <QObject>
#include <QTimer>
#include <QDateTime>
#include <QThread>
#include <QDebug>
#include <QtCore/qglobal.h>

#include "../../CommonData/CommonDataType_TestCase.h"
#include "PublicDataStruct/abstractscript.h"
#include "PublicDataStruct/abstractscripthost.h"
#include "PublicDataStruct/commdatatype.h"


enum BuildNetwork_Zhejiang_State
{
    Init,
    Wait_SetBaudRate_Finish,
//    Wait_SetBaudRate_CCO_Finish,
//    Wait_SetBaudRate_645_Finish,
//    Wait_SetBaudRate_698_Finish,
//    Wait_PowerOff12V_CCO_Finish,
//    Wait_PowerOff12V_STA_Finish,
    Wait_PowerOff12V_Finish,
    Wait_PowerOn12V_CCO_Finish,
    Wait_PowerOn12V_STA_Finish,
    Wait_RST_CCO_Finish,
    Wait_RST_STA_Finish,
    Wait_AssignAddrsFinish,
    Wait_00F1_for_05F1_SetCcoAddr,
    Wait_00F1_for_01F2_ParamInit,
    Wait_00F1_for_01F1_HardInit,
    Wait_00F1_for_05F16_SetFreq,
    Wait_AddMetersFinish,
    Wait_BuildNetFinish,
    BuildNetFinish
};
struct NodeInfo
{
    Address node_address_;//!<从节点地址
    char node_protocol_;//!<从节点协议
    char deviceType;
    bool operator==(const NodeInfo &unit_)const
    {
        if(this->node_address_==unit_.node_address_
                &&this->node_protocol_==unit_.node_protocol_
                &&this->deviceType==unit_.deviceType)
        {
            return true;
        }
        else
            return false;
    }
};

class BuildNetwork_Zhejiang : public QObject//, public AbstractScript
{
    Q_OBJECT
    //    Q_PLUGIN_METADATA(IID "com.topscomm.AbstractScript")
    //    Q_INTERFACES(AbstractScript)
public:
    explicit BuildNetwork_Zhejiang(QObject *parent = nullptr);
    ~BuildNetwork_Zhejiang();
    BuildNetwork_Zhejiang_State emScriptRunState;
    DvcType emDvcType;

    ushort concentratorCnt;
    bool resultFlag=false;
    bool startBuildNetFlag=false;
    shared_ptr<QList<shared_ptr<CtrInfo>>> p_CtrInfoList;

    QList<double> sendParams;
    QList<int> idList;

    shared_ptr<QTimer> p_timer;
    shared_ptr<QTimer> p_maxAllowTimer;
    shared_ptr<QTimer> p_delayTimer;
    AbstractScriptHost *p_AbstractScriptHost;

    QByteArray sendMsgOct;
    QString sendMsgLog;
    QString logMsgStr;

    shared_ptr<qgdw_3762_protocol::Frame3762Helper> p_MsgBase_3762;
    shared_ptr<Afn00F1> p_Confirm_00F1;//回复确认
    shared_ptr<qgdw_3762_protocol::Afn05F1> p_SetCcoAddr_05F1;//设置主节点地址
    shared_ptr<qgdw_3762_protocol::Afn01F1> p_HardInit_01F1;//硬件初始化
    shared_ptr<qgdw_3762_protocol::Afn01F2> p_ParamInit_01F2;//参数初始化
    shared_ptr<qgdw_3762_protocol::Afn05F16> p_SetFreq_05F16;//设置频段
    shared_ptr<qgdw_3762_protocol::Afn11F1> p_AddSlaveNode_11F1;//添加从节点
    shared_ptr<qgdw_3762_protocol::Afn10F21> p_QueryNetTopoInfo_10F21;//查询网络拓扑信息
    shared_ptr<qgdw_3762_protocol::Afn10F9> p_QueryNetScale_10F9;//查询网络规模
    //
    shared_ptr<qgdw_3762_protocol::Afn03F2> p_QueryNoise_03F2;

    shared_ptr<dlt_645_Protocol::Frame645Helper> p_MsgBase_645;
    shared_ptr<dlt_645_Protocol::RspsNormal_ReadAddr_0x93> p_MeterAddrResp_93;

    shared_ptr<GetResponseNormal> p_GetResponseNormal_ReadAddr;


    uchar addr[6];

    uchar temp_DvcType = 0x00;//标记电表档案中存在哪些槽位需要12V上电

    int tryTimes=0;
    bool isFirstChkTopo=true;
    int roundIndex=1;
    uchar addCntThisTime=20;
    ushort times=0;
    ushort index=0;
    uchar msgSeq=0;
    bool haveChangedFreq=false;
    double havePassedTimeLen=0.0;
    bool haveStartContinueTimer=false;
    int checkTopologyIntervalTime = 5;

    uchar freqBand=2;
    /***可配置参数***/
    uchar addCntPerTime=20;
    uchar topoCntPerTime=20;

    ushort timerForReachThresld=1200; //单位:s
    double netSucRateThresld=1.0;
    ushort timerAfterReachThresld=120; //单位:s    
    //参数
    bool flagStaHighComBaud=false;
    bool flagCJQHighComBaud=false;
    bool flagPowerOnCJQ=true;
    bool flagNeedAddMeter=true;


    QList<NodeInfo> reportNodeList_06F4;
public:
    void  execute();
    void  stop();
    void  addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq);
    void  setHost(AbstractScriptHost *host);
    bool  config(const QMap<QString,QString> *paraDic);
    void  processMsg(DvcType dvcType,int id,uchar* data,int datalen);
    void  processCtrlDvcRes(DvcType dvcType,QList<int> idList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params);

    void  setCtrInfoListAndFreq(shared_ptr<QList<shared_ptr<CtrInfo>>> p_CtrInfoList, uchar freq);
    void initBuildNetWork();
signals:
    void signalBuildNetFinish();

public slots:
    void timer_timeoutProc();
    void maxAllowTimer_timeoutProc();
    void delayTimer_timeoutProc();

private:
    void processMsgFromCco(DvcType dvcType,int dvcId);
    void processMsgFromMeter645(DvcType dvcType,int dvcId,int mtrlID);
    void processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID);
    void processMsgFromCJQ(DvcType dvcType,int dvcId);
    //void sendMsg(DvcType dvcType,int dvcId,int meterID,void *frame);
    void sendMsg(DvcType dvcType,int dvcId,int meterID,shared_ptr<void> frame);
    void sendSrcMsg(DvcType dvcType,int dvcId,QByteArray msg);

    QList<int> getDvcIdList(DvcType dvcType);
    uchar findDvcTypeToPowerOnOrRst();
    void findDvcToPowerOnOrRst(QString str);


};
#endif // BUILDNETWORK_ZHEJIANG_H
