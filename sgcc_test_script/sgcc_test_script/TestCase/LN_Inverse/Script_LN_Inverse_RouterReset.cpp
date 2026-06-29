#include "Script_LN_Inverse_RouterReset.h"

Script_LN_Inverse_RouterReset::Script_LN_Inverse_RouterReset(QObject *parent) : QObject(parent)
{
    emScriptRunState=TestInit;
    resultFlag=false;
    sendMsgOct.clear();

    p_CtrInfoList=make_shared<QList<shared_ptr<CtrInfo>>>();
    p_BuildNetwork_GW=new BuildNetwork_GW();

    p_Frame3762Helper=make_shared<Frame3762Helper>();
    p_RouterPause_12F2=make_shared<Afn12F2>();
    p_QueryLNInfo_10F31=make_shared<Afn10F31>();
    p_MonitorSlaveNode_13F1=make_shared<Afn13F1>();
    p_AddSlaveNode_11F1=make_shared<Afn11F1>();
    p_HardReset_01F1=make_shared<Afn01F1>();
    p_QueryNetScale_10F9=make_shared<Afn10F9>();

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
Script_LN_Inverse_RouterReset::~Script_LN_Inverse_RouterReset()
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

void Script_LN_Inverse_RouterReset::execute()
{
    p_AbstractScriptHost->updateProgress(ProcessState_Start, "Test start!");

    emScriptRunState=TestInit;
    resultFlag=false;
    addrList.clear();

    if(needBuildNet==true)
    {
        p_BuildNetwork_GW->execute();
        emScriptRunState=Wait_BuildNetFinish_Whole;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "等待全网组网完成");
        p_timer->start((timerForReachThresld+timerAfterReachThresld)*1000);
    }
    else
    {
        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_RouterPause_12F2);
        emScriptRunState=Wait_00F1_for_12F2_Pause;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--路由暂停（12F2），等待--确认");
        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
    }
    p_maxAllowTimer->start((timerForReachThresld+timerAfterReachThresld)*1000);

}
void Script_LN_Inverse_RouterReset::stop()
{
    if(p_AbstractScriptHost==nullptr)
        return;
    p_AbstractScriptHost->updateProgress(ProcessState_Error, "Test stop!");
    if(needPowerOff==true)
        powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost);
}
void Script_LN_Inverse_RouterReset::addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq)
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
void Script_LN_Inverse_RouterReset::setHost(AbstractScriptHost *host)
{
    p_AbstractScriptHost=host;
    p_BuildNetwork_GW->setHost(host);
}
bool Script_LN_Inverse_RouterReset::config(const QMap<QString,QString> *paraDic)
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
        if(paraDic->keys().contains("querySpan"))
        {
            this->querySpan = (*paraDic)["querySpan"].toUShort();
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
void Script_LN_Inverse_RouterReset::processMsg(DvcType dvcType, int id, uchar *data, int datalen)
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
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,id,p_RouterPause_12F2);
                emScriptRunState=Wait_00F1_for_12F2_Pause;
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--路由暂停（12F2），等待--确认");
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
void Script_LN_Inverse_RouterReset::processCtrlDvcRes(DvcType dvcType, QList<int> idList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params)
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
        case Wait_00F1_for_12F2_Pause:
        {
            break;
        }
        case Wait_QueryLN_13F1_Finish:
        {
            break;
        }
        case Wait_QueryLN_10F31_Finish:
        {
            break;
        }
        case ScriptSuccess:
        {
            break;
        }
    }
}
void Script_LN_Inverse_RouterReset::processMsgFromCCO(DvcType dvcType, int dvcId)
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
            case Wait_00F1_for_12F2_Pause:
            {
                if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();//接收到一条完整报文
                    nodeInfoList.clear();
                    nodeIndex=1;
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryLNInfo_10F31);
                    emScriptRunState=Wait_QueryLN_10F31_Finish;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询从节点相线信息（10F31），等待--确认");
                    p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_QueryLN_10F31_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x10&&p_Frame3762Base->dt1_==0x40&&p_Frame3762Base->dt2_==0x03&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    shared_ptr<Afn10F31> p_QueryLNInfo_10F31_Up=dynamic_pointer_cast<Afn10F31>(p_Frame3762Base);
                    //查询本次的相线情况，如果有未知的则等待s再查询
                    for(int i=0;i<p_QueryLNInfo_10F31_Up->node_phase_info_unit_.this_node_num_;i++)
                    {
                        if(uchar(p_QueryLNInfo_10F31_Up->node_phase_info_unit_.node_info_list_.at(i).node_phase_info_.phase_)==0)
                        {
                            p_delayTimer->start(querySpan*1000);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("出现相位未知情况，等待%1s后再查询").arg(querySpan));
                            return;
                        }
                    }
                    nodeInfoList.append(p_QueryLNInfo_10F31_Up->node_phase_info_unit_.node_info_list_);
                    nodeIndex+=topoCntPerTime;
                    if(nodeIndex<p_QueryLNInfo_10F31_Up->node_phase_info_unit_.node_total_num_)
                    {
                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryLNInfo_10F31);
                        emScriptRunState=Wait_QueryLN_10F31_Finish;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询从节点相线信息（10F31），等待--回复");
                        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                    {
                        statisticsResult();
                        index=0;
                        times=ushort(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size());
                        ReadLNInfoList_13F1.clear();
                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,index,p_MonitorSlaveNode_13F1);
                        emScriptRunState=Wait_QueryLN_13F1_Finish;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--监控从节点（13F1），等待--回复");

                    }
                }
                else if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    if(emCmdType==AddNode)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "添加从节点，回复确认！");
                        index+=addCntPerTime;
                        if(index<p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size())
                        {
                            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_AddSlaveNode_11F1);
                            emScriptRunState=Wait_QueryLN_10F31_Finish;
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--添加从节点（11F1），等待--确认");
                            p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                        }
                        else
                        {
                            nodeInfoList.clear();
                            nodeIndex=1;
                            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryLNInfo_10F31);
                            emScriptRunState=Wait_QueryLN_10F31_Finish;
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询从节点相线信息（10F31），等待--确认");
                            p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                        }
                    }
                    else if(emCmdType==HardReset)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "硬件初始化，回复确认！");
                    }
                }
                else if(p_Frame3762Base->afn_ == 0x03&&p_Frame3762Base->dt1_==0x02&&p_Frame3762Base->dt2_==0x01&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    index=0;
                    flagQueryNetScale=true;
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryNetScale_10F9);
                    emScriptRunState=Wait_QueryLN_10F31_Finish;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询网络规模（10F9），等待--回复");
                    p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                }
                else if(p_Frame3762Base->afn_ == 0x10&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x01&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    shared_ptr<Afn10F9> p_QueryNetScale_10F9_Up=std::dynamic_pointer_cast<Afn10F9>(p_Frame3762Base);
                    if(p_QueryNetScale_10F9_Up->network_scale_==p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size()+1)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "网络规模与档案一致，符合要求！");
                        flagQueryNetScale=false;
                        nodeInfoList.clear();
                        nodeIndex=1;
                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryLNInfo_10F31);
                        emScriptRunState=Wait_QueryLN_10F31_Finish;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询从节点相线信息（10F31），等待--确认");
                        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("查询网络规模（%1轮）！").arg(++index));
                        p_delayTimer->start(5*1000);

                    }
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_QueryLN_13F1_Finish:
            {
                if(p_Frame3762Base->afn_==0x13&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    shared_ptr<Afn13F1> p_MonitorSlaveNode_13F1_Up=dynamic_pointer_cast<Afn13F1>(p_Frame3762Base);

                    ReadLNInfo_Struct LNInfo;
                    memcpy(LNInfo.nodeAddress.addr,p_MonitorSlaveNode_13F1_Up->address_field_.src_addr,6);
                    LNInfo.hasRead=true;
                    LNInfo.lineFlag=p_MonitorSlaveNode_13F1_Up->info_field_.info_field_up.line_flag;
                    ReadLNInfoList_13F1.append(LNInfo);
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到电表%1的线路标志;").arg(QString(QByteArray(p_MonitorSlaveNode_13F1_Up->address_field_.src_addr,6).toHex())));

                    if(++index>=times)
                    {
                        index=0;
                        statisticsResult();
                        if(emCmdType==NormalCmd)
                        {
                            index=0;
                            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_AddSlaveNode_11F1);
                            emScriptRunState=Wait_QueryLN_10F31_Finish;
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--添加从节点（11F1），等待--确认");
                            p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                            emCmdType=AddNode;
                        }
                        else if(emCmdType==AddNode)
                        {
                            index=0;
                            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_HardReset_01F1);
                            emScriptRunState=Wait_QueryLN_10F31_Finish;
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--路由硬件初始化（01F1），等待--确认");
                            p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                            emCmdType=HardReset;
                        }
                        else if(emCmdType==HardReset)
                        {
                            emScriptRunState=ScriptSuccess;
                            if(errorTimes==0)
                            {
                                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "未出现识别错误情况");
                                p_AbstractScriptHost->updateProgress(ProcessState_Success, QString("LN互逆-路由复位测试成功！"));
                            }
                            else
                            {
                                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "出现识别错误情况");
                                p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("LN互逆-路由复位测试失败！"));
                            }
                        }

                    }
                    else
                    {
                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,index,p_MonitorSlaveNode_13F1);
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--监控从节点（13F1），等待--回复");

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
void Script_LN_Inverse_RouterReset::processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID)
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
void Script_LN_Inverse_RouterReset::processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID)
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

void Script_LN_Inverse_RouterReset::sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame)
{
    if(frame==p_HardReset_01F1)
    {
        p_HardReset_01F1->ctrl_field_.dir=kDirDown;
        p_HardReset_01F1->ctrl_field_.prm=kActive;
        p_HardReset_01F1->ctrl_field_.comn_type=kHplc;

        p_HardReset_01F1->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_HardReset_01F1->info_field_.info_field_down.comu_rate=0;
        p_HardReset_01F1->info_field_.info_field_down.comu_module_ident=0;
        sendMsgOct=p_HardReset_01F1->EncodeFrame();
        sendMsgLog=QString("》》硬件初始化01F1：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_AddSlaveNode_11F1)
    {
        p_AddSlaveNode_11F1->ctrl_field_.dir=kDirDown;
        p_AddSlaveNode_11F1->ctrl_field_.prm=kActive;
        p_AddSlaveNode_11F1->ctrl_field_.comn_type=kHplc;

        p_AddSlaveNode_11F1->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_AddSlaveNode_11F1->info_field_.info_field_down.comu_rate=0;
        p_AddSlaveNode_11F1->info_field_.info_field_down.comu_module_ident=0;

        //////
        p_AddSlaveNode_11F1->node_parameter_list_.clear();
        if(index+addCntPerTime<p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size())
        {
            for(int i=0;i<addCntPerTime;i++)
            {
                NodeParameter nodePara;
                memcpy(nodePara.node_address_.addr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(index+i)->mtrAddr,6);
                nodePara.protocol_type_=char(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(index+i)->prtcl);
                p_AddSlaveNode_11F1->node_parameter_list_.append(nodePara);
            }
        }
        else
        {
            for(int i=0;i<p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size()-index;i++)
            {
                NodeParameter nodePara;
                memcpy(nodePara.node_address_.addr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(index+i)->mtrAddr,6);
                nodePara.protocol_type_=char(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(index+i)->prtcl);
                p_AddSlaveNode_11F1->node_parameter_list_.append(nodePara);
            }
        }
        p_AddSlaveNode_11F1->node_num_=uchar(p_AddSlaveNode_11F1->node_parameter_list_.size());
        sendMsgOct=p_AddSlaveNode_11F1->EncodeFrame();
        sendMsgLog=QString("》》添加从节点11F1：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_RouterPause_12F2)
    {
        p_RouterPause_12F2->ctrl_field_.dir=kDirDown;
        p_RouterPause_12F2->ctrl_field_.prm=kActive;
        p_RouterPause_12F2->ctrl_field_.comn_type=kHplc;

        p_RouterPause_12F2->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_RouterPause_12F2->info_field_.info_field_down.comu_rate=0;
        p_RouterPause_12F2->info_field_.info_field_down.comu_module_ident=0;

        sendMsgOct=p_RouterPause_12F2->EncodeFrame();
        sendMsgLog=QString("》》路由暂停12F2：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_QueryNetScale_10F9)
    {
        p_QueryNetScale_10F9->ctrl_field_.dir=kDirDown;
        p_QueryNetScale_10F9->ctrl_field_.prm=kActive;
        p_QueryNetScale_10F9->ctrl_field_.comn_type=kHplc;

        p_QueryNetScale_10F9->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_QueryNetScale_10F9->info_field_.info_field_down.comu_rate=0;
        p_QueryNetScale_10F9->info_field_.info_field_down.comu_module_ident=0;

        sendMsgOct=p_QueryNetScale_10F9->EncodeFrame();
        sendMsgLog=QString("》》查询网络规模10F9：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_QueryLNInfo_10F31)
    {
        p_QueryLNInfo_10F31->ctrl_field_.dir=kDirDown;
        p_QueryLNInfo_10F31->ctrl_field_.prm=kActive;
        p_QueryLNInfo_10F31->ctrl_field_.comn_type=kHplc;

        p_QueryLNInfo_10F31->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_QueryLNInfo_10F31->info_field_.info_field_down.comu_rate=0;
        p_QueryLNInfo_10F31->info_field_.info_field_down.comu_module_ident=0;

        p_QueryLNInfo_10F31->node_start_no_=nodeIndex;
        p_QueryLNInfo_10F31->node_num_=topoCntPerTime;

        sendMsgOct=p_QueryLNInfo_10F31->EncodeFrame();
        sendMsgLog=QString("》》查询相线信息10F31：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_MonitorSlaveNode_13F1)
    {
        if(index>=p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size())
        {
            index=0;
        }
        uchar tmpAddr[6];
        memcpy(tmpAddr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(index)->mtrAddr,6);
        uchar comPrtclType=p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(index)->prtcl;

        if(comPrtclType==0x02)
        {
            uchar CrntPosEneTotal[4]={0x00,0x00,0x01,0x00}; //DI0_DI3
            shared_ptr<Rqst_ReadData_0x11> p_ReadData_0x11=make_shared<Rqst_ReadData_0x11>(addr,4,0);
            //reverseAddr(tmpAddr, 6);
            memcpy(p_ReadData_0x11->addr_,tmpAddr,6);
            memcpy(p_ReadData_0x11->di,CrntPosEneTotal,4);
            QByteArray msg645=p_ReadData_0x11->EncodeFrame();

            p_MonitorSlaveNode_13F1->data_field_down_.delay_tag_=0x00;
            p_MonitorSlaveNode_13F1->data_field_down_.sub_node_num_=0x00;
            p_MonitorSlaveNode_13F1->data_field_down_.protocol_type_=0x02;
            p_MonitorSlaveNode_13F1->data_field_down_.frame_content_=msg645;
            p_MonitorSlaveNode_13F1->data_field_down_.frame_length_=uchar(msg645.size());

            p_MonitorSlaveNode_13F1->ctrl_field_.dir=kDirDown;
            p_MonitorSlaveNode_13F1->ctrl_field_.prm=kActive;
            p_MonitorSlaveNode_13F1->ctrl_field_.comn_type=kHplc;

            p_MonitorSlaveNode_13F1->info_field_.info_field_down.msg_seq=char(msgSeq++);
            p_MonitorSlaveNode_13F1->info_field_.info_field_down.comu_rate=0;
            p_MonitorSlaveNode_13F1->info_field_.info_field_down.comu_module_ident=1;

            uchar tmpCcoAddr[6];
            memcpy(tmpCcoAddr,p_CtrInfoList->at(0)->ccoAddr,6);
            memcpy(p_MonitorSlaveNode_13F1->address_field_.src_addr,tmpCcoAddr,6);
            memcpy(p_MonitorSlaveNode_13F1->address_field_.dst_addr,tmpAddr,6);

            sendMsgOct=p_MonitorSlaveNode_13F1->EncodeFrame();
            sendMsgLog=QString("》》监控从节点13F1,抄读645电表：%1\n").arg(QString(sendMsgOct.toHex()));
        }
        else if(comPrtclType==0x03)
        {
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

            QByteArray msg698=p_GetRequestNormal_ReadData->EncodeFrame();

            p_MonitorSlaveNode_13F1->data_field_down_.delay_tag_=0x00;
            p_MonitorSlaveNode_13F1->data_field_down_.sub_node_num_=0x00;
            p_MonitorSlaveNode_13F1->data_field_down_.protocol_type_=0x03;
            p_MonitorSlaveNode_13F1->data_field_down_.frame_content_=msg698;
            p_MonitorSlaveNode_13F1->data_field_down_.frame_length_=uchar(msg698.size());

            p_MonitorSlaveNode_13F1->ctrl_field_.dir=kDirDown;
            p_MonitorSlaveNode_13F1->ctrl_field_.prm=kActive;
            p_MonitorSlaveNode_13F1->ctrl_field_.comn_type=kHplc;

            p_MonitorSlaveNode_13F1->info_field_.info_field_down.msg_seq=char(msgSeq++);
            p_MonitorSlaveNode_13F1->info_field_.info_field_down.comu_rate=0;
            p_MonitorSlaveNode_13F1->info_field_.info_field_down.comu_module_ident=1;

            uchar tmpCcoAddr[6];
            memcpy(tmpCcoAddr,p_CtrInfoList->at(0)->ccoAddr,6);
            memcpy(p_MonitorSlaveNode_13F1->address_field_.src_addr,tmpCcoAddr,6);
            memcpy(p_MonitorSlaveNode_13F1->address_field_.dst_addr,tmpAddr,6);

            sendMsgOct=p_MonitorSlaveNode_13F1->EncodeFrame();
            sendMsgLog=QString("》》监控从节点13F1,抄读OOP电表：%1\n").arg(QString(sendMsgOct.toHex()));

        }
        else
            return;
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
void Script_LN_Inverse_RouterReset::sendSrcMsg(DvcType dvcType, int dvcId, QByteArray msg)
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

void Script_LN_Inverse_RouterReset::timer_timeoutProc()
{
    switch(emScriptRunState)
    {
        case Wait_BuildNetFinish_Whole:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait build whole net finish timeout!!!");
            break;
        }
        case Wait_00F1_for_12F2_Pause:
        {
            if(++tryTimes>=3)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_00F1_for_12F2_Pause1st timeout!!!");
            }
            else
            {
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_RouterPause_12F2);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--路由暂停命令（12F2），等待--确认");
            }
            break;
        }
        case Wait_QueryLN_13F1_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "Wait_13F1_UP timeout!!! Read Next Meter");
            //需要判断是否切换电表
            if(++index>=times)
            {
                index=0;
                return;
            }
            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,index,p_MonitorSlaveNode_13F1);
            p_timer->start(maxMonitorTime*1000);
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--监控从节点报文（13F1），等待--回复");

            break;
        }
        case Wait_QueryLN_10F31_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_QueryLN_10F31_Finish timeout!!!");
            break;
        }
        default:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==p_timer");
            break;
        }
    }
}
void Script_LN_Inverse_RouterReset::maxAllowTimer_timeoutProc()
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
void Script_LN_Inverse_RouterReset::delayTimer_timeoutProc()
{
    p_delayTimer->stop();
    if(flagQueryNetScale==true)
    {
        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryNetScale_10F9);
        emScriptRunState=Wait_QueryLN_10F31_Finish;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询网络规模（10F9），等待--回复");
        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
        return;
    }
    if(emScriptRunState==Wait_QueryLN_10F31_Finish)
    {
        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryLNInfo_10F31);
        emScriptRunState=Wait_QueryLN_10F31_Finish;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询从节点相线信息（10F31），等待--回复");
        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
    }

}

void Script_LN_Inverse_RouterReset::statisticsResult()
{
    int count10F31=0;
    int count13F1=0;
    QString identifyError10F31;
    QString identifyError13F1;
    for(int i=0;i<p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size();i++)
    {
        for(int j=0;j<nodeInfoList.size();j++)
        {
            if(memcmp(nodeInfoList.at(j).node_address_.addr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(i)->mtrAddr,6)==0)
            {
                if(uchar(nodeInfoList.at(j).node_phase_info_.line_abnormal_flag_)!=p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(i)->phaseSeq)
                {
                    count10F31++;
                    identifyError10F31+=QString(QByteArray(nodeInfoList.at(j).node_address_.addr,6).toHex())+";";
                    if(count10F31%8==0)
                        identifyError10F31+="\n";
                }
            }
        }
        for(int j=0;j<ReadLNInfoList_13F1.size();j++)
        {
            if(memcmp(ReadLNInfoList_13F1.at(j).nodeAddress.addr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(i)->mtrAddr,6)==0)
            {
                if(uchar(ReadLNInfoList_13F1.at(j).lineFlag)!=p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(i)->phaseSeq)
                {
                    count13F1++;
                    identifyError13F1+=QString(QByteArray(ReadLNInfoList_13F1.at(j).nodeAddress.addr,6).toHex())+";";
                    if(count13F1%8==0)
                        identifyError13F1+="\n";
                }
            }
        }
        nodeInfoList.clear();
        ReadLNInfoList_13F1.clear();
    }
    if(!identifyError10F31.isEmpty())
    {
        errorTimes++;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("10F31查询识别错误电表如下\n%1").arg(identifyError10F31));
    }
    if(!identifyError13F1.isEmpty())
    {
        errorTimes++;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("13F1查询识别错误电表如下\n%1").arg(identifyError13F1));
    }
}
