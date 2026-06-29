#include "BuildNetworkDetect.h"


BuildNetworkDetect::BuildNetworkDetect(QObject *parent) : QObject(parent)
{
    p_MsgBase_3762 = make_shared<qgdw_3762_protocol::Frame3762Helper>();
    p_ParamInit_01F2 = make_shared<qgdw_3762_protocol::Afn01F2>();
    p_AddSlaveNode_11F1 = make_shared<qgdw_3762_protocol::Afn11F1>();
    p_QueryNetTopoInfo_10F21 = make_shared<qgdw_3762_protocol::Afn10F21>();

    p_MsgBase_645 = make_shared<dlt_645_Protocol::Frame645Helper>();
    p_MeterAddrResp_93 = make_shared<dlt_645_Protocol::RspsNormal_ReadAddr_0x93>(addr,6);

    p_GetResponseNormal_ReadAddr=make_shared<GetResponseNormal>();
    p_CtrInfoList=make_shared<QList<shared_ptr<CtrInfo>>>();

    p_timer = make_shared<QTimer>();
    p_maxAllowTimer = make_shared<QTimer>();
    p_delayTimer = make_shared<QTimer>();
    connect(p_timer.get(),SIGNAL(timeout()),this,SLOT(timer_timeoutProc()));
    connect(p_maxAllowTimer.get(),SIGNAL(timeout()),this,SLOT(maxAllowTimer_timeoutProc()));
    connect(p_delayTimer.get(),SIGNAL(timeout()),this,SLOT(delayTimer_timeoutProc()));

}

BuildNetworkDetect::~BuildNetworkDetect()
{
    p_timer->stop();
    disconnect(p_timer.get(),nullptr,this,nullptr);
    p_maxAllowTimer->stop();
    disconnect(p_maxAllowTimer.get(),nullptr,this,nullptr);
    p_delayTimer->stop();
    disconnect(p_delayTimer.get(),nullptr,this,nullptr);
}

void BuildNetworkDetect::execute()
{
    p_AbstractScriptHost->updateProgress(ProcessState_Start, QString("开始组网探测流程!!!"));
    index=1;//拓扑从1开始查
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("查询网络拓扑(index=%1，per=%2，total=%3)").arg(QString::number(index)).arg(QString::number(topoCntPerTime)).arg(QString::number(p_CtrInfo->p_MeterInfoForSingleNetList->size()+1)));
    sendMsg(CCO_GW,p_CtrInfo->ctrlID,index,p_QueryNetTopoInfo_10F21);
    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
    emScriptRunState=Wait_QueryNetTopo_10F21;
    p_maxAllowTimer->start((timerForReachThresld+timerAfterReachThresld)*1000);
}

void BuildNetworkDetect::stop()
{
    p_timer->stop();
    disconnect(p_timer.get(),nullptr,this,nullptr);
    p_maxAllowTimer->stop();
    disconnect(p_maxAllowTimer.get(),nullptr,this,nullptr);
    p_delayTimer->stop();
    disconnect(p_delayTimer.get(),nullptr,this,nullptr);
}

void BuildNetworkDetect::addAddrsInfo(shared_ptr<CtrInfo> p_CtrInfo)
{
    this->p_CtrInfo=p_CtrInfo;
    meterParameterList.clear();
    meterParameterList.append(Address(p_CtrInfo->ccoAddr));
    for(auto i:p_CtrInfo->p_MeterInfoForSingleNetList->values())
        meterParameterList.append(Address(i->mtrAddr));
    if(p_CtrInfoList==nullptr)
        p_CtrInfoList=make_shared<QList<shared_ptr<CtrInfo>>>();
    p_CtrInfoList->clear();
    p_CtrInfoList->append(p_CtrInfo);
}

void BuildNetworkDetect::setHost(AbstractScriptHost *host)
{
    p_AbstractScriptHost=host;
}

bool BuildNetworkDetect::config(const QMap<QString, QString> *paraDic)
{
    bool result = false;
    if(paraDic!=nullptr)
    {
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
        if(paraDic->keys().contains("freqBand"))
        {
            //            this->freqBand = (*paraDic)["freqBand"].toUShort();
        }
        if(paraDic->keys().contains("addCntPerTime"))
        {
            this->addCntPerTime = (*paraDic)["addCntPerTime"].toUShort()&0xff;

            if(this->addCntPerTime<=0)
                this->addCntPerTime=20;
        }
        if(paraDic->keys().contains("topoCntPerTime"))
        {
            this->topoCntPerTime = (*paraDic)["topoCntPerTime"].toUShort()&0xff;

            if(this->topoCntPerTime<=0)
                this->topoCntPerTime=20;
        }
        result = true;
    }
    return result;
}

void BuildNetworkDetect::processMsg(DvcType dvcType, int id, uchar *data, int datalen)
{
    QByteArray tmpRecvTempData=QByteArray::fromRawData(reinterpret_cast<char*>(data),datalen);
    QByteArray recvTempData;
    recvTempData.append(tmpRecvTempData);
    delete[]data;

    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到报文：%1").arg(QString(recvTempData.toHex())));

    if(p_CtrInfo==nullptr)
        return;

    if(dvcType==CCO_GW || dvcType==CCO_NW)
    {
        if(dvcType == p_CtrInfo->slotPosition && id == p_CtrInfo->dvcId)
        {
            p_CtrInfo->buf.append(recvTempData);
            processMsgFromCco(dvcType,id);
        }
    }
    else if(dvcType==SingleSTA || dvcType==ThreeSTA)
    {
        for(int i=0; i<p_CtrInfo->keyList.size(); i++)
        {
            if(dvcType == p_CtrInfo->p_MeterInfoForSingleNetList->value(p_CtrInfo->keyList.at(i))->slotPosition
                    && id == p_CtrInfo->p_MeterInfoForSingleNetList->value(p_CtrInfo->keyList.at(i))->dvcId)
            {
                if((*p_CtrInfo->p_MeterInfoForSingleNetList)[p_CtrInfo->keyList.at(i)]->prtcl==0x02)
                {
                    (*p_CtrInfo->p_MeterInfoForSingleNetList)[p_CtrInfo->keyList.at(i)]->buf645.append(recvTempData);
                    processMsgFromMeter645(dvcType,id,p_CtrInfo->keyList.at(i));
                    break;
                }
                else if((*p_CtrInfo->p_MeterInfoForSingleNetList)[p_CtrInfo->keyList.at(i)]->prtcl==0x03)
                {
                    (*p_CtrInfo->p_MeterInfoForSingleNetList)[p_CtrInfo->keyList.at(i)]->buf698.append(recvTempData);
                    processMsgFromMeterOOP(dvcType,id,p_CtrInfo->keyList.at(i));
                    break;
                }
            }
        }
    }
    else if(dvcType==CJQ)
    {
        for(int i=0; i<p_CtrInfo->keyList.size(); i++)
        {
            if(dvcType == p_CtrInfo->p_MeterInfoForSingleNetList->value(p_CtrInfo->keyList.at(i))->slotPosition
                    && id == p_CtrInfo->p_MeterInfoForSingleNetList->value(p_CtrInfo->keyList.at(i))->dvcId)
            {
                if((*p_CtrInfo->p_MeterInfoForSingleNetList)[p_CtrInfo->keyList.at(i)]->prtcl==0x02)
                {
                    (*p_CtrInfo->p_MeterInfoForSingleNetList)[p_CtrInfo->keyList.at(i)]->buf645.append(recvTempData);
                    processMsgFromMeter645(dvcType,id,p_CtrInfo->keyList.at(i));
                    break;
                }
                else if((*p_CtrInfo->p_MeterInfoForSingleNetList)[p_CtrInfo->keyList.at(i)]->prtcl==0x03)
                {
                    (*p_CtrInfo->p_MeterInfoForSingleNetList)[p_CtrInfo->keyList.at(i)]->buf698.append(recvTempData);
                    processMsgFromMeterOOP(dvcType,id,p_CtrInfo->keyList.at(i));
                    break;
                }
            }
        }
    }
    else
    {
        return;
    }
}

void BuildNetworkDetect::processCtrlDvcRes(DvcType , QList<int> , CtrlCmdType , bool , QList<double> )
{

}

void BuildNetworkDetect::processMsgFromCco(DvcType dvcType, int dvcId)
{
    bool haveCompleteMsg=true;
    while(haveCompleteMsg)
    {
        qInfo()<<QString("CCO-解析前 buf3762=%1").arg(QString((p_CtrInfo->buf.toHex())));
        shared_ptr<Frame3762Base> p_Frame3762Base = p_MsgBase_3762->DecodeLocalMsg(&(p_CtrInfo->buf),haveCompleteMsg);
        qInfo()<<QString("CCO-解析后 buf3762=%1").arg(QString((p_CtrInfo->buf.toHex())));

        if(p_Frame3762Base==nullptr)
        {
            continue;
        }
        switch(emScriptRunState)
        {
            case Init:
            {
                break;
            }
            case Wait_QueryNetTopo_10F21:
            {
                if(p_Frame3762Base->afn_==0x10&&p_Frame3762Base->dt1_==0x10&&p_Frame3762Base->dt2_==0x02&&p_Frame3762Base->ctrl_field_.dir==kDirUp)
                {
                    p_timer->stop();
                    shared_ptr<Afn10F21> p_ResTopoInfo_Afn10F21=std::dynamic_pointer_cast<Afn10F21>(p_Frame3762Base);
                    if(p_ResTopoInfo_Afn10F21->network_typelogy_info_unit_.node_total_num_!=p_CtrInfo->p_MeterInfoForSingleNetList->size()+1)//网络规模和总节点数量不一致，则直接重新组网
                    {
                        isNeedDetect=false;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("探测到当前网络已入网规模与总规模不一致(入网规模%1，总规模应为%2)，需要启动重新组网流程！").arg(QString::number(p_ResTopoInfo_Afn10F21->network_typelogy_info_unit_.node_total_num_)).arg(QString::number(p_CtrInfo->p_MeterInfoForSingleNetList->size()+1)));
                        emit signalSendRebuildNetFlag(true);
                    }
                    else//规模一致则判断，节点信息是否与档案信息一致
                    {
                        if(!judgeTopoNodeBelongCtrInfo(p_ResTopoInfo_Afn10F21->network_typelogy_info_unit_.network_typelogy_info_List_))
                        {
                            isNeedDetect=false;
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("探测到当前网络存在入网表不在档案内的情况，需要启动重新组网流程！").arg(QString::number(p_ResTopoInfo_Afn10F21->network_typelogy_info_unit_.node_total_num_)).arg(QString::number(p_CtrInfo->p_MeterInfoForSingleNetList->size()+1)));
                            emit signalSendRebuildNetFlag(true);
                        }
                        else
                        {
                            index+=topoCntPerTime;
                            if(index<p_CtrInfo->p_MeterInfoForSingleNetList->size()+1)
                            {
                                sendMsg(CCO_GW,p_CtrInfo->ctrlID,index,p_QueryNetTopoInfo_10F21);
                                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                                p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("查询网络拓扑(index=%1，per=%2，total=%3)").arg(QString::number(index)).arg(QString::number(topoCntPerTime)).arg(QString::number(p_CtrInfo->p_MeterInfoForSingleNetList->size()+1)));
                            }
                            else
                            {
                                isNeedDetect=false;
                                p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("探测到当前网络已组网完成，不需要需要启动重新组网流程！"));
                                emit signalSendRebuildNetFlag(false);
                            }
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
        }
    }
}

void BuildNetworkDetect::processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID)
{
    bool haveCompleteMsg=true;
    while(haveCompleteMsg)
    {
        shared_ptr<dlt_645_Protocol::Frame645Base> MsgBase_645_ptr = dlt_645_Protocol::Frame645Helper::DecodeLocalMsg(&((*(p_CtrInfo->p_MeterInfoForSingleNetList))[mtrlID]->buf645),haveCompleteMsg);
        if(MsgBase_645_ptr==nullptr)
        {
            break;
        }
        switch(emScriptRunState)
        {
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

                        QByteArray tmpSendMsg=prcsOther645Msg(MsgBase_645_ptr->ctrlCode_,MsgBase_645_ptr->dataLen_,di,MsgBase_645_ptr->addr_,(*(p_CtrInfo->p_MeterInfoForSingleNetList))[mtrlID]->mtrAddr,p_CtrInfoList);
                        sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                    }
                    else
                    {
                        QByteArray tmpSendMsg=prcsOther645Msg(MsgBase_645_ptr->ctrlCode_,MsgBase_645_ptr->dataLen_,di,MsgBase_645_ptr->addr_,(*(p_CtrInfo->p_MeterInfoForSingleNetList))[mtrlID]->mtrAddr,p_CtrInfoList);
                        sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                    }
                }
                break;
            }
        }
    }
}

void BuildNetworkDetect::processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID)
{
    bool haveCompleteMsg=true;
    while(haveCompleteMsg)
    {
        shared_ptr<FrameOOPBase> MsgBase_OOP_ptr = FrameOOPHelper::DecodeMsg(&((*(p_CtrInfo->p_MeterInfoForSingleNetList))[mtrlID]->buf698),haveCompleteMsg);

        if(MsgBase_OOP_ptr==nullptr)
        {
            break;
        }
        switch(emScriptRunState)
        {
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
                        QByteArray tmpSendMsg=prcsOther698Msg(MsgBase_OOP_ptr,(*(p_CtrInfo->p_MeterInfoForSingleNetList))[mtrlID]->mtrAddr,p_CtrInfoList);
                        sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                    }
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther698Msg(MsgBase_OOP_ptr,(*(p_CtrInfo->p_MeterInfoForSingleNetList))[mtrlID]->mtrAddr,p_CtrInfoList);
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
        }
    }
}

void BuildNetworkDetect::processMsgFromCJQ(DvcType , int )
{

}

void BuildNetworkDetect::sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame)
{
    QByteArray sendMsgOct;
    QString sendMsgLog;
    if(frame==p_QueryNetTopoInfo_10F21)
    {
        p_QueryNetTopoInfo_10F21->ctrl_field_.dir=kDirDown;
        p_QueryNetTopoInfo_10F21->ctrl_field_.prm=kActive;
        p_QueryNetTopoInfo_10F21->ctrl_field_.comn_type=kHplc;

        p_QueryNetTopoInfo_10F21->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_QueryNetTopoInfo_10F21->info_field_.info_field_down.comu_rate=0;
        p_QueryNetTopoInfo_10F21->info_field_.info_field_down.comu_module_ident=0;

        p_QueryNetTopoInfo_10F21->node_start_no_=ushort(meterID);
        p_QueryNetTopoInfo_10F21->node_num_=topoCntPerTime;

        sendMsgOct=p_QueryNetTopoInfo_10F21->EncodeFrame();
        sendMsgLog=QString("》》查询网络拓扑10F21：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
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

void BuildNetworkDetect::sendSrcMsg(DvcType dvcType, int dvcId, QByteArray msg)
{
    if(msg.size()<1)
    {
        return;
    }
    uchar *sendMsg=new uchar[static_cast<uint>(msg.size())];
    memcpy(sendMsg,reinterpret_cast<uchar*>(msg.data()),uint(msg.size()));

    p_AbstractScriptHost->sendMsg2Dvc(dvcType,dvcId,sendMsg,msg.size());

    QStringList dvcList;
    dvcList<<"单通"<<"三通"<<"国网路由"<<"南网路由"<<"采集器";

    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("测试脚本==>>>>上位机【设备类型:%1】的报文=%2").arg(dvcList.at(int(dvcType))).arg(QString(msg.toHex())));
}
void BuildNetworkDetect::timer_timeoutProc()
{
    switch(emScriptRunState)
    {
        case Wait_QueryNetTopo_10F21:
        {
            isNeedDetect=false;
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("查询拓扑超时，需要启动重新组网流程！"));
            emit signalSendRebuildNetFlag(true);
            break;
        }
        default:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==p_timer");
            break;
        }
    }
}

void BuildNetworkDetect::maxAllowTimer_timeoutProc()
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

void BuildNetworkDetect::delayTimer_timeoutProc()
{

}
bool BuildNetworkDetect::judgeTopoNodeBelongCtrInfo(const QList<NetworkTypelogyInfo10F21> list)
{
    for(auto i:list)
    {
        if(!meterParameterList.contains(i.node_address_))
            return false;
    }
    return true;
}

bool BuildNetworkDetect::getIsNeedDetect() const
{
    return isNeedDetect;
}


