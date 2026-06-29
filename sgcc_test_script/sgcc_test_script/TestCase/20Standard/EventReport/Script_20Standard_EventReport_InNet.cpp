#include "Script_20Standard_EventReport_InNet.h"

Script_20Standard_EventReport_InNet::Script_20Standard_EventReport_InNet(QObject *parent) : QObject(parent)
{
    p_BuildNetwork_GW=new BuildNetwork_GW();
    p_CtrInfoList=make_shared<QList<shared_ptr<CtrInfo>>>();
    p_Frame3762Helper=make_shared<Frame3762Helper>();
    p_Frame645Helper=make_shared<Frame645Helper>();
    p_MeterAddrResp_93=make_shared<RspsNormal_ReadAddr_0x93>(addr,6);
    p_FrameOOPHelper=make_shared<FrameOOPHelper>();
    p_GetResponseNormal_ReadAddr=make_shared<GetResponseNormal>();
    p_Afn00F1=make_shared<Afn00F1>();
    p_Afn00F2=make_shared<Afn00F2>();
    p_ReportNotificationList=make_shared<ReportNotificationList>();

    p_timer=new QTimer(this);
    p_maxAllowTimer=new QTimer(this);
    p_delayTimer=new QTimer(this);
    connect(p_timer,SIGNAL(timeout()),this,SLOT(timer_timeoutProc()));
    connect(p_maxAllowTimer,SIGNAL(timeout()),this,SLOT(maxAllowTimer_timeoutProc()));
    connect(p_delayTimer,SIGNAL(timeout()),this,SLOT(delayTimer_timeoutProc()));
}
Script_20Standard_EventReport_InNet::~Script_20Standard_EventReport_InNet()
{
    if(p_EventReportPrepare!=nullptr)
        delete p_EventReportPrepare;
    p_BuildNetwork_GW->initBuildNetWork();
    if(needPowerOff==true)//断电处理
        powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost);
}
void Script_20Standard_EventReport_InNet::setHost(AbstractScriptHost *host)
{
    p_AbstractScriptHost=host;
    p_BuildNetwork_GW->setHost(host);
}
void Script_20Standard_EventReport_InNet::addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq)
{
    p_CtrInfoList->clear();
    comnAddAddrsInfo(p_CtrInfoList, p_ConcentratorInfoList, p_MeterInfoList, p_SchemeCfgInfoList);
    uchar dstFreq=freq&0x0f;

    uchar dstPrtcl=uchar(freq)>>4;
    if(dstPrtcl==0x03)
        setMeterAddrsPrtcl(p_CtrInfoList,dstPrtcl);

    p_BuildNetwork_GW->setCtrInfoListAndFreq(p_CtrInfoList, dstFreq);

}
bool Script_20Standard_EventReport_InNet::config(const QMap<QString,QString> *paraDic)
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
            (*paraDic)["needBuildNet"].toLower()=="true"?this->needBuildNet=true:this->needBuildNet=false;
        }
        if(paraDic->keys().contains("needPowerOff"))
        {
            (*paraDic)["needPowerOff"].toLower()=="true"?this->needPowerOff=true:this->needPowerOff=false;
        }
        result = true;
    }
    return result;
}
void Script_20Standard_EventReport_InNet::execute()
{
    p_AbstractScriptHost->updateProgress(ProcessState_Start, "Test start!");
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "流程描述\n"
                                         "1.组网通用流程，SENCE3(需要配置CCO，需要组网) Wait_BuildNetFinish_Whole\n"
                                         "2.事件上报准备 Wait_EventReportPrepare_Finish\n"
                                         "3.事件上报回复确认 Wait_EventReport_Confirm_Finish\n"
                                         "4.事件上报重发测试 Wait_EventReport_Repeat_Finish\n"
                                         "5.事件上报回复否认 Wait_EventReport_Deny_Finish\n"
                                         "6.事件上报不回复  Wait_EventReport_NoReply_Finish\n");
    for(int i=0; i<p_CtrInfoList->at(0)->keyList.size(); i++)
    {
        if(SingleSTA == p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->value(p_CtrInfoList->at(0)->keyList.at(i))->slotPosition)
        {
            singleStaIndex=i;
            break;
        }
    }
    if(needBuildNet==true)//场景2-4
    {
#ifdef SENCE4
        p_BuildNetwork_GW->needRebuildNetwork=true;
#endif
        p_BuildNetwork_GW->execute();//执行组网通用脚本
        emScriptRunState=Wait_BuildNetFinish_Whole;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "等待全网组网完成");
    }
    else//场景1
    {
//        tryTimes=0;
//        index=0;
////        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,index,make_shared<void>());//make_shared<void>()应该替换成实际的376.2命令
//        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--命令，等待--回复");
//        emScriptRunState=Wait;
//        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
    }
    p_maxAllowTimer->start((timerForReachThresld+timerAfterReachThresld)*1000);//此处要启动脚本的最大执行时间定时器
}
void Script_20Standard_EventReport_InNet::stop()
{
    p_timer->stop();
    p_delayTimer->stop();
    p_maxAllowTimer->stop();
    if(p_AbstractScriptHost==nullptr)
        return;
    p_AbstractScriptHost->updateProgress(ProcessState_Error, "Test stop!");
    if(needPowerOff==true)
        powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost);
}
void Script_20Standard_EventReport_InNet::processMsg(DvcType dvcType, int id, uchar *data, int datalen)
{
    if(p_CtrInfoList->size()==0)
        return;
    if(emScriptRunState==Wait_BuildNetFinish_Whole)//场景2-4
    {
#ifdef SENCE2
        if(!p_BuildNetwork_GW->startBuildNetFlag)//场景2 关注p_BuildNetwork_GW->startBuildNetFlag标志,开始组网标志
            p_BuildNetwork_GW->processMsg(dvcType,id,data,datalen);
#elif defined(SENCE3)||defined(SENCE4)
        if(!p_BuildNetwork_GW->buildNetworkResultFlag)//场景3、4 关注p_BuildNetwork_GW->buildNetworkResultFlag标志，组网完成标志
            p_BuildNetwork_GW->processMsg(dvcType,id,data,datalen);
#elif defined SENCE1
        if(true){}
#endif
        else
        {
            p_BuildNetwork_GW->initBuildNetWork();
            tryTimes=0;
            index=0;
            emScriptRunState=Wait_EventReportPrepare_Finish;
            p_EventReportPrepare=new EventReportPrepare(p_CtrInfoList,p_AbstractScriptHost);
            connect(p_EventReportPrepare,&EventReportPrepare::signalNoticeEventReportPrepareState,this,&Script_20Standard_EventReport_InNet::slotNoticeEventReportPrepareState);
            p_EventReportPrepare->execute();
        }
    }
    else if(emScriptRunState==Wait_EventReportPrepare_Finish)
    {
        p_EventReportPrepare->processMsg(dvcType,id,data,datalen);
    }
    else//当测试脚本开始执行脚本自己的操作时，均从此进入
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
        else if(dvcType==SingleSTA)//单通全部按照OOP处理
        {
            for(int i=0; i<p_CtrInfoList->at(0)->keyList.size(); i++)
            {
                if(dvcType == p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->value(p_CtrInfoList->at(0)->keyList.at(i))->slotPosition
                        && id == p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->value(p_CtrInfoList->at(0)->keyList.at(i))->dvcId)
                {
                    (*p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList)[p_CtrInfoList->at(0)->keyList.at(i)]->buf698.append(recvTempData);
                    processMsgFromMeterOOP(dvcType,id,p_CtrInfoList->at(0)->keyList.at(i));
                    break;
                }
            }
        }
        else if(dvcType==ThreeSTA)
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
void Script_20Standard_EventReport_InNet::processCtrlDvcRes(DvcType dvcType, QList<int> idList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params)
{
    if(isSucs==false)
        return;
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到控制命令回复，参数个数=%1").arg(QString::number(params.size())));
    QList<int> sendParams;
    switch(emScriptRunState)
    {
        case ScriptInit:
        {
            break;
        }
        case Wait_BuildNetFinish_Whole:
        {
            p_BuildNetwork_GW->processCtrlDvcRes(dvcType,idList,ctrlCmdType,isSucs,params);
            break;
        }
        case Wait_EventReportPrepare_Finish:
        {
            p_EventReportPrepare->processCtrlDvcRes(dvcType,idList,ctrlCmdType,isSucs,params);
            break;
        }
        default:
        {
            break;
        }
    }
}

void Script_20Standard_EventReport_InNet::processMsgFromCCO(DvcType dvcType, int dvcId)
{
    if(dvcId!=p_CtrInfoList->at(0)->ctrlID)
        return;
    bool haveCompleteMsg=true;
    while(haveCompleteMsg)
    {
        shared_ptr<Frame3762Base> p_Frame3762Base = p_Frame3762Helper->DecodeLocalMsg(&(p_CtrInfoList->at(0)->buf),haveCompleteMsg);
        if(p_Frame3762Base==nullptr)
        {
            continue;
        }
        if(p_Frame3762Base->afn_ == 0x03&&p_Frame3762Base->dt1_==0x02&&p_Frame3762Base->dt2_==0x01&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("收到CCO上报03F10，疑似复位，运行状态为%1").arg(metaEnum.valueToKey(emScriptRunState)));
            p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("收到CCO上报03F10，疑似复位，运行状态为%1").arg(metaEnum.valueToKey(emScriptRunState)));
            return;
        }
        switch(emScriptRunState)
        {
            case ScriptInit:
            {
                break;
            }
            case Wait_BuildNetFinish_Whole:
            {
                break;
            }
            case Wait_EventReport_Confirm_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x06&&p_Frame3762Base->dt1_==0x10&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    shared_ptr<Afn06F5> ptr=dynamic_pointer_cast<Afn06F5>(p_Frame3762Base);
                    if(ptr->report_event_unit_.frame_content_==meterMsg)
                    {
                        p_timer->stop();
                        msgSeq=uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq);
                        sendMsg(dvcType,p_CtrInfoList->at(0)->ctrlID,index,p_Afn00F1);
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("%1收到事件上报回复确认").arg(metaEnum.valueToKey(emScriptRunState)));
                        EventReportPrepare::delay(200);
                        tryTimes=0;
                        emScriptRunState=Wait_EventReport_Repeat_Finish;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("%1工装单相表进行事件主动上报").arg(metaEnum.valueToKey(emScriptRunState)));
                        sendMsg(SingleSTA,p_CtrInfoList->at(0)->ctrlID,singleStaIndex,p_ReportNotificationList);
                        p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                    {
                        QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                        sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                    }
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_EventReport_Repeat_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x06&&p_Frame3762Base->dt1_==0x10&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    shared_ptr<Afn06F5> ptr=dynamic_pointer_cast<Afn06F5>(p_Frame3762Base);
                    if(ptr->report_event_unit_.frame_content_==meterMsg)
                    {
                        p_timer->stop();
                        if(++tryTimes>=2)
                        {
                            msgSeq=uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq);
                            sendMsg(dvcType,p_CtrInfoList->at(0)->ctrlID,index,p_Afn00F1);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("%1收到事件上报回复确认").arg(metaEnum.valueToKey(emScriptRunState)));
                            EventReportPrepare::delay(200);
                            tryTimes=0;
                            emScriptRunState=Wait_EventReport_Deny_Finish;
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("%1工装单相表进行事件主动上报").arg(metaEnum.valueToKey(emScriptRunState)));
                            sendMsg(SingleSTA,p_CtrInfoList->at(0)->ctrlID,singleStaIndex,p_ReportNotificationList);
                            p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
                        }
                        else
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("%1收到事件上报").arg(metaEnum.valueToKey(emScriptRunState)));
                            p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
                        }
                    }
                    else
                    {
                        QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                        sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                    }
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_EventReport_Deny_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x06&&p_Frame3762Base->dt1_==0x10&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    shared_ptr<Afn06F5> ptr=dynamic_pointer_cast<Afn06F5>(p_Frame3762Base);
                    if(ptr->report_event_unit_.frame_content_==meterMsg)
                    {
                        p_timer->stop();
						//if(++tryTimes>=4)
                        if(++tryTimes>=6)
                        {
                            QString state=QString("%1回复否认收到%2次事件上报，不符合要求")
                                    .arg(metaEnum.valueToKey(emScriptRunState))
                                    .arg(QString::number(tryTimes));
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing,state+testCase);
                            p_AbstractScriptHost->updateProgress(ProcessState_Failed,state+testCase);
                        }
                        else if(tryTimes>=3)
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("%1回复否认收到%2次事件上报").arg(metaEnum.valueToKey(emScriptRunState)).arg(QString::number(tryTimes)));
                            p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
                        }
                        else
                        {
                            msgSeq=uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq);
                            sendMsg(dvcType,p_CtrInfoList->at(0)->ctrlID,index,p_Afn00F2);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("%1收到事件上报回复否认").arg(metaEnum.valueToKey(emScriptRunState)));
                            p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
                        }
                    }
                    else
                    {
                        QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                        sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                    }
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_EventReport_NoReply_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x06&&p_Frame3762Base->dt1_==0x10&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    shared_ptr<Afn06F5> ptr=dynamic_pointer_cast<Afn06F5>(p_Frame3762Base);
                    if(ptr->report_event_unit_.frame_content_==meterMsg)
                    {
                        p_timer->stop();
						//if(++tryTimes>=4)
                        if(++tryTimes>=6) 
                        {
                            QString state=QString("%1不回复确认帧，收到%2次事件上报，不符合要求")
                                    .arg(metaEnum.valueToKey(emScriptRunState))
                                    .arg(QString::number(tryTimes));
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing,state+testCase);
                            p_AbstractScriptHost->updateProgress(ProcessState_Failed,state+testCase);
                        }
                        else if(tryTimes>=3)
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("%1不回复确认帧，收到%2次事件上报").arg(metaEnum.valueToKey(emScriptRunState)).arg(QString::number(tryTimes)));
                            p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
                        }
                        else
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("%1收到事件上报").arg(metaEnum.valueToKey(emScriptRunState)));
                            p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
                        }
                    }
                    else
                    {
                        QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                        sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                    }
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
            default:
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                break;
            }
        }
    }
}

void Script_20Standard_EventReport_InNet::slotNoticeEventReportPrepareState(ProcessState pState, const QString &state)
{
    switch(emScriptRunState)
    {
        case Wait_EventReportPrepare_Finish:
        {
            if(pState==ProcessState_Processing)
            {
                p_AbstractScriptHost->updateProgress(pState,QString(metaEnum.valueToKey(emScriptRunState))+"事件主动上报准备完成，"+state+testCase);

                emScriptRunState=Wait_EventReport_Confirm_Finish;
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("%1工装单相表进行事件主动上报").arg(metaEnum.valueToKey(emScriptRunState)));
                sendMsg(SingleSTA,p_CtrInfoList->at(0)->ctrlID,singleStaIndex,p_ReportNotificationList);
                p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
            }
            else
                p_AbstractScriptHost->updateProgress(pState,QString(metaEnum.valueToKey(emScriptRunState))+"事件主动上报准备异常，"+state+testCase);
            break;
        }
        default:
        {
            break;
        }
    }
}
void Script_20Standard_EventReport_InNet::processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID)
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
            case ScriptInit:
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
void Script_20Standard_EventReport_InNet::processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID)
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
            case ScriptInit:
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
void Script_20Standard_EventReport_InNet::sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame)
{
    if(frame==p_Afn00F1)
    {
        p_Afn00F1->ctrl_field_={kHplc,kActive,kDirDown};
        p_Afn00F1->info_field_.info_field_down.msg_seq=char(msgSeq); //本地代码00F1有做ssn判断 如果和上行不一致，就不会从重试列表删除和次数--操作；导致重发逻辑异常
        uchar data[6]={0xFF,0xFF,0xFF,0xFF,0x00,0x00};
        memcpy(p_Afn00F1->data_info_,data,6);
        sendMsgOct=p_Afn00F1->EncodeFrame();
        sendMsgLog=QString("》》确认00F1：%1\n").arg(QString(sendMsgOct.toHex()));
    }
    else if(frame==p_Afn00F2)
    {
        p_Afn00F2->ctrl_field_={kHplc,kActive,kDirDown};
        p_Afn00F2->info_field_.info_field_down.msg_seq=char(msgSeq); 
        p_Afn00F2->error_code_=0x01;
        sendMsgOct=p_Afn00F2->EncodeFrame();
        sendMsgLog=QString("》》否认00F2：%1\n").arg(QString(sendMsgOct.toHex()));
    }
    else if(frame==p_ReportNotificationList)
    {
        p_ReportNotificationList->ctrl_field_.dir = 1;
        p_ReportNotificationList->ctrl_field_.prm = 0;
        p_ReportNotificationList->ctrl_field_.fra = 0;
        p_ReportNotificationList->ctrl_field_.res = 0;
        p_ReportNotificationList->ctrl_field_.sc = 0;
        p_ReportNotificationList->ctrl_field_.func = 1;

        p_ReportNotificationList->address_field_.sa.addr_type = 0;
        p_ReportNotificationList->address_field_.sa.logic_addr = 0;
        p_ReportNotificationList->address_field_.sa.addr_len = 5;
        p_ReportNotificationList->address_field_.sa.address = Address(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(meterID)->mtrAddr).toString();
        p_ReportNotificationList->address_field_.ca.address = 0x10;

        p_ReportNotificationList->piid_acd_.serve_priority = 0;
        p_ReportNotificationList->piid_acd_.request_acd = 0;
        p_ReportNotificationList->piid_acd_.serve_seq = 0;
        AResultNormal ptr;
        ptr.oad.OI = 0x2000;
        ptr.oad.attribute.feature = 0;
        ptr.oad.attribute.seq = 2;
        ptr.oad.element_index = 1;
        ptr.get_result_ptr = make_shared<GetResultData>();
        dynamic_pointer_cast<GetResultData>(ptr.get_result_ptr)->value_ptr_ = make_shared<DataBasic>();
        dynamic_pointer_cast<GetResultData>(ptr.get_result_ptr)->value_ptr_->type_ = DataType::kLong_unsigned;
        dynamic_pointer_cast<DataBasic>(dynamic_pointer_cast<GetResultData>(ptr.get_result_ptr)->value_ptr_)->data_=QByteArray::fromHex(QString("08d6").toLatin1());
        p_ReportNotificationList->list_result_normal_.clear();
        p_ReportNotificationList->list_result_normal_.append(ptr);

        p_ReportNotificationList->follow_report_field_.optional_ = 0;
        p_ReportNotificationList->time_tag_field_.optional_ = 0;
        sendMsgOct=p_ReportNotificationList->EncodeFrame();
        meterMsg=sendMsgOct;
        sendMsgLog=QString("》》 事件主动上报(OOP)：%1\n").arg(QString(sendMsgOct.toHex()));
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
void Script_20Standard_EventReport_InNet::sendSrcMsg(DvcType dvcType, int dvcId, QByteArray msg)
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

void Script_20Standard_EventReport_InNet::timer_timeoutProc()
{
    p_timer->stop();
    switch(emScriptRunState)
    {
        case Wait_BuildNetFinish_Whole:
        {
            p_CtrInfoList->at(0)->inNetResult=false;
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("全网组网成功率：%1%").arg(p_CtrInfoList->at(0)->inNetSuccessRate*100));
            break;
        }
        case Wait_EventReport_Confirm_Finish:
        case Wait_EventReport_Repeat_Finish:
        {
            QString state=QString("%1未收到事件上报，不符合要求")
                    .arg(metaEnum.valueToKey(emScriptRunState));
            p_AbstractScriptHost->updateProgress(ProcessState_Processing,state+testCase);
            p_AbstractScriptHost->updateProgress(ProcessState_Failed,state+testCase);
            break;

        }
        case Wait_EventReport_Deny_Finish:
        {
		    //if(tryTimes==3)
            if(tryTimes>=3)
            {
				//QString state=QString("%1事件上报收到否认帧，最大重发3次，符合要求")
                QString state=QString("%1事件上报收到否认帧，最大重发多于3次，符合要求")
                        .arg(metaEnum.valueToKey(emScriptRunState));
                p_AbstractScriptHost->updateProgress(ProcessState_Processing,state+testCase);

                EventReportPrepare::delay(200);
                tryTimes=0;
                emScriptRunState=Wait_EventReport_NoReply_Finish;
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("%1工装单相表进行事件主动上报").arg(metaEnum.valueToKey(emScriptRunState)));
                sendMsg(SingleSTA,p_CtrInfoList->at(0)->ctrlID,singleStaIndex,p_ReportNotificationList);
                p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
            }
            else
            {
                QString state=QString("%1未收到事件上报，不符合要求")
                        .arg(metaEnum.valueToKey(emScriptRunState));
                p_AbstractScriptHost->updateProgress(ProcessState_Processing,state+testCase);
                p_AbstractScriptHost->updateProgress(ProcessState_Failed,state+testCase);
            }
            break;
        }
        case Wait_EventReport_NoReply_Finish:
        {
			//if(tryTimes==3)
            if(tryTimes>=3)
            {
				//QString state=QString("%1事件上报未收到确认帧，最大重发3次，符合要求")
                QString state=QString("%1事件上报未收到确认帧，最大重发大于3次，符合要求")
                        .arg(metaEnum.valueToKey(emScriptRunState));
                p_AbstractScriptHost->updateProgress(ProcessState_Processing,state+testCase);
                p_AbstractScriptHost->updateProgress(ProcessState_Success,"脚本执行成功"+testCase);
            }
            else
            {
                QString state=QString("%1未收到事件上报，不符合要求")
                        .arg(metaEnum.valueToKey(emScriptRunState));
                p_AbstractScriptHost->updateProgress(ProcessState_Processing,state+testCase);
                p_AbstractScriptHost->updateProgress(ProcessState_Failed,state+testCase);
            }
            break;
        }
        default:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==p_timer");
            break;
        }
    }
}
void Script_20Standard_EventReport_InNet::maxAllowTimer_timeoutProc()
{
    switch(emScriptRunState)
    {
        default:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==p_maxAllowTimer");
            break;
        }
    }
}
void Script_20Standard_EventReport_InNet::delayTimer_timeoutProc()
{
    switch(emScriptRunState)
    {
        default:
        {
            break;
        }
    }
}
