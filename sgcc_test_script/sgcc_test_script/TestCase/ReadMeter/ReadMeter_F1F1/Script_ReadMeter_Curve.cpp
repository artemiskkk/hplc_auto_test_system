#include "Script_ReadMeter_Curve.h"

Script_ReadMeter_Curve::Script_ReadMeter_Curve(QObject *parent) : QObject(parent)
{
    emScriptRunState=ReadMeterInit;
    resultFlag=false;
    sendMsgOct.clear();

    p_CtrInfoList=make_shared<QList<shared_ptr<CtrInfo>>>();
    p_BuildNetwork_GW=new BuildNetwork_GW();

    p_Frame3762Helper=make_shared<Frame3762Helper>();
    p_RouterPause_12F2=make_shared<Afn12F2>();
    p_RouterRecover_12F3=make_shared<Afn12F3>();
    p_ParallelReadMeter_F1F1=make_shared<AfnF1F1>();

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
Script_ReadMeter_Curve::~Script_ReadMeter_Curve()
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
void Script_ReadMeter_Curve::execute()
{
    p_AbstractScriptHost->updateProgress(ProcessState_Start, "Test start!");

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
        readInfoInit();
        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_RouterPause_12F2);
        emScriptRunState=Wait_00F1_for_12F2_Pause;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--路由暂停（12F2），等待--确认");
        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
    }
}
void Script_ReadMeter_Curve::stop()
{
    if(p_AbstractScriptHost==nullptr)
        return;
    p_AbstractScriptHost->updateProgress(ProcessState_Error, "Test stop!");
    if(needPowerOff==true)
        powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost);
}
void Script_ReadMeter_Curve::addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq)
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
void Script_ReadMeter_Curve::setHost(AbstractScriptHost *host)
{
    p_AbstractScriptHost=host;
    p_BuildNetwork_GW->setHost(host);
}
bool Script_ReadMeter_Curve::config(const QMap<QString,QString> *paraDic)
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
        if(paraDic->keys().contains("maxFrameNum"))
        {
            this->maxFrameNum = (*paraDic)["maxFrameNum"].toUShort();
        }
        if(paraDic->keys().contains("maxParallelNum"))
        {
            this->maxParallelNum = (*paraDic)["maxParallelNum"].toUShort();
        }
        if(paraDic->keys().contains("readPeriod"))
        {
            this->readPeriod = (*paraDic)["readPeriod"].toUShort();
        }
        if(paraDic->keys().contains("maxReadRound"))
        {
            this->maxReadRound = (*paraDic)["maxReadRound"].toUShort();
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
void Script_ReadMeter_Curve::processMsg(DvcType dvcType, int id, uchar *data, int datalen)
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
                readInfoInit();
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

        }
        else
        {
            return;
        }
    }
}
void Script_ReadMeter_Curve::processCtrlDvcRes(DvcType dvcType, QList<int> idList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params)
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
        case Wait_Finish_Curve:
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

void Script_ReadMeter_Curve::processMsgFromCCO(DvcType dvcType, int dvcId)
{
    if(dvcId!=p_CtrInfoList->at(0)->ctrlID)
        return;
    QByteArray completeFrame;
    QByteArray& buf = p_CtrInfoList->at(0)->buf;
    bool haveCompleteMsg=extractAndProcess3762Frame(buf,completeFrame);
    while(haveCompleteMsg)
    {
        qInfo()<<QString("CCO-解析前 buf3762=%1").arg(QString((completeFrame.toHex())));
        shared_ptr<Frame3762Base> p_Frame3762Base = p_Frame3762Helper->DecodeLocalMsg(&(completeFrame),haveCompleteMsg);
        qInfo()<<QString("CCO-解析后 buf3762=%1").arg(QString((completeFrame.toHex())));
        if(p_Frame3762Base==nullptr)
        {
            // 继续处理下一帧
            haveCompleteMsg = extractAndProcess3762Frame(buf, completeFrame);
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
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "路由暂停完成，开始曲线抄读！");
                    /////////////
                    /////////////
                    ///
                    ///
                    ///
                    p_delayTimer->start(readPeriod*60*1000);

                    p_timer->stop();//接收到一条完整报文
                    meterIndex=0;
                    parallelCount=0;//并行数
                    currentReadRound=0;//当前轮次
                    //times=ushort(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size());

                    emScriptRunState=Wait_Finish_Curve;
                    currentReadRound++;
                    //p_timer->start(maxMonitorTime*1000);
                    //p_maxAllowTimer->start(timerForReachThresld*1000);
                    //1
                    while(parallelCount<maxParallelNum)
                    {
                        if(meterIndex>=readInfoList.size())
                            break;
                        sendMsg(CCO_GW,dvcId,meterIndex,p_ParallelReadMeter_F1F1);
                        parallelCount++;
                        meterIndex++;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--并发抄表（F1F1），等待--抄表完成");
                   }
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_Finish_Curve:
            {
                if(p_Frame3762Base->afn_==char(0xF1)&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    shared_ptr<AfnF1F1> p_ParallelReadMeter_F1F1_Up=dynamic_pointer_cast<AfnF1F1>(p_Frame3762Base);
                    Address srcAddr;
                    memcpy(srcAddr.addr,p_ParallelReadMeter_F1F1_Up->address_field_.src_addr,6);
                    int currentMeterIndex=getReadInfo(srcAddr);
                    if(currentMeterIndex!=-1)
                    {
                        if(p_ParallelReadMeter_F1F1_Up->unit_up_.frame_length_==0)
                            continue;
                        QByteArray msgBuf=p_ParallelReadMeter_F1F1_Up->unit_up_.frame_content_;
                        if(p_ParallelReadMeter_F1F1_Up->unit_up_.protocol_type_==DLT645_2007)
                        {
                            QString spendTime;
                            while(msgBuf.size()>0)
                            {
                                bool res=true;
                                shared_ptr<Frame645Base> MsgBase_645_ptr = Frame645Helper::DecodeLocalMsg(&msgBuf,res);
                                if(MsgBase_645_ptr==nullptr)
                                    continue;
                                if(MsgBase_645_ptr->ctrlCode_!=NORMAL_RESP)
                                    continue;
                                shared_ptr<RspsNormal_ReadData_0x91> p_ReadData_0x91=dynamic_pointer_cast<RspsNormal_ReadData_0x91>(MsgBase_645_ptr);
                                if(memcmp(p_ReadData_0x91->addr_,srcAddr.addr,6)!=0)
                                    continue;
                                for(int i=0;i<readInfoList.at(currentMeterIndex).dataUnitList.size();i++)
                                {
                                    if(memcmp(readInfoList.at(currentMeterIndex).dataUnitList.at(i).dataID,p_ReadData_0x91->di,4)==0)
                                    {
                                        readInfoList[currentMeterIndex].dataUnitList[i].successRound++;
                                        break;
                                    }
                                    else
                                        continue;
                                }
                            }
                        }
                        else if(p_ParallelReadMeter_F1F1_Up->unit_up_.protocol_type_==OOP)
                        {
                            bool res=true;
                            shared_ptr<FrameOOPBase> MsgBase_OOP_ptr = FrameOOPHelper::DecodeMsg(&msgBuf,res);
                            if(MsgBase_OOP_ptr==nullptr)
                                continue;
                            if(0!=memcmp(QByteArray::fromHex(MsgBase_OOP_ptr->address_field_.sa.address.toLatin1()),readInfoList.at(currentMeterIndex).meterNo.addr,6))
                                continue;
                            if(MsgBase_OOP_ptr->service_type_!=GET_RESPONSE_SERVER && MsgBase_OOP_ptr->service_sub_type_!=uchar(GetResponseType::kGetResponseNormal))
                                continue;
                            shared_ptr<GetResponseNormal> p_GetResponseNormal=dynamic_pointer_cast<GetResponseNormal>(MsgBase_OOP_ptr);
                            //待修改
                            OAD oad;
                            oad.OI=PosActEne_OI;
                            oad.attribute.feature=0;
                            oad.attribute.seq=2;
                            oad.element_index=0;
                            for(int i=0;i<readInfoList.at(currentMeterIndex).dataUnitList.size();i++)
                            {
                                if(p_GetResponseNormal->a_result_normal_.oad==oad)
                                {
                                    readInfoList[currentMeterIndex].dataUnitList[i].successRound++;
                                    break;
                                }
                                else
                                    continue;
                            }
                        }
                        for(int i=0;i<readInfoList.at(currentMeterIndex).dataUnitList.size();i++)
                        {
                            if(readInfoList.at(currentMeterIndex).dataUnitList.at(i).readRound==currentReadRound)
                            {
                                if(readInfoList.at(currentMeterIndex).dataUnitList.at(i).readRound>readInfoList.at(currentMeterIndex).dataUnitList.at(i).successRound)
                                {
                                    readInfoList[currentMeterIndex].dataUnitList[i].failRound++;
                                }
                            }
                        }
                    }
                    if(isAllMeterReadThisRound()==false)
                    {
                        if(isAllReadThisMeter(currentMeterIndex)==true)
                        {
                            parallelCount--;
                            //抄读下一只
                            while(parallelCount<maxParallelNum)
                            {
                                if(meterIndex>=readInfoList.size())
                                    break;
                                sendMsg(CCO_GW,dvcId,meterIndex,p_ParallelReadMeter_F1F1);
                                parallelCount++;
                                meterIndex++;
                                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--曲线抄读（F1F1），等待--抄表完成");
                            }
                        }
                        else
                        {
                            parallelCount--;
                            while(parallelCount<maxParallelNum)
                            {
                                sendMsg(CCO_GW,dvcId,currentMeterIndex,p_ParallelReadMeter_F1F1);
                                parallelCount++;
                                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--曲线抄读（F1F1），等待--抄表完成");
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
            case Wait_00F1_for_12F3_Recover:
            {
                if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    index=0;
                    emScriptRunState=ScriptSuccess;
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
        // 继续处理下一帧
        haveCompleteMsg = extractAndProcess3762Frame(buf, completeFrame);
    }
}
void Script_ReadMeter_Curve::processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID)
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
void Script_ReadMeter_Curve::processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID)
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
void Script_ReadMeter_Curve::sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame)
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
        sendMsgLog=QString("》》路由回复12F3：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_ParallelReadMeter_F1F1)
    {
        p_ParallelReadMeter_F1F1->ctrl_field_.dir=kDirDown;
        p_ParallelReadMeter_F1F1->ctrl_field_.prm=kActive;
        p_ParallelReadMeter_F1F1->ctrl_field_.comn_type=kHplc;

        p_ParallelReadMeter_F1F1->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_ParallelReadMeter_F1F1->info_field_.info_field_down.comu_rate=0;
        p_ParallelReadMeter_F1F1->info_field_.info_field_down.comu_module_ident=1;

        uchar tmpAddr[6];
        memcpy(tmpAddr,readInfoList.at(meterID).meterNo.addr,6);
        uchar tmpCcoAddr[6];
        memcpy(tmpCcoAddr,p_CtrInfoList->at(0)->ccoAddr,6);
        memcpy(p_ParallelReadMeter_F1F1->address_field_.src_addr,tmpCcoAddr,6);
        memcpy(p_ParallelReadMeter_F1F1->address_field_.dst_addr,tmpAddr,6);

        if(readInfoList.at(meterID).protocolType==DLT645_2007)
        {
            QByteArray msg645;
            shared_ptr<Rqst_ReadData_0x11> p_ReadData_0x11=make_shared<Rqst_ReadData_0x11>(addr,4,0);
            memcpy(p_ReadData_0x11->addr_,tmpAddr,6);
            //2
            int frameCount=0;//单帧报文数
            for(int i=0;i<readInfoList.at(meterID).dataUnitList.size();i++)
            {
                if(frameCount<maxFrameNum)
                {
                    if(readInfoList.at(meterID).dataUnitList.at(i).readRound<currentReadRound)
                    {
                        memcpy(p_ReadData_0x11->di,readInfoList.at(meterID).dataUnitList.at(i).dataID,4);
                        msg645.append(p_ReadData_0x11->EncodeFrame());
                        frameCount++;
                        readInfoList[meterID].dataUnitList[i].readRound++;
                    }
                }
                else
                    break;
            }

            p_ParallelReadMeter_F1F1->unit_down_.protocol_type_=0x02;
            p_ParallelReadMeter_F1F1->unit_down_.subsidiary_node_num_=0x00;
            p_ParallelReadMeter_F1F1->unit_down_.frame_content_=msg645;
            p_ParallelReadMeter_F1F1->unit_down_.frame_length_=uchar(msg645.size());

            sendMsgOct=p_ParallelReadMeter_F1F1->EncodeFrame();
            sendMsgLog=QString("》》曲线抄读F1F1,抄读645电表%0：%1\n").arg(QString((QByteArray(reinterpret_cast<char*>(tmpAddr),6).toHex()))).arg(QString(sendMsgOct.toHex()));
        }
        else if(readInfoList.at(meterID).protocolType==OOP)
        {
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
            readInfoList[meterID].dataUnitList[0].readRound++;

            p_ParallelReadMeter_F1F1->unit_down_.subsidiary_node_num_=0x00;
            p_ParallelReadMeter_F1F1->unit_down_.protocol_type_=0x03;
            p_ParallelReadMeter_F1F1->unit_down_.frame_content_=msg698;
            p_ParallelReadMeter_F1F1->unit_down_.frame_length_=uchar(msg698.size());

            sendMsgOct=p_ParallelReadMeter_F1F1->EncodeFrame();
            sendMsgLog=QString("》》曲线抄读F1F1,抄读OOP电表%0：%1\n").arg(QString((QByteArray(reinterpret_cast<char*>(tmpAddr),6).toHex()))).arg(QString(sendMsgOct.toHex()));

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
void Script_ReadMeter_Curve::sendSrcMsg(DvcType dvcType, int dvcId, QByteArray msg)
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

void Script_ReadMeter_Curve::timer_timeoutProc()
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
        case Wait_Finish_Curve:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "Wait_F1F1_UP timeout!!! Read Next Meter");
            //需要判断是否切换电表

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
void Script_ReadMeter_Curve::maxAllowTimer_timeoutProc()
{
    switch(emScriptRunState)
    {
        case Wait_Finish_Curve:
        {            
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
void Script_ReadMeter_Curve::delayTimer_timeoutProc()
{
    p_delayTimer->stop();

    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("第【%1】轮曲线抄读完成").arg(currentReadRound));
    displayResult();
    if(currentReadRound==maxReadRound)
    {
        emScriptRunState=ScriptSuccess;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("曲线抄读完成"));
        displayResult();
        p_AbstractScriptHost->updateProgress(ProcessState_Success, QString("曲线抄读结束;"));
    }
    else
    {
        p_delayTimer->start(readPeriod*60*1000);
        currentReadRound++;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("开始第【%1】轮曲线抄读；").arg(currentReadRound));
        p_timer->stop();//接收到一条完整报文
        meterIndex=0;
        parallelCount=0;//并行数
        emScriptRunState=Wait_Finish_Curve;

        //1
        while(parallelCount<maxParallelNum)
        {
            if(meterIndex>=readInfoList.size())
                break;
            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,meterIndex,p_ParallelReadMeter_F1F1);
            parallelCount++;
            meterIndex++;
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--曲线抄读（F1F1），等待--抄表完成");
       }
    }
}

void Script_ReadMeter_Curve::readInfoInit()
{
    struct DataID_Struct
    {
        uchar dataID[4];
    };

    QList<DataID_Struct> dataIDList;
    DataID_Struct dataID_ST;
    memcpy(dataID_ST.dataID,PosActEne_Total,4);
    DataID_Struct dataID_ST1;
    memcpy(dataID_ST1.dataID,Last_DailyFre_ActEne,4);
    dataIDList.append(dataID_ST);//当前正向
    dataIDList.append(dataID_ST1);//日冻结正向
    readInfoList.clear();
    for(int i=0;i<p_CtrInfoList->size();i++)
    {
        QList<MeterInfoForSingleNet*> meterInfoList=p_CtrInfoList->at(i)->p_MeterInfoForSingleNetList->values();
        for(int j=0;j<meterInfoList.size();j++)
        {
            ReadInfo_Curve readInfo_ST;
            memcpy(readInfo_ST.meterNo.addr,meterInfoList.at(j)->mtrAddr,6);
            readInfo_ST.protocolType=meterInfoList.at(j)->prtcl;
            if(readInfo_ST.protocolType==0x02)
            {
                for(int n=0;n<dataIDList.size();n++)
                {
                    ReadDataUnit readData;
                    char id[4];
                    memcpy(id,dataIDList.at(n).dataID,4);
                    readData.dataID.append(QByteArray(id,4));
                    readInfo_ST.dataUnitList.append(readData);
                }
            }
            else if(readInfo_ST.protocolType==0x03)
            {
                ReadDataUnit readData;
                char id[4]={0x00,0x10,0x02,0x00};
                readData.dataID.append(QByteArray(id,4));
                readInfo_ST.dataUnitList.append(readData);
            }
            readInfoList.append(readInfo_ST);
        }
    }
}
bool Script_ReadMeter_Curve::isMeterExist(Address address)
{
    for(int i=0;i<readInfoList.size();i++)
    {
        if(address==readInfoList.at(i).meterNo)
        {
            return true;
        }
    }
    return false;
}
int Script_ReadMeter_Curve::getReadInfo(Address address)
{
    for(int i=0;i<readInfoList.size();i++)
    {
        if(address==readInfoList.at(i).meterNo)
        {
            return i;
        }
    }
    return -1;
}
void Script_ReadMeter_Curve::calSuccessRate()
{
    for(int i=0;i<readInfoList.size();i++)
    {
        for(int j=0;j<readInfoList.at(i).dataUnitList.size();j++)
        {
            readInfoList[i].dataUnitList[j].successRate=double(readInfoList.at(i).dataUnitList.at(j).successRound)/double(readInfoList.at(i).dataUnitList.at(j).readRound);
        }
    }
}

void Script_ReadMeter_Curve::displayResult()
{
    calSuccessRate();
    //////曲线抄读结果展示
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("曲线抄读结果如下:\n"));
    QString displayContent;
    for(int i=0;i<readInfoList.size();i++)
    {
        displayContent.append(QString("表号%1 ").arg(i+1));
        displayContent.append(QString(QByteArray(readInfoList.at(i).meterNo.addr,6).toHex())+" ");
        for(int j=0;j<readInfoList.at(i).dataUnitList.size();j++)
        {
            if(readInfoList.at(i).protocolType==0x02)
                displayContent.append(QString("数据项%1：").arg(QString(reverseArray(readInfoList.at(i).dataUnitList.at(j).dataID).toHex())));
            else if(readInfoList.at(i).protocolType==0x03)
                displayContent.append(QString("OAD项%1：").arg(QString(readInfoList.at(i).dataUnitList.at(j).dataID.toHex())));
            displayContent.append(QString("抄读%1次;").arg(readInfoList.at(i).dataUnitList.at(j).readRound));
            displayContent.append(QString("成功%1次;").arg(readInfoList.at(i).dataUnitList.at(j).successRound));
            displayContent.append(QString("成功率%1% ").arg(readInfoList.at(i).dataUnitList.at(j).successRate*100));
            if(j==readInfoList.at(i).dataUnitList.size()-1)
                displayContent.append("\n");
        }
    }
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, displayContent);
}
QString Script_ReadMeter_Curve::calCostTime()
{
    double totalConsume=0.0;
    double successCount=0.0;
//    for(int i=0;i<readInfoList.size();i++)
//    {
//        if(readInfoList.at(i).readFlag==ReadSuccess)
//        {
//            double singleConsume=0.0;
//            successCount++;
//            for(int j=0;j<readInfoList.at(i).dataUnitList.size();j++)
//            {
//                singleConsume+=readInfoList.at(i).dataUnitList.at(j).costTime;
//            }
//            totalConsume+=singleConsume/double(readInfoList.at(i).dataUnitList.size());
//        }
//    }
    return QString::number(totalConsume/successCount,'g',3);
}
QString Script_ReadMeter_Curve::getFailMeterNo()
{
    QString failMeterNo;
//    for(int i=0;i<readInfoList.size();i++)
//    {
//        if(readInfoList.at(i).readFlag!=ReadSuccess)
//            failMeterNo+=QString(QByteArray(readInfoList.at(i).meterNo.addr,6).toHex())+";";
//    }
    return failMeterNo;
}

bool Script_ReadMeter_Curve::isAllReadThisMeter(int meterID)
{
    bool sus=true;
    for(int i=0;i<readInfoList.at(meterID).dataUnitList.size();i++)
    {
        if(readInfoList.at(meterID).dataUnitList.at(i).readRound!=currentReadRound)
        {
            sus=false;
            break;
        }
    }
    return sus;
}

bool Script_ReadMeter_Curve::isAllMeterReadThisRound()
{
    bool sus=true;
    for(int i=0;i<readInfoList.size();i++)
    {
        for(int j=0;j<readInfoList.at(i).dataUnitList.size();j++)
        {
            if(readInfoList.at(i).dataUnitList.at(j).readRound!=readInfoList.at(i).dataUnitList.at(j).successRound+readInfoList.at(i).dataUnitList.at(j).failRound)
            {
                sus=false;
                break;
            }
        }
    }

    return sus;
}
void Script_ReadMeter_Curve::CalcAvrgConsumeTimeLen(uchar rdFlag)
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

bool Script_ReadMeter_Curve::extractAndProcess3762Frame(QByteArray& buf,QByteArray& completeFrame)
{
    completeFrame.clear();

    while (buf.size() > 0 && ((unsigned char)buf[0] == 0xFE || (unsigned char)buf[0] == 0x1E)) {
        buf.remove(0, 1);
    }

    if (buf.size() < 6) {
        return false;
    }

    int frameStart = -1;
    for (int i = 0; i < buf.size() - 5; i++) {
        if ((unsigned char)buf[i] == 0x68) {
            uint16_t potentialLength = 0;
            memcpy(&potentialLength, buf.constData() + i + 1, 2);
            if (potentialLength > 3 && potentialLength < 500) {
                if (buf.size() >= i + potentialLength) {
                    if ((unsigned char)buf[i + potentialLength - 1] == 0x16) {
                        frameStart = i;
                        break;
                    }
                } else {
                    frameStart = i;
                    break;
                }
            }
        }
    }

    if (frameStart == -1) {
        if (buf.size() > 5) buf.remove(0, buf.size() - 5);
        return false;
    }

    if (frameStart > 0) buf.remove(0, frameStart);
    if (buf.size() < 6) return false;

    uint16_t frameLength = 0;
    memcpy(&frameLength, buf.constData() + 1, 2);

    if (frameLength < 4 || frameLength > 500) {
        buf.remove(0, 1);
        return extractAndProcess3762Frame(buf, completeFrame);
    }

    if (buf.size() < frameLength) {
        p_AbstractScriptHost->updateProgress(ProcessState_Processing,
            QString("等待完整帧，需要%1字节，当前%2字节").arg(frameLength).arg(buf.size()));
        return false;
    }

    if ((unsigned char)buf[frameLength - 1] != 0x16) {
        buf.remove(0, 1);
        return extractAndProcess3762Frame(buf, completeFrame);
    }

    completeFrame = buf.left(frameLength);
    buf.remove(0, frameLength);

    p_AbstractScriptHost->updateProgress(ProcessState_Processing,
        QString("提取到完整3762帧: %1").arg(QString(completeFrame.toHex())));


    return true;
}

