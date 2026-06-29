#include "Script_ParameterSynchronousFlow.h"

Script_ParameterSynchronousFlow::Script_ParameterSynchronousFlow(QObject *parent) : InteroperateBase_GW(parent)
{

}

Script_ParameterSynchronousFlow::~Script_ParameterSynchronousFlow()
{
    stop();
}
void Script_ParameterSynchronousFlow::setHost(AbstractScriptHost *host)
{
    p_AbstractScriptHost=host;
    p_InteroperateInit_GW->setHost(host);
}
void Script_ParameterSynchronousFlow::addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq)
{
    p_CtrInfoList->clear();
    comnAddAddrsInfo(p_CtrInfoList, p_ConcentratorInfoList, p_MeterInfoList, p_SchemeCfgInfoList);
    uchar dstPrtcl=uchar(freq)>>4;
    if(dstPrtcl==0x03)
        setMeterAddrsPrtcl(p_CtrInfoList,dstPrtcl);
    p_InteroperateInit_GW->setCtrInfoList(p_CtrInfoList);
    for(int i=0; i<p_CtrInfoList->at(0)->keyList.size(); i++)
    {
        if(SingleSTA == p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->value(p_CtrInfoList->at(0)->keyList.at(i))->slotPosition
        || ThreeSTA==p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->value(p_CtrInfoList->at(0)->keyList.at(i))->slotPosition)
        {
            protocol=(*p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList)[p_CtrInfoList->at(0)->keyList.at(i)]->prtcl;
                break;
        }
    }
}
bool Script_ParameterSynchronousFlow::config(const QMap<QString,QString> *paraDic)
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

void Script_ParameterSynchronousFlow::execute()
{
    p_AbstractScriptHost->updateProgress(ProcessState_Start, "档案同步流程: 开始测试!");
    emScriptRunState=ScriptInit;
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "使用“暂停”命令暂停路由工作 开始...");
    emScriptRunState=Wait_PauseRouter_Finish;
    QString msg=QString("68 0F 00 43 00 00 28 32 00 4C 12 02 00 FD 16 ").replace(" ","");
    sendSrcMsg(CCO_GW,p_CtrInfoList->at(0)->dvcId,QByteArray::fromHex(msg.toLatin1()));
    msgSeq=0x4C;
    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
    p_maxAllowTimer->start((timerForReachThresld+timerAfterReachThresld)*1000);
}
void Script_ParameterSynchronousFlow::slotStartFollowTest()
{

}
void Script_ParameterSynchronousFlow::stop()
{
    p_timer->stop();
    p_maxAllowTimer->stop();
    p_delayTimer->stop();
}

void Script_ParameterSynchronousFlow::processMsg(DvcType dvcType, int id, uchar *data, int datalen)
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

void Script_ParameterSynchronousFlow::processCtrlDvcRes(DvcType dvcType, QList<int> idList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params)
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

void Script_ParameterSynchronousFlow::processMsgFromCCO(DvcType dvcType, int dvcId)
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
                    emScriptRunState=Wait_ParameterInit_Finish;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("参数区初始化 开始"));
                    QString msg=QString("68 0F 00 43 00 00 28 32 00 4D 01 02 00 ED 16 ").replace(" ","");
                    sendSrcMsg(dvcType,dvcId,QByteArray::fromHex(msg.toLatin1()));
                    msgSeq=0x4D;
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_ParameterInit_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp&&uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq)==msgSeq)
                {
                    p_timer->stop();
                    InteroperateInit_GW::delayTime(30*1000);
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("参数区初始化 结束"));
                    emScriptRunState=Wait_QueryNodeNum_Finish;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("下发从节点数量，查询路由从节点数量 开始"));
                    QString msg=QString("68 0F 00 43 00 00 28 32 00 4E 10 01 00 FC 16").replace(" ","");
                    sendSrcMsg(dvcType,dvcId,QByteArray::fromHex(msg.toLatin1()));
                    msgSeq=0x4E;
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_QueryNodeNum_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x10&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp&&uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq)==msgSeq)
                {
                    p_timer->stop();
                    if(dynamic_pointer_cast<Afn10F1>(p_Frame3762Base)->node_total_num_==0)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("下发从节点数量，查询路由从节点数量 结束"));
                        emScriptRunState=Wait_AddNode_Finish;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("添加从节点，添加电表地址 开始"));
                        QString msg;
                        msg=(protocol==0x02?QString("68 17 00 43 00 00 28 32 00 4F 11 01 00 01 04 33 00 00 00 00 02 38 16").replace(" ",""):QString("68 17 00 43 00 00 28 32 00 4F 11 01 00 01 04 33 00 00 00 00 03 39 16").replace(" ",""));
                        sendSrcMsg(dvcType,dvcId,QByteArray::fromHex(msg.toLatin1()));
                        msgSeq=0x4F;
                        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("查询路由从节点数量不为0"));
                        p_AbstractScriptHost->updateProgress(ProcessState_Error, QString("查询路由从节点数量不为0"));
                    }
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_AddNode_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp&&uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq)==msgSeq)
                {
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("添加从节点，添加电表地址 结束"));
                    emScriptRunState=Wait_QueryNodeInfo_Finish;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("下发从节点信息，读取路由模块的表档案地址 开始"));
                    QString msg=QString("68 12 00 43 00 00 28 32 00 50 10 02 00 01 00 01 01 16").replace(" ","");
                    sendSrcMsg(dvcType,dvcId,QByteArray::fromHex(msg.toLatin1()));
                    msgSeq=0x50;
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_QueryNodeInfo_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x10&&p_Frame3762Base->dt1_==0x02&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp&&uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq)==msgSeq)
                {
                    p_timer->stop();

                    shared_ptr<Afn10F2> ptr=dynamic_pointer_cast<Afn10F2>(p_Frame3762Base);
                    uchar addr[6]={0x00,0x00,0x00,0x00,0x33,0x04};
                    if(ptr->node_info_data_unit_.node_total_num_==1
                        &&ptr->node_info_data_unit_.this_node_num_==1
                        &&memcmp(ptr->node_info_data_unit_.node_info_group_list_.first().node_address_.addr,addr,6)==0
                        &&uchar(ptr->node_info_data_unit_.node_info_group_list_.first().node_info_.protocol_type_)==protocol)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("下发从节点信息，读取路由模块的表档案地址 结束"));
                        emScriptRunState=Wait_QueryRouterRunState_Finish;
                        QString msg=QString("68 0F 00 43 00 00 28 32 00 51 10 08 00 06 16").replace(" ","");
                        sendSrcMsg(CCO_GW,p_CtrInfoList->at(0)->dvcId,QByteArray::fromHex(msg.toLatin1()));
                        msgSeq=0x51;
                        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("下发“路由运行状态” 开始"));
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("查询从节点信息存在异常"));
                        p_AbstractScriptHost->updateProgress(ProcessState_Error, QString("查询从节点信息存在异常"));
                    }
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_QueryRouterRunState_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x10&&p_Frame3762Base->dt1_==0x08&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp&&uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq)==msgSeq)
                {
                    p_timer->stop();
                    shared_ptr<Afn10F4> ptr=dynamic_pointer_cast<Afn10F4>(p_Frame3762Base);
                    if(ptr->router_operate_state_unit_.node_total_num_==1)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("下发“路由运行状态” 结束"));
                        p_AbstractScriptHost->updateProgress(ProcessState_Success, QString("档案同步流程:流程测试成功"));
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("查询路由运行状态存在异常"));
                        p_AbstractScriptHost->updateProgress(ProcessState_Error, QString("档案同步流程:流程测试异常"));
                    }
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

void Script_ParameterSynchronousFlow::processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID)
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

void Script_ParameterSynchronousFlow::processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID)
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

void Script_ParameterSynchronousFlow::sendMsg(DvcType , int , int , shared_ptr<void> )
{

}

void Script_ParameterSynchronousFlow::timer_timeout()
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

void Script_ParameterSynchronousFlow::maxAllowTimer_timeout()
{
    p_maxAllowTimer->stop();
    p_AbstractScriptHost->updateProgress(ProcessState_Error, QString("timeout p_maxAllowTimer!!!"));
}

void Script_ParameterSynchronousFlow::delayTimer_timeout()
{

}


