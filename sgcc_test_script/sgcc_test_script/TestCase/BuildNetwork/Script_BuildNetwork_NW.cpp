#include "Script_BuildNetwork_NW.h"

Script_BuildNetwork_NW::Script_BuildNetwork_NW(QObject *parent) : QObject(parent)
{
    emScriptRunState=Script_BuildNetwork_Init;
    resultFlag=false;
//    p_CtrInfoList=new QList<CtrInfo*>();
    p_CtrInfoList = make_shared<QList<shared_ptr<CtrInfo>>>();

    p_BuildNetwork_NW = make_shared<BuildNetwork_NW>();
    connect(p_BuildNetwork_NW.get(),SIGNAL(signalBuildNetFinish()),this,SLOT(slotBuildNetFinish()));

    p_timer = make_shared<QTimer>(this);
    connect(p_timer.get(),SIGNAL(timeout()),this,SLOT(timer_timeoutProc()));
}

Script_BuildNetwork_NW::~Script_BuildNetwork_NW()
{
    p_BuildNetwork_NW->initBuildNetWork();
    powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost);

//    delete p_BuildNetwork_NW;
//    p_BuildNetwork_NW=nullptr;

//    if(p_timer==nullptr)
//        return;

    p_timer->stop();
//    delete p_timer;
//    p_timer=nullptr;

//    qDeleteAll(p_CtrInfoList->begin(),p_CtrInfoList->end());
//    p_CtrInfoList->clear();
//    delete p_CtrInfoList;
//    p_CtrInfoList=nullptr;
}

void  Script_BuildNetwork_NW::execute()
{
    p_AbstractScriptHost->updateProgress(ProcessState_Start, "Test start!");

    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "等待全网组网完成");
    emScriptRunState=Script_BuildNetwork_Init;
    resultFlag=false;

    p_BuildNetwork_NW->execute();
    emScriptRunState=Wait_BuildNetFinish_Whole;  
    p_timer->start((timerForReachThresld+timerAfterReachThresld)*1000);
}

void  Script_BuildNetwork_NW::stop()
{
    if(p_AbstractScriptHost==nullptr)
        return;

    p_timer->stop();
    p_BuildNetwork_NW->stop();

    p_AbstractScriptHost->updateProgress(ProcessState_Error, "Test stop!");
    powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost);
}

void Script_BuildNetwork_NW::addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq)
{
    p_CtrInfoList->clear();
    comnAddAddrsInfo(p_CtrInfoList, p_ConcentratorInfoList, p_MeterInfoList, p_SchemeCfgInfoList);
    concentratorCnt=ushort(p_CtrInfoList->size());

    uchar freq_temp = freq&0xff;
    p_BuildNetwork_NW->setCtrInfoListAndFreq(p_CtrInfoList, freq_temp);
}
void  Script_BuildNetwork_NW::setHost(AbstractScriptHost *host)
{
    p_AbstractScriptHost=host;
    p_BuildNetwork_NW->setHost(host);
}
bool  Script_BuildNetwork_NW::config(const QMap<QString,QString> *paraDic)
{
    bool result = false;
    if(paraDic!=nullptr)
    {
        p_BuildNetwork_NW->config(paraDic);

        if(paraDic->keys().contains("timerForReachThresld"))
        {
            this->timerForReachThresld = (*paraDic)["timerForReachThresld"].toUShort();
        }
        if(paraDic->keys().contains("netSucRateThresld"))
        {
            this->netSucRateThresld = (*paraDic)["netSucRateThresld"].toDouble();
        }
        if(paraDic->keys().contains("timerAfterReachThresld"))
        {
            this->timerAfterReachThresld = (*paraDic)["timerAfterReachThresld"].toUShort();
        }
        result = true;
    }
    return result;
}

void Script_BuildNetwork_NW::processMsg(DvcType dvcType, int id, uchar *data, int datalen)
{
    if(p_CtrInfoList->size()==0)
        return;

    p_BuildNetwork_NW->processMsg(dvcType, id, data, datalen);


    switch(emScriptRunState)
    {
    case Script_BuildNetwork_Init:
    {
        break;
    }
    case Wait_BuildNetFinish_Whole:
    {
//        if(p_BuildNetwork_NW->resultFlag)
//        {
//            //p_CtrInfoList->at(0)->inNetConsume=(double)((timerForReachThresld+timerAfterReachThresld)*1000-p_timer->remainingTime())/1000.0;
//            p_CtrInfoList->at(0)->inNetResult=true;
//            p_timer->stop();
//            p_AbstractScriptHost->updateProgress(ProcessState_Success, QString("  全网组网成功率：%1%; 全网组网耗时：%2秒;").arg(p_CtrInfoList->at(0)->inNetSuccessRate*100).arg(p_CtrInfoList->at(0)->inNetConsume));
//        }
        break;
    }
//    default:
//    {
//        p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==processCollectorMsg");
//        break;
//    }
    }
}

void Script_BuildNetwork_NW::processCtrlDvcRes(DvcType dvcType, QList<int> idList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params)
{
    p_BuildNetwork_NW->processCtrlDvcRes(dvcType, idList, ctrlCmdType, isSucs,  params);
}

void Script_BuildNetwork_NW::slotBuildNetFinish()
{
    p_CtrInfoList->at(0)->inNetResult=true;
    p_timer->stop();
    p_AbstractScriptHost->updateProgress(ProcessState_Success, QString("  全网组网成功率：%1%; 全网组网耗时：%2秒;").arg(p_CtrInfoList->at(0)->inNetSuccessRate*100).arg(p_CtrInfoList->at(0)->inNetConsume));
}

void  Script_BuildNetwork_NW::timer_timeoutProc()
{
    switch(emScriptRunState)
    {
    case Wait_BuildNetFinish_Whole:
    {
        p_CtrInfoList->at(0)->inNetResult=false;
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("  全网组网成功率：%1%").arg(p_CtrInfoList->at(0)->inNetSuccessRate*100));
        break;
    }
    default:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==p_timer");
        break;
    }
    }
}
