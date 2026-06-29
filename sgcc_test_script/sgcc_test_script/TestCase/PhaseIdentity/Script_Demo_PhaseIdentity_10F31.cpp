#include "Script_Demo_PhaseIdentity_10F31.h"
Script_Demo_PhaseIdentity_10F31::Script_Demo_PhaseIdentity_10F31(QObject *parent) : QObject(parent)
{
    emScriptRunState=TestInit;
    resultFlag=false;
    sendMsgOct.clear();

    p_CtrInfoList=make_shared<QList<shared_ptr<CtrInfo>>>();
    p_BuildNetwork_GW=new BuildNetwork_GW();

    p_Frame3762Helper=make_shared<Frame3762Helper>();
    p_RouterPause_12F2=make_shared<Afn12F2>();
    p_QueryNodeInfo_10F2=make_shared<Afn10F2>();
    p_QueryPhaseInfo_10F31=make_shared<Afn10F31>();
    p_MonitorSlaveNode_13F1=make_shared<Afn13F1>();
    p_AddSlaveNode_11F1=make_shared<Afn11F1>();
    p_HardReset_01F1=make_shared<Afn01F1>();
    p_QueryNetTopoInfo_10F21=make_shared<Afn10F21>();
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
Script_Demo_PhaseIdentity_10F31::~Script_Demo_PhaseIdentity_10F31()
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
void Script_Demo_PhaseIdentity_10F31::execute()
{
    p_AbstractScriptHost->updateProgress(ProcessState_Start, "Test start!");

    resultFlag=false;
    p_BuildNetwork_GW->needRebuildNetwork=true;
    if(needBuildNet==true)
    {
        p_BuildNetwork_GW->execute();
        emScriptRunState=Wait_BuildNetFinish_Whole;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "等待全网组网完成");
        p_timer->start((timerForReachThresld+timerAfterReachThresld)*1000);
    }
    else
    {
        index=0;
        times=ushort(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size());
        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_RouterPause_12F2);
        emScriptRunState=Wait_00F1_for_12F2_Pause;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--路由暂停（12F2），等待--确认");
        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
    }

    p_maxAllowTimer->start((timerForReachThresld+timerAfterReachThresld)*1000);

}
void Script_Demo_PhaseIdentity_10F31::stop()
{
    if(p_AbstractScriptHost==nullptr)
        return;
    p_AbstractScriptHost->updateProgress(ProcessState_Error, "Test stop!");
    if(needPowerOff==true)
        powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost);
}
void Script_Demo_PhaseIdentity_10F31::addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq)
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
void Script_Demo_PhaseIdentity_10F31::setHost(AbstractScriptHost *host)
{
    p_AbstractScriptHost=host;
    p_BuildNetwork_GW->setHost(host);
}
bool Script_Demo_PhaseIdentity_10F31::config(const QMap<QString,QString> *paraDic)
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
        if(paraDic->keys().contains("addCntPerTime"))
        {
            this->addCntPerTime = uchar((*paraDic)["addCntPerTime"].toUShort());
        }
        if(paraDic->keys().contains("topoCntPerTime"))
        {
            this->topoCntPerTime = uchar((*paraDic)["topoCntPerTime"].toUShort());
        }
        if(paraDic->keys().contains("querySpan"))
        {
            this->querySpan = (*paraDic)["querySpan"].toUShort();
        }
        if(paraDic->keys().contains("maxQueryIndex"))
        {
            this->maxQueryIndex = (*paraDic)["maxQueryIndex"].toUShort();
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
void Script_Demo_PhaseIdentity_10F31::processMsg(DvcType dvcType, int id, uchar *data, int datalen)
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
                index=0;
                times=ushort(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size());
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_RouterPause_12F2);
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
void Script_Demo_PhaseIdentity_10F31::processCtrlDvcRes(DvcType dvcType, QList<int> idList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params)
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
    case Wait_00F1_for_12F2_Pause:
    {
        break;
    }
    case Wait_BuildNetFinish_Whole:
    {
        p_BuildNetwork_GW->processCtrlDvcRes(dvcType,idList,ctrlCmdType,isSucs,params);
        break;
    }
    case Wait_PhaseIdentity_10F31_Finish:
    {
        break;
    }

    case ScriptSuccess:
    {
        break;
    }
    }
}

void Script_Demo_PhaseIdentity_10F31::processMsgFromCCO(DvcType dvcType, int dvcId)
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
               emScriptRunState=Wait_PhaseIdentity_10F31_Finish;
                p_delayTimer->start(querySpan*1000);
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case Wait_PhaseIdentity_10F31_Finish:
        {
            if(p_Frame3762Base->afn_ == 0x10&&p_Frame3762Base->dt1_==0x40&&p_Frame3762Base->dt2_==0x03&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                tryTimes=0;
                shared_ptr<Afn10F31> p_QueryPhaseInfo_10F31_Up=dynamic_pointer_cast<Afn10F31>(p_Frame3762Base);
                nodeInfo10F31List_10F31.append(p_QueryPhaseInfo_10F31_Up->node_phase_info_unit_.node_info_list_);
                nodeIndex+=topoCntPerTime;
                if(nodeIndex<=p_QueryPhaseInfo_10F31_Up->node_phase_info_unit_.node_total_num_)
                {
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryPhaseInfo_10F31);
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询从节点相位信息（10F31），等待--确认");
                    p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                }
                else
                {
                    generateStandardList(Query_10F31);
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
void Script_Demo_PhaseIdentity_10F31::processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID)
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
void Script_Demo_PhaseIdentity_10F31::processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID)
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
void Script_Demo_PhaseIdentity_10F31::sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame)
{
    if(frame==p_RouterPause_12F2)
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
    else if(frame==p_HardReset_01F1)
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
    else if(frame==p_QueryNetTopoInfo_10F21)
    {
        p_QueryNetTopoInfo_10F21->ctrl_field_.dir=kDirDown;
        p_QueryNetTopoInfo_10F21->ctrl_field_.prm=kActive;
        p_QueryNetTopoInfo_10F21->ctrl_field_.comn_type=kHplc;

        p_QueryNetTopoInfo_10F21->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_QueryNetTopoInfo_10F21->info_field_.info_field_down.comu_rate=0;
        p_QueryNetTopoInfo_10F21->info_field_.info_field_down.comu_module_ident=0;

        p_QueryNetTopoInfo_10F21->node_start_no_=0;
        p_QueryNetTopoInfo_10F21->node_num_=topoCntPerTime;

        sendMsgOct=p_QueryNetTopoInfo_10F21->EncodeFrame();
        sendMsgLog=QString("》》查询网络拓扑10F21：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }

    else if(frame==p_QueryPhaseInfo_10F31)
    {
        p_QueryPhaseInfo_10F31->ctrl_field_.dir=kDirDown;
        p_QueryPhaseInfo_10F31->ctrl_field_.prm=kActive;
        p_QueryPhaseInfo_10F31->ctrl_field_.comn_type=kHplc;

        p_QueryPhaseInfo_10F31->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_QueryPhaseInfo_10F31->info_field_.info_field_down.comu_rate=0;
        p_QueryPhaseInfo_10F31->info_field_.info_field_down.comu_module_ident=0;

        p_QueryPhaseInfo_10F31->node_start_no_=nodeIndex;
        p_QueryPhaseInfo_10F31->node_num_=topoCntPerTime;

        sendMsgOct=p_QueryPhaseInfo_10F31->EncodeFrame();
        sendMsgLog=QString("》》查询从节点相线信息10F31：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
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
void Script_Demo_PhaseIdentity_10F31::sendSrcMsg(DvcType dvcType, int dvcId, QByteArray msg)
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

void Script_Demo_PhaseIdentity_10F31::timer_timeoutProc()
{
    switch(emScriptRunState)
    {
    case Wait_BuildNetFinish_Whole:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_BuildNetFinish_Whole timeout!!!");
        break;
    }
    case Wait_PhaseIdentity_10F31_Finish:
    {
        if(++tryTimes>=3)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_PhaseIdentity_10F31_Finish timeout!!!");
        }
        else
        {
            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryPhaseInfo_10F31);
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询从节点相位信息（10F31），等待--确认");
            p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
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
void Script_Demo_PhaseIdentity_10F31::maxAllowTimer_timeoutProc()
{
    p_delayTimer->stop();
    switch(emScriptRunState)
    {
    case Wait_PhaseIdentity_10F31_Finish:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_PhaseIdentity_10F31_Finish maxAllow timeout!!!");
        break;
    }

    default:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==p_maxAllowTimer");
        break;
    }
    }
}
void Script_Demo_PhaseIdentity_10F31::delayTimer_timeoutProc()
{
    if(emScriptRunState==Wait_PhaseIdentity_10F31_Finish)
    {
        nodeIndex=1;
        nodeInfoList_10F31.clear();
        nodeInfo10F31List_10F31.clear();
        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryPhaseInfo_10F31);
        emScriptRunState=Wait_PhaseIdentity_10F31_Finish;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询从节点相位信息（10F31），等待--确认");
        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
    }
}

void Script_Demo_PhaseIdentity_10F31::generateStandardList(Script_Demo_PhaseIdentity_10F31::QueryType emQueryType)
{
    //档案配置时，相序：对于单相表为零火反接标识，对于三相表为相序
    if(emQueryType==Query_10F31)
    {
        nodeInfoList_10F31.clear();
        failInfo.clear();
        for(int i=0;i<nodeInfo10F31List_10F31.size();i++)
        {
            NodeInfoStruct nodeInfoST;
            nodeInfoST.phaseIsRight=true;
            nodeInfoST.phaseIsComplete=false;

            nodeInfoST.nodeAddress=nodeInfo10F31List_10F31.at(i).node_address_;
            nodeInfoST.nodePhase=getNodePhase(emQueryType,uchar(nodeInfo10F31List_10F31.at(i).node_phase_info_.phase_));
            nodeInfoST.lineException=nodeInfo10F31List_10F31.at(i).node_phase_info_.line_abnormal_flag_;
            nodeInfoST.phaseSeq=nodeInfo10F31List_10F31.at(i).node_phase_info_.phase_sequence_;

            bool findFlag=false;
            if(memcmp(nodeInfoST.nodeAddress.addr,p_CtrInfoList->at(0)->ccoAddr,6)==0)
                findFlag=true;

            for(int j=0;j<p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().size();j++)
            {
                //找到同一只表
                if(memcmp(nodeInfoST.nodeAddress.addr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(j)->mtrAddr,6)==0)
                {
                    findFlag=true;
                    uchar configPhase = p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(j)->realPhase;
                    uchar configPhaseSeq = p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(j)->phaseSeq;
                    DvcType dvctype=p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(j)->slotPosition;

//                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("nodeInfoST.nodePhase:%1 nodeInfoST.lineException:%2")
//                                                         .arg(uchar(nodeInfoST.nodePhase)).arg(nodeInfoST.lineException)
//                                                         );

//                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("configPhase:%1 configPhaseSeq:%2")
//                                                         .arg(configPhase).arg(configPhaseSeq)
//                                                         );

                    if(nodeInfo10F31List_10F31.at(i).node_phase_info_.phase_>0)
                    {
//                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("%1 ").arg(nodeInfo10F31List_10F31.at(i).node_phase_info_.phase_));

                        nodeInfoST.phaseIsComplete=true;
//                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("%1").arg(nodeInfoST.phaseIsComplete));

                    }

                    if(dvctype==DvcType::SingleSTA||dvctype==DvcType::SingleMeter)
                    {
                        nodeInfoST.meterType=0x00;//0单相表，1三相表
                        if(nodeInfo10F31List_10F31.at(i).node_phase_info_.phase_==configPhase&&nodeInfoST.lineException==configPhaseSeq)
                            nodeInfoST.phaseIsRight=true;
                        else
                            nodeInfoST.phaseIsRight=false;
                    }
                    else if(dvctype==DvcType::ThreeMeter||dvctype==DvcType::ThreeSTA)
                    {
                        nodeInfoST.meterType=0x01;
                        if(nodeInfo10F31List_10F31.at(i).node_phase_info_.phase_==configPhase&&nodeInfoST.phaseSeq==configPhaseSeq)
                            nodeInfoST.phaseIsRight=true;
                        else
                            nodeInfoST.phaseIsRight=false;
                    }

                    QString tmpInfo;
                    if(nodeInfoST.meterType==0x00)
                    {

                        tmpInfo=  QString("表号(单相表)：%1 [识别相位: %2 线路异常:%3];[配置表: %4 线路异常:%5]\n")
                                .arg(QString(QByteArray(nodeInfoST.nodeAddress.addr,6).toHex()))
                                .arg(getPhaseInfo(nodeInfoST.nodePhase)).arg(nodeInfoST.lineException==0x00?"无异常":"零火反接")
                                .arg(getPhaseInfo((Phase)configPhase)).arg(configPhaseSeq==0x00?"无异常":"零火反接");
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,tmpInfo);

                    }
                    else if(nodeInfoST.meterType==0x01)
                    {
                        tmpInfo= QString("表号(三相表）：%1 [识别相位: %2 相序:%3];[配置表: %4 相序:%5]\n")
                                .arg(QString(QByteArray(nodeInfoST.nodeAddress.addr,6).toHex()))
                                .arg(getPhaseInfo(nodeInfoST.nodePhase)).arg(nodeInfoST.phaseSeq)
                                .arg(getPhaseInfo((Phase)configPhase)).arg(configPhaseSeq);
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, tmpInfo);

                    }

                    if(!nodeInfoST.phaseIsRight) failInfo+=tmpInfo;


                      nodeInfoList_10F31.append(nodeInfoST);
                }
            }


            if(findFlag==false)//找不到对应的电表号
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("表号：%1，在档案内查询不到").arg(QString(QByteArray(nodeInfoST.nodeAddress.addr,6).toHex())));
                return;
            }
        }
        getFailList(emQueryType);
    }

}

void Script_Demo_PhaseIdentity_10F31::getFailList(Script_Demo_PhaseIdentity_10F31::QueryType emQueryType)
{
    if(emQueryType==Query_10F31)
    {
        bool isIdentifyComplete=true;
        bool isIdentifyRight=true;
        for(int i=0;i<nodeInfoList_10F31.size();i++)
        {

            if(nodeInfoList_10F31.at(i).phaseIsComplete==false)
            {
                isIdentifyComplete=false;
            }
            if(nodeInfoList_10F31.at(i).phaseIsRight==false)
            {
                isIdentifyRight=false;
            }
        }

        if(isIdentifyComplete)
        {
            if(isIdentifyRight)
            {
                p_delayTimer->stop();
                p_timer->stop();
                p_maxAllowTimer->stop();
                p_AbstractScriptHost->updateProgress(ProcessState_Success, QString("10F31查询相位识别正确"));
            }
            else
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("10F31查询相位识别错误电表信息如下\n%1").arg(failInfo));
            }
        }
        else
        {

        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("10F31查询相位识别未成功电表信息如下\n%1").arg(failInfo));

        }

    }

}
Script_Demo_PhaseIdentity_10F31::Phase Script_Demo_PhaseIdentity_10F31::getNodePhase(Script_Demo_PhaseIdentity_10F31::QueryType emQueryType, uchar phase)
{
    if(emQueryType==Query_10F2||emQueryType==Query_10F31)
    {
        if(phase==1)
            return A;
        else if(phase==2)
            return B;
        else if(phase==3)
            return AB;
        else if(phase==4)
            return C;
        else if(phase==5)
            return AC;
        else if(phase==6)
            return BC;
        else if(phase==7)
            return ABC;
        else
            return Error;
    }
    else if(emQueryType==Query_13F1)
    {
        if(phase==1)
            return A;
        else if(phase==2)
            return B;
        else if(phase==3)
            return C;
        else if(phase==7)
            return ABC;
        else
            return Error;
    }
    return Error;
}
Script_Demo_PhaseIdentity_10F31::Phase Script_Demo_PhaseIdentity_10F31::getThreePhase_10F31(uchar phase_sequence)
{
    if(phase_sequence==0)
        return ABC;
    else if(phase_sequence==1)
        return ACB;
    else if(phase_sequence==2)
        return BAC;
    else if(phase_sequence==3)
        return BCA;
    else if(phase_sequence==4)
        return CAB;
    else if(phase_sequence==5)
        return CBA;
    else
        return Error;
}
Script_Demo_PhaseIdentity_10F31::Phase Script_Demo_PhaseIdentity_10F31::getThreePhase_config(uchar config)
{
    if(config==7)
        return ABC;
    else if(config==8)
        return ACB;
    else if(config==9)
        return BAC;
    else if(config==10)
        return BCA;
    else if(config==11)
        return CAB;
    else if(config==12)
        return CBA;
    else
        return Error;
}
QString Script_Demo_PhaseIdentity_10F31::getPhaseInfo(Script_Demo_PhaseIdentity_10F31::Phase phase)
{
    if(phase==A)
        return "A相";
    else if(phase==B)
        return "B相";
    else if(phase==C)
        return "C相";
    else if(phase==AB)
        return "AB相";
    else if(phase==AC)
        return "AC相";
    else if(phase==BC)
        return "BC相";
    else if(phase==ABC)
        return "ABC相";
    else if(phase==ACB)
        return "ACB相";
    else if(phase==BAC)
        return "BAC相";
    else if(phase==BCA)
        return "BCA相";
    else if(phase==CAB)
        return "CAB相";
    else if(phase==CBA)
        return "CBA相";
    else
        return "未知";
}
