#ifndef SCRIPT_NODESUPPORTMINUTECOLLECTION_H
#define SCRIPT_NODESUPPORTMINUTECOLLECTION_H

#include <QObject>
#include "TestCase/BuildNetwork_GW.h"

enum Script_NodeSupportMinuteCollection_RunState
{
      TestInit,
      Script_BuildNetwork_Init,
      Wait_ParaInit_Finish,
      Wait_HardReset_Finish,
      Wait_03F10_LocalCommunicationModeInfo_Finish,
      Wait_QueryNetScale_Init_Finish,
      Wait_QueryNetScale_Finish,
//      Wait_SetStaId_Finish,
//      Wait_QueryStaId_Finish,
      Wait_10F7_QuerySlaveNodeIdInfo_Finish,
      Wait_BuildNetFinish_Whole,
      ScriptSuccess
};

struct NodeAddressInfo
{
    char addressInfo[6];
};

class Script_NodeSupportMinuteCollection : public QObject, public AbstractScript, public DynamicCreate<Script_NodeSupportMinuteCollection>
{
    Q_OBJECT
public:
    explicit Script_NodeSupportMinuteCollection(QObject *parent = nullptr);
    ~Script_NodeSupportMinuteCollection();
    Script_NodeSupportMinuteCollection_RunState emScriptRunState;
    BuildNetwork_GW *p_BuildNetwork_GW;

    ushort concentratorCnt;
    bool resultFlag;
    shared_ptr<QList<shared_ptr<CtrInfo>>> p_CtrInfoList;

    QTimer *p_timer;
    QTimer *p_maxAllowTimer;
    QTimer *p_delayTimer;
    AbstractScriptHost *p_AbstractScriptHost;

    QByteArray sendMsgOct;
    QString sendMsgLog;
    QString logMsgStr;
    uchar addr[6];

    shared_ptr<Frame3762Helper> p_Frame3762Helper;
    shared_ptr<Afn01F2> p_ParaInit_01F2;
    shared_ptr<Afn01F1> p_HardReset_01F1;
    shared_ptr<Afn03F10>p_LocalCommunicationMode_03F10;//**本地从节点运行模式信息
    shared_ptr<Afn10F7> p_QuerySlaveNodeIdInfo_10F7; //**查询从节点ID信息
    shared_ptr<Afn10F9> p_QueryNetScale_10F9; //**查询网络规模
    shared_ptr<Afn10F1> p_QueryNodeNum_10F1; //**查询从节点数量
    shared_ptr<Afn11F1> p_AddSlaveNode_11F1; //**添加从节点
//    shared_ptr<Afn02F1> p_TransmitData_02F1; //**抄控器透传数据转发
//    shared_ptr<Afn10F40> p_QueryStaID_10F40; //**模块ID扩展查询命令

    shared_ptr<Frame645Helper> p_Frame645Helper;
    shared_ptr<RspsNormal_ReadAddr_0x93> p_MeterAddrResp_93;
    shared_ptr<FrameOOPHelper> p_FrameOOPHelper;
    shared_ptr<GetResponseNormal> p_GetResponseNormal_ReadAddr;

    int tryTimes=0;
    uchar msgSeq=0;
    ushort times=0;
    //QString startTime;
    //QString endTime;
    double havePassedTimeLen=0.0;
    bool haveStartContinueTimer=false;
    QList<QByteArray> addrList;
    /***可配置参数***/
    ushort timerForReachThresld=1200; //单位:s
    ushort timerAfterReachThresld=120; //单位:s

    double netSucRateThresld=1.0;
    bool needBuildNet=true;
    uchar addCntPerTime=5;
    uchar topoCntPerTime=5;
    bool needPowerOff=true;
    ////////
    ushort netScale;
    const int routerPeriod=10;//单位:s
    ushort index=0;
    int num=5;
    int node_num=0;

    QList<NodeAddressInfo> meterAdder;

    bool queFlag;

    //**配置模块ID参数
//    bool flag02F1=true;
//    bool flagFind=false;
//    int idIndex=0;
//    QStringList idList;
//    QString dstId;
//    QString idHeader;
//    QString id_0="0708091011121314151617181920212223242526272829303132333435363738394041424344454647484950";
//    QString id_1="070809101112131415161718192021222324252627282930313233343536373839404142434445464748495051525354555657585960";
//    QString id_2="";
//    QString idType="02";

//    struct SlaveNodeIdInfo_Struct
//    {
//        Address nodeAddress;
//        ushort SlaveNodeTotalNum;
//        uchar ThisAnswerSlaveNodeNum;
//        char ModuleType;
//        char MinuteCollection;
//        bool isExist=false;
//    };
//    QList<SlaveNodeIdInfo_Struct> SlaveNodeIdInfo;
//    QList<SlaveNodeIdInfo_Struct> UnSlaveNodeIdInfo;

public:
    void  execute();
    void  stop();
    void  addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq);
    void  setHost(AbstractScriptHost *host);
    bool  config(const QMap<QString,QString> *paraDic);
    void  processMsg(DvcType dvcType,int id,uchar* data,int datalen);
    void  processCtrlDvcRes(DvcType dvcType,QList<int> idList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params);

public slots:
    void timer_timeoutProc();
    void maxAllowTimer_timeoutProc();
    void delayTimer_timeoutProc();
private:
    void processMsgFromCCO(DvcType dvcType, int dvcId);
//    void processMsgFromReadCtrlDvc(DvcType dvcType, int dvcId);
    void processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID);
    void processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID);
    void sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame);
    void sendSrcMsg(DvcType dvcType,int dvcId,QByteArray msg);
};

#endif // SCRIPT_NODESUPPORTMINUTECOLLECTION_H
