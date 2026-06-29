#ifndef INTEROPERATEINIT_GW_H
#define INTEROPERATEINIT_GW_H

#include <QObject>
#include <QTimer>
#include <QDateTime>
#include <QThread>
#include <QDebug>
#include <QtCore/qglobal.h>

#include "CommonData/CommonDataType_TestCase.h"
#include "PublicDataStruct/abstractscript.h"
#include "PublicDataStruct/abstractscripthost.h"
#include "PublicDataStruct/commdatatype.h"
enum InteroperateInit_GW_State
{
    Init,
    Wait_SetBaudRate_Finish,
    Wait_PowerOff12V_Finish,
    Wait_PowerOn12V_Finish,
    Wait_ResetModule_Finish,
    Wait_AssignAddrsFinish,
    InitFinish
};
class InteroperateInit_GW : public QObject
{
    Q_OBJECT
public:
    explicit InteroperateInit_GW(QObject *parent = nullptr);
    InteroperateInit_GW_State emScriptRunState=Init;
    DvcType emDvcType=CCO_GW;
    AbstractScriptHost *p_AbstractScriptHost;
    shared_ptr<QList<shared_ptr<CtrInfo>>> p_CtrInfoList;

    Q_ENUM(DvcType)

    inline static void delayTime(int msec)
    {
        QEventLoop loop;//定义一个新的事件循环
        QTimer::singleShot(msec, &loop, &QEventLoop::quit);//创建单次定时器，槽函数为事件循环的退出函数
        loop.exec();//事件循环开始执行，程序会卡在这里，直到定时时间到，本循环被退出
    }
    inline static void calNewMsg(char msgSeq,QByteArray& msg)
    {
        msg[9]=msgSeq;
        QByteArray tmp=msg.mid(3,msg.size()-5);
        char cs=0x00;
        for(auto i:tmp)
            cs+=i;
        msg[msg.size()-2]=cs;
    }
    bool flagPowerOnSTA=true;
public:
    void execute();
    void stop();
    void setHost(AbstractScriptHost *host);
    bool config(const QMap<QString,QString> *paraDic);
    void setCtrInfoList(shared_ptr<QList<shared_ptr<CtrInfo> > > p_CtrInfoList);
    void processMsg(DvcType dvcType,int id,uchar* data,int datalen);
    void processCtrlDvcRes(DvcType dvcType,QList<int> idList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params);
signals:
    void signalStartFollowTest();
private slots:
    void timer_timeout();
    void maxAllowTimer_timeout();
    void delayTimer_timeout();
private:
    shared_ptr<QTimer> p_timer;
    shared_ptr<QTimer> p_maxAllowTimer;
    shared_ptr<QTimer> p_delayTimer;
    shared_ptr<qgdw_3762_protocol::Frame3762Helper> p_MsgBase_3762;
    shared_ptr<dlt_645_Protocol::Frame645Helper> p_MsgBase_645;
    shared_ptr<dlt_645_Protocol::RspsNormal_ReadAddr_0x93> p_MeterAddrResp_93;
    shared_ptr<GetResponseNormal> p_GetResponseNormal_ReadAddr;
    uchar addr[6]={0x00};

    void processMsgFromCco(DvcType dvcType,int dvcId);
    void processMsgFromMeter645(DvcType dvcType,int dvcId,int mtrlID);
    void processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID);
    void processMsgFromCJQ(DvcType dvcType,int dvcId);
    void sendMsg(DvcType dvcType,int dvcId,int meterID,shared_ptr<void> frame);
    void sendSrcMsg(DvcType dvcType,int dvcId,QByteArray msg);

    void setBaudRate(QString,QString);
    void setPowerOff12V(QString);
    void setPowerOn12V(QString);
    void setResetModule(QString);
    QList<int> getDvcIdList(DvcType dvcType);
};

#endif // INTEROPERATEINIT_GW_H
