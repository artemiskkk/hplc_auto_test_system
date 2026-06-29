#include "InteroperateBase_GW.h"

InteroperateBase_GW::InteroperateBase_GW(QObject *parent) : QObject(parent)
{
    p_InteroperateInit_GW=make_shared<InteroperateInit_GW>();
    connect(p_InteroperateInit_GW.get(),&InteroperateInit_GW::signalStartFollowTest,this,&InteroperateBase_GW::slotStartFollowTest);
    p_timer=new QTimer(this);
    connect(p_timer,&QTimer::timeout,this,&InteroperateBase_GW::timer_timeout);
    p_delayTimer=new QTimer(this);
    connect(p_delayTimer,&QTimer::timeout,this,&InteroperateBase_GW::delayTimer_timeout);
    p_maxAllowTimer=new QTimer(this);
    connect(p_maxAllowTimer,&QTimer::timeout,this,&InteroperateBase_GW::maxAllowTimer_timeout);

    p_CtrInfoList=make_shared<QList<shared_ptr<CtrInfo>>>();
    p_Frame3762Helper=make_shared<Frame3762Helper>();
    p_Frame645Helper=make_shared<Frame645Helper>();
    p_MeterAddrResp_93=make_shared<RspsNormal_ReadAddr_0x93>(addr,6);
    p_FrameOOPHelper=make_shared<FrameOOPHelper>();
    p_GetResponseNormal_ReadAddr=make_shared<GetResponseNormal>();
}

InteroperateBase_GW::~InteroperateBase_GW()
{

}
void InteroperateBase_GW::sendSrcMsg(DvcType dvcType, int dvcId, QByteArray msg)
{
    if(msg.size()<1)
        return;
    uchar *sendMsg=new uchar[uint(msg.size())];
    memcpy(sendMsg,reinterpret_cast<uchar*>(msg.data()),uint(msg.size()));
    p_AbstractScriptHost->sendMsg2Dvc(dvcType,dvcId,sendMsg,msg.size());

    QStringList dvcList;
    dvcList<<"单通"<<"三通"<<"国网路由"<<"南网路由"<<"采集器";
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("脚本给%1发送报文：%2").arg(dvcList.at(int(dvcType))).arg(QString(msg.toHex())));
//    QTimer::singleShot(1000,this,[=]{delete [] sendMsg;});
}

void InteroperateBase_GW::sendSrcMsg(DvcType dvcType, int dvcId, QString msgStr)
{
    QByteArray msg=QByteArray::fromHex(msgStr.toLatin1());
    sendSrcMsg(dvcType,dvcId,msg);
}

void InteroperateBase_GW::slotStartFollowTest()
{

}

//void InteroperateBase_GW::timer_timeout()
//{

//}

//void InteroperateBase_GW::maxAllowTimer_timeout()
//{

//}

//void InteroperateBase_GW::delayTimer_timeout()
//{

//}
