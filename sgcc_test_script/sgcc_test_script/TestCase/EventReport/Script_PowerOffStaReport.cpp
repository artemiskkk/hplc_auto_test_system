#include "Script_PowerOffStaReport.h"

Script_PowerOffStaReport::Script_PowerOffStaReport(QObject *parent) : QObject(parent)
{
    emScriptRunState=TestInit;
    resultFlag=false;
    sendMsgOct.clear();

    p_CtrInfoList=make_shared<QList<shared_ptr<CtrInfo>>>();
    p_BuildNetwork_GW=new BuildNetwork_GW();

    p_Frame3762Helper=make_shared<Frame3762Helper>();
    p_Confirm_00F1=make_shared<Afn00F1>();

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

Script_PowerOffStaReport::~Script_PowerOffStaReport()
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
void Script_PowerOffStaReport::execute()
{
    p_AbstractScriptHost->updateProgress(ProcessState_Start, "Test start!");

    resultFlag=false;

    if(needBuildNet==true)
    {
        p_BuildNetwork_GW->execute();
        emScriptRunState=Wait_BuildNetFinish_Whole;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "等待全网组网完成");
        p_timer->start((timerForReachThresld+timerAfterReachThresld)*1000);
    }
    else
    {
        //发送电表断电
        //sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryNodeInfo_10F2);
        emScriptRunState=Wait_EventReport_Finish;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "电表断电--等待事件上报");
        //时间再定
        //p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
    }
    for(int i=0;i<p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().size();i++)
    {
        if(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(i)->slotPosition == SingleSTA)
        {
            singleStaAddr = QByteArray(reinterpret_cast<char*>(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(i)->mtrAddr),6).toHex();
        }
        if(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(i)->slotPosition == ThreeSTA)
        {
            threeStaAddr = QByteArray(reinterpret_cast<char*>(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(i)->mtrAddr),6).toHex();
        }
    }
    singlePowerOffFlag=false;
    singlePowerOnFlag=false;
    threePowerOffFlag=false;
    threePowerOnFlag=false;
    p_maxAllowTimer->start((timerForReachThresld+timerAfterReachThresld)*1000);
}
void Script_PowerOffStaReport::stop()
{
    if(p_AbstractScriptHost==nullptr)
        return;
    p_AbstractScriptHost->updateProgress(ProcessState_Error, "Test stop!");
    if(needPowerOff==true)
        powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost);
}
void Script_PowerOffStaReport::addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq)
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
void Script_PowerOffStaReport::setHost(AbstractScriptHost *host)
{
    p_AbstractScriptHost=host;
    p_BuildNetwork_GW->setHost(host);
}
bool Script_PowerOffStaReport::config(const QMap<QString,QString> *paraDic)
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
void Script_PowerOffStaReport::processMsg(DvcType dvcType, int id, uchar *data, int datalen)
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
                //发送电表断电
                //sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryNodeInfo_10F2);
                emScriptRunState=Wait_EventReport_Finish;
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("工装单三相断电%1s后上电--等待停电事件上报").arg(maxWaitReportTime));
                powerOff12V_STA(p_CtrInfoList,p_AbstractScriptHost);
                p_timer->start(maxWaitReportTime*1000);
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
void Script_PowerOffStaReport::processCtrlDvcRes(DvcType dvcType, QList<int> idList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params)
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
        case Wait_EventReport_Finish:
        {
            break;
        }
        case ScriptSuccess:
        {
            break;
        }
    }
}

void Script_PowerOffStaReport::processMsgFromCCO(DvcType dvcType, int dvcId)
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
            case Wait_EventReport_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x06&&p_Frame3762Base->dt1_==0x10&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    //p_timer->stop();
                    shared_ptr<Afn06F5> p_EventReport_06F5_Up=dynamic_pointer_cast<Afn06F5>(p_Frame3762Base);
                    if(p_EventReport_06F5_Up->report_event_unit_.frame_content_.at(0)==0x01)//停电
                    {
                        PowerEvent_Struct eventST;
                        eventST.eventType=p_EventReport_06F5_Up->report_event_unit_.frame_content_.at(0);
                        for(int i=0;i<6;i++)
                        {
                            eventST.reportNodeAddress.addr[i]=p_EventReport_06F5_Up->report_event_unit_.frame_content_.at(6-i);
                        }
                        powerOffReportList.append(eventST);
                        QString reportAddr = QByteArray(eventST.reportNodeAddress.addr,6);
                        if(singleStaAddr == reportAddr)
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到 单通 停电事件").arg(QString(QByteArray(eventST.reportNodeAddress.addr,6).toHex())));
                            singlePowerOffFlag = true;
                        }
                        else if(threeStaAddr == reportAddr)
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到 三通 停电事件").arg(QString(QByteArray(eventST.reportNodeAddress.addr,6).toHex())));
                            threePowerOffFlag = true;
                        }
                        else
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("收到表号%1停电事件").arg(reportAddr));
                        }
                    }
                    else if(p_EventReport_06F5_Up->report_event_unit_.frame_content_.at(0)==0x02)//复电
                    {
//                        if(firstPowerOnReport==true)
//                        {
//                            firstPowerOnReport=false;
//                            p_delayTimer->start(maxWaitReportTime*1000);//等待上报时间
//                        }
                        PowerEvent_Struct eventST;
                        eventST.eventType=p_EventReport_06F5_Up->report_event_unit_.frame_content_.at(0);
                        for(int i=0;i<6;i++)
                        {
                            eventST.reportNodeAddress.addr[i]=p_EventReport_06F5_Up->report_event_unit_.frame_content_.at(6-i);
                        }
                        powerOnReportList.append(eventST);
                        QString reportAddr = QByteArray(eventST.reportNodeAddress.addr,6);
                        if(singleStaAddr == reportAddr)
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到 单通 复电事件").arg(QString(QByteArray(eventST.reportNodeAddress.addr,6).toHex())));
                            singlePowerOnFlag = true;
                        }
                        else if(threeStaAddr == reportAddr)
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到 三通 复电事件").arg(QString(QByteArray(eventST.reportNodeAddress.addr,6).toHex())));
                            threePowerOnFlag = true;
                        }
                        else
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("收到表号%1复电事件").arg(reportAddr));
                        }

                        if(singlePowerOnFlag==true && threePowerOnFlag==true)
                        {
                            p_delayTimer->stop();
                            p_AbstractScriptHost->updateProgress(ProcessState_Success, QString("单三通复电已上报"));
                            emScriptRunState = ScriptSuccess;
                        }
                    }
                    else
                    {
                        PowerEvent_Struct eventST;
                        eventST.eventType=char(0xFF);
                        for(int i=0;i<6;i++)
                        {
                            eventST.reportNodeAddress.addr[i]=p_EventReport_06F5_Up->report_event_unit_.frame_content_.at(6-i);
                        }
                        otherEventReportList.append(eventST);
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到从节点%1其它事件").arg(QString(QByteArray(eventST.reportNodeAddress.addr,6).toHex())));
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
void Script_PowerOffStaReport::processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID)
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
void Script_PowerOffStaReport::processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID)
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
void Script_PowerOffStaReport::sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame)
{
    if(frame==p_Confirm_00F1)
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
void Script_PowerOffStaReport::sendSrcMsg(DvcType dvcType, int dvcId, QByteArray msg)
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

void Script_PowerOffStaReport::timer_timeoutProc()
{
    p_timer->stop();
    switch(emScriptRunState)
    {
    case Wait_BuildNetFinish_Whole:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_BuildNetFinish_Whole timeout!!!");
        break;
    }
    case Wait_EventReport_Finish:
    {
        //p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_EventReport_Finish timeout!!!");
        if(singlePowerOffFlag==true && threePowerOffFlag==true)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "单三通停电已上报，上电，等待复电上报--");
            powerOn12V_STA(p_CtrInfoList,p_AbstractScriptHost);
            p_delayTimer->start(5*60*1000);
        }
        else if(singlePowerOffFlag==false && threePowerOffFlag==false)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "断电1min未收到 单通和三通 停电上报");
        }
        else if(singlePowerOffFlag==false)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "断电1min未收到 单通 停电上报");
        }
        else if(threePowerOffFlag==false)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "断电1min未收到 三通 停电上报");
        }
        else
        {}
        break;
    }
    default:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==p_timer");
        break;
    }
    }
}
void Script_PowerOffStaReport::maxAllowTimer_timeoutProc()
{
    switch(emScriptRunState)
    {
        case Wait_EventReport_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_EventReport_Finish maxAllow timeout!!!");
            break;
        }
        default:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==p_maxAllowTimer");
            break;
        }
    }
}
void Script_PowerOffStaReport::delayTimer_timeoutProc()
{
    p_delayTimer->stop();
    //calPowerOnReportRate();
    if(singlePowerOnFlag==false && threePowerOnFlag==false)
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "上电5min未收到 单通和三通 复电上报");
    }
    else if(singlePowerOnFlag==false)
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "上电5min未收到 单通 复电上报");
    }
    else if(threePowerOnFlag==false)
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "上电5min未收到 三通 复电上报");
    }
    else
    {}
}

void Script_PowerOffStaReport::calPowerOnReportRate()
{
    QString powerOnFailMeter;
    QString powerOffFailMeter;
    QString otherEventReportMeter;
    struct MeterStruct
    {
        Address meterNo;
        bool powerOnReportFlag=false;
        bool powerOffReportFlag=false;
    };
    QList<MeterStruct> allMeterList;
    for(int i=0;i<p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().size();i++)
    {
        if(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(i)->slotPosition==SingleSTA||p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(i)->slotPosition==ThreeSTA)
            continue;
        MeterStruct meter;
        memcpy(meter.meterNo.addr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(i)->mtrAddr,6);
        allMeterList.append(meter);
    }
    for(int i=0;i<allMeterList.size();i++)
    {
        for(int j=0;j<powerOnReportList.size();j++)
        {
            if(allMeterList.at(i).meterNo==powerOnReportList.at(j).reportNodeAddress)
            {
                allMeterList[i].powerOnReportFlag=true;
            }
        }
        for(int j=0;j<powerOffReportList.size();j++)
        {
            if(allMeterList.at(i).meterNo==powerOffReportList.at(j).reportNodeAddress)
            {
                allMeterList[i].powerOffReportFlag=true;
            }
        }
    }
    for(int i=0;i<otherEventReportList.size();i++)
    {
        otherEventReportMeter.append(QString(QByteArray(otherEventReportList.at(i).reportNodeAddress.addr,6).toHex())+";");
    }
    for(int i=0;i<allMeterList.size();i++)
    {
        if(allMeterList.at(i).powerOnReportFlag==false)
            powerOnFailMeter.append(QString(QByteArray(allMeterList.at(i).meterNo.addr,6).toHex())+";");
        if(allMeterList.at(i).powerOffReportFlag==false)
            powerOffFailMeter.append(QString(QByteArray(allMeterList.at(i).meterNo.addr,6).toHex())+";");
    }
    if(!otherEventReportMeter.isEmpty())
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("上报其它事件的电表如下：\n%1").arg(powerOffFailMeter));
    }
    if(powerOnFailMeter.isEmpty()==true&&powerOffFailMeter.isEmpty()==true)
    {
        emScriptRunState=ScriptSuccess;
        resultFlag=true;
        p_AbstractScriptHost->updateProgress(ProcessState_Success, QString("停复电事件上报测试成功！"));
    }
    else if(powerOnFailMeter.isEmpty()!=true&&powerOffFailMeter.isEmpty()!=true)
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("停电事件上报不全！\n失败电表如下：\n%1").arg(powerOffFailMeter));
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("上电事件上报不全！\n失败电表如下：\n%1").arg(powerOnFailMeter));
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("停复电事件均上报不全！"));
    }
    else if(powerOnFailMeter.isEmpty()!=true&&powerOffFailMeter.isEmpty()==true)
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("上电事件上报不全！\n失败电表如下：\n%1").arg(powerOnFailMeter));
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("复电事件上报不全！"));
    }
    else if(powerOnFailMeter.isEmpty()==true&&powerOffFailMeter.isEmpty()!=true)
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("停电事件上报不全！\n失败电表如下：\n%1").arg(powerOffFailMeter));
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("停电事件上报不全！"));
    }
}
