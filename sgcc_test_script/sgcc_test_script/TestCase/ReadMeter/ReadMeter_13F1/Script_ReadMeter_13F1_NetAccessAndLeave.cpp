#include "Script_ReadMeter_13F1_NetAccessAndLeave.h"





Script_ReadMeter_13F1_NetAccessAndLeave::Script_ReadMeter_13F1_NetAccessAndLeave(QObject *parent) : QObject(parent)
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
    p_DeleteArchives_11F2=make_shared<Afn11F2>();

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
Script_ReadMeter_13F1_NetAccessAndLeave::~Script_ReadMeter_13F1_NetAccessAndLeave()
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

void Script_ReadMeter_13F1_NetAccessAndLeave::execute()
{
    p_AbstractScriptHost->updateProgress(ProcessState_Start, "Test start!");
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "测试编号：【GW-STA-F003-0006-V01】\n"
                                                                  "测试流程：\n"
                                                                  "1：组网，Wait_11F1_BuildNet_Finish；\n"
                                                                  "2：给路由下发12F2，暂停路由命令，Wait_12F12_RouterPause_Finish；\n"
                                                                  "3：删除第1块表档案，Wait_11F2_DeleteArchives_Finish；\n"
                                                                  "4：马上对第1块表下发13F1抄表命令，等待路由返回应答，Wait_13F1_MeterReadDelete_Finish；\n"
                                                                  "5：等待70S，继续对第1块表下发13F1抄表命令（模块不回复，载波没有上行抄表报文），Wait_13F1_MeterReadDeleteContinue_Finish;");

    emScriptRunState=ReadMeterInit;
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
        emScriptRunState=Wait_12F12_RouterPause_Finish;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--路由暂停（12F2），等待--确认");
        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
    }
    p_maxAllowTimer->start((timerForReachThresld+timerAfterReachThresld)*1000);

}
void Script_ReadMeter_13F1_NetAccessAndLeave::stop()
{
    if(p_AbstractScriptHost==nullptr)
        return;
    p_AbstractScriptHost->updateProgress(ProcessState_Error, "Test stop!");
    if(needPowerOff==true)
        powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost);
}
void Script_ReadMeter_13F1_NetAccessAndLeave::addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq)
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
void Script_ReadMeter_13F1_NetAccessAndLeave::setHost(AbstractScriptHost *host)
{
    p_AbstractScriptHost=host;
    p_BuildNetwork_GW->setHost(host);
}
bool Script_ReadMeter_13F1_NetAccessAndLeave::config(const QMap<QString,QString> *paraDic)
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
        if(paraDic->keys().contains("readMeterSucRateThresld"))
        {
            this->readMeterSucRateThresld = (*paraDic)["readMeterSucRateThresld"].toDouble();
        }
        result = true;
    }
    return result;
}
void Script_ReadMeter_13F1_NetAccessAndLeave::processMsg(DvcType dvcType, int id, uchar *data, int datalen)
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
                emScriptRunState=Wait_12F12_RouterPause_Finish;
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
void Script_ReadMeter_13F1_NetAccessAndLeave::processCtrlDvcRes(DvcType dvcType, QList<int> idList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params)
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
    case Wait_12F12_RouterPause_Finish:
    {
        break;
    }
    case Wait_13F1_MeterReadDelete_Finish:
    {
        break;
    }
    case ScriptSuccess:
    {
        break;
    }
    }
}
void Script_ReadMeter_13F1_NetAccessAndLeave::processMsgFromCCO(DvcType dvcType, int dvcId)
{
    if(dvcId!=p_CtrInfoList->at(0)->ctrlID)
        return;
    bool haveCompleteMsg=true;
    while(haveCompleteMsg)
    {
        qInfo()<<QString("CCO-解析前 buf3762=%1").arg(QString((p_CtrInfoList->at(0)->buf.toHex())));
        shared_ptr<Frame3762Base> p_Frame3762Base = nullptr;
        
        try {
            p_Frame3762Base = p_MsgBase_3762->DecodeLocalMsg(&(p_CtrInfoList->at(0)->buf),haveCompleteMsg);
        }
        catch(...) {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, 
                QString("3762帧解析异常，跳过此帧: %1").arg(QString(p_CtrInfoList->at(0)->buf.toHex())));
            // 清空缓冲区或跳过错误数据
            p_CtrInfoList->at(0)->buf.clear();
            continue;
        }
        
        qInfo()<<QString("CCO-解析后 buf3762=%1").arg(QString((p_CtrInfoList->at(0)->buf.toHex())));
        if(p_Frame3762Base==nullptr)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, 
                QString("3762帧解析失败，跳过此帧"));
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
        case Wait_12F12_RouterPause_Finish:
        {
            if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();//接收到一条完整报文
                tryTimes=0;
                index=0;
                //times=ushort(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size());
                sendMsg(CCO_GW,dvcId,index,p_DeleteArchives_11F2);
                emScriptRunState=Wait_11F2_DeleteArchives_Finish;
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--删除第一个从节点（11F2），等待--完成");
                p_timer->start(30*1000);
            }
            else if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x02&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_12F12_RouterPause_Finish，12F1回复异常，期望回复确认，实际回复否认，【GW-STA-F003-0006-V01】");
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case Wait_11F2_DeleteArchives_Finish:
        {
            if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到路由11F2回复确认，符合期望，测试继续！");
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "等待节点离网！");
                emScriptRunState=Wait_13F1_MeterReadDelete_Finish;
                times=ushort(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size());
                sendMsg(CCO_GW,dvcId,index,p_MonitorSlaveNode_13F1);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--监控从节点（13F1），等待--点抄完成");
                p_timer->start(90*1000);
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case Wait_13F1_MeterReadDelete_Finish:
        {
            if(p_Frame3762Base->afn_==0x13&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                //p_timer->stop();
                shared_ptr<Afn13F1> p_MonitorSlaveNode_13F1_Up=dynamic_pointer_cast<Afn13F1>(p_Frame3762Base);
                if(p_MonitorSlaveNode_13F1_Up->data_field_up_.frame_content_.size()!=0)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到13F1上报，符合期望，等待从节点离网，测试继续！");

                }
                else
                {
                    QString tmp_str = "Wait_13F1_MeterReadDelete_Finish，13F1回复异常，期望数据域不为0，实际数据域为0，【GW-STA-F003-0006-V01】";
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, tmp_str);
                }
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case Wait_13F1_MeterReadDeleteContinue_Finish:
        {
            if(p_Frame3762Base->afn_==0x13&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                shared_ptr<Afn13F1> p_MonitorSlaveNode_13F1_Up=dynamic_pointer_cast<Afn13F1>(p_Frame3762Base);
                if(p_MonitorSlaveNode_13F1_Up->data_field_up_.frame_content_.size()==0)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到13F1空数据域上报，符合期望，测试通过！");
                    if(p_maxAllowTimer!=nullptr)
                        p_maxAllowTimer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Success, "脚本执行成功，【GW-STA-F003-0006-V01】");
                }
                else
                {
                    QString tmp_str = "Wait_13F1_MeterReadDeleteContinue_Finish，13F1回复异常，期望数据域不为0，实际数据域为：" + QString::fromLatin1(p_MonitorSlaveNode_13F1_Up->data_field_up_.frame_content_.toHex()) + "，【GW-STA-F003-0006-V01】";
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, tmp_str);
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
            //            default:
            //            {
            //                p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==emScriptRunState");
            //                break;
            //            }
        }
    }
}
void Script_ReadMeter_13F1_NetAccessAndLeave::processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID)
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
void Script_ReadMeter_13F1_NetAccessAndLeave::processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID)
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

void Script_ReadMeter_13F1_NetAccessAndLeave::sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame)
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
    else if(frame==p_DeleteArchives_11F2)
    {
        p_DeleteArchives_11F2->ctrl_field_.dir=kDirDown;
        p_DeleteArchives_11F2->ctrl_field_.prm=kActive;
        p_DeleteArchives_11F2->ctrl_field_.comn_type=kHplc;

        p_DeleteArchives_11F2->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_DeleteArchives_11F2->info_field_.info_field_down.comu_rate=0;
        p_DeleteArchives_11F2->info_field_.info_field_down.comu_module_ident=0;
        p_DeleteArchives_11F2->node_address_list_.clear();
        Address nodeAddr;
        for(int i=0; i<p_CtrInfoList->size(); i++)
        {
            if(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(i)->prtcl == 0x02)
            {
                memcpy(nodeAddr.addr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(i)->mtrAddr,6);
                memcpy(tmp_645_addr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(i)->mtrAddr,6);
            }
        }

        p_DeleteArchives_11F2->node_address_list_.append(nodeAddr);
        p_DeleteArchives_11F2->node_num_=uchar(p_DeleteArchives_11F2->node_address_list_.size());
        sendMsgOct=p_DeleteArchives_11F2->EncodeFrame();
        sendMsgLog=QString("》》删除从节点11F2：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
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
        if(index>=p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size())
        {
            index=0;
        }
        uchar tmpAddr[6];


        memcpy(tmpAddr,tmp_645_addr,6);
        uchar comPrtclType=0x02;

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
void Script_ReadMeter_13F1_NetAccessAndLeave::sendSrcMsg(DvcType dvcType, int dvcId, QByteArray msg)
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

void Script_ReadMeter_13F1_NetAccessAndLeave::timer_timeoutProc()
{
    switch(emScriptRunState)
    {
    case Wait_BuildNetFinish_Whole:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait build whole net finish timeout!!!");
        break;
    }
    case Wait_12F12_RouterPause_Finish:
    {
        if(++tryTimes>=3)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_12F12_RouterPause_Finish timeout!!!");
        }
        else
        {
            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_RouterPause_12F2);
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--路由暂停命令（12F2），等待--确认");
        }
        break;
    }
    case Wait_11F2_DeleteArchives_Finish:
    {
        if(++tryTimes>=3)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_11F2_DeleteArchives_Finish，“timeout！”，【GW-STA-F003-0006-V01】");
        }
        else
        {
            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_DeleteArchives_11F2);
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--路由删除从节点（11F2），等待--确认");
        }
        break;
    }
    case Wait_13F1_MeterReadDelete_Finish:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "Wait_13F1_MeterReadDelete_Finish，等待离网时间到，抄读离网节点！！");
        emScriptRunState = Wait_13F1_MeterReadDeleteContinue_Finish;
        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,index,p_MonitorSlaveNode_13F1);
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--监控从节点报文（13F1），等待--点抄完成");
        p_timer->start(120*1000);

        break;
    }
    case Wait_13F1_MeterReadDeleteContinue_Finish:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_13F1_MeterReadDeleteContinue_Finish，路由未上报13F1空数据域，测试失败！！【GW-STA-F003-0006-V01】");
        break;
    }
    default:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==p_timer");
        break;
    }
    }
}
void Script_ReadMeter_13F1_NetAccessAndLeave::maxAllowTimer_timeoutProc()
{
    switch(emScriptRunState)
    {
    case Wait_13F1_MeterReadDelete_Finish:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "Wait_13F1_MeterReadDelete_Finish timeout!!!"+QString("  13F1抄表成功率：%1%").arg(p_CtrInfoList->at(0)->successRate[1]*100));
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
void Script_ReadMeter_13F1_NetAccessAndLeave::delayTimer_timeoutProc()
{
    p_delayTimer->stop();

    index=0;
    p_maxAllowTimer->stop();

    CalcAvrgConsumeTimeLen(1);
    QString failedMeter = GenerateFailedMeterStr_ReadMeter(p_CtrInfoList->at(0),3);
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, failedMeter);

    if(p_CtrInfoList->at(0)->successRate[3]>=netSucRateThresld)
    {
        if(p_CtrInfoList->at(0)->successRate[1]>=netSucRateThresld)
        {
            emScriptRunState=ScriptSuccess;
            resultFlag=true;

            QString failedMeter = GenerateFailedMeterStr_ReadMeter(p_CtrInfoList->at(0),3);
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, failedMeter);
            p_AbstractScriptHost->updateProgress(ProcessState_Success, QString("  13F1抄表成功率：%1%; 13F1抄表平均耗时：%2秒;").arg(p_CtrInfoList->at(0)->successRate[1]*100).arg(p_CtrInfoList->at(0)->successConsume[1]));
        }
        else
        {
            QString failedMeter = GenerateFailedMeterStr_ReadMeter(p_CtrInfoList->at(0),3);
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, failedMeter);
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("  13F1抄表成功率：%1%; 13F1抄表平均耗时：%2秒;").arg(p_CtrInfoList->at(0)->successRate[1]*100).arg(p_CtrInfoList->at(0)->successConsume[1]));
        }
    }
    else
    {
        QString failedMeter = GenerateFailedMeterStr_ReadMeter(p_CtrInfoList->at(0),3);
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, failedMeter);

        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("  13F1抄表成功率：%1%; 13F1抄表平均耗时：%2秒;").arg(p_CtrInfoList->at(0)->successRate[1]*100).arg(p_CtrInfoList->at(0)->successConsume[1]));
    }
}

void Script_ReadMeter_13F1_NetAccessAndLeave::statisticResult()
{
    index=0;
    CalcAvrgConsumeTimeLen(1);
    QString failedMeter = GenerateFailedMeterStr_ReadMeter(p_CtrInfoList->at(0),1);
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, failedMeter);

    if(p_CtrInfoList->at(0)->successRate[1]>=readMeterSucRateThresld)
    {
        tryTimes=0;
        emScriptRunState=ScriptSuccess;
        p_AbstractScriptHost->updateProgress(ProcessState_Success, QString("13F1抄表测试成功，成功率：%1%; 13F1抄表平均耗时：%2秒;").arg(p_CtrInfoList->at(0)->successRate[1]*100).arg(p_CtrInfoList->at(0)->successConsume[1]));
    }
    else
    {
        if(++tryTimes>=3)
        {
            tryTimes=0;
            QString failedMeter = GenerateFailedMeterStr_ReadMeter(p_CtrInfoList->at(0),1);
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, failedMeter);
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("13F1抄表测试成功，成功率：%1%; 13F1抄表平均耗时：%2秒;").arg(p_CtrInfoList->at(0)->successRate[1]*100).arg(p_CtrInfoList->at(0)->successConsume[1]));

        }
        else
        {
            for(int i=0; i<p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().size(); i++)
            {
                if(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(i)->testResultList[1]==true)
                    continue;
                index=ushort(i);
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,index,p_MonitorSlaveNode_13F1);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--监控从节点（13F1），等待--点抄完成");
                break;
            }
        }
    }
}


void Script_ReadMeter_13F1_NetAccessAndLeave::CalcAvrgConsumeTimeLen(uchar rdFlag)
{
    if(rdFlag!=1 && rdFlag!=2 && rdFlag!=3)
        return;
    double totalConsume=0.0;
    for(int i=0; i<p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().size(); i++)
    {
        if(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(i)->testResultList[rdFlag])
            totalConsume+=p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(i)->timeConsumList[rdFlag];
    }
    if(p_CtrInfoList->at(0)->successCnt[rdFlag]!=0)
        p_CtrInfoList->at(0)->successConsume[rdFlag]=totalConsume/(p_CtrInfoList->at(0)->successCnt[rdFlag]);
    if(rdFlag==2)
    {
        totalConsume=double(timerForReachThresld*1000-p_maxAllowTimer->remainingTime())/1000.0;
        if(p_CtrInfoList->at(0)->successCnt[rdFlag]!=0)
            p_CtrInfoList->at(0)->successConsume[rdFlag]=totalConsume/(p_CtrInfoList->at(0)->successCnt[rdFlag]);
    }
}

