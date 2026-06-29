#ifndef SCRIPT_STACHIPIDANDKEY_H
#define SCRIPT_STACHIPIDANDKEY_H

#include <QObject>
#include "TestCase/BuildNetwork_GW.h"

enum Script_StaChipIdAndKey_RunState
{
    ScriptInit,
    Wait_BuildNetFinish_Whole,
    Wait_QueryChipIDAndKeyInit_F0F44_Finish,
    Wait_SetChipIDAndKey_F0F43_Finish,
    Wait_SetNormalChipIDAndKey_F0F43_Finish,
    Wait_SetChipID_F0F39_Finish,
    Wait_QueryChipIDAndKey_F0F44_Finish,
    ScriptSuccess
};

class Script_StaChipIdAndKey : public QObject, public AbstractScript,public DynamicCreate<Script_StaChipIdAndKey>
{
    Q_OBJECT
    //    Q_PLUGIN_METADATA(IID "com.topscomm.AbstractScript")
    //    Q_INTERFACES(AbstractScript)
public:
    explicit Script_StaChipIdAndKey(QObject *parent = nullptr);
    ~Script_StaChipIdAndKey();
    Script_StaChipIdAndKey_RunState emScriptRunState;

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
    shared_ptr<AfnF0F39> p_SetChipID_F0F39;
    shared_ptr<AfnF0F40> p_QueryStaChipID_F0F40;
    shared_ptr<Frame645Helper> p_Frame645Helper;
    shared_ptr<RspsNormal_ReadAddr_0x93> p_MeterAddrResp_93;
    shared_ptr<FrameOOPHelper> p_FrameOOPHelper;
    shared_ptr<GetResponseNormal> p_GetResponseNormal_ReadAddr;

    int tryTimes=0;
    uchar msgSeq=0;
    /***可配置参数***/
    ushort timerForReachThresld=1200; //单位:s
    ushort timerAfterReachThresld=120; //单位:s

    double netSucRateThresld=1.0;
    bool needBuildNet=true;
    bool needPowerOff=true;
    uchar addCntPerTime=5;
    uchar topoCntPerTime=5;
    //参数
    bool flagBuildNetOver=false;
    //新参数
    QString normal_id_and_key_ = "01029c01c1fb0354434630000000000018f83e3038c1f072"//每行24字节
                                 "019e48134a7f4aa4a9dc0ccc42efdb0a3ed3b54a730122f1"
                                 "d2878417d7320a46eb5be4f359142187e17878abab7b30d5"
                                 "05db387e586b98b2bb5e661b8fd0343a02ccf4ea34f50b2c"
                                 "dbb2a00a8d879cb1ca12ea5c2106e9c40c08770a2a3562de"
                                 "c1eb87f797c9c9de789473c2125003d2de88101f308fe911"
                                 "dc2396cfaeb90f0c56006f070d08b6224512065587f89f51"
                                 "61f3d2f331d7836763ab05da518550d43a2a8b9c6595f1c4"
                                 "a9e7c1e645626ec757465244a34383e83c64415e2b88a906"
                                 "2fbfb462464b4122469001e031e60e3e2062af11b1dbf922"
                                 "334566d467ff2f65d737395362c2a6107860ab33b867c678"
                                 "d3f79daadf773b44fd863203448ad3f1d5f53fb362b7a655"
                                 "c212fa0b6af9acdb24b898188436541311d6ef1d65189145"
                                 "30d01c57b635de40ff9a9bed041e5b7bb6a31f6e9862ab77"
                                 "90faba688c8a2983e04d";//每行10字节
    QString abnormal_id_and_key_ = "0102030405060708091011121314151617181920212223242526272829303132333435363738394041424344454647484950"
                                   "0102030405060708091011121314151617181920212223242526272829303132333435363738394041424344454647484950"
                                   "0102030405060708091011121314151617181920212223242526272829303132333435363738394041424344454647484950"
                                   "0102030405060708091011121314151617181920212223242526272829303132333435363738394041424344454647484950"
                                   "0102030405060708091011121314151617181920212223242526272829303132333435363738394041424344454647484950"
                                   "0102030405060708091011121314151617181920212223242526272829303132333435363738394041424344454647484950"
                                   "01020304050607080910111213141516171819202122232425262728293031323334353637383940414243444546";
    QString chip_id_ = "";
    QString test_name_ = "【GW-STA-F010-00011-V01】STA模块芯片ID+密钥管理-常规;【GW-STA-F010-00012-V01】STA模块芯片ID+密钥管理-身份信息签字验证;GW-STA-F010-00013-V01】STA模块芯片ID+密钥管理-密钥ID";
    QString test_result = "状态，结果，目标值，实际值，【用例编号1】【用例编号n】";
    int index = 0;
    bool is_query_normal_id_and_key_ =false;
    bool is_query_abnormal_id_and_key_ =false;
    bool is_query_chip_id_ =false;


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

#endif // SCRIPT_STACHIPIDANDKEY_H
