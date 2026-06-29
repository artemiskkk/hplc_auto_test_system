#include "Script_TopoIndentity_Router_645.h"

Script_TopoIndentity_Router_645::Script_TopoIndentity_Router_645(QObject *parent) : QObject(parent)
{
    emScriptRunState=ReadMeterInit;
    resultFlag=false;
    sendMsgOct.clear();

    p_CtrInfoList=make_shared<QList<shared_ptr<CtrInfo>>>();
    p_BuildNetwork_GW=new BuildNetwork_GW();

    p_MsgBase_3762=make_shared<Frame3762Helper>();
    p_RouterPause_12F2=make_shared<Afn12F2>();
    p_RouterRecover_12F3=make_shared<Afn12F3>();
    p_MonitorSlaveNode_13F1=make_shared<Afn13F1>();

    p_Frame645Helper=make_shared<Frame645Helper>();
    p_MeterAddrResp_93=make_shared<RspsNormal_ReadAddr_0x93>(addr,6);
    p_FrameOOPHelper=make_shared<FrameOOPHelper>();
    p_GetResponseNormal_ReadAddr=make_shared<GetResponseNormal>();
//    p_MstrStnNrmlRqst=new MstrStnNrmlRqst();
//    p_MeterAddrResp_93=new MeterAddrResp_93();
//    p_SlaveNodeNormalResp=new SlaveNodeNormalResp();

    p_timer=new QTimer();
    p_maxAllowTimer=new QTimer();
    p_delayTimer=new QTimer();
    connect(p_timer,SIGNAL(timeout()),this,SLOT(timer_timeoutProc()));
    connect(p_maxAllowTimer,SIGNAL(timeout()),this,SLOT(maxAllowTimer_timeoutProc()));
    connect(p_delayTimer,SIGNAL(timeout()),this,SLOT(delayTimer_timeoutProc()));
}
Script_TopoIndentity_Router_645::~Script_TopoIndentity_Router_645()
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

void Script_TopoIndentity_Router_645::execute()
{
    p_AbstractScriptHost->updateProgress(ProcessState_Start, "Test start!");

    emScriptRunState=ReadMeterInit;
    resultFlag=false;
    addrList.clear();
    readInfoInit();

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
void Script_TopoIndentity_Router_645::stop()
{
    if(p_AbstractScriptHost==nullptr)
        return;
    p_AbstractScriptHost->updateProgress(ProcessState_Error, "Test stop!");
    if(needPowerOff==true)
        powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost);
}
void Script_TopoIndentity_Router_645::addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq)
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
void Script_TopoIndentity_Router_645::setHost(AbstractScriptHost *host)
{
    p_AbstractScriptHost=host;
    p_BuildNetwork_GW->setHost(host);
}
bool Script_TopoIndentity_Router_645::config(const QMap<QString,QString> *paraDic)
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
void Script_TopoIndentity_Router_645::processMsg(DvcType dvcType, int id, uchar *data, int datalen)
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
                p_timer->start(10*1000);
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
        else
        {
            return;
        }
    }
}
void Script_TopoIndentity_Router_645::processCtrlDvcRes(DvcType dvcType, QList<int> idList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params)
{
    if(isSucs==false)
        return;
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到控制命令回复，参数个数=%1").arg(QString::number(params.size())));
    //    if(emScriptRunState!=Wait_BuildNetFinish_Whole)
    //        Refresh_CtrInfo_Result_for_CtrlCmdRes(p_CtrInfoList->at(0), dvcType, idList.at(0), ctrlCmdType);
    QList<int> sendParams;
    switch(emScriptRunState)
    {
        case ReadMeterInit:
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
        case Wait_Finish_13F1:
        {
            break;
        }
        case Wait_00F1_for_12F3_Recover:
        {
            break;
        }
        case ScriptSuccess:
        {
            break;
        }
    }
}
void Script_TopoIndentity_Router_645::processMsgFromCCO(DvcType dvcType, int dvcId)
{
    if(dvcId!=p_CtrInfoList->at(0)->ctrlID)
        return;
    bool haveCompleteMsg=true;
    while(haveCompleteMsg)
    {
        qInfo()<<QString("CCO-解析前 buf3762=%1").arg(QString((p_CtrInfoList->at(0)->buf.toHex())));
        shared_ptr<Frame3762Base> p_Frame3762Base = p_MsgBase_3762->DecodeLocalMsg(&(p_CtrInfoList->at(0)->buf),haveCompleteMsg);
        qInfo()<<QString("CCO-解析后 buf3762=%1").arg(QString((p_CtrInfoList->at(0)->buf.toHex())));
        if(p_Frame3762Base==nullptr)
        {
            continue;
        }
        switch(emScriptRunState)
        {
            case ReadMeterInit:
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
                    tryTimes=0;
                    index=0;
                    times=ushort(diStructList.size());
                    sendMsg(CCO_GW,dvcId,index,p_MonitorSlaveNode_13F1);
                    emScriptRunState=Wait_Finish_13F1;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--监控从节点（13F1），等待--完成");
                    p_timer->start(maxMonitorTime*1000);
                    p_maxAllowTimer->start(timerForReachThresld*1000);
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_Finish_13F1:
            {
                if(p_Frame3762Base->afn_==0x13&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    shared_ptr<Afn13F1> p_MonitorSlaveNode_13F1_Up=dynamic_pointer_cast<Afn13F1>(p_Frame3762Base);
                    if(p_MonitorSlaveNode_13F1_Up->data_field_up_.frame_content_.size()==0)
                        continue;
                    QByteArray msgBuf=p_MonitorSlaveNode_13F1_Up->data_field_up_.frame_content_;
                    if(p_MonitorSlaveNode_13F1_Up->data_field_up_.protocol_type_==0x02)
                    {
                        bool res=true;
                        shared_ptr<Frame645Base> MsgBase_645_ptr = Frame645Helper::DecodeLocalMsg(&msgBuf,res);
                        if(MsgBase_645_ptr==nullptr)
                            continue;
                        if(MsgBase_645_ptr->ctrlCode_==NORMAL_RESP_WRITE_DATA||MsgBase_645_ptr->ctrlCode_==AbNORMAL_RESP_WRITE_DATA)
                        {
                            shared_ptr<RspsNormal_WriteData_0x94> p_WriteData_0x94=dynamic_pointer_cast<RspsNormal_WriteData_0x94>(MsgBase_645_ptr);
                            shared_ptr<RspsAbNormal_WriteData_0xD4> p_WriteData_0xD4=dynamic_pointer_cast<RspsAbNormal_WriteData_0xD4>(MsgBase_645_ptr);
                            if(p_WriteData_0x94!=nullptr)
                            {
                                diStructList[index].readFlag="正常";
                                p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("回复正常！"));
                                index++;
                            }
                            else if(p_WriteData_0xD4!=nullptr)
                            {
                                p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("回复异常！"));
                                index++;
                            }
                            else
                                continue;
                        }
                        else
                            continue;
                    }
                    else if(p_MonitorSlaveNode_13F1_Up->data_field_up_.protocol_type_==0x03)
                    {
                        bool res=true;
                        shared_ptr<FrameOOPBase> MsgBase_OOP_ptr = FrameOOPHelper::DecodeMsg(&msgBuf,res);
                        if(MsgBase_OOP_ptr==nullptr)
                            continue;
                        if(0!=memcmp(QByteArray::fromHex(MsgBase_OOP_ptr->address_field_.sa.address.toLatin1()),p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(index)->mtrAddr,6))
                            continue;
                        if(MsgBase_OOP_ptr->service_type_!=GET_RESPONSE_SERVER && MsgBase_OOP_ptr->service_sub_type_!=uchar(GetResponseType::kGetResponseNormal))
                            continue;
                        shared_ptr<GetResponseNormal> p_GetResponseNormal=dynamic_pointer_cast<GetResponseNormal>(MsgBase_OOP_ptr);

                        OAD oad;
                        oad.OI=PosActEne_OI;
                        oad.attribute.feature=0;
                        oad.attribute.seq=2;
                        oad.element_index=0;

                        if(p_GetResponseNormal->a_result_normal_.oad!=oad)
                        {
                            MsgBase_OOP_ptr=nullptr;
                            continue;
                        }
                        else
                        {
                            endTime=QDateTime::currentDateTime().toString(CURRENT_TIME_FORMAT);
                            (p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values())[index]->timeConsumList[1]=calculateConsumeLen(startTime,endTime).toDouble();
                            MsgBase_OOP_ptr=nullptr;
                        }
                    }
                    //Refresh_CtrInfo_Result_for_ReadMeter_13F1(p_CtrInfoList->at(0),p_MonitorSlaveNode_13F1_Up);
                    //p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("当前点抄（13F1）成功率：%1%;").arg(p_CtrInfoList->at(0)->successRate[1]*100));

                    if(index>=times)
                    {
                        index=0;
                        int denyCount=0;
                        QString info;
                        for(int i=0;i<diStructList.size();i++)
                        {
                            if(diStructList.at(i).readFlag=="异常")
                                denyCount++;
                            info+=QString("序号%1 DI为%2 特征信号频率为%3Hz 回复%4\n").arg(i+1).arg(QString(reverseArray(QByteArray(diStructList.at(i).di,4)).toHex())).arg(QString::number(diStructList.at(i).frequence)).arg(diStructList.at(i).readFlag);
                        }
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, info);
                        emScriptRunState=ScriptSuccess;
                        if(denyCount==0)
                            p_AbstractScriptHost->updateProgress(ProcessState_Success, QString("户变识别测试完毕;"));
                        else
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("存在%1次异常回复！").arg(denyCount));
                            p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("户变识别测试完毕，存在%1次异常回复！").arg(denyCount));
                        }
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("等待200s，开始下一次发送！"));
                        p_delayTimer->start(200*1000);
                    }
                }

                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_00F1_for_12F3_Recover:
            {
                if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    index=0;
                    emScriptRunState=ScriptSuccess;
                    //p_AbstractScriptHost->updateProgress(ProcessState_Success, QString
                    //p_AbstractScriptHost->updateProgress(ProcessState_Processing, "等待--路由主动抄表完成");
                    //p_maxAllowTimer->start(timerForReachThresld*1000);
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
//            default:
//            {
//                p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==emScriptRunState");
//                break;
//            }
        }
    }
}
void Script_TopoIndentity_Router_645::processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID)
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
            case ReadMeterInit:
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
                        if(!tmpSendMsg.isEmpty())
                        {
                            sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                        }
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
void Script_TopoIndentity_Router_645::processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID)
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
            case ReadMeterInit:
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

void Script_TopoIndentity_Router_645::sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame)
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
    else if(frame==p_RouterRecover_12F3)
    {
        p_RouterRecover_12F3->ctrl_field_.dir=kDirDown;
        p_RouterRecover_12F3->ctrl_field_.prm=kActive;
        p_RouterRecover_12F3->ctrl_field_.comn_type=kHplc;

        p_RouterRecover_12F3->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_RouterRecover_12F3->info_field_.info_field_down.comu_rate=0;
        p_RouterRecover_12F3->info_field_.info_field_down.comu_module_ident=0;

        sendMsgOct=p_RouterRecover_12F3->EncodeFrame();
        sendMsgLog=QString("》》路由恢复12F3：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_MonitorSlaveNode_13F1)
    {
        uchar tmpAddr[6];
        memcpy(tmpAddr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(0)->mtrAddr,6);
        uchar comPrtclType=p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(0)->prtcl;

        if(comPrtclType==0x02)
        {
            shared_ptr<rqst_WriteData_Current_0x14> p_WriteData_0x14=make_shared<rqst_WriteData_Current_0x14>(addr,0);
            //reverseAddr(tmpAddr, 6);
            memcpy(p_WriteData_0x14->addr_,tmpAddr,6);
            memcpy(p_WriteData_0x14->di,diStructList.at(index).di,4);
            //特征电流参数
            p_WriteData_0x14->current_data_=diStructList.at(index).currentST;
//            p_WriteData_0x14->current_data_.devive_type_=0x00;
//            memset(p_WriteData_0x14->current_data_.send_time_,0xff,6);
//            p_WriteData_0x14->current_data_.bit_width_=600;
//            p_WriteData_0x14->current_data_.sequence_=833.33;
//            p_WriteData_0x14->current_data_.word_info_length_=2;
//            p_WriteData_0x14->current_data_.word_info_=QByteArray::fromHex("AAE9");
//            p_WriteData_0x14->current_data_.high_pulse_width_=400;
//            p_WriteData_0x14->current_data_.low_pulse_width_=800;
            QByteArray msg645=p_WriteData_0x14->EncodeFrame();

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

            startTime=QDateTime::currentDateTime().toString(CURRENT_TIME_FORMAT);
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

            startTime=QDateTime::currentDateTime().toString(CURRENT_TIME_FORMAT);
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
void Script_TopoIndentity_Router_645::sendSrcMsg(DvcType dvcType, int dvcId, QByteArray msg)
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

void Script_TopoIndentity_Router_645::timer_timeoutProc()
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
        case Wait_Finish_13F1:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "Wait_13F1_UP timeout!!! Read Next Meter");
            //需要判断是否切换电表
            if(++index>=times)
            {
                index=0;
                //parallelReadMeterList.clear();
            }
            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,index,p_MonitorSlaveNode_13F1);
            p_timer->start(maxMonitorTime*1000);
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--监控从节点报文（13F1），等待--点抄完成");

            break;
        }
        case Wait_00F1_for_12F3_Recover:
        {
            if(++tryTimes>=3)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_00F1_for_12F1_Restart timeout!!!");
            }
            else
            {
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_RouterRecover_12F3);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--路由恢复命令（12F3），等待--确认");
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
void Script_TopoIndentity_Router_645::maxAllowTimer_timeoutProc()
{
    switch(emScriptRunState)
    {
        case Wait_Finish_13F1:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "Wait_Finish_13F1 timeout!!!"+QString("  13F1抄表成功率：%1%").arg(p_CtrInfoList->at(0)->successRate[1]*100));
            QString failedMeter = GenerateFailedMeterStr_ReadMeter(p_CtrInfoList->at(0),1);
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, failedMeter);

            p_CtrInfoList->at(0)->successConsume[1]=double(timerForReachThresld)/double(p_CtrInfoList->at(0)->successCnt[1]);
            p_maxAllowTimer->stop();

            p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==p_maxAllowTimer");
            break;
        }
        default:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==p_maxAllowTimer");
            break;
        }
    }
}
void Script_TopoIndentity_Router_645::delayTimer_timeoutProc()
{
    p_delayTimer->stop();

    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,index,p_MonitorSlaveNode_13F1);
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--监控从节点（13F1），等待--完成");
    p_timer->start(maxMonitorTime*1000);
//    QString failedMeter = GenerateFailedMeterStr_ReadMeter(p_CtrInfoList->at(0),3);
//    p_AbstractScriptHost->updateProgress(ProcessState_Processing, failedMeter);

//    if(p_CtrInfoList->at(0)->successRate[3]>=netSucRateThresld)
//    {
//        if(p_CtrInfoList->at(0)->successRate[1]>=netSucRateThresld)
//        {
//            emScriptRunState=ScriptSuccess;
//            resultFlag=true;

//            QString failedMeter = GenerateFailedMeterStr_ReadMeter(p_CtrInfoList->at(0),3);
//            p_AbstractScriptHost->updateProgress(ProcessState_Processing, failedMeter);
//            p_AbstractScriptHost->updateProgress(ProcessState_Success, QString("  13F1抄表成功率：%1%; 13F1抄表平均耗时：%2秒;").arg(p_CtrInfoList->at(0)->successRate[1]*100).arg(p_CtrInfoList->at(0)->successConsume[1]));
//        }
//        else
//        {
//            QString failedMeter = GenerateFailedMeterStr_ReadMeter(p_CtrInfoList->at(0),3);
//            p_AbstractScriptHost->updateProgress(ProcessState_Processing, failedMeter);
//            p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("  13F1抄表成功率：%1%; 13F1抄表平均耗时：%2秒;").arg(p_CtrInfoList->at(0)->successRate[1]*100).arg(p_CtrInfoList->at(0)->successConsume[1]));
//        }
//    }
//    else
//    {
//        QString failedMeter = GenerateFailedMeterStr_ReadMeter(p_CtrInfoList->at(0),3);
//        p_AbstractScriptHost->updateProgress(ProcessState_Processing, failedMeter);

//        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("  13F1抄表成功率：%1%; 13F1抄表平均耗时：%2秒;").arg(p_CtrInfoList->at(0)->successRate[1]*100).arg(p_CtrInfoList->at(0)->successConsume[1]));
    //    }
}


void Script_TopoIndentity_Router_645::readInfoInit()
{
    currentStructList.clear();
    CurrentStruct currentST1;
    currentST1.devive_type_=0x00;
    memset(currentST1.send_time_,0xff,6);
    currentST1.bit_width_=600;
    currentST1.frequence_=833.33;
    currentST1.word_info_length_=2;
    currentST1.word_info_=QByteArray::fromHex("AAE9");
    currentST1.high_pulse_width_=400;
    currentST1.low_pulse_width_=800;
    currentStructList.append(currentST1);

    CurrentStruct currentST2;
    currentST2.devive_type_=0x00;
    memset(currentST2.send_time_,0xff,6);
    currentST2.bit_width_=600;
    currentST2.frequence_=1666.66;
    currentST2.word_info_length_=2;
    currentST2.word_info_=QByteArray::fromHex("AAE9");
    currentST2.high_pulse_width_=200;
    currentST2.low_pulse_width_=400;
    currentStructList.append(currentST2);

    diStructList.clear();
    uchar DI_Array[3][4]={{0x09,0x01,0x08,0x09},{0x06,0x01,0x08,0x09},{0x04,0x01,0x08,0x09}};
    for(int i=0;i<currentStructList.size();i++)
    {
        for(int j=0;j<3;j++)
        {
            if(i!=0&&j>0)
                continue;
            DIStruct diST;
            memcpy(diST.di,DI_Array[j],4);
            diST.currentST=currentStructList.at(i);            
            if(j==0)
                diST.currentST.need_Frequence_=true;
            diST.frequence=currentStructList.at(i).frequence_;
            diStructList.append(diST);
        }
    }
}
