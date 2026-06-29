#include "Script_20Standard_BaudConsult_ConfirmDelay_CCO.h"
Script_20Standard_BaudConsult_ConfirmDelay_CCO::Script_20Standard_BaudConsult_ConfirmDelay_CCO(QObject *parent) : QObject(parent)
{
    p_BuildNetwork_GW=new BuildNetwork_GW();
    p_CtrInfoList=make_shared<QList<shared_ptr<CtrInfo>>>();
    p_Frame3762Helper=make_shared<Frame3762Helper>();
    p_Frame645Helper=make_shared<Frame645Helper>();
    p_MeterAddrResp_93=make_shared<RspsNormal_ReadAddr_0x93>(addr,6);
    p_FrameOOPHelper=make_shared<FrameOOPHelper>();
    p_GetResponseNormal_ReadAddr=make_shared<GetResponseNormal>();
    p_Afn02F1=make_shared<Afn02F1>();

    p_timer=new QTimer(this);
    p_maxAllowTimer=new QTimer(this);
    p_delayTimer=new QTimer(this);
    connect(p_timer,SIGNAL(timeout()),this,SLOT(timer_timeoutProc()));
    connect(p_maxAllowTimer,SIGNAL(timeout()),this,SLOT(maxAllowTimer_timeoutProc()));
    connect(p_delayTimer,SIGNAL(timeout()),this,SLOT(delayTimer_timeoutProc()));
}
Script_20Standard_BaudConsult_ConfirmDelay_CCO::~Script_20Standard_BaudConsult_ConfirmDelay_CCO()
{
    if(p_BaudConsult!=nullptr)
        delete p_BaudConsult;
    if(p_BaudConsultPrepare!=nullptr)
        delete p_BaudConsultPrepare;
    p_BuildNetwork_GW->initBuildNetWork();
    if(needPowerOff==true)//断电处理
        powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost);
}
void Script_20Standard_BaudConsult_ConfirmDelay_CCO::setHost(AbstractScriptHost *host)
{
    p_AbstractScriptHost=host;
    p_BuildNetwork_GW->setHost(host);
}
void Script_20Standard_BaudConsult_ConfirmDelay_CCO::addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq)
{
    p_CtrInfoList->clear();
    comnAddAddrsInfo(p_CtrInfoList, p_ConcentratorInfoList, p_MeterInfoList, p_SchemeCfgInfoList);
    uchar dstFreq=freq&0x0f;

    uchar dstPrtcl=uchar(freq)>>4;
    if(dstPrtcl==0x03)
        setMeterAddrsPrtcl(p_CtrInfoList,dstPrtcl);

    p_BuildNetwork_GW->setCtrInfoListAndFreq(p_CtrInfoList, dstFreq);

}
bool Script_20Standard_BaudConsult_ConfirmDelay_CCO::config(const QMap<QString,QString> *paraDic)
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
void Script_20Standard_BaudConsult_ConfirmDelay_CCO::execute()
{
    p_AbstractScriptHost->updateProgress(ProcessState_Start, "Test start!");
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "流程描述\n"
                                         "1.组网通用流程，SENCE2(需要配置CCO，但不需要组网) Wait_BuildNetFinish_Whole\n"
                                         "2.波特率协商准备 Wait_BaudConsultPrepare_Finish\n"
                                         "3.波特率协商19200bps，工装切换波特率后再回复确认 Wait_BaudConsult_19200bps_Finish\n"
                                         "4.抄表9600bps， Wait_ReadMeter_9600bps_Finish\n"
                                         "4.波特率协商115200bps，工装切换波特率后再回复确认 Wait_BaudConsult_115200bps_Finish\n"
                                         "5.抄表115200bps  Wait_ReadMeter_115200bps_Finish\n");
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
void Script_20Standard_BaudConsult_ConfirmDelay_CCO::stop()
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
void Script_20Standard_BaudConsult_ConfirmDelay_CCO::processMsg(DvcType dvcType, int id, uchar *data, int datalen)
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
            emScriptRunState=Wait_BaudConsultPrepare_Finish;
            p_BaudConsultPrepare=new BaudConsultPrepare(p_CtrInfoList,p_AbstractScriptHost);
            connect(p_BaudConsultPrepare,&BaudConsultPrepare::signalNoticeBaudConsultPrepareState,this,&Script_20Standard_BaudConsult_ConfirmDelay_CCO::slotNoticeBaudConsultPrepareState);
            p_BaudConsultPrepare->execute();
        }
    }
    else if(emScriptRunState==Wait_BaudConsultPrepare_Finish)
    {
        p_BaudConsultPrepare->processMsg(dvcType,id,data,datalen);
    }
    else if(emScriptRunState==Wait_BaudConsult_115200bps_Finish
            ||emScriptRunState==Wait_BaudConsult_19200bps_Finish)
    {
        p_BaudConsult->processMsg(dvcType,id,data,datalen);
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
                    index=i;
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
void Script_20Standard_BaudConsult_ConfirmDelay_CCO::processCtrlDvcRes(DvcType dvcType, QList<int> idList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params)
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
        case Wait_BaudConsultPrepare_Finish:
        {
            p_BaudConsultPrepare->processCtrlDvcRes(dvcType,idList,ctrlCmdType,isSucs,params);
            break;
        }
        case Wait_BaudConsult_115200bps_Finish:
        case Wait_BaudConsult_19200bps_Finish:
        {
            p_BaudConsult->processCtrlDvcRes(dvcType,idList,ctrlCmdType,isSucs,params);
            break;
        }
        default:
        {
            break;
        }
    }
}

void Script_20Standard_BaudConsult_ConfirmDelay_CCO::processMsgFromCCO(DvcType dvcType, int dvcId)
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
            case Wait_ReadMeter_9600bps_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x02&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp&&uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq)==msgSeq)
                {
                    p_timer->stop();
                    shared_ptr<Afn02F1> ptr=dynamic_pointer_cast<Afn02F1>(p_Frame3762Base);

                    if(Address(QByteArray(ptr->address_field_.src_addr,6))==Address(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->value(p_CtrInfoList->at(0)->keyList.at(index))->mtrAddr)
                            &&ptr->frame_content_==meterMsg)
                    {
                        QString state=QString("波特率协商-确认后发%1，9600bps抄表收到正确回复")
                                .arg(metaEnum.valueToKey(emScriptRunState));
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,state);

                        meterMsg.clear();
                        emScriptRunState=Wait_BaudConsult_115200bps_Finish;
                        p_BaudConsult=new BaudConsult(p_CtrInfoList,p_AbstractScriptHost,int(emScriptRunState),true,BaudConsult::ConfirmDelay);
                        connect(p_BaudConsult,&BaudConsult::signalNoticeBaudConsultState,this,&Script_20Standard_BaudConsult_ConfirmDelay_CCO::slotNoticeBaudConsultState);
                        p_BaudConsult->execute();
                    }
                    else
                    {
                        QString state=QString("波特率协商-确认后发%1，9600bps抄表收到异常回复")
                                .arg(metaEnum.valueToKey(emScriptRunState));
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,state+testCase);
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed,state+testCase);
                    }
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_ReadMeter_115200bps_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x02&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp&&uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq)==msgSeq)
                {
                    p_timer->stop();
                    shared_ptr<Afn02F1> ptr=dynamic_pointer_cast<Afn02F1>(p_Frame3762Base);

                    if(Address(QByteArray(ptr->address_field_.src_addr,6))==Address(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->value(p_CtrInfoList->at(0)->keyList.at(index))->mtrAddr)
                    &&ptr->frame_content_==meterMsg)
                    {
                        if(readMeterNoFlag==false&&ptr->frame_content_.isEmpty())
                        {
                            QString state=QString("波特率协商-确认后发%1，115200bps抄表回复数据为空，符合要求")
                                    .arg(metaEnum.valueToKey(emScriptRunState));
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing,state);

                            if(++tryTimes>=15)
                            {
                                QString state=QString("波特率协商-确认后发%1，回复异常，串口超时15次以上，STA仍未复位")
                                        .arg(metaEnum.valueToKey(emScriptRunState));
                                p_AbstractScriptHost->updateProgress(ProcessState_Processing,state+testCase);
                                p_AbstractScriptHost->updateProgress(ProcessState_Failed,state+testCase);
                            }
                            else
                            {
                                sendMsg(dvcType,p_CtrInfoList->at(0)->ctrlID,index,p_Afn02F1);
                                p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("发送抄读电表02F1，等待回复").arg(tryTimes));
                                p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
                            }
                        }
                        else if(readMeterNoFlag==true&&ptr->frame_content_.isEmpty())
                        {
                            QString state=QString("波特率协商-确认后发%1，115200bps抄表回复数据为空，符合要求")
                                    .arg(metaEnum.valueToKey(emScriptRunState));
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing,state);

                            if(++tryTimes>=5)
                            {
                                QString state=QString("波特率协商-确认后发%1，STA复位重新读表号后，115200bps抄表回复异常")
                                        .arg(metaEnum.valueToKey(emScriptRunState));
                                p_AbstractScriptHost->updateProgress(ProcessState_Processing,state+testCase);
                                p_AbstractScriptHost->updateProgress(ProcessState_Failed,state+testCase);
                            }
                            else
                            {
                                sendMsg(dvcType,p_CtrInfoList->at(0)->ctrlID,index,p_Afn02F1);
                                p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("发送抄读电表02F1，等待回复").arg(tryTimes));
                                p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
                            }
                        }
                        else if(readMeterNoFlag==true&&!ptr->frame_content_.isEmpty())
                        {
                            QString state=QString("波特率协商-确认后发%1，STA复位重新读表号后，115200bps抄表回复数据，符合要求")
                                    .arg(metaEnum.valueToKey(emScriptRunState));
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing,state+testCase);
                            p_AbstractScriptHost->updateProgress(ProcessState_Success,"脚本执行成功"+testCase);
                        }
                        else
                        {
                            QString state=QString("波特率协商-确认后发%1，115200bps抄表回复异常")
                                    .arg(metaEnum.valueToKey(emScriptRunState));
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing,state+testCase);
                            p_AbstractScriptHost->updateProgress(ProcessState_Failed,state+testCase);
                        }
                    }
                    else
                    {
                        QString state=QString("波特率协商-确认后发%1，115200bps抄表回复异常")
                                .arg(metaEnum.valueToKey(emScriptRunState));
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,state+testCase);
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed,state+testCase);
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

void Script_20Standard_BaudConsult_ConfirmDelay_CCO::slotNoticeBaudConsultPrepareState(ProcessState pState, const QString &state)
{
    switch(emScriptRunState)
    {
        case Wait_BaudConsultPrepare_Finish:
        {
            if(pState==ProcessState_Processing)
            {
                p_AbstractScriptHost->updateProgress(pState,QString(metaEnum.valueToKey(emScriptRunState))+"波特率协商准备完成，"+state+testCase);
                emScriptRunState=Wait_BaudConsult_19200bps_Finish;
                p_BaudConsult=new BaudConsult(p_CtrInfoList,p_AbstractScriptHost,int(emScriptRunState),true,BaudConsult::ConfirmDelay);
                connect(p_BaudConsult,&BaudConsult::signalNoticeBaudConsultState,this,&Script_20Standard_BaudConsult_ConfirmDelay_CCO::slotNoticeBaudConsultState);
                p_BaudConsult->execute();
            }
            else
                p_AbstractScriptHost->updateProgress(pState,QString(metaEnum.valueToKey(emScriptRunState))+"波特率协商准备异常，"+state+testCase);
            break;
        }
        default:
        {
            break;
        }
    }
}
void Script_20Standard_BaudConsult_ConfirmDelay_CCO::slotNoticeBaudConsultState(ProcessState pState, const QString &state)
{
    if(p_BaudConsult!=nullptr)
        delete p_BaudConsult;
    p_BaudConsult=nullptr;
    switch(emScriptRunState)
    {
        case Wait_BaudConsult_115200bps_Finish:
        case Wait_BaudConsult_19200bps_Finish:
        {
            if(pState==ProcessState_Processing)
            {
                p_AbstractScriptHost->updateProgress(pState,QString(metaEnum.valueToKey(emScriptRunState))+state);
                if(state.split(":").last().isEmpty())
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString(metaEnum.valueToKey(emScriptRunState))+"回复数据为空，符合要求"+testCase);
                    if(emScriptRunState==Wait_BaudConsult_19200bps_Finish)
                    {
                        for(int i=0; i<p_CtrInfoList->at(0)->keyList.size(); i++)
                        {
                            if(SingleSTA == p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->value(p_CtrInfoList->at(0)->keyList.at(i))->slotPosition)
                            {
                                index=i;
                                break;
                            }
                        }

                        emScriptRunState=Wait_ReadMeter_9600bps_Finish;
                        //设置单通工装波特率
                        p_AbstractScriptHost->controlDvc(SingleSTA,QList<int>()<<p_CtrInfoList->at(0)->dvcId,CtrlCmd_SetBaudRate,QList<double>()<<9600);
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("发送设置单通工装波特率%1bps，等待确认").arg(QString::number(9600)));
						//BaudConsult::delay(1000);
                        BaudConsult::delay(10000); //
                        tryTimes=0;
                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,index,p_Afn02F1);
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("发送抄读电表02F1，等待回复"));
                        p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
                    }
                    else if(emScriptRunState==Wait_BaudConsult_115200bps_Finish)
                    {
                        tryTimes=0;
                        emScriptRunState=Wait_ReadMeter_115200bps_Finish;
                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,index,p_Afn02F1);
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("发送抄读电表02F1，等待回复"));
                        p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
                    }
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString(metaEnum.valueToKey(emScriptRunState))+"抄表回复数据不为空，不符合要求"+testCase);
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString(metaEnum.valueToKey(emScriptRunState))+"抄表回复数据不为空，不符合要求"+testCase);
                }
            }
            else
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString(metaEnum.valueToKey(emScriptRunState))+"波特率协商流程异常，"+state);
                p_AbstractScriptHost->updateProgress(pState,QString(metaEnum.valueToKey(emScriptRunState))+"波特率协商流程异常，"+state+testCase);
            }

            break;
        }
        default:
        {
            break;
        }
    }
}

void Script_20Standard_BaudConsult_ConfirmDelay_CCO::processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID)
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
void Script_20Standard_BaudConsult_ConfirmDelay_CCO::processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID)
{
    bool haveCompleteMsg=true;
    while(haveCompleteMsg)
    {
        shared_ptr<FrameOOPBase> MsgBase_OOP_ptr = FrameOOPHelper::DecodeMsg(&((*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[mtrlID]->buf698),haveCompleteMsg);

        if(MsgBase_OOP_ptr==nullptr)
        {
            continue;
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
                        if(dvcType==SingleSTA) {
                            readMeterNoFlag=true;
                            tryTimes=0;
                        }
                    }
                    else
                    {
                        QByteArray tmpSendMsg=prcsOther698Msg(MsgBase_OOP_ptr,(*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[mtrlID]->mtrAddr,p_CtrInfoList);
                        sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                        if(dvcType==SingleSTA) {
                            QString tmp=tmpSendMsg.toHex().toUpper();
                            meterMsg.clear();
                            if(tmp.contains("00100200"))
                                meterMsg=tmpSendMsg;
                        }
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
void Script_20Standard_BaudConsult_ConfirmDelay_CCO::sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame)
{
    if(frame==p_Afn02F1)
    {
        p_Afn02F1->ctrl_field_={kHplc,kActive,kDirDown};
        p_Afn02F1->info_field_.info_field_down.comu_module_ident=1;
        p_Afn02F1->info_field_.info_field_down.msg_seq=char(++msgSeq);

        p_Afn02F1->protocol_type_=0x03;
        QByteArray cco=QByteArray::fromHex(Address(p_CtrInfoList->at(0)->ccoAddr).toString().toLatin1());
        QByteArray meter=QByteArray::fromHex(Address(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(index)->mtrAddr).toString().toLatin1());
        uchar tmpAddr[6];
        memcpy(tmpAddr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(index)->mtrAddr,6);

        shared_ptr<GetRequestNormal> p_GetRequestNormal_ReadData=make_shared<GetRequestNormal>();
        p_GetRequestNormal_ReadData->ctrl_field_.dir = 0;
        p_GetRequestNormal_ReadData->ctrl_field_.prm = 1;
        p_GetRequestNormal_ReadData->ctrl_field_.fra = 0;
        p_GetRequestNormal_ReadData->ctrl_field_.res = 0;
        p_GetRequestNormal_ReadData->ctrl_field_.sc = 0;
        p_GetRequestNormal_ReadData->ctrl_field_.func = 3;

        p_GetRequestNormal_ReadData->address_field_.sa.addr_type = 0;
        p_GetRequestNormal_ReadData->address_field_.sa.logic_addr = 0;
        p_GetRequestNormal_ReadData->address_field_.sa.addr_len = 5;
        p_GetRequestNormal_ReadData->address_field_.sa.address = QString((QByteArray(reinterpret_cast<char*>(tmpAddr),6).toHex()));
        p_GetRequestNormal_ReadData->address_field_.ca.address = 0x00;

        p_GetRequestNormal_ReadData->oad_.OI = PosActEne_OI;
        p_GetRequestNormal_ReadData->oad_.attribute.feature = 0;
        p_GetRequestNormal_ReadData->oad_.attribute.seq = 2;
        p_GetRequestNormal_ReadData->oad_.element_index = 0;

        p_GetRequestNormal_ReadData->piid_.reserve = 0;
        p_GetRequestNormal_ReadData->piid_.serve_priority = 0;
        p_GetRequestNormal_ReadData->piid_.serve_seq = 1;

        p_GetRequestNormal_ReadData->time_tag_field_.optional_ = 0;
        p_Afn02F1->frame_content_=p_GetRequestNormal_ReadData->EncodeFrame();
        p_Afn02F1->frame_length_=uchar(p_Afn02F1->frame_content_.size());

        memcpy(&p_Afn02F1->address_field_.src_addr,cco,6);
        memcpy(&p_Afn02F1->address_field_.dst_addr,meter,6);

        sendMsgOct=p_Afn02F1->EncodeFrame();
        sendMsgLog=QString("》》抄读电表02F1：%1\n").arg(QString(sendMsgOct.toHex()));
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
void Script_20Standard_BaudConsult_ConfirmDelay_CCO::sendSrcMsg(DvcType dvcType, int dvcId, QByteArray msg)
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

void Script_20Standard_BaudConsult_ConfirmDelay_CCO::timer_timeoutProc()
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
void Script_20Standard_BaudConsult_ConfirmDelay_CCO::maxAllowTimer_timeoutProc()
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
void Script_20Standard_BaudConsult_ConfirmDelay_CCO::delayTimer_timeoutProc()
{
    switch(emScriptRunState)
    {
        default:
        {
            break;
        }
    }
}
