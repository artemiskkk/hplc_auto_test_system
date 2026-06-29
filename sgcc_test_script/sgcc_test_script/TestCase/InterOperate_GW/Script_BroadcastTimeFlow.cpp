#include "Script_BroadcastTimeFlow.h"

Script_BroadcastTimeFlow::Script_BroadcastTimeFlow(QObject *parent) : InteroperateBase_GW(parent)
{

}
Script_BroadcastTimeFlow::~Script_BroadcastTimeFlow()
{
    stop();
}
void Script_BroadcastTimeFlow::setHost(AbstractScriptHost *host)
{
    p_AbstractScriptHost=host;
    p_InteroperateInit_GW->setHost(host);
}
void Script_BroadcastTimeFlow::addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq)
{
    p_CtrInfoList->clear();
    comnAddAddrsInfo(p_CtrInfoList, p_ConcentratorInfoList, p_MeterInfoList, p_SchemeCfgInfoList);
    uchar dstPrtcl=uchar(freq)>>4;
    if(dstPrtcl==0x03)
        setMeterAddrsPrtcl(p_CtrInfoList,dstPrtcl);
    p_InteroperateInit_GW->setCtrInfoList(p_CtrInfoList);
}
bool Script_BroadcastTimeFlow::config(const QMap<QString,QString> *paraDic)
{
    bool result = true;
    if(paraDic!=nullptr)
    {
//        p_BuildNetwork_GW->config(paraDic);

//        if(paraDic->keys().contains("timerForReachThresld"))
//        {
//            this->timerForReachThresld = (*paraDic)["timerForReachThresld"].toUShort();
//        }
//        if(paraDic->keys().contains("timerAfterReachThresld"))
//        {
//            this->timerAfterReachThresld = (*paraDic)["timerAfterReachThresld"].toUShort();
//        }
//        if(paraDic->keys().contains("netSucRateThresld"))
//        {
//            this->netSucRateThresld = (*paraDic)["netSucRateThresld"].toDouble();
//        }
//        if(paraDic->keys().contains("needBuildNet"))
//        {
//            (*paraDic)["needBuildNet"].toLower()=="true"?this->needBuildNet=true:this->needBuildNet=false;
//        }
//        if(paraDic->keys().contains("needPowerOff"))
//        {
//            (*paraDic)["needPowerOff"].toLower()=="true"?this->needPowerOff=true:this->needPowerOff=false;
//        }
//        result = true;
    }
    return result;
}

void Script_BroadcastTimeFlow::execute()
{
    p_AbstractScriptHost->updateProgress(ProcessState_Start, "广播流程中通信时延相关报文通信机制流程: 开始测试!");
    emScriptRunState=ScriptInit;
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "使用“暂停”命令暂停路由工作 开始...");
    emScriptRunState=Wait_PauseRouter_Finish;
    QString msg=QString("68 0F 00 43 00 00 28 32 00 5F 12 02 00 10 16").replace(" ","");
    sendSrcMsg(CCO_GW,p_CtrInfoList->at(0)->dvcId,QByteArray::fromHex(msg.toLatin1()));
    msgSeq=0x5F;
    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
    p_maxAllowTimer->start((timerForReachThresld+timerAfterReachThresld)*1000);
}
void Script_BroadcastTimeFlow::slotStartFollowTest()
{

}
void Script_BroadcastTimeFlow::stop()
{
    p_timer->stop();
    p_maxAllowTimer->stop();
    p_delayTimer->stop();
}

void Script_BroadcastTimeFlow::processMsg(DvcType dvcType, int id, uchar *data, int datalen)
{
    if(p_CtrInfoList->size()==0)
        return;

    switch(emScriptRunState)
    {
        case ScriptInit:
            break;
        case Wait_InteroperateInit_Finish:
        {
            p_InteroperateInit_GW->processMsg(dvcType, id, data, datalen);
            break;
        }
        default:
        {
            QByteArray tmpRecvTempData=QByteArray::fromRawData(reinterpret_cast<char*>(data),datalen);
            QByteArray recvTempData;
            recvTempData.append(tmpRecvTempData);
            delete[]data;

//            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到报文：%1").arg(QString(recvTempData.toHex())));

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
            break;
        }
    }
}

void Script_BroadcastTimeFlow::processCtrlDvcRes(DvcType dvcType, QList<int> idList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params)
{
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到工装命令回复，参数个数=%1").arg(QString::number(params.size())));
    if(isSucs==false)
        return;
    switch(emScriptRunState)
    {
        case Wait_InteroperateInit_Finish:
        {
            p_InteroperateInit_GW->processCtrlDvcRes(dvcType,idList,ctrlCmdType,isSucs,params);
            break;
        }
        default:
        {
            break;
        }
    }
}

void Script_BroadcastTimeFlow::processMsgFromCCO(DvcType dvcType, int dvcId)
{
    bool haveCompleteMsg=true;
    while(haveCompleteMsg)
    {
        qInfo()<<QString("CCO-解析前 buf3762=%1").arg(QString(p_CtrInfoList->at(0)->buf.toHex()));
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到CCO数据：%1").arg(QString(p_CtrInfoList->at(0)->buf.toHex())));
        shared_ptr<Frame3762Base> p_Frame3762Base = p_Frame3762Helper->DecodeLocalMsg(&(p_CtrInfoList->at(0)->buf),haveCompleteMsg);
        qInfo()<<QString("CCO-解析后 buf3762=%1").arg(QString((p_CtrInfoList->at(0)->buf.toHex())));

        if(p_Frame3762Base==nullptr)
            continue;
        switch(emScriptRunState)
        {
            case Wait_PauseRouter_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp&&uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq)==msgSeq)
                {
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("使用“暂停”命令暂停路由工作 结束"));
                    InteroperateInit_GW::delayTime(10*1000);
                    emScriptRunState=Wait_BroadcastTime_Finish;
                    QString msg=QString("68 21 00 43 00 00 28 32 00 60 03 01 01 02 10 68 99 99 99 99 99 99 68 11 04 33 33 34 33 48 16 BA 16").replace(" ","");
                    sendSrcMsg(dvcType,dvcId,QByteArray::fromHex(msg.toLatin1()));
                    msgSeq=0x60;
                    p_timer->start(90*1000);
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("发送“通信延时相关广播通信时长” 开始"));
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_BroadcastTime_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x03&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x01&&p_Frame3762Base->ctrl_field_.dir == kDirUp&&uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq)==msgSeq)
                {
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("发送“通信延时相关广播通信时长” 结束"));
                    emScriptRunState=Wait_BroadcastTime_Finish;
                    QString msg=QString("68 2D 00 43 04 00 28 32 00 61 88 88 88 99 99 04 99 99 99 99 99 99 05 04 00 02 10 68 99 99 99 99 99 99 68 11 04 33 33 34 33 48 16 27 16 ").replace(" ","");
                    sendSrcMsg(dvcType,dvcId,QByteArray::fromHex(msg.toLatin1()));
                    msgSeq=0x61;
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("发送“启动广播” 开始"));
                }
                else if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp&&uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq)==msgSeq)
                {
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("发送“启动广播” 结束"));
                    emScriptRunState=Wait_RecoverRouter_Finish;
                    QString msg=QString("68 0F 00 43 00 00 28 32 00 62 12 04 00 15 16 ").replace(" ","");
                    sendSrcMsg(dvcType,dvcId,QByteArray::fromHex(msg.toLatin1()));
                    msgSeq=0x62;
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("向路由发送恢复命令AFN=12H-F3 开始"));
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_RecoverRouter_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp&&uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq)==msgSeq)
                {
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("向路由发送恢复命令AFN=12H-F3 结束"));
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("广播流程中通信时延相关报文通信机制流程:流程测试成功"));
                    p_AbstractScriptHost->updateProgress(ProcessState_Success, QString("广播流程中通信时延相关报文通信机制流程:流程测试成功"));
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
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

void Script_BroadcastTimeFlow::processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID)
{
    QString stringDvcType;
    if(dvcType == SingleSTA)
    {
        stringDvcType = QString("单通;");
    }
    else if(dvcType == ThreeSTA)
    {
        stringDvcType = QString("三通;");
    }

    bool haveCompleteMsg=true;
    while(haveCompleteMsg)
    {
        shared_ptr<dlt_645_Protocol::Frame645Base> MsgBase_645_ptr = dlt_645_Protocol::Frame645Helper::DecodeLocalMsg(&((*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[mtrlID]->buf645),haveCompleteMsg);

        if(MsgBase_645_ptr==nullptr)
            break;

        switch(emScriptRunState)
        {
            default:
            {
                if(MsgBase_645_ptr->ctrlCode_==dlt_645_Protocol::READ_ADDR)
                {
                    sendMsg(dvcType,dvcId,mtrlID,p_MeterAddrResp_93);
                }
                else
                {
                    uchar di[4]={0x00};
                    if(MsgBase_645_ptr->ctrlCode_==dlt_645_Protocol::READ_DATA)
                    {
                        shared_ptr<dlt_645_Protocol::Rqst_ReadData_0x11> Rqst_ReadData_0x11_ptr = std::dynamic_pointer_cast<dlt_645_Protocol::Rqst_ReadData_0x11>(MsgBase_645_ptr);
                        memcpy(di,Rqst_ReadData_0x11_ptr->di,4);

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

void Script_BroadcastTimeFlow::processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID)
{
    QString stringDvcType;
    if(dvcType == SingleSTA)
    {
        stringDvcType = QString("单通;");
    }
    else
    {
        stringDvcType = QString("三通;");
    }

    bool haveCompleteMsg=true;
    while(haveCompleteMsg)
    {
        shared_ptr<FrameOOPBase> MsgBase_OOP_ptr = FrameOOPHelper::DecodeMsg(&((*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[mtrlID]->buf698),haveCompleteMsg);

        if(MsgBase_OOP_ptr==nullptr)
            continue;

        switch(emScriptRunState)
        {
            default:
            {
                if(MsgBase_OOP_ptr->service_type_==GET_REQUEST_CLIENT&&MsgBase_OOP_ptr->service_sub_type_==uchar(GetRequestType::kGetRequestNormal))
                {
                    shared_ptr<GetRequestNormal> p_GetRequestNormal=dynamic_pointer_cast<GetRequestNormal>(MsgBase_OOP_ptr);
                    if(p_GetRequestNormal->oad_.OI==ComuAddr)
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

void Script_BroadcastTimeFlow::sendMsg(DvcType , int , int , shared_ptr<void> )
{

}

void Script_BroadcastTimeFlow::timer_timeout()
{
    switch(emScriptRunState)
    {
        default:
        {
            p_timer->stop();
            p_AbstractScriptHost->updateProgress(ProcessState_Error, QString("timeout p_timer!!!"));
            break;
        }
    }
}

void Script_BroadcastTimeFlow::maxAllowTimer_timeout()
{
    p_maxAllowTimer->stop();
    p_AbstractScriptHost->updateProgress(ProcessState_Error, QString("timeout p_maxAllowTimer!!!"));
}

void Script_BroadcastTimeFlow::delayTimer_timeout()
{

}
