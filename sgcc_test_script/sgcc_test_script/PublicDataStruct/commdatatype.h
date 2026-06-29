
#ifndef COMMDATATYPE_H
#define COMMDATATYPE_H

#include <QString>
#include <QDateTime>
#include <QSet>
#include <QMap>
#include <QtSerialPort>

const uchar SysIniInfo_Cnts=12;
const uchar DvcSerialInfo_Colums=7;
const uchar DvcIpAndPortInfo_Colums=4;
const uchar ConcentratorInfo_Colums=4;
const uchar MeterInfo_Colums=8;
const uchar SchemeCfgInfo_Colums=3;
const uchar TestCaseInfo_Colums=6;

const uchar WaitDvcReadyTimeLen=0;

#define Dvc_Slot_Position_CCO_GW           QString("国网")
#define Dvc_Slot_Position_CCO_NW           QString("南网")
#define Dvc_Slot_Position_STA_Single       QString("单通")
#define Dvc_Slot_Position_STA_Three        QString("三通")
#define Dvc_Slot_Position_CJQ              QString("采集器")
#define Dvc_Slot_Position_Meter_Single     QString("单相表")
#define Dvc_Slot_Position_Meter_Three      QString("三相表")
#define Dvc_Slot_Position_Meter_CJQ        QString("采集器表")

#define DvcName_ReadCtrlDvc_SouthernState       QString("南网抄控器")
#define DvcName_ReadCtrlDvc_SG       QString("国网抄控器")

enum TestStatus
{
    InTestProcess,
    NotInTestProcess
};
enum DvcType
{
    SingleSTA,
    ThreeSTA,
    CCO_GW,
    CCO_NW,
    CJQ,
    Dvc_Board,
    SingleMeter,
    ThreeMeter,
    CJQMeter,
    ReadCtrlDvc,
};
enum CtrlCmdType
{
    CtrlCmd_PowerOn_12V,
    CtrlCmd_PowerOff_12V,
    CtrlCmd_PowerOn_220V,
    CtrlCmd_PowerOff_220V,
    CtrlCmd_PowerOnAll,
    CtrlCmd_PowerOffAll,
    CtrlCmd_SetBaudRate,
    CtrlCmd_EventPinHigh,
    CtrlCmd_EventPinLow,
    CtrlCmd_ModuleRST,
    CtrlCmd_DeviceRST,
    ReadCtrlDvc_SetBaudRate
};
enum ProcessState
{
    ProcessState_Init,
    ProcessState_Start,
    ProcessState_Processing,
    ProcessState_Error,
    ProcessState_Success,
    ProcessState_Failed,
};
enum TestCaseResult
{
    TestCaseResult_PASS,
    TestCaseResult_FAIL,
    TestCaseResult_ERROR,
    TestCaseResult_UNKNOW
};


struct SysIniInfo
{
    int TOTAL_TEST_TIMES;
    QString LOG_LEVEL;
    bool B_SAVEMSG;
    bool B_SHOWMSG;
    bool B_WINDOWSSHOW;
    int FREQ_BAND;
    QString MsgSaveFileName;
    QString COMPANY_NAME;
    QStringList CompanyNameList;
    QString ChipCompanay;
    QStringList ChipCompanyNameList;
    QString ArticleBatchNumber;
};

struct DvcSerial
{
    int dvcId;
    QString dvcName;
    QString portName;
    uint baudRate;
    QSerialPort::DataBits dataBits;
    QSerialPort::StopBits stopBits;
    QSerialPort::Parity parity;
};

struct DvcIpAndPort
{
    int dvcId;
    QString dvcName;
    QString ip;
    ushort port;
};

struct ConcentratorInfo
{
    int ctrID;
    uchar ccoAddr[6];
    DvcType slotPosition;
    int dvcId;
};

struct MeterInfo
{
    int mtrID;
    uchar mtrAddr[6];
    DvcType slotPosition;
    uchar realPhase;
    uchar phaseSeq;
    uchar prtcl;
    uchar CJQAddr[6];
    int dvcId;
};

struct SchemeCfgInfo
{
    int schmID;
    int ctrID;
    QList<int> mtrIDs;
};


struct TestCaseInfo
{
    int tcID;
    QString tcName;
    QSet<int> schmIDs;
    QString tcDLLName;

    QString startTime;
    QString endTime;
    TestCaseResult result;
    QString resultInfo;
    QString paramFileName;
    QMap<QString,QString> paraDic;
    QString catalogueName;
};

typedef QList<DvcSerial> DvcSerialList;
typedef QList<DvcIpAndPort> DvcIpAndPortList;
typedef QList<ConcentratorInfo> ConcentratorList;
typedef QList<MeterInfo> MeterList;
typedef QList<SchemeCfgInfo> SchemeCfgList;
typedef QList<TestCaseInfo> TestCaseList;



bool rvrsAddr(uchar *src, uchar *des, uchar len);
QByteArray reverseArray(QByteArray array);
void reverseAddr(uchar *addr, int len);
bool isArrayEqual(const uchar *dst, uchar *src, uchar len);
uchar dec2hex(uchar value);
uint bcd2hex(uint value);  //形式上的bcd转换成相应的16进制，12345678->0x12345678

uchar calcCs(QByteArray msg);




#endif // COMMDATATYPE_H
