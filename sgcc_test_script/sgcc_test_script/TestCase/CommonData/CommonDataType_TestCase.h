#ifndef COMMONDATATYPE_TESTCASE_H
#define COMMONDATATYPE_TESTCASE_H

#include <qglobal.h>
#include <QList>
#include <QString>
#include <QByteArray>
#include <QDebug>
#include <QSet>
#include <QThread>
#include "PublicDataStruct/abstractscript.h"
#include "PublicDataStruct/abstractscripthost.h"
#include "PublicDataStruct/commdatatype.h"
#include "frame645helper.h"
#include "frame3762helper.h"
#include "frameoophelper.h"
#include "DynamicCreate.h"
//#include "../../3rdparty/DynamicLib/DLT_645_Protocol_Lib/include/frame645helper.h"
//#include "../../3rdparty/DynamicLib/QGDW_376_2_Protocol_Lib/include/frame3762helper.h"
//#include "../../3rdparty/DynamicLib/OOP_Lib/include/frameoophelper.h"
//#include "DLT_698_45/MsgBase_698_45.h"
//using namespace std;
using namespace object_oriented_electic_data_exchange_protocol;
using namespace dlt_645_Protocol;
using namespace qgdw_3762_protocol;

#define CURRENT_TIME_FORMAT "yyyy-MM-dd hh:mm:ss zzz"
#define CTRL_CMD_TIMEOUT_TIME     10
#define CCO_CMD_TIMEOUT_TIME      10
#define STA_CMD_TIMEOUT_TIME      60
#define CJQ_CMD_TIMEOUT_TIME      10
#define CCO_CMD_TIMEOUT_LONGTIME   30
#define MAX_13F1_TIMEOUT_TIME      91
#define INSIGNIFICANCE     0
#define SUCCESS 1
#define FAIL 0

const uchar StarNetwork=1;
const uchar TreeNetwork=2;
const uchar LineNetwork=3;
const uchar MultiNetwork=4;

const uchar meterCntOfClct=4;

const uchar Role_CCO=4;
const uchar Role_PCO=2;
const uchar Role_STA=1;

//OAD的OI
const ushort DateTime_OI=0x4000;
const ushort Volt_OI=0x2000;
const ushort Crnt_OI=0x2001;
const ushort ActPower_OI=0x2004;
const ushort ReActPower_OI=0x2005;
const ushort PowerFactor_OI=0x200A;
const ushort CombActEne_OI=0x0000;
const ushort PosActEne_OI=0x0010;
const ushort NegActEne_OI=0x0020;
const ushort CombReActEne1_OI=0x0030;
const ushort CombReActEne2_OI=0x0040;
const ushort ReActEneQuadrant1_OI=0x0050;
const ushort ReActEneQuadrant2_OI=0x0060;
const ushort ReActEneQuadrant3_OI=0x0070;
const ushort ReActEneQuadrant4_OI=0x0080;
const ushort ActDemand_OI=0x1010;
const ushort ReActDemand_OI=0x1030;
const ushort KEYID_OI=0xff01;

const ushort ComuAddr=0x4001;

const ushort TripCtrl_OI=0x8000;
//规约类型
const uchar DLT645_2007=0x02;
const uchar OOP=0x03;

struct CjqNodeStruct
{
    uchar cjqAddr[6];
    QList<Address> subsidiaryNodeAddrList;
};
class MeterInfoForSingleNet
{
public:
    MeterInfoForSingleNet();
    ~MeterInfoForSingleNet();


    int mtrlID;
    uchar mtrAddr[6];
    uchar realPhase;
    uchar phaseSeq;
    uchar prtcl;
    uchar cjqAddr[6];
    DvcType slotPosition;//槽位
    int dvcId;

    bool needAdd=true;
    uchar ccoAddr[6];
    bool inNetResult;
    double timeConsumList[4];    //应用层业务耗时(单个业务时，占据[0];多个业务时，[0]代表汇总，具体业务类型依次类推)
    double successRateList[4];   //应用层业务本表成功率(单个业务时，占据[0];多个业务时，[0]代表汇总，具体业务类型依次类推)
    bool testResultList[4];     //应用层业务成功标志(单个业务时，占据[0];多个业务时，[0]代表汇总，具体业务类型依次类推)
    bool ctrlCmdExecList[20];    //控制命令执行结果标志(0:获取表号; 1:12V上电; 2:12V断电; 3:220V上电; 4:220V断电; 5:12V&220V同时上电; 6:12V&220V同时断电; 7:设置波特率; 8:eventout管脚置高; 9:eventout管脚拉低; )
    QByteArray buf645;
    QByteArray buf698;

};

class CtrInfo
{
public:
    CtrInfo();
    ~CtrInfo();


    int ctrlID;
    uchar ccoAddr[6];
    DvcType slotPosition;//槽位
    int dvcId;

    ushort totalMeterCnt;   //电表个数
    ushort totalNodeCnt;    //档案个数
    ushort inNetSuccessCnt;
    double inNetSuccessRate;
    double inNetConsume;
    bool inNetResult;
    ushort successCnt[4];   //应用层业务成功个数(单个业务时，占据[0];多个业务时，[0]代表汇总，具体业务类型依次类推)
    double successRate[4];   //应用层业务成功率(单个业务时，占据[0];多个业务时，[0]代表汇总，具体业务类型依次类推)
    double successConsume[4];   //应用层业务平均耗时(单个业务时，占据[0];多个业务时，[0]代表汇总，具体业务类型依次类推)
    bool testResult[4];     //应用层业务测试结果(单个业务时，占据[0];多个业务时，[0]代表汇总，具体业务类型依次类推)
    bool res_CtrlCmd[20];    //控制命令执行结果标志(0:获取表号; 1:12V上电; 2:12V断电; 3:220V上电; 4:220V断电; 5:12V&220V同时上电; 6:12V&220V同时断电; 7:设置波特率; 8:eventout管脚置高; 9:eventout管脚拉低; )
    ushort sucsCnt_CtrlCmd[20];    //控制命令执行成功个数
    double sucsRate_CtrlCmd[20];    //控制命令执行成功率
    QList<int> keyList;
    QMap<int,MeterInfoForSingleNet*> *p_MeterInfoForSingleNetList;
//    QList<MeterInfoForSingleNet*> *p_MeterInfoForSingleNetList;
    QByteArray buf;
    ////
    QByteArray bufReadCtrlDvc;
    QList<CjqNodeStruct> cjqNodeList;
    int notInParameterCjqNum=0;
    bool cjqAddressAccessNetFlag=false;
};



void memsetDouble(double *buffer, double c, int count);

void Init_AttachedNodeInfo(ushort startIndex, ushort v_meterNum, QList<int> keyList, QMap<int,MeterInfoForSingleNet*> *p_MeterInfoForSingleNetList, shared_ptr<QList<NodeParameter>> p_AttachedNodeInfoList);

void Refresh_CtrInfo_Result_for_CtrlCmdRes(shared_ptr<CtrInfo> p_CtrInfo, DvcType dvcType,int id, CtrlCmdType ctrlCmdType);
void Refresh_CtrInfo_Result_for_AssignAddr(shared_ptr<CtrInfo> p_CtrInfo, int mtrId);
void Refresh_CtrInfo_Result_for_BuildNet(ushort index, shared_ptr<CtrInfo> p_CtrInfo, shared_ptr<Afn10F21> p_QueryTopoInfo_10F21);
//void Refresh_CtrInfo_Result_for_BuildNet_Special(ushort index, CtrInfo *p_CtrInfoList, ReqNetTopoInfo_10F21_Up *p_ReqNetTopoInfo_10F21_Up);
void Refresh_CtrInfo_Result_for_ReadMeter_13F1(shared_ptr<CtrInfo> p_CtrInfo, shared_ptr<Afn13F1> p_Afn13F1);
//void Refresh_CtrInfo_Result_for_ReadMeter_F1F1(CtrInfo *p_CtrInfo, ParallelReadMeter_F1F1_Up *p_ParallelReadMeter_F1F1_Up);
void Init_CtrInfo_Result_for_ReadMeter_F1F1(CtrInfo *p_CtrInfo);
//void Refresh_CtrInfo_Result_for_SearchMeter_06F1(CtrInfo *p_CtrInfo, RptSlaveNodeInfo_06F1_Up *p_RptSlaveNodeInfo_06F1_Up);
//void Refresh_CtrInfo_Result_for_SearchMeter_06F4(CtrInfo *p_CtrInfo, RptSlaveNodeInfoAndDvcType_06F4_Up *p_RptSlaveNodeInfoAndDvcType_06F4_Up);

QString GenerateFailedMeterStr_Net(shared_ptr<CtrInfo> p_CtrInfo);
QString GenerateFailedMeterStr_SiteLeave(shared_ptr<CtrInfo> p_CtrInfo, int cnt);
QString GenerateFailedMeterStr_ProxyChange(shared_ptr<CtrInfo> p_CtrInfo, int cnt);
QString GenerateFailedMeterStr_ReadMeter(shared_ptr<CtrInfo> p_CtrInfo, int readMode);//readMode为1时，代表点抄；readMode为2时，代表点抄+并发；readMode为3时，代表点抄+并发+路由主动
QString GenerateFailedMeterStr_ReadMeterSingleType(shared_ptr<CtrInfo> p_CtrInfo, int readMode);//readMode为1时，代表点抄；readMode为2时，代表并发；readMode为3时，代表路由主动
QString GenerateFailedMeterStr_Others(shared_ptr<CtrInfo> p_CtrInfo);

void comnAddAddrsInfo(shared_ptr<QList<shared_ptr<CtrInfo>>> p_CtrInfoList, const ConcentratorList *p_ConcentratorInfoList, const MeterList *p_MeterInfoList, const SchemeCfgList *p_SchemeCfgInfoList);
void setMeterAddrsPrtcl(shared_ptr<QList<shared_ptr<CtrInfo>>> p_CtrInfoList, uchar prtcl);

QString findDiNameAcordDiValue(uchar *di);
QByteArray prcsOther3762Msg(uchar afn, char dt1,char dt2, uchar msgSeq);
QByteArray prcsOther645Msg(uchar ctrlWord, uchar len, uchar *di, uchar *msgAddr, uchar *assignedAddr, shared_ptr<QList<shared_ptr<CtrInfo>>> p_CtrInfoList=nullptr);
QByteArray prcsOther698Msg(shared_ptr<FrameOOPBase> p_FrameOOPBase, uchar *assignedAddr, shared_ptr<QList<shared_ptr<CtrInfo>>> p_CtrInfoList);

uchar findCommPrtclType(uchar *addr,CtrInfo *p_CtrInfo);
QString calculateConsumeLen(QString start, QString end);
uchar findIndexOfAddr(uchar *addr,CtrInfo *p_CtrInfo);
uchar findMeterType(uchar *addr,shared_ptr<CtrInfo> p_CtrInfo);
QList<int> findDvcIdList(shared_ptr<QList<shared_ptr<CtrInfo> > > p_CtrInfoList, DvcType dvcType);

struct ParallelReadMeter
{
    uchar addr[6];
    QString startTime;
    QString endTime;
    double consumLen;
    QString crntEne;
    QString readRes;
};
ParallelReadMeter findDstMeterAndRemoveIt(QList<ParallelReadMeter> *parallelReadMeterList, uchar dstAddr[], bool *find);
void powerOff220V_CJQ(shared_ptr<QList<shared_ptr<CtrInfo>>> p_CtrInfoList, AbstractScriptHost *p_AbstractScriptHost);
void powerOn220V_CJQ(shared_ptr<QList<shared_ptr<CtrInfo>>> p_CtrInfoList, AbstractScriptHost *p_AbstractScriptHost);
void powerOff12V_STA(shared_ptr<QList<shared_ptr<CtrInfo>>> p_CtrInfoList, AbstractScriptHost *p_AbstractScriptHost);
void powerOn12V_STA(shared_ptr<QList<shared_ptr<CtrInfo>>> p_CtrInfoList, AbstractScriptHost *p_AbstractScriptHost);
void powerOffAll12V(shared_ptr<QList<shared_ptr<CtrInfo>>> p_CtrInfoList, AbstractScriptHost *p_AbstractScriptHost,bool isRealPowerOff=false);
void pullPinReset_CCO(shared_ptr<QList<shared_ptr<CtrInfo>>> p_CtrInfoList, AbstractScriptHost *p_AbstractScriptHost);//sgl
void pullPinReset_STA(shared_ptr<QList<shared_ptr<CtrInfo>>> p_CtrInfoList, AbstractScriptHost *p_AbstractScriptHost);//sgl
uchar get3762Dt(char dt1_,char dt2_);

void delayTime(int msec);

/************************载波协议************************/
class QuerySystemInfo_Down
{
public:
    QuerySystemInfo_Down();
    ~QuerySystemInfo_Down();

    char fixedHead[4];

    uchar prtclVersn;
    uchar msgHeadLen;
    uchar reserve1;

    uchar reserve2;
    uchar dvcType;
    uint msgSeq;
    char srcAddr[6];
    char dstAddr[6];

    static QByteArray encode_QuerySystemInfo_Down(shared_ptr<QuerySystemInfo_Down> p_QuerySystemInfo_Down);
};

class QuerySystemInfo_Up
{
public:
    QuerySystemInfo_Up();
    ~QuerySystemInfo_Up();

    uchar fixedHead[4];

    uchar prtclVersn;
    uchar msgHeadLen;
    uchar ansState;

    uchar reserve;
    uchar dvcType;
    uint msgSeq;
    uchar srcAddr[6];
    uchar dstAddr[6];
    char mnfctrCode[2];
    char chipCode[2];
    char bootVersn;
    char softVersn[4];
    char softVersnTime[7];
    uint versnCSFlashVal;
    uint versnCSCaculValue;
    uchar nodeAddr[6];

    static shared_ptr<QuerySystemInfo_Up> decode_QuerySystemInfo_Up(QByteArray * data);
};



const uchar FIXED_HEAD[4]={0xFE,0xFE,0x0F, 0x00};
const uchar SET_PARAM[4]={0xFE,0xF9,0x0F, 0x00};
const uchar CHK_PARAM[4]={0xFE,0xF8,0x0F, 0x00};

struct ParamInfo
{
    ushort id;
    ushort idLen;
    QByteArray idCntnt;
};
struct ParamSetRlstInfo
{
    ushort id;
    uchar setRes;
};
class SetParam_Down
{
public:
    SetParam_Down();
    ~SetParam_Down();

    char fixedHead[4];

    uchar prtclVersn;
    uchar msgHeadLen;
    uchar reserve1;

    ushort idCnt;
    uint msgSeq;
    char srcAddr[6];
    char dstAddr[6];

    QList<ParamInfo> paramInfoList;

    static QByteArray encode_SetParam_Down(shared_ptr<SetParam_Down> p_SetParam_Down);
};
class SetParam_Up
{
public:
    SetParam_Up();
    ~SetParam_Up();

    uchar fixedHead[4];

    uchar prtclVersn;
    uchar msgHeadLen;
    uchar resStatus;

    ushort idCnt;
    uint msgSeq;
    uchar srcAddr[6];
    uchar dstAddr[6];

    QList<ParamSetRlstInfo> paramSetRlstInfoList;

    static shared_ptr<SetParam_Up> decode_SetParam_Up(QByteArray * data);
};

class ChkParam_Down
{
public:
    ChkParam_Down();
    ~ChkParam_Down();

    char fixedHead[4];

    uchar prtclVersn;
    uchar msgHeadLen;
    uchar reserve1;

    ushort idCnt;
    uint msgSeq;
    char srcAddr[6];
    char dstAddr[6];

    QList<ushort> idList;

    static QByteArray encode_ChkParam_Down(shared_ptr<ChkParam_Down> p_ChkParam_Down);
};
class ChkParam_Up
{
public:
    ChkParam_Up();
    ~ChkParam_Up();

    char fixedHead[4];

    uchar prtclVersn;
    uchar msgHeadLen;
    uchar resStatus;

    ushort idCnt;
    uint msgSeq;
    char srcAddr[6];
    char dstAddr[6];

    QList<ParamInfo> paramInfoList;

    static shared_ptr<ChkParam_Up> decode_ChkParam_Up(QByteArray * data);
};




#endif // COMMONDATATYPE_TESTCASE_H
