#include "Script_BuildNetwork_GW.h"

Script_BuildNetwork_GW::Script_BuildNetwork_GW(QObject *parent) : QObject(parent)
{
    emScriptRunState=Script_BuildNetwork_Init;
    resultFlag=false;
    p_CtrInfoList = make_shared<QList<shared_ptr<CtrInfo>>>();

    p_BuildNetwork_GW = make_shared<BuildNetwork_GW>();
    //connect(p_BuildNetwork_GW.get(),SIGNAL(signalBuildNetFinish()),this,SLOT(slotBuildNetFinish()));

    p_timer = make_shared<QTimer>(this);
    connect(p_timer.get(),SIGNAL(timeout()),this,SLOT(timer_timeoutProc()));
}
Script_BuildNetwork_GW::~Script_BuildNetwork_GW()
{
    p_BuildNetwork_GW->initBuildNetWork();
    if(needPowerOff==true)
        powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost);
    p_timer->stop();
}

void  Script_BuildNetwork_GW::execute()
{
    p_AbstractScriptHost->updateProgress(ProcessState_Start, "Test start!");

    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "等待全网组网完成");
    emScriptRunState=Script_BuildNetwork_Init;
    resultFlag=false;
    p_BuildNetwork_GW->needRebuildNetwork=true;
    p_BuildNetwork_GW->execute();
    emScriptRunState=Wait_BuildNetFinish_Whole;
    p_timer->start((timerForReachThresld+timerAfterReachThresld)*1000);
}

void  Script_BuildNetwork_GW::stop()
{
    if(p_AbstractScriptHost==nullptr)
        return;

    p_timer->stop();
    p_BuildNetwork_GW->stop();

    p_AbstractScriptHost->updateProgress(ProcessState_Error, "Test stop!");
    if(needPowerOff==true)
        powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost);
}

void Script_BuildNetwork_GW::addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq)
{
    p_CtrInfoList->clear();
    comnAddAddrsInfo(p_CtrInfoList, p_ConcentratorInfoList, p_MeterInfoList, p_SchemeCfgInfoList);
    concentratorCnt=ushort(p_CtrInfoList->size());

    uchar freq_temp = freq&0xff;
    p_BuildNetwork_GW->setCtrInfoListAndFreq(p_CtrInfoList, freq_temp);
}
void Script_BuildNetwork_GW::setHost(AbstractScriptHost *host)
{
    p_AbstractScriptHost=host;
    p_BuildNetwork_GW->setHost(host);
}
bool Script_BuildNetwork_GW::config(const QMap<QString,QString> *paraDic)
{
    bool result = false;
    if(paraDic!=nullptr)
    {
        p_BuildNetwork_GW->config(paraDic);

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
        if(paraDic->keys().contains("needBuildNet"))
        {
            if((*paraDic)["needBuildNet"].toLower()=="false")
            {
                this->needBuildNet=false;
            }
            else
            {
                this->needBuildNet=true;
            }
        }
        if(paraDic->keys().contains("needPowerOff"))
        {
            if((*paraDic)["needPowerOff"].toLower()=="false")
            {
                this->needPowerOff=false;
            }
            else
            {
                this->needPowerOff=true;
            }
        }
        result = true;
    }
    return result;
}

void Script_BuildNetwork_GW::processMsg(DvcType dvcType, int id, uchar *data, int datalen)
{
    if(p_CtrInfoList->size()==0)
        return;

    p_BuildNetwork_GW->processMsg(dvcType, id, data, datalen);

    switch(emScriptRunState)
    {
        case Script_BuildNetwork_Init:
        {
            break;
        }
        case Wait_BuildNetFinish_Whole:
        {
            if(p_BuildNetwork_GW->emScriptRunState==BuildNetFinish&&p_BuildNetwork_GW->buildNetworkResultFlag==true)
            {
                p_CtrInfoList->at(0)->inNetResult=true;
                p_timer->stop();
                p_AbstractScriptHost->updateProgress(ProcessState_Success, QString("全网组网成功率：%1%; 全网组网耗时：%2秒;").arg(p_CtrInfoList->at(0)->inNetSuccessRate*100).arg(p_CtrInfoList->at(0)->inNetConsume));
            }
            break;
        }
    }
}

void Script_BuildNetwork_GW::processCtrlDvcRes(DvcType dvcType, QList<int> idList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params)
{
    p_BuildNetwork_GW->processCtrlDvcRes(dvcType, idList, ctrlCmdType, isSucs,  params);
}

void Script_BuildNetwork_GW::slotBuildNetFinish()
{

}

void  Script_BuildNetwork_GW::timer_timeoutProc()
{
    switch(emScriptRunState)
    {
        case Wait_BuildNetFinish_Whole:
        {
            p_CtrInfoList->at(0)->inNetResult=false;
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("全网组网成功率：%1%").arg(p_CtrInfoList->at(0)->inNetSuccessRate*100));
            break;
        }
        default:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==p_timer");
            break;
        }
    }
}
