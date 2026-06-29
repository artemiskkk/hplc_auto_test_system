#ifndef SCRIPT_CHIPIDMANAGE_STA_V3KEY_H
#define SCRIPT_CHIPIDMANAGE_STA_V3KEY_H

#include <QObject>
#include "TestCase/BuildNetwork_GW.h"

enum Script_ChipIdManage_STA_V3Key_RunState
{
    TestInit,
    Wait_BuildNetFinish_Whole,
    SetStaChipIDKey_Before,
    QueryStaChipID_ReadCtrlDvc,
    SetStaChipIDKey_After,
    ScriptSuccess
};
class Script_ChipIdManage_STA_V3Key : public QObject, public AbstractScript,public DynamicCreate<Script_ChipIdManage_STA_V3Key>
{
    Q_OBJECT
//    Q_PLUGIN_METADATA(IID "com.topscomm.AbstractScript")
//    Q_INTERFACES(AbstractScript)
public:
    explicit Script_ChipIdManage_STA_V3Key(QObject *parent = nullptr);
    ~Script_ChipIdManage_STA_V3Key();
    Script_ChipIdManage_STA_V3Key_RunState emScriptRunState;

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
    shared_ptr<Afn10F112> p_QueryStaChipID_10F112;
    shared_ptr<Afn10F40> p_QueryStaChipID_10F40;
    shared_ptr<Afn02F1> p_TransmitData_02F1;
    shared_ptr<Afn10F9> p_QueryNetScale_10F9;
    shared_ptr<Afn01F1> p_HardReset_01F1;
    shared_ptr<AfnF0F43> p_SetStaChipIDKey_F0F43;
    shared_ptr<AfnF0F44> p_QueryStaChipIDKey_F0F44;
    shared_ptr<AfnF0F40> p_QueryStaChipID_F0F40;
    shared_ptr<Frame645Helper> p_Frame645Helper;
    shared_ptr<RspsNormal_ReadAddr_0x93> p_MeterAddrResp_93;
    shared_ptr<FrameOOPHelper> p_FrameOOPHelper;
    shared_ptr<GetResponseNormal> p_GetResponseNormal_ReadAddr;

    int tryTimes=0;
    uchar msgSeq=0;
//    ushort times=0;
//    ushort index=0;
//    QString startTime;
//    QString endTime;
//    const int maxMonitorTime=60;
//    double havePassedTimeLen=0.0;
//    bool haveStartContinueTimer=false;
//    QList<QByteArray> addrList;
    /***可配置参数***/
    ushort timerForReachThresld=1200; //单位:s
    ushort timerAfterReachThresld=120; //单位:s

    double netSucRateThresld=1.0;
    bool needBuildNet=true;
    bool needPowerOff=true;
    uchar addCntPerTime=5;
    uchar topoCntPerTime=5;
    //参数
    int chipIdKeyIndex;
    QString dstChipId;
//    QString chipIdHeader;
    QString chipId_0="01029c01c1fb035443443000000009eb0721cfe32a3fd4dd";
    QString key_0="0156a221628a760d97dfcda5826bf4e33c863f92f3ce8e555583c31fc898f827dda8a724c6cb18ded25716e31b2298d8b54256de9a4"
                  "0156711a723d459a7a6af34d4e913fed1f0412ab4d800b02acb1dd1af95e125d579f5d8e3e188c2eee73b67c5388ee43cce41766d08"
                  "1796b50e9ff61f9dc07da55ad9ce1e6910f7a73bae4e001e633ca48169a37a74851dd6f5789bd669a8c33006d321d767b311373e7dc"
                  "f25b9bc51e70357df30cd61531626f5ae2ee4babc9afaaec9e5a0cfc4d4bb2d106261e59ffc5b50d31fe6cf945ca2cfcc9a2d91567e"
                  "755734d7880be8c1c1b3c231974bf70b8af8f48b106edfaa603112dd5a4b2032278b393a4677097a43ddc82655a65d876794ad76113"
                  "74df6c8956b452b588af340dad5f9d2cf7cf3d6237927622a764e481f0e23d16a286059e3e250b80b39660a80ab342929e0511011e391";
    QString chipId_1="01029c01c1fb035443443000000009e4d6a69b4f3ad3781d";
    QString key_1="01014cc945fcb1a29a3f089ce14f444712b63f63e91ce0598da96c8e4b566595091f73391584bb5a0a98e08e6fb37ebdbc7166e8ef8"
                  "7500d5d9c567aabb2e487147caab5e7e921c11e0dacc5eb03685f773f8534368f5aa4df0689b22a4179a969597f3f7c722d09d487ec"
                  "446291dd377cecc591d06eee986bfe82dad841ab13c700776817117cf49feb2f83d56be1326f50e304a64afceb08560d626a2084cbc"
                  "0a79cd4b6231fefcd20e284cffae7be466a6484f1dcad2719d4c9ce23d636f200b9ec4318b92ac5e5f3afa1939939a91f62906c8a5b"
                  "eb98643df8d2ab9891af79682e9abf361b88e58257b9ee357bf6aa3c729dd1b1dfc721e896ba1d44b3b8284437f6f8d754e5e80fbdf"
                  "96d0d3ebadafeac95fac56f30b4295d76170781e5f29026a4e09e6778494278ff951ada88183e665030568d11771261b031512b419047";
    QStringList chipIdKeyList;
    bool flagBuildNetOver=false;
    bool setflag=false;
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
    QByteArray getQueryFrame();
    void processMsgFromCCO(DvcType dvcType, int dvcId);
    void processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID);
    void processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID);
    void processMsgFromReadCtrlDvc(DvcType dvcType, int dvcId);
    void sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame);
    void sendSrcMsg(DvcType dvcType,int dvcId,QByteArray msg);
};

#endif // SCRIPT_CHIPIDMANAGE_STA_V3KEY_H
