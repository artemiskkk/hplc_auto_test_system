#include "Script_TaskConfig_Enable_Hunan.h"

Script_TaskConfig_Enable_Hunan::Script_TaskConfig_Enable_Hunan(QObject *parent) : QObject(parent)
{
    emScriptRunState=TestInit;
    resultFlag=false;
    sendMsgOct.clear();

    p_CtrInfoList=make_shared<QList<shared_ptr<CtrInfo>>>();
    p_BuildNetwork_GW=new BuildNetwork_GW();

    p_Frame3762Helper=make_shared<Frame3762Helper>();
    p_SetTaskConfig_05F103=make_shared<Afn05F103>();
    p_QueryTaskConfig_CCO_03F102=make_shared<Afn03F102>();
    p_QueryTaskConfig_STA_10F103=make_shared<Afn10F103_Hunan>();

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
Script_TaskConfig_Enable_Hunan::~Script_TaskConfig_Enable_Hunan()
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
void Script_TaskConfig_Enable_Hunan::execute()
{
    p_AbstractScriptHost->updateProgress(ProcessState_Start, "Test start!");

    emScriptRunState=TestInit;
    resultFlag=false;
    addrList.clear();
    curveDataInit();
    //路由周期计算
    if(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size()>420)
        routerPeriod=420;
    else
        routerPeriod=300;
    if(needBuildNet==true)
    {
        p_BuildNetwork_GW->execute();
        emScriptRunState=Wait_BuildNetFinish_Whole;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "等待全网组网完成");
        p_timer->start((timerForReachThresld+timerAfterReachThresld)*1000);
    }
    else
    {
        emScriptRunState=Wait_QueryTaskConfig_645_Init_Finish;
        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryTaskConfig_CCO_03F102);
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询任务配置（03F102），等待--回复");
        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
        emScriptRunState=Wait_SetTaskConfig_645_05F103_Finish;
        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_SetTaskConfig_05F103);
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--设置任务配置（05F103），等待--回复");
        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
    }
    p_maxAllowTimer->start((timerForReachThresld+timerAfterReachThresld)*1000);

}
void Script_TaskConfig_Enable_Hunan::stop()
{
    if(p_AbstractScriptHost==nullptr)
        return;
    p_AbstractScriptHost->updateProgress(ProcessState_Error, "Test stop!");
    if(needPowerOff==true)
        powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost);
}
void Script_TaskConfig_Enable_Hunan::addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq)
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
void Script_TaskConfig_Enable_Hunan::setHost(AbstractScriptHost *host)
{
    p_AbstractScriptHost=host;
    p_BuildNetwork_GW->setHost(host);
}
bool Script_TaskConfig_Enable_Hunan::config(const QMap<QString,QString> *paraDic)
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
        result = true;
    }
    return result;
}
void Script_TaskConfig_Enable_Hunan::processMsg(DvcType dvcType, int id, uchar *data, int datalen)
{
    if(p_CtrInfoList->size()==0)
        return;
    if(emScriptRunState==Wait_BuildNetFinish_Whole)
    {
        if(!p_BuildNetwork_GW->buildNetworkResultFlag)
            p_BuildNetwork_GW->processMsg(dvcType,id,data,datalen);
        else
        {
            if(p_BuildNetwork_GW->emScriptRunState==BuildNetFinish&&p_BuildNetwork_GW->buildNetworkResultFlag==true)
            {
                tryTimes=0;
                emScriptRunState=Wait_QueryTaskConfig_645_Init_Finish;
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryTaskConfig_CCO_03F102);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询任务配置（03F102），等待--回复");
                p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
            }
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
void Script_TaskConfig_Enable_Hunan::processCtrlDvcRes(DvcType dvcType, QList<int> idList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params)
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
        case ScriptSuccess:
        {
            break;
        }
        default:
            break;
    }
}

void Script_TaskConfig_Enable_Hunan::processMsgFromCCO(DvcType dvcType, int dvcId)
{
    if(dvcId!=p_CtrInfoList->at(0)->ctrlID)
        return;
    bool haveCompleteMsg=true;
    while(haveCompleteMsg)
    {
        qInfo()<<QString("CCO-解析前 buf3762=%1").arg(QString((p_CtrInfoList->at(0)->buf.toHex())));
        shared_ptr<Frame3762Base> p_Frame3762Base = p_Frame3762Helper->DecodeLocalMsg(&(p_CtrInfoList->at(0)->buf),haveCompleteMsg,Hunan);
        qInfo()<<QString("CCO-解析后 buf3762=%1").arg(QString((p_CtrInfoList->at(0)->buf.toHex())));
        if(p_Frame3762Base==nullptr)
        {
            continue;
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
            case Wait_QueryTaskConfig_645_Init_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x02&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();

                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("回复否认"));

                    emScriptRunState=Wait_QueryTaskConfig_OOP_Init_Finish;
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryTaskConfig_CCO_03F102);
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询任务配置（03F102），等待--回复");
                    p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_QueryTaskConfig_OOP_Init_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x02&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();

                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("回复否认"));

                    emScriptRunState=Wait_SetTaskConfig_645_05F103_Finish;
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_SetTaskConfig_05F103);
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--设置任务配置（05F103），等待--回复");
                    p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_SetTaskConfig_645_05F103_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();

                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("回复确认"));

                    emScriptRunState=Wait_SetTaskConfig_OOP_05F103_Finish;
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_SetTaskConfig_05F103);
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--设置任务配置（05F103），等待--回复");
                    p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_SetTaskConfig_OOP_05F103_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();

                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("回复确认"));
                    //p_delayTimer->start(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size()*5*1000);//等待所有电表发送完毕。
                    //查询CCO的任务方案
                    emScriptRunState=Wait_QueryTaskConfig_645_CCO_03F102_Finish;
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryTaskConfig_CCO_03F102);
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询CCO（645）任务配置（03F102），等待--回复");
                    p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_QueryTaskConfig_645_CCO_03F102_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x03&&p_Frame3762Base->dt1_==0x20&&p_Frame3762Base->dt2_==0x0C&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    shared_ptr<Afn03F102> p_QueryTaskConfig_CCO_03F102_Up=dynamic_pointer_cast<Afn03F102>(p_Frame3762Base);
                    if(p_QueryTaskConfig_CCO_03F102_Up->data_unit_up_==curveData_645_03F102)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("查询CCO（645）任务配置信息一致"));
                        //查询CCO的任务方案
                        emScriptRunState=Wait_QueryTaskConfig_OOP_CCO_03F102_Finish;
                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryTaskConfig_CCO_03F102);
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询CCO（OOP）任务配置（03F102），等待--回复");
                        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("查询CCO（645）任务配置信息不一致"));
                    }
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_QueryTaskConfig_OOP_CCO_03F102_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x03&&p_Frame3762Base->dt1_==0x20&&p_Frame3762Base->dt2_==0x0C&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    shared_ptr<Afn03F102> p_QueryTaskConfig_CCO_03F102_Up=dynamic_pointer_cast<Afn03F102>(p_Frame3762Base);
                    if(p_QueryTaskConfig_CCO_03F102_Up->data_unit_up_==curveData_OOP_03F102)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("查询CCO（OOP）任务配置信息一致"));
                        //查询STA的任务配置状态
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("等待%1s,开始查询STA任务配置状态").arg(routerPeriod));
                        p_delayTimer->start(routerPeriod*routerPeriodNum*1000);
                        emScriptRunState=Wait_QueryTaskConfig_STA_10F103_Finish;
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("查询CCO（OOP）任务配置信息不一致"));
                    }
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_QueryTaskConfig_STA_10F103_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x10&&p_Frame3762Base->dt1_==0x40&&p_Frame3762Base->dt2_==0x0C&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    shared_ptr<Afn10F103_Hunan> p_QueryTaskConfig_STA_10F103_Up=dynamic_pointer_cast<Afn10F103_Hunan>(p_Frame3762Base);
                    for(int i=0;i<p_QueryTaskConfig_STA_10F103_Up->module_config_unit_.node_info_list_.size();i++)
                    {
                        if(p_QueryTaskConfig_STA_10F103_Up->module_config_unit_.node_info_list_.at(i).config_state_!=SUCCESS)
                        {
                            //失败表号记录
                            failStateSta+=QString(QByteArray(p_QueryTaskConfig_STA_10F103_Up->module_config_unit_.node_info_list_.at(i).node_address_.addr,6).toHex()+"\n");
                        }
                    }
                    index+=queryNodeNumPerTime;
                    if(index<times)
                    {
                        emScriptRunState=Wait_QueryTaskConfig_STA_10F103_Finish;
                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryTaskConfig_STA_10F103);
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询STA任务配置状态（10F103），等待--回复");
                        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                    {
                        if(failStateSta.isEmpty())
                        {
                            emScriptRunState=ScriptSuccess;
                            p_AbstractScriptHost->updateProgress(ProcessState_Success, QString("任务配置-使能，测试成功"));
                        }
                        else
                        {
                            failStateSta.prepend("配置状态失败STA如下：\n");
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing,failStateSta);
                            p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("任务配置-使能，测试失败"));
                        }
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
        }
    }
}
void Script_TaskConfig_Enable_Hunan::processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID)
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
void Script_TaskConfig_Enable_Hunan::processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID)
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
void Script_TaskConfig_Enable_Hunan::sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame)
{
    if(frame==p_SetTaskConfig_05F103)
    {
        p_SetTaskConfig_05F103->ctrl_field_.dir=kDirDown;
        p_SetTaskConfig_05F103->ctrl_field_.prm=kActive;
        p_SetTaskConfig_05F103->ctrl_field_.comn_type=kHplc;

        p_SetTaskConfig_05F103->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_SetTaskConfig_05F103->info_field_.info_field_down.comu_rate=0;
        p_SetTaskConfig_05F103->info_field_.info_field_down.comu_module_ident=0;

        //配置曲线数据
        QByteArray curveConfigData;
        if(emScriptRunState==Wait_SetTaskConfig_645_05F103_Finish)
        {
            p_SetTaskConfig_05F103->curve_config_data_=curveData_645_05F103;
        }
        else if(emScriptRunState==Wait_SetTaskConfig_OOP_05F103_Finish)
        {
            p_SetTaskConfig_05F103->curve_config_data_=curveData_OOP_05F103;
        }
        sendMsgOct=p_SetTaskConfig_05F103->EncodeFrame();
        sendMsgLog=QString("》》设置任务配置05F103：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_QueryTaskConfig_CCO_03F102)
    {
        p_QueryTaskConfig_CCO_03F102->ctrl_field_.dir=kDirDown;
        p_QueryTaskConfig_CCO_03F102->ctrl_field_.prm=kActive;
        p_QueryTaskConfig_CCO_03F102->ctrl_field_.comn_type=kHplc;

        p_QueryTaskConfig_CCO_03F102->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_QueryTaskConfig_CCO_03F102->info_field_.info_field_down.comu_rate=0;
        p_QueryTaskConfig_CCO_03F102->info_field_.info_field_down.comu_module_ident=0;

        if(emScriptRunState==Wait_QueryTaskConfig_645_CCO_03F102_Finish||emScriptRunState==Wait_QueryTaskConfig_645_Init_Finish)
        {
            p_QueryTaskConfig_CCO_03F102->protocol_=DLT645_2007;
        }
        else if(emScriptRunState==Wait_QueryTaskConfig_OOP_CCO_03F102_Finish||emScriptRunState==Wait_QueryTaskConfig_OOP_Init_Finish)
        {
            p_QueryTaskConfig_CCO_03F102->protocol_=OOP;
        }
        sendMsgOct=p_QueryTaskConfig_CCO_03F102->EncodeFrame();
        sendMsgLog=QString("》》查询CCO任务配置03F102：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_QueryTaskConfig_STA_10F103)
    {
        p_QueryTaskConfig_STA_10F103->ctrl_field_.dir=kDirDown;
        p_QueryTaskConfig_STA_10F103->ctrl_field_.prm=kActive;
        p_QueryTaskConfig_STA_10F103->ctrl_field_.comn_type=kHplc;

        p_QueryTaskConfig_STA_10F103->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_QueryTaskConfig_STA_10F103->info_field_.info_field_down.comu_rate=0;
        p_QueryTaskConfig_STA_10F103->info_field_.info_field_down.comu_module_ident=0;

        p_QueryTaskConfig_STA_10F103->node_start_no_=index;
        p_QueryTaskConfig_STA_10F103->node_num_=queryNodeNumPerTime;

        sendMsgOct=p_QueryTaskConfig_STA_10F103->EncodeFrame();
        sendMsgLog=QString("》》查询STA任务配置状态10F103：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
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
void Script_TaskConfig_Enable_Hunan::sendSrcMsg(DvcType dvcType, int dvcId, QByteArray msg)
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

void Script_TaskConfig_Enable_Hunan::timer_timeoutProc()
{
    switch(emScriptRunState)
    {
        case Wait_BuildNetFinish_Whole:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_BuildNetFinish_Whole timeout!!!");
            break;
        }
        default:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==p_timer");
            break;
        }
    }
}
void Script_TaskConfig_Enable_Hunan::maxAllowTimer_timeoutProc()
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
void Script_TaskConfig_Enable_Hunan::delayTimer_timeoutProc()
{
    p_delayTimer->stop();
    times=ushort(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size());
    index=0;
    emScriptRunState=Wait_QueryTaskConfig_STA_10F103_Finish;
    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryTaskConfig_STA_10F103);
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询STA任务配置状态（10F103），等待--回复");
    p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
}

QList<DataIdLen> Script_TaskConfig_Enable_Hunan::dataIdInit(QString dataId)
{
    QList<DataIdLen> dataIdUnitList;
    QStringList dataIdList=dataId.split(',');
    for(int i=0;i<dataIdList.size();i++)
    {
        QStringList dataIdAndLen=dataIdList.at(i).split(' ');
        DataIdLen dataIdST;
        memcpy(dataIdST.data_id_,QByteArray::fromHex(dataIdAndLen.at(0).toLatin1()),4);
        dataIdST.reply_data_len_=uchar(dataIdAndLen.at(1).toUInt(nullptr,16));
        dataIdUnitList.append(dataIdST);
    }
    return dataIdUnitList;
}

void Script_TaskConfig_Enable_Hunan::curveDataInit()
{
    //645数据项
    singleDataId_645_List=dataIdInit(single645);
    triDataId_645_List=dataIdInit(tri645);
    //OOP OAD
    singleDataId_OOP_List=dataIdInit(singleOOP);
    triDataId_OOP_List=dataIdInit(triOOP);
    //待增加配置功能
    //05F103
    /////////////645
    //645 单
    curveData_645_05F103.set_flag_=0x01;
    curveData_645_05F103.read_period_=0x01;//1min
    curveData_645_05F103.protocol=0x02;
    curveData_645_05F103.single_meter_=0x00;
    curveData_645_05F103.single_data_id_num_=0x06;
    uchar single_645_len=0;
    for(int i=0;i<6;i++)
    {
        curveData_645_05F103.single_data_id_len_list_.append(singleDataId_645_List.at(i));
        single_645_len+=singleDataId_645_List.at(i).reply_data_len_;
    }
    curveData_645_05F103.single_reply_data_len_=single_645_len;
    //645 三
    curveData_645_05F103.tri_meter_=0x01;
    curveData_645_05F103.tri_data_id_num_=0x06;
    uchar tri_645_len=0;
    for(int i=0;i<6;i++)
    {
        curveData_645_05F103.tri_data_id_len_list_.append(triDataId_645_List.at(i));
        tri_645_len+=triDataId_645_List.at(i).reply_data_len_;
    }
    curveData_645_05F103.tri_reply_data_len_=tri_645_len;
    /////////////OOP
    //OOP 单
    curveData_OOP_05F103.set_flag_=0x01;
    curveData_OOP_05F103.read_period_=0x01;//1min
    curveData_OOP_05F103.protocol=0x03;
    curveData_OOP_05F103.single_meter_=0x00;
    curveData_OOP_05F103.single_data_id_num_=0x06;
    uchar single_OOP_len=0;
    for(int i=0;i<6;i++)
    {
        curveData_OOP_05F103.single_data_id_len_list_.append(singleDataId_OOP_List.at(i));
        single_OOP_len+=singleDataId_OOP_List.at(i).reply_data_len_;
    }
    curveData_OOP_05F103.single_reply_data_len_=single_645_len;
    //645 三
    curveData_OOP_05F103.tri_meter_=0x01;
    curveData_OOP_05F103.tri_data_id_num_=0x06;
    uchar tri_OOP_len=0;
    for(int i=0;i<6;i++)
    {
        curveData_OOP_05F103.tri_data_id_len_list_.append(triDataId_OOP_List.at(i));
        tri_OOP_len+=triDataId_OOP_List.at(i).reply_data_len_;
    }
    curveData_OOP_05F103.tri_reply_data_len_=tri_OOP_len;
    //03F102
    //待增加配置功能
    /////////////645
    //645 单
    curveData_645_03F102.protocol=curveData_645_05F103.protocol;
    curveData_645_03F102.single_meter_=curveData_645_05F103.single_meter_;
    curveData_645_03F102.single_data_id_num_=curveData_645_05F103.single_data_id_num_;
    curveData_645_03F102.single_reply_data_len_=curveData_645_05F103.single_reply_data_len_;
    curveData_645_03F102.single_data_id_len_list_=curveData_645_05F103.single_data_id_len_list_;
    //645 三
    curveData_645_03F102.tri_meter_=curveData_645_05F103.tri_meter_;
    curveData_645_03F102.tri_data_id_num_=curveData_645_05F103.tri_data_id_num_;
    curveData_645_03F102.tri_reply_data_len_=curveData_645_05F103.tri_reply_data_len_;
    curveData_645_03F102.tri_data_id_len_list_=curveData_645_05F103.tri_data_id_len_list_;
    /////////////OOP
    //OOP 单
    curveData_OOP_03F102.protocol=curveData_OOP_05F103.protocol;
    curveData_OOP_03F102.single_meter_=curveData_OOP_05F103.single_meter_;
    curveData_OOP_03F102.single_data_id_num_=curveData_OOP_05F103.single_data_id_num_;
    curveData_OOP_03F102.single_reply_data_len_=curveData_OOP_05F103.single_reply_data_len_;
    curveData_OOP_03F102.single_data_id_len_list_=curveData_OOP_05F103.single_data_id_len_list_;
    //645 三
    curveData_OOP_03F102.tri_meter_=curveData_OOP_05F103.tri_meter_;
    curveData_OOP_03F102.tri_data_id_num_=curveData_OOP_05F103.tri_data_id_num_;
    curveData_OOP_03F102.tri_reply_data_len_=curveData_OOP_05F103.tri_reply_data_len_;
    curveData_OOP_03F102.tri_data_id_len_list_=curveData_OOP_05F103.tri_data_id_len_list_;
}

