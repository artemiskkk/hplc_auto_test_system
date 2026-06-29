#include "Script_RefuseListReport_ResetPoweroff.h"

Script_RefuseListReport_ResetPoweroff::Script_RefuseListReport_ResetPoweroff(QObject *parent) : QObject(parent)
{
    emScriptRunState=TestInit;
    resultFlag=false;
    sendMsgOct.clear();

    p_CtrInfoList=make_shared<QList<shared_ptr<CtrInfo>>>();
    p_BuildNetwork_GW=new BuildNetwork_GW();

    p_Frame3762Helper=make_shared<Frame3762Helper>();
    p_RefuseNodeReport_05F200=make_shared<Afn05F200>();
    p_Confirm_00F1=make_shared<Afn00F1>();
    p_HardInit_01F1_Down=make_shared<Afn01F1>();

    p_Frame645Helper=make_shared<Frame645Helper>();
    p_MeterAddrResp_93=make_shared<RspsNormal_ReadAddr_0x93>(addr,6);
    p_FrameOOPHelper=make_shared<FrameOOPHelper>();
    p_GetResponseNormal_ReadAddr=make_shared<GetResponseNormal>();

    p_timer=new QTimer();
    p_maxAllowTimer=new QTimer();
    p_delayTimer=new QTimer();
    connect(p_timer,SIGNAL(timeout()),this,SLOT(timer_timeoutProc()));
    connect(p_maxAllowTimer,SIGNAL(timeout()),this,SLOT(maxAllowTimer_timeoutProc()));
    connect(p_delayTimer,SIGNAL(timeout()),this,SLOT(delayTimer_timeoutProc()));
}
Script_RefuseListReport_ResetPoweroff::~Script_RefuseListReport_ResetPoweroff()
{
    p_BuildNetwork_GW->initBuildNetWork();
    if(needPowerOff==true)
        powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost);
    p_BuildNetwork_GW->initBuildNetWork();
    delete p_BuildNetwork_GW;
    p_BuildNetwork_GW=nullptr;

    if(p_timer==nullptr)
        return;
    p_timer->stop();
    delete p_timer;
    p_timer=nullptr;

    delete p_maxAllowTimer;
}
void Script_RefuseListReport_ResetPoweroff::execute()
{
    p_AbstractScriptHost->updateProgress(ProcessState_Start, "Test start!");

    resultFlag=false;

    if(needBuildNet==true)
    {
        p_BuildNetwork_GW->needRebuildNetwork=true;
        p_BuildNetwork_GW->execute();
        emScriptRunState=Wait_BuildNetFinish_Whole;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "等待全网组网完成");
    }
    else
    {
//        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_ActiveRegister_11F5);
//        emScriptRunState=Wait_SearchMeter_11F5_Finish;
//        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--激活主动注册11F5(2min)，等待--确认");
//        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
     //   p_delayTimer->start((activeTime+2)*60*1000);//2min
    }
    p_maxAllowTimer->start((timerForReachThresld+timerAfterReachThresld)*1000);
}
void Script_RefuseListReport_ResetPoweroff::stop()
{
    if(p_AbstractScriptHost==nullptr)
        return;
    p_AbstractScriptHost->updateProgress(ProcessState_Error, "Test stop!");
    if(needPowerOff==true)
        powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost);
}
void Script_RefuseListReport_ResetPoweroff::addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq)
{
    p_CtrInfoList->clear();
    comnAddAddrsInfo(p_CtrInfoList, p_ConcentratorInfoList, p_MeterInfoList, p_SchemeCfgInfoList);
    concentratorCnt=ushort(p_CtrInfoList->size());
    uchar dstFreq=freq&0x0f;
    uchar dstPrtcl=uchar(freq)>>4;
    if(dstPrtcl==0x03)
        setMeterAddrsPrtcl(p_CtrInfoList,dstPrtcl);

    p_BuildNetwork_GW->setCtrInfoListAndFreq(p_CtrInfoList, dstFreq);
}
void Script_RefuseListReport_ResetPoweroff::setHost(AbstractScriptHost *host)
{
    p_AbstractScriptHost=host;
    p_BuildNetwork_GW->setHost(host);
}
bool Script_RefuseListReport_ResetPoweroff::config(const QMap<QString,QString> *paraDic)
{
    bool result = false;
    if(paraDic!=nullptr)
    {
        p_BuildNetwork_GW->config(paraDic);

        if(paraDic->keys().contains("timerForReachThresld"))
        {
            this->timerForReachThresld = (*paraDic)["timerForReachThresld"].toUShort();
        }
        if(paraDic->keys().contains("timerAfterReachThresld"))
        {
            this->timerAfterReachThresld = (*paraDic)["timerAfterReachThresld"].toUShort();
        }

        if(paraDic->keys().contains("netSucRateThresld"))
        {
            this->netSucRateThresld = (*paraDic)["netSucRateThresld"].toDouble();
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
        if(paraDic->keys().contains("resetNum"))
        {
            this->resetNum = (*paraDic)["resetNum"].toUShort();
        }
        if(paraDic->keys().contains("poweroffNum"))
        {
            this->poweroffNum = (*paraDic)["poweroffNum"].toUShort();
        }
        result = true;
    }
    QSettings property(QString("PropertyConfig.ini"),QSettings::IniFormat);
    chipType=property.value("SYSTEM_PROPERTY/ChipType","GY").toString();
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("chipType=%1").arg(chipType));
    return result;
}
void Script_RefuseListReport_ResetPoweroff::processMsg(DvcType dvcType, int id, uchar *data, int datalen)
{
    if(p_CtrInfoList->size()==0)
        return;
    if(emScriptRunState==Wait_BuildNetFinish_Whole)
    {
        if(!p_BuildNetwork_GW->startBuildNetFlag)
            p_BuildNetwork_GW->processMsg(dvcType,id,data,datalen);
        else
        {
            p_BuildNetwork_GW->initBuildNetWork();
         //   p_timer->stop();
            emScriptRunState=EnableRefuseListReport;
            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_RefuseNodeReport_05F200);
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--允许拒绝节点信息上报05F200，等待--确认");
            p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
        }
    }
    else
    {
        QByteArray tmpRecvTempData=QByteArray::fromRawData(reinterpret_cast<char*>(data),datalen);
        QByteArray recvTempData;
        recvTempData.append(tmpRecvTempData);
        delete[] data;

        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到报文：%1").arg(QString(recvTempData.toHex())));

        if(dvcType==CCO_GW || dvcType==CCO_NW)
        {
            if(dvcType == p_CtrInfoList->at(0)->slotPosition && id == p_CtrInfoList->at(0)->dvcId)
            {
                p_CtrInfoList->at(0)->buf.append(recvTempData);
                processMsgFromCCO(dvcType,id);
            }
        }
        else if(dvcType==SingleSTA || dvcType==ThreeSTA)
        {
            for(int i=0; i<p_CtrInfoList->at(0)->keyList.size(); i++)
            {
                if(dvcType == p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->value(p_CtrInfoList->at(0)->keyList.at(i))->slotPosition
                        && id == p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->value(p_CtrInfoList->at(0)->keyList.at(i))->dvcId)
                {
                    if((*p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList)[p_CtrInfoList->at(0)->keyList.at(i)]->prtcl==0x02)
                    {
                        (*p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList)[p_CtrInfoList->at(0)->keyList.at(i)]->buf645.append(recvTempData);
                        processMsgFromMeter645(dvcType,id,p_CtrInfoList->at(0)->keyList.at(i));
                        break;
                    }
                    else if((*p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList)[p_CtrInfoList->at(0)->keyList.at(i)]->prtcl==0x03)
                    {
                        (*p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList)[p_CtrInfoList->at(0)->keyList.at(i)]->buf698.append(recvTempData);
                        processMsgFromMeterOOP(dvcType,id,p_CtrInfoList->at(0)->keyList.at(i));
                        break;
                    }
                }
            }
        }
        else if(dvcType==CJQ)
        {

        }
        else
        {
            return;
        }
    }
}
void Script_RefuseListReport_ResetPoweroff::processCtrlDvcRes(DvcType dvcType, QList<int> idList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params)
{
    if(isSucs==false)
        return;
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到控制命令回复，参数个数=%1").arg(QString::number(params.size())));
    QList<int> sendParams;
    switch(emScriptRunState)
    {
        case TestInit:
        {
            break;
        }
        case Wait_BuildNetFinish_Whole:
        {
            p_BuildNetwork_GW->processCtrlDvcRes(dvcType,idList,ctrlCmdType,isSucs,params);
            break;
        }
        case Wait_Poweroff1_finish:
        {
            if(chipType=="V3B"&&powerOnFlag)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("V3B路由，等待10s进行操作"));
                QThread::sleep(10);
                if(++poweroffNo>poweroffNum)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("路由断上电结束，开始等待拒绝列表上报"));
                    emScriptRunState=WaitReportAfterPoweroff;
                    p_delayTimer->start(5*60*1000);
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("开始断上电路由第%0次，路由断电5s").arg(poweroffNo));
                    QList<double> sendParams;
                    QList<int> idList;
                    sendParams.clear();
                    idList.clear();
                    idList = findDvcIdList(p_CtrInfoList,CCO_GW);
                    p_AbstractScriptHost->controlDvc(CCO_GW,idList,CtrlCmd_PowerOff_12V,sendParams);
                    QThread::msleep(50);
                    p_timer->start(5*1000);
                }
            }
            break;
        }
        case Wait_Poweroff2_finish:
        {
            if(chipType=="V3B"&&powerOnFlag)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("V3B路由，等待10s进行操作"));
                QThread::sleep(10);
                if(++poweroffNo>poweroffNum)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("路由断上电结束，等待5min看是否有拒绝列表上报"));
                    p_delayTimer->start(5*60*1000);
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("开始断上电路由第%0次，路由断电5s").arg(poweroffNo));
                    QList<double> sendParams;
                    QList<int> idList;
                    sendParams.clear();
                    idList.clear();
                    idList = findDvcIdList(p_CtrInfoList,CCO_GW);
                    p_AbstractScriptHost->controlDvc(CCO_GW,idList,CtrlCmd_PowerOff_12V,sendParams);
                    QThread::msleep(50);
                    p_timer->start(5*1000);
                }
            }
            break;
        }
        default:
        {
            break;
        }
    }
}

void Script_RefuseListReport_ResetPoweroff::processMsgFromCCO(DvcType dvcType, int dvcId)
{
    if(dvcId!=p_CtrInfoList->at(0)->ctrlID)
        return;
    bool haveCompleteMsg=true;
    while(haveCompleteMsg)
    {
        qInfo()<<QString("CCO-解析前 buf3762=%1").arg(QString((p_CtrInfoList->at(0)->buf.toHex())));
        shared_ptr<Frame3762Base> p_Frame3762Base = p_Frame3762Helper->DecodeLocalMsg(&(p_CtrInfoList->at(0)->buf),haveCompleteMsg);
        qInfo()<<QString("CCO-解析后 buf3762=%1").arg(QString((p_CtrInfoList->at(0)->buf.toHex())));
        if(p_Frame3762Base==nullptr)
        {
            continue;
        }
        msgSeq=uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq);
        switch(emScriptRunState)
        {
        case TestInit:
        {
            break;
        }
        case Wait_BuildNetFinish_Whole:
        {
            break;
        }
        case EnableRefuseListReport:
        {
            if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "允许拒绝节点信息上报回复确认");

                resetNo=1;
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_HardInit_01F1_Down);
                emScriptRunState=Wait_Reset1_finish;
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("开始硬件初始化第%0次").arg(resetNo));
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case Wait_Reset1_finish:
        {
            if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "硬件初始化回复确认");
            }
            else if(p_Frame3762Base->afn_ == 0x03&&p_Frame3762Base->dt1_==0x02&&p_Frame3762Base->dt2_==0x01&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("收到03F10上报"));
                if(++resetNo>resetNum)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("硬件初始化结束，开始等待拒绝列表上报"));
                    emScriptRunState=WaitReportAfterReset;
                    p_delayTimer->start(5*60*1000);
                }
                else
                {
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_HardInit_01F1_Down);
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("开始硬件初始化第%0次").arg(resetNo));
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                }
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case WaitReportAfterReset:
        {
            if(p_Frame3762Base->afn_ == 0x06&&p_Frame3762Base->dt1_==0x10&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                shared_ptr<Afn06F5> p_EventReport_06F5_Up=dynamic_pointer_cast<Afn06F5>(p_Frame3762Base);
                if(p_EventReport_06F5_Up->report_event_unit_.node_protocol_type_==0x05)
                {
                    p_delayTimer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到06F5拒绝列表事件上报"));
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_Confirm_00F1);
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "回复确认");

                    emScriptRunState=Wait_Poweroff1_finish;
                    poweroffNo=1;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("开始断上电路由第%0次，路由断电5s").arg(poweroffNo));
                    QList<double> sendParams;
                    powerOnFlag=false;
                    QList<int> idList;
                    sendParams.clear();
                    idList.clear();
                    idList = findDvcIdList(p_CtrInfoList,CCO_GW);
                    p_AbstractScriptHost->controlDvc(CCO_GW,idList,CtrlCmd_PowerOff_12V,sendParams);
                    QThread::msleep(50);
                    p_timer->start(5*1000);
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到06F5其他事件上报"));
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_Confirm_00F1);
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "回复确认");
                }   
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case Wait_Poweroff1_finish:
        {
            if(p_Frame3762Base->afn_ == 0x03&&p_Frame3762Base->dt1_==0x02&&p_Frame3762Base->dt2_==0x01&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("收到路由上电03F10上报"));

                if(++poweroffNo>poweroffNum)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("路由断上电结束，开始等待拒绝列表上报"));
                    emScriptRunState=WaitReportAfterPoweroff;
                    p_delayTimer->start(5*60*1000);
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("开始断上电路由第%0次，路由断电5s").arg(poweroffNo));
                    QList<double> sendParams;
                    QList<int> idList;
                    sendParams.clear();
                    idList.clear();
                    idList = findDvcIdList(p_CtrInfoList,CCO_GW);
                    p_AbstractScriptHost->controlDvc(CCO_GW,idList,CtrlCmd_PowerOff_12V,sendParams);
                    QThread::msleep(50);
                    p_timer->start(5*1000);
                }
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case WaitReportAfterPoweroff:
        {
            if(p_Frame3762Base->afn_ == 0x06&&p_Frame3762Base->dt1_==0x10&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                shared_ptr<Afn06F5> p_EventReport_06F5_Up=dynamic_pointer_cast<Afn06F5>(p_Frame3762Base);
                if(p_EventReport_06F5_Up->report_event_unit_.node_protocol_type_==0x05)
                {
                    p_delayTimer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到06F5拒绝列表事件上报"));
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_Confirm_00F1);
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "回复确认");

                    emScriptRunState=DisableRefuseListReport;
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_RefuseNodeReport_05F200);
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--禁止拒绝节点信息上报05F200，等待--确认");
                    p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到06F5其他事件上报"));
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_Confirm_00F1);
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "回复确认");
                }
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case DisableRefuseListReport:
        {
            if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "禁止拒绝节点信息上报回复确认");

                resetNo=1;
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_HardInit_01F1_Down);
                emScriptRunState=Wait_Reset2_finish;
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("开始硬件初始化第%0次").arg(resetNo));
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case Wait_Reset2_finish:
        {
            if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "硬件初始化回复确认");
            }
            else if(p_Frame3762Base->afn_ == 0x03&&p_Frame3762Base->dt1_==0x02&&p_Frame3762Base->dt2_==0x01&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("收到03F10上报"));
                if(++resetNo>resetNum)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("硬件初始化结束，等待5min看是否有拒绝列表上报"));
                    p_delayTimer->start(5*60*1000);
                }
                else
                {
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_HardInit_01F1_Down);
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("开始硬件初始化第%0次").arg(resetNo));
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                }
            }
            else if(p_Frame3762Base->afn_ == 0x06&&p_Frame3762Base->dt1_==0x10&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                shared_ptr<Afn06F5> p_EventReport_06F5_Up=dynamic_pointer_cast<Afn06F5>(p_Frame3762Base);
                if(p_EventReport_06F5_Up->report_event_unit_.node_protocol_type_==0x05)
                {
                    p_delayTimer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("禁止拒绝列表上报后收到拒绝列表上报"));
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到06F5其他事件上报"));
                }
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_Confirm_00F1);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "回复确认");
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case Wait_Poweroff2_finish:
        {
            if(p_Frame3762Base->afn_ == 0x03&&p_Frame3762Base->dt1_==0x02&&p_Frame3762Base->dt2_==0x01&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("收到路由上电03F10上报"));

                if(++poweroffNo>poweroffNum)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("路由断上电结束，等待5min看是否有拒绝列表上报"));
                    p_delayTimer->start(5*60*1000);
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("开始断上电路由第%0次，路由断电5s").arg(poweroffNo));
                    QList<double> sendParams;
                    QList<int> idList;
                    sendParams.clear();
                    idList.clear();                    
                    powerOnFlag=false;
                    idList = findDvcIdList(p_CtrInfoList,CCO_GW);
                    p_AbstractScriptHost->controlDvc(CCO_GW,idList,CtrlCmd_PowerOff_12V,sendParams);
                    QThread::msleep(50);
                    p_timer->start(5*1000);
                }
            }
            else if(p_Frame3762Base->afn_ == 0x06&&p_Frame3762Base->dt1_==0x10&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                shared_ptr<Afn06F5> p_EventReport_06F5_Up=dynamic_pointer_cast<Afn06F5>(p_Frame3762Base);
                if(p_EventReport_06F5_Up->report_event_unit_.node_protocol_type_==0x05)
                {
                    p_delayTimer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("禁止拒绝列表上报后收到拒绝列表上报"));
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到06F5其他事件上报"));
                }
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_Confirm_00F1);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "回复确认");
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case ScriptSuccess:
        {
            break;
        }
        }
    }
}
void Script_RefuseListReport_ResetPoweroff::processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID)
{
    bool haveCompleteMsg=true;
    while(haveCompleteMsg)
    {
        shared_ptr<dlt_645_Protocol::Frame645Base> MsgBase_645_ptr = dlt_645_Protocol::Frame645Helper::DecodeLocalMsg(&((*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[mtrlID]->buf645),haveCompleteMsg);
        if(MsgBase_645_ptr==nullptr)
        {
            break;
        }
        switch(emScriptRunState)
        {
            case TestInit:
            {
                break;
            }
            case Wait_BuildNetFinish_Whole:
            {
                break;
            }
            case ScriptSuccess:
            {
                break;
            }
            default:
            {
                if(MsgBase_645_ptr->ctrlCode_==READ_ADDR)
                {
                    sendMsg(dvcType,dvcId,mtrlID,p_MeterAddrResp_93);
                }
                else
                {
                    uchar di[4]={0x00};
                    if(MsgBase_645_ptr->ctrlCode_==READ_DATA)
                    {
                        shared_ptr<dlt_645_Protocol::Rqst_ReadData_0x11> p_ReadData_0x11 = std::dynamic_pointer_cast<dlt_645_Protocol::Rqst_ReadData_0x11>(MsgBase_645_ptr);
                        memcpy(di,p_ReadData_0x11->di,4);

                        QByteArray tmpSendMsg=prcsOther645Msg(MsgBase_645_ptr->ctrlCode_,MsgBase_645_ptr->dataLen_,di,MsgBase_645_ptr->addr_,(*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[mtrlID]->mtrAddr,p_CtrInfoList);
                        sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                    }
                    else
                    {
                        QByteArray tmpSendMsg=prcsOther645Msg(MsgBase_645_ptr->ctrlCode_,MsgBase_645_ptr->dataLen_,di,MsgBase_645_ptr->addr_,(*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[mtrlID]->mtrAddr,p_CtrInfoList);
                        sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                    }
                }
                break;
            }
        }
    }
}
void Script_RefuseListReport_ResetPoweroff::processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID)
{
    bool haveCompleteMsg=true;
    while(haveCompleteMsg)
    {
        shared_ptr<FrameOOPBase> MsgBase_OOP_ptr = FrameOOPHelper::DecodeMsg(&((*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[mtrlID]->buf698),haveCompleteMsg);

        if(MsgBase_OOP_ptr==nullptr)
        {
            break;
        }
        switch(emScriptRunState)
        {
            case TestInit:
            {
                break;
            }
            case Wait_BuildNetFinish_Whole:
            {
                break;
            }
            case ScriptSuccess:
            {
                break;
            }
            default:
            {
                if(MsgBase_OOP_ptr->service_type_==GET_REQUEST_CLIENT && MsgBase_OOP_ptr->service_sub_type_==uchar(GetRequestType::kGetRequestNormal))
                {
                    OAD oad;
                    oad.OI=ComuAddr;
                    oad.attribute.feature=0;
                    oad.attribute.seq=2;
                    oad.element_index=0;
                    shared_ptr<GetRequestNormal> p_GetRequestNormal=dynamic_pointer_cast<GetRequestNormal>(MsgBase_OOP_ptr);
                    if(p_GetRequestNormal->oad_.OI==oad.OI)
                    {
                        sendMsg(dvcType,dvcId,mtrlID,p_GetResponseNormal_ReadAddr);
                    }
                    else
                    {
                        QByteArray tmpSendMsg=prcsOther698Msg(MsgBase_OOP_ptr,(*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[mtrlID]->mtrAddr,p_CtrInfoList);
                        sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                    }
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther698Msg(MsgBase_OOP_ptr,(*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[mtrlID]->mtrAddr,p_CtrInfoList);
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
        }
    }
}
void Script_RefuseListReport_ResetPoweroff::sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame)
{
    if(frame==p_RefuseNodeReport_05F200)
    {
        p_RefuseNodeReport_05F200->ctrl_field_.dir=kDirDown;
        p_RefuseNodeReport_05F200->ctrl_field_.prm=kActive;
        p_RefuseNodeReport_05F200->ctrl_field_.comn_type=kHplc;

        p_RefuseNodeReport_05F200->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_RefuseNodeReport_05F200->info_field_.info_field_down.comu_rate=0;
        p_RefuseNodeReport_05F200->info_field_.info_field_down.comu_module_ident=0;
        if(emScriptRunState==EnableRefuseListReport)
        {
            p_RefuseNodeReport_05F200->refuse_sta_report_flag_=1;
            sendMsgOct=p_RefuseNodeReport_05F200->EncodeFrame();
            sendMsgLog=QString("》》允许拒绝节点信息上报05F200：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
        }
        else if(emScriptRunState==DisableRefuseListReport)
        {
            p_RefuseNodeReport_05F200->refuse_sta_report_flag_=0;
            sendMsgOct=p_RefuseNodeReport_05F200->EncodeFrame();
            sendMsgLog=QString("》》禁止拒绝节点信息上报05F200：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
        }
        else
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Error, "sendMsg状态机错误");
        }
    }
    else if(frame==p_Confirm_00F1)
    {
        p_Confirm_00F1->ctrl_field_.dir=kDirDown;
        p_Confirm_00F1->ctrl_field_.prm=kPassive;
        p_Confirm_00F1->ctrl_field_.comn_type=kHplc;

        p_Confirm_00F1->info_field_.info_field_down.msg_seq=char(msgSeq);
        p_Confirm_00F1->info_field_.info_field_down.comu_rate=0;
        p_Confirm_00F1->info_field_.info_field_down.comu_module_ident=0;

        memset(p_Confirm_00F1->data_info_,char(0xff),4);
        p_Confirm_00F1->data_info_[4]=0x00;
        p_Confirm_00F1->data_info_[5]=0x00;

        sendMsgOct=p_Confirm_00F1->EncodeFrame();
        sendMsgLog=QString("》》确认00F1：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_HardInit_01F1_Down)
    {
        p_HardInit_01F1_Down->ctrl_field_.dir=kDirDown;
        p_HardInit_01F1_Down->ctrl_field_.prm=kActive;
        p_HardInit_01F1_Down->ctrl_field_.comn_type=kHplc;

        p_HardInit_01F1_Down->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_HardInit_01F1_Down->info_field_.info_field_down.comu_rate=0;
        p_HardInit_01F1_Down->info_field_.info_field_down.comu_module_ident=0;

        sendMsgOct=p_HardInit_01F1_Down->EncodeFrame();
        sendMsgLog=QString("》》路由硬件初始化01F1：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_MeterAddrResp_93)
    {
        memcpy(p_MeterAddrResp_93->addr,(*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[meterID]->mtrAddr,6);
        memcpy(p_MeterAddrResp_93->addr_,(*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[meterID]->mtrAddr,6);

        sendMsgOct=p_MeterAddrResp_93->EncodeFrame();
        sendMsgLog=QString("》》 读通信地址应答(0x93)：%1\n").arg(QString(sendMsgOct.toHex()));
    }
    else if(frame==p_GetResponseNormal_ReadAddr)
    {
        p_GetResponseNormal_ReadAddr->ctrl_field_.dir = 1;
        p_GetResponseNormal_ReadAddr->ctrl_field_.prm = 0;
        p_GetResponseNormal_ReadAddr->ctrl_field_.fra = 0;
        p_GetResponseNormal_ReadAddr->ctrl_field_.res = 0;
        p_GetResponseNormal_ReadAddr->ctrl_field_.sc = 0;
        p_GetResponseNormal_ReadAddr->ctrl_field_.func = 1;

        p_GetResponseNormal_ReadAddr->address_field_.sa.addr_type = 0;
        p_GetResponseNormal_ReadAddr->address_field_.sa.logic_addr = 0;
        p_GetResponseNormal_ReadAddr->address_field_.sa.addr_len = 5;
        p_GetResponseNormal_ReadAddr->address_field_.sa.address = QString((QByteArray(reinterpret_cast<char*>((*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[meterID]->mtrAddr),6).toHex()));
        p_GetResponseNormal_ReadAddr->address_field_.ca.address = 0x00;

        p_GetResponseNormal_ReadAddr->piid_acd_.serve_priority = 0;
        p_GetResponseNormal_ReadAddr->piid_acd_.request_acd = 0;
        p_GetResponseNormal_ReadAddr->piid_acd_.serve_seq = 1;

        p_GetResponseNormal_ReadAddr->a_result_normal_.oad.OI = ComuAddr;
        p_GetResponseNormal_ReadAddr->a_result_normal_.oad.attribute.feature = 0;
        p_GetResponseNormal_ReadAddr->a_result_normal_.oad.attribute.seq = 2;
        p_GetResponseNormal_ReadAddr->a_result_normal_.oad.element_index = 0;

        p_GetResponseNormal_ReadAddr->a_result_normal_.get_result_ptr = std::make_shared<GetResultData>();
        std::dynamic_pointer_cast<GetResultData>(p_GetResponseNormal_ReadAddr->a_result_normal_.get_result_ptr)->value_ptr_ = std::make_shared<DataString>();
        std::dynamic_pointer_cast<DataString>(std::dynamic_pointer_cast<GetResultData>(p_GetResponseNormal_ReadAddr->a_result_normal_.get_result_ptr)->value_ptr_)->type_ = DataType::kOctet_string;

        std::dynamic_pointer_cast<DataString>(std::dynamic_pointer_cast<GetResultData>(p_GetResponseNormal_ReadAddr->a_result_normal_.get_result_ptr)->value_ptr_)->data_ = QByteArray(reinterpret_cast<char*>((*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[meterID]->mtrAddr),6);

        p_GetResponseNormal_ReadAddr->follow_report_field_.optional_ = 0;
        p_GetResponseNormal_ReadAddr->time_tag_field_.optional_ = 0;

        sendMsgOct=p_GetResponseNormal_ReadAddr->EncodeFrame();
        sendMsgLog=QString("》》 读通信地址应答(OOP)：%1\n").arg(QString(sendMsgOct.toHex()));
    }

    p_AbstractScriptHost->updateProgress(ProcessState_Processing, sendMsgLog);

    uchar *sendMsg=new uchar[uint(sendMsgOct.size())];
    memcpy(sendMsg,reinterpret_cast<uchar*>(sendMsgOct.data()),uint(sendMsgOct.size()));
    p_AbstractScriptHost->sendMsg2Dvc(dvcType,dvcId,sendMsg,sendMsgOct.size());
}
void Script_RefuseListReport_ResetPoweroff::sendSrcMsg(DvcType dvcType, int dvcId, QByteArray msg)
{
    if(msg.size()<1)
        return;
    uchar *sendMsg=new uchar[static_cast<uint>(msg.size())];
    memcpy(sendMsg,reinterpret_cast<uchar*>(msg.data()),uint(msg.size()));
    p_AbstractScriptHost->sendMsg2Dvc(dvcType,dvcId,sendMsg,msg.size());
    QStringList dvcList;
    dvcList<<"单通"<<"三通"<<"国网路由"<<"南网路由"<<"采集器";
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("测试脚本==>>>>上位机【设备类型:%1】的报文=%2").arg(dvcList.at(int(dvcType))).arg(QString(msg.toHex())));
}

void Script_RefuseListReport_ResetPoweroff::timer_timeoutProc()
{
    p_timer->stop();
    switch(emScriptRunState)
    {
    case Wait_BuildNetFinish_Whole:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_BuildNetFinish_Whole timeout!!!");
        break;
    }
    case Wait_Poweroff1_finish:
    case Wait_Poweroff2_finish:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("路由上电"));
        QList<double> sendParams;
        QList<int> idList;
        powerOnFlag=true;
        sendParams.clear();
        idList.clear();
        idList = findDvcIdList(p_CtrInfoList,CCO_GW);
        p_AbstractScriptHost->controlDvc(CCO_GW,idList,CtrlCmd_PowerOn_12V,sendParams);
        QThread::msleep(50);
        break;
    }
    default:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==p_timer");
        break;
    }
    }
}

void Script_RefuseListReport_ResetPoweroff::maxAllowTimer_timeoutProc()
{
    p_maxAllowTimer->stop();
    switch(emScriptRunState)
    {
        default:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Error, "p_maxAllowTimer timeout");
            break;
        }
    }
}

void Script_RefuseListReport_ResetPoweroff::delayTimer_timeoutProc()
{
    p_delayTimer->stop();
    if(emScriptRunState==WaitReportAfterReset)
    {
//        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "路由硬件复位后5min内未收到06F5拒绝列表上报");
          p_AbstractScriptHost->updateProgress(ProcessState_Success, "cco不支持该测试项，测试结束");
    }
    else if(emScriptRunState==WaitReportAfterPoweroff)
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "路由断上电后5min内未收到06F5拒绝列表上报");
    }
    else if(emScriptRunState==Wait_Reset2_finish)
    {
        emScriptRunState=Wait_Poweroff2_finish;
        poweroffNo=1;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("路由硬件复位后5min内未收到拒绝列表上报，开始断上电路由第%0次，路由断电5s").arg(poweroffNo));
        QList<double> sendParams;
        QList<int> idList;
        sendParams.clear();
        idList.clear();
        idList = findDvcIdList(p_CtrInfoList,CCO_GW);
        p_AbstractScriptHost->controlDvc(CCO_GW,idList,CtrlCmd_PowerOff_12V,sendParams);
        QThread::msleep(50);
        p_timer->start(5*1000);
    }
    else if(emScriptRunState==Wait_Poweroff2_finish)
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Success, QString("路由断上电后5min内未收到拒绝列表上报"));
    }
    else
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Error, "delayTimer状态机错误");
    }
}
