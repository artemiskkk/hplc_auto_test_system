#ifndef ERASEWRITEPROGRAM_H
#define ERASEWRITEPROGRAM_H

#include <QObject>
#include "../BuildNetwork_GW.h"
class EraseWriteProgram : public QObject
{
    Q_OBJECT
public:
    explicit EraseWriteProgram(shared_ptr<QList<shared_ptr<CtrInfo>>> p_CtrInfoList,AbstractScriptHost *host,DvcType type=CCO_GW,QObject *parent = nullptr);
    ~EraseWriteProgram();
    enum EraseWriteProgram_State
    {
        ScriptInit,
        Wait_DeviceInit_Finish,
        Wait_ModeChange_Finish,
        Wait_UnlockFlash_Finish,
        Wait_EarseFlash_Finish,
        Wait_WriteProgram_Finish,
        Wait_ReadVersion_Finish,
        ScriptSuccess
    };
private:
    Q_ENUM(EraseWriteProgram_State)
    EraseWriteProgram_State emScriptRunState=ScriptInit;
    QMetaEnum metaEnum=QMetaEnum::fromType<EraseWriteProgram_State>();
    AbstractScriptHost *p_AbstractScriptHost;
    shared_ptr<Afn03F1> p_Afn03F1=nullptr;
    shared_ptr<Frame3762Helper> p_Frame3762Helper;
    shared_ptr<Frame645Helper> p_Frame645Helper;
    shared_ptr<RspsNormal_ReadAddr_0x93> p_MeterAddrResp_93;
    shared_ptr<FrameOOPHelper> p_FrameOOPHelper;
    shared_ptr<GetResponseNormal> p_GetResponseNormal_ReadAddr;

    shared_ptr<QList<shared_ptr<CtrInfo>>> p_CtrInfoList;
    QTimer *p_timer;//命令发送超时定时器
    QTimer *p_maxAllowTimer;//脚本最大运行时间定时器
    QTimer *p_delayTimer;//命令延迟定时器

    QByteArray sendMsgOct;//发送
    QString sendMsgLog;
    QString logMsgStr;
    uchar addr[6]={0x00};
    int tryTimes=0;//命令重试次数
    uchar msgSeq=0;//帧序号
    int index=0;//一般用于索引表号

    DvcType m_dvcType=CCO_GW;//默认国网
    const int m_baud=115200;//
    const char m_parity=0x00;//无校验
    QTimer *p_repeatTimer;//重发定时器

    char address[4]={0x00,0x00,0x00,0x00};
    uint length=0x00400000;
    uchar freq=0x02;
signals:
    void signalNoticeWriteState(ProcessState,const QString& state, uchar freq=0x02);
public:
    //脚本接口函数，测试系统通过这些接口函数与脚本进行交互
    void  addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq);//添加档案信息
    void  setHost(AbstractScriptHost *host);//设置脚本抽象类，定义与测试系统之间的交互接口，脚本发送消息给测试系统
    void  execute();//脚本启动执行
    void  processMsg(DvcType dvcType,int id,uchar* data,int datalen);//处理从测试系统传入的五种槽位的数据信息
    void  processCtrlDvcRes(DvcType dvcType,QList<int> idList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params);//处理从测试系统传入的关于工装控制的回复信息

    static void delay(int ms)
    {
        QEventLoop loop;
        QTimer::singleShot(ms,&loop,&QEventLoop::quit);
        loop.exec();
    }
private:
    void processMsgFromCCO(DvcType dvcType, int dvcId);//处理来自CCO槽位的报文
    void processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID);//处理来自槽位的645报文
    void processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID);//处理来自槽位的OOP报文
    void sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame);//发送报文到测试系统，主要处理状态机内的报文发送
    void sendSrcMsg(DvcType dvcType,int dvcId,QByteArray msg);//处理其他报文的发送，如其它事件上报时，处理之后的报文通过此处发送
    QByteArray getChangeModeCMD();
    QByteArray getUnLockCMD();
    QByteArray getWriteCMD(QByteArray data);
    QByteArray getReadCMD();
    QByteArray getEarseCMD();
    QByteArray upgradeFileData();
    void writeData();
private slots:
    void timer_timeoutProc();//命令发送超时定时器槽函数
    void maxAllowTimer_timeoutProc();//脚本最大运行时间定时器槽函数
    void delayTimer_timeoutProc();//命令延迟定时器槽函数
};

#endif // ERASEWRITEPROGRAM_H
