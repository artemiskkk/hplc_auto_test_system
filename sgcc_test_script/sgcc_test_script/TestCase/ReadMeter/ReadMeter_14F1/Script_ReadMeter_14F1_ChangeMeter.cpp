#include "Script_ReadMeter_14F1_ChangeMeter.h"

Script_ReadMeter_14F1_ChangeMeter::Script_ReadMeter_14F1_ChangeMeter(QObject *parent) : QObject(parent)
{
    emScriptRunState=TestInit;
    resultFlag=false;
    sendMsgOct.clear();

    p_CtrInfoList=make_shared<QList<shared_ptr<CtrInfo>>>();
    p_BuildNetwork_GW=new BuildNetwork_GW();

    p_Frame3762Helper=make_shared<Frame3762Helper>();
    p_Confirm_00F1=make_shared<Afn00F1>();
    p_Deny_00F2=make_shared<Afn00F2>();
    p_RouterRestart_12F1=make_shared<Afn12F1>();
    p_RouterPause_12F2=make_shared<Afn12F2>();
    p_RouterRecover_12F3=make_shared<Afn12F3>();
    p_RouterRequestRead_14F1=make_shared<Afn14F1>();

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
Script_ReadMeter_14F1_ChangeMeter::~Script_ReadMeter_14F1_ChangeMeter()
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

void Script_ReadMeter_14F1_ChangeMeter::execute()
{
    p_AbstractScriptHost->updateProgress(ProcessState_Start, "Test start!");

    emScriptRunState=TestInit;
    resultFlag=false;
    addrList.clear();

    if(needBuildNet==true)
    {
        p_BuildNetwork_GW->execute();
        emScriptRunState=Wait_BuildNetFinish_Whole;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "等待组网完成");
        p_timer->start((timerForReachThresld+timerAfterReachThresld)*1000);
    }
    else
    {
//        tryTimes=0;
//        readInfoInit();
//        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_RouterRestart_12F1);
//        emScriptRunState=Wait_00F1_for_12F1_Restart;
//        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--路由重启（12F1），等待--确认");
//        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
    }
    p_maxAllowTimer->start((timerForReachThresld+timerAfterReachThresld)*1000);

}
void Script_ReadMeter_14F1_ChangeMeter::stop()
{
    if(p_AbstractScriptHost==nullptr)
        return;
    p_AbstractScriptHost->updateProgress(ProcessState_Error, "Test stop!");
    if(needPowerOff==true)
        powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost);
}
void Script_ReadMeter_14F1_ChangeMeter::addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq)
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
void Script_ReadMeter_14F1_ChangeMeter::setHost(AbstractScriptHost *host)
{
    p_AbstractScriptHost=host;
    p_BuildNetwork_GW->setHost(host);
}
bool Script_ReadMeter_14F1_ChangeMeter::config(const QMap<QString,QString> *paraDic)
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
void Script_ReadMeter_14F1_ChangeMeter::processMsg(DvcType dvcType, int id, uchar *data, int datalen)
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
                emScriptRunState=Wait_14F1_FirstTime;
                QList<double> sendParams;
                QList<int> idList;
                sendParams.clear();
                idList.clear();
                idList = findDvcIdList(p_CtrInfoList,SingleSTA);
                p_AbstractScriptHost->controlDvc(SingleSTA,idList,CtrlCmd_PowerOff_12V,sendParams);
                QThread::msleep(50);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "单通12V断电，等待90s");
                p_delayTimer->start(90*1000);
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
void Script_ReadMeter_14F1_ChangeMeter::processCtrlDvcRes(DvcType dvcType, QList<int> idList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params)
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
        default:
        {
            break;
        }
    }
}
void Script_ReadMeter_14F1_ChangeMeter::processMsgFromCCO(DvcType dvcType, int dvcId)
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
            continue;
        }
        msgSeq=uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq);
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
        case Wait_14F1_FirstTime:
        {
            if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到确认，等待14F1请求");
                p_timer->start(10*1000);
            }
            else if(p_Frame3762Base->afn_==0x14&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                shared_ptr<Afn14F1> p_RouterRequestRead_14F1_Up=dynamic_pointer_cast<Afn14F1>(p_Frame3762Base);
                int currentMeterIndex=getReadInfo(p_RouterRequestRead_14F1_Up->node_address_);
                firstMeterIndex=currentMeterIndex;
                if(currentMeterIndex!=-1)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到第%0个14F1请求，回复抄读数据项，表号").arg(++No_14F1)+QByteArray(p_RouterRequestRead_14F1_Up->node_address_.addr,6).toHex());
                    sendMsg(dvcType,dvcId,currentMeterIndex,p_RouterRequestRead_14F1);
                    emScriptRunState=Wait_14F1_SecondTime;
                    p_timer->start(40*1000);
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, "收到14F1请求非档案中的表，表号"+QByteArray(p_RouterRequestRead_14F1_Up->node_address_.addr,6).toHex());
                }
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case Wait_14F1_SecondTime:
        {
            if(p_Frame3762Base->afn_==0x06&&p_Frame3762Base->dt1_==0x02&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                shared_ptr<Afn06F2> p_ReportReadData_Up=dynamic_pointer_cast<Afn06F2>(p_Frame3762Base);
                Address srcAddr = extractAddressFromAfn06F2(p_ReportReadData_Up);
                int currentMeterIndex=getReadInfo(srcAddr);
                readMeterIndex=currentMeterIndex;
                if(currentMeterIndex!=-1)
                {
                    if(p_ReportReadData_Up->report_data_unit_.frame_content_.length()>16)
                    {
                        readSucFlag=true;
                        readInfoList[currentMeterIndex].dataUnitList[0].notRead=false;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到06F2上报抄读数据，表号"+QByteArray(srcAddr.addr,6).toHex());
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "06F2上报抄读数据内容为空："+p_ReportReadData_Up->report_data_unit_.frame_content_.toHex());
                    }
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, "06F2上报抄读数据非档案中的表，表号"+QByteArray(srcAddr.addr,6).toHex());
                }
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "回复确认");
                sendMsg(dvcType,dvcId,INSIGNIFICANCE,p_Confirm_00F1);
                p_timer->start(10*1000);
            }
            else if(p_Frame3762Base->afn_==0x14&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                shared_ptr<Afn14F1> p_RouterRequestRead_14F1_Up=dynamic_pointer_cast<Afn14F1>(p_Frame3762Base);
                int currentMeterIndex=getReadInfo(p_RouterRequestRead_14F1_Up->node_address_);
                if(readSucFlag==true)
                {
                    if(currentMeterIndex!=-1 && currentMeterIndex==readMeterIndex)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到第%0个14F1请求，再次请求刚抄到的表，回复抄读成功，表号").arg(++No_14F1)+QByteArray(p_RouterRequestRead_14F1_Up->node_address_.addr,6).toHex());
                        sendMsg(dvcType,dvcId,currentMeterIndex,p_RouterRequestRead_14F1);
                        p_timer->start(10*1000);
                        readSucFlag=false;
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "收到14F1请求非档案中的表 或 请求的不是刚抄到的表，表号"+QByteArray(p_RouterRequestRead_14F1_Up->node_address_.addr,6).toHex());
                    }
                }
                else
                {
                    if(currentMeterIndex!=-1 && currentMeterIndex!=firstMeterIndex)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到第%0个14F1请求，回复抄读数据项，表号").arg(++No_14F1)+QByteArray(p_RouterRequestRead_14F1_Up->node_address_.addr,6).toHex());
                        sendMsg(dvcType,dvcId,currentMeterIndex,p_RouterRequestRead_14F1);
                        emScriptRunState=Wait_14F1_Round;
                        p_timer->start(40*1000);
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "收到14F1请求非档案中的表 或 和上一次请求表号一致，表号"+QByteArray(p_RouterRequestRead_14F1_Up->node_address_.addr,6).toHex());
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
        case Wait_14F1_Round:
        {
            if(p_Frame3762Base->afn_==0x14&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                shared_ptr<Afn14F1> p_RouterRequestRead_14F1_Up=dynamic_pointer_cast<Afn14F1>(p_Frame3762Base);
                int currentMeterIndex=getReadInfo(p_RouterRequestRead_14F1_Up->node_address_);
                if(readSucFlag==true)
                {
                    if(currentMeterIndex!=-1 && currentMeterIndex==readMeterIndex)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到第%0个14F1请求，再次请求刚抄到的表，回复抄读成功，表号").arg(++No_14F1)+QByteArray(p_RouterRequestRead_14F1_Up->node_address_.addr,6).toHex());
                        sendMsg(dvcType,dvcId,currentMeterIndex,p_RouterRequestRead_14F1);
                        p_timer->start(10*1000);
                        readSucFlag=false;
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "收到14F1请求非档案中的表 或 请求的不是刚抄到的表，表号"+QByteArray(p_RouterRequestRead_14F1_Up->node_address_.addr,6).toHex());
                    }
                }
                else
                {
                    if(currentMeterIndex!=-1 && currentMeterIndex!=readMeterIndex)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到第%0个14F1请求，回复抄读数据项，表号").arg(++No_14F1)+QByteArray(p_RouterRequestRead_14F1_Up->node_address_.addr,6).toHex());
                        sendMsg(dvcType,dvcId,currentMeterIndex,p_RouterRequestRead_14F1);
                        p_timer->start(40*1000);
                        if(No_14F1==6)
                        {
                            p_timer->stop();
                            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_RouterPause_12F2);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--路由暂停（12F2），等待--确认");
                            p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                        }
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "收到14F1请求非档案中的表 或 请求了之前抄回来的表，表号"+QByteArray(p_RouterRequestRead_14F1_Up->node_address_.addr,6).toHex());
                    }
                }
            }
            else if(p_Frame3762Base->afn_==0x06&&p_Frame3762Base->dt1_==0x02&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                shared_ptr<Afn06F2> p_ReportReadData_Up=dynamic_pointer_cast<Afn06F2>(p_Frame3762Base);
                Address srcAddr = extractAddressFromAfn06F2(p_ReportReadData_Up);
                int currentMeterIndex=getReadInfo(srcAddr);
                readMeterIndex=currentMeterIndex;
                if(currentMeterIndex!=-1)
                {
                    if(p_ReportReadData_Up->report_data_unit_.frame_content_.length()>16)
                    {
                        readSucFlag=true;
                        readInfoList[currentMeterIndex].dataUnitList[0].notRead=false;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到06F2上报抄读数据，表号"+QByteArray(srcAddr.addr,6).toHex());
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "06F2上报抄读数据内容为空："+p_ReportReadData_Up->report_data_unit_.frame_content_.toHex());
                    }
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, "06F2上报抄读数据非档案中的表，表号"+QByteArray(srcAddr.addr,6).toHex());
                }
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "回复确认");
                sendMsg(dvcType,dvcId,INSIGNIFICANCE,p_Confirm_00F1);
                p_timer->start(10*1000);
            }
            else if(p_Frame3762Base->afn_==0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到路由暂停确认");
                emScriptRunState=After_Pause;
                p_delayTimer->start(40*1000);
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case After_Pause:
        {
            if(p_Frame3762Base->afn_==0x14&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_delayTimer->stop();
                p_AbstractScriptHost->updateProgress(ProcessState_Failed, "路由暂停后又收到14F1请求");
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
void Script_ReadMeter_14F1_ChangeMeter::processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID)
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
void Script_ReadMeter_14F1_ChangeMeter::processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID)
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

void Script_ReadMeter_14F1_ChangeMeter::sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame)
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
    else if(frame==p_RouterRestart_12F1)
    {
        p_RouterRestart_12F1->ctrl_field_.dir=kDirDown;
        p_RouterRestart_12F1->ctrl_field_.prm=kActive;
        p_RouterRestart_12F1->ctrl_field_.comn_type=kHplc;

        p_RouterRestart_12F1->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_RouterRestart_12F1->info_field_.info_field_down.comu_rate=0;
        p_RouterRestart_12F1->info_field_.info_field_down.comu_module_ident=0;

        sendMsgOct=p_RouterRestart_12F1->EncodeFrame();
        sendMsgLog=QString("》》路由重启12F1：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
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
    else if(frame==p_RouterRequestRead_14F1)
    {
        p_RouterRequestRead_14F1->ctrl_field_.dir=kDirDown;
        p_RouterRequestRead_14F1->ctrl_field_.prm=kPassive;
        p_RouterRequestRead_14F1->ctrl_field_.comn_type=kHplc;

        p_RouterRequestRead_14F1->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_RouterRequestRead_14F1->info_field_.info_field_down.comu_rate=0;
        p_RouterRequestRead_14F1->info_field_.info_field_down.comu_module_ident=1;

        memcpy(p_RouterRequestRead_14F1->address_field_.dst_addr,readInfoList.at(meterID).meterNo.addr,6);
        memcpy(p_RouterRequestRead_14F1->address_field_.src_addr,p_CtrInfoList->at(0)->ccoAddr,6);

        if(readInfoList.at(meterID).protocolType==DLT645_2007)
        {
            QByteArray msg645;
            if(readInfoList.at(meterID).readFlag==Reading)
            {
                bool needRead=false;
                for(int i=0;i<readInfoList.at(meterID).dataUnitList.size();i++)
                {
                    if(readInfoList.at(meterID).dataUnitList.at(i).notRead==true)
                    {
                        shared_ptr<Rqst_ReadData_0x11> p_ReadData_0x11=make_shared<Rqst_ReadData_0x11>(addr,4,0);
                        memcpy(p_ReadData_0x11->addr_,readInfoList.at(meterID).meterNo.addr,6);
                        memcpy(p_ReadData_0x11->di,readInfoList.at(meterID).dataUnitList.at(i).dataID,4);
                        msg645=p_ReadData_0x11->EncodeFrame();
                        //添加645抄表报文前缀
                        QString tmp_msg645 = "FEFEFEFE" + QString::fromLatin1(msg645.toHex());
                        QByteArray tmp_array = QByteArray::fromHex(tmp_msg645.toLatin1());
                        msg645 = tmp_array;
                        needRead=true;
                        break;
                    }
                }
                if(needRead==false)
                {
                    readInfoList[meterID].readFlag=ReadSuccess;
                }
            }
            p_RouterRequestRead_14F1->router_request_read_unit_.read_flag_=char(readInfoList.at(meterID).readFlag);
            p_RouterRequestRead_14F1->router_request_read_unit_.delay_related_flag_=0x00;
            p_RouterRequestRead_14F1->router_request_read_unit_.subsidiary_node_num_=0x00;
            p_RouterRequestRead_14F1->router_request_read_unit_.frame_length_=uchar(msg645.size());
            p_RouterRequestRead_14F1->router_request_read_unit_.frame_content_=msg645;

            sendMsgOct.clear();
            sendMsgOct=p_RouterRequestRead_14F1->EncodeFrame();
            sendMsgLog=QString("》》路由请求抄读14F1,抄读645电表：%1\n").arg(QString(sendMsgOct.toHex()));

            startTime=QDateTime::currentDateTime().toString(CURRENT_TIME_FORMAT);
        }
        else if(readInfoList.at(meterID).protocolType==OOP)
        {
            QByteArray msgOOP;
            if(readInfoList.at(meterID).readFlag==Reading)
            {
                bool needRead=false;
                for(int i=0;i<readInfoList.at(meterID).dataUnitList.size();i++)
                {
                    if(readInfoList.at(meterID).dataUnitList.at(i).notRead==true)
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
                        p_GetRequestNormal_ReadData->address_field_.sa.address = QString((QByteArray(readInfoList.at(meterID).meterNo.addr,6).toHex()));
                        p_GetRequestNormal_ReadData->address_field_.ca.address = 0x00;

                        p_GetRequestNormal_ReadData->oad_.OI = PosActEne_OI;
                        p_GetRequestNormal_ReadData->oad_.attribute.feature = 0;
                        p_GetRequestNormal_ReadData->oad_.attribute.seq = 2;
                        p_GetRequestNormal_ReadData->oad_.element_index = 0;

                        p_GetRequestNormal_ReadData->piid_.reserve = 0;
                        p_GetRequestNormal_ReadData->piid_.serve_priority = 0;
                        p_GetRequestNormal_ReadData->piid_.serve_seq = 1;

                        p_GetRequestNormal_ReadData->time_tag_field_.optional_ = 0;

                        msgOOP=p_GetRequestNormal_ReadData->EncodeFrame();
                        needRead=true;
                        break;
                    }

                    if(needRead==false)
                    {
                        readInfoList[meterID].readFlag=ReadSuccess;
                    }
                }
                p_RouterRequestRead_14F1->router_request_read_unit_.read_flag_=char(readInfoList.at(meterID).readFlag);
                p_RouterRequestRead_14F1->router_request_read_unit_.delay_related_flag_=0x00;
                p_RouterRequestRead_14F1->router_request_read_unit_.subsidiary_node_num_=0x00;
                p_RouterRequestRead_14F1->router_request_read_unit_.frame_length_=uchar(msgOOP.size());
                p_RouterRequestRead_14F1->router_request_read_unit_.frame_content_=msgOOP;

                sendMsgOct.clear();
                sendMsgOct=p_RouterRequestRead_14F1->EncodeFrame();
                sendMsgLog=QString("》》路由请求抄读14F1,抄读OOP电表：%1\n").arg(QString(sendMsgOct.toHex()));

                startTime=QDateTime::currentDateTime().toString(CURRENT_TIME_FORMAT);
            }
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
void Script_ReadMeter_14F1_ChangeMeter::sendSrcMsg(DvcType dvcType, int dvcId, QByteArray msg)
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

void Script_ReadMeter_14F1_ChangeMeter::timer_timeoutProc()
{
    p_timer->stop();
    switch(emScriptRunState)
    {
        case Wait_BuildNetFinish_Whole:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait build whole net finish timeout!!!");
            break;
        }
        case Wait_14F1_FirstTime:
        case Wait_14F1_SecondTime:
        case Wait_14F1_Round:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "等待14F1请求超时");
            break;
        }
        default:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==p_timer");
            break;
        }
    }
}
void Script_ReadMeter_14F1_ChangeMeter::maxAllowTimer_timeoutProc()
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
void Script_ReadMeter_14F1_ChangeMeter::delayTimer_timeoutProc()
{
    p_delayTimer->stop();
    if(emScriptRunState==Wait_14F1_FirstTime)
    {
        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_RouterRestart_12F1);
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--路由重启（12F1），等待--确认");
        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
    }
    else if(emScriptRunState==After_Pause)
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Success, "路由暂停后40s内未再收到14F1请求");
        if(p_maxAllowTimer!=nullptr) p_maxAllowTimer->stop();
        emScriptRunState=ScriptSuccess;
    }
    else
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Error, "delayTimer状态机错误");
    }
}

void Script_ReadMeter_14F1_ChangeMeter::readInfoInit()
{
    readInfoList.clear();
    for(int i=0;i<p_CtrInfoList->size();i++)
    {
        QList<MeterInfoForSingleNet*> meterInfoList=p_CtrInfoList->at(i)->p_MeterInfoForSingleNetList->values();
        for(int j=0;j<meterInfoList.size();j++)
        {
            ReadInfo_14F1 readInfo_ST;
            readInfo_ST.phase=meterInfoList.at(j)->realPhase;
            memcpy(readInfo_ST.meterNo.addr,meterInfoList.at(j)->mtrAddr,6);
            readInfo_ST.protocolType=meterInfoList.at(j)->prtcl;
            readInfo_ST.readFlag=Reading;
            if(readInfo_ST.protocolType==0x02)
            {
                ReadDataUnit readData;
                char id[4]={0x00,0x00,0x01,0x00};
                readData.dataID.append(QByteArray(id,4));
                readData.notRead=true;
                readData.costTime=0.0;
                readInfo_ST.dataUnitList.append(readData);
            }
            else if(readInfo_ST.protocolType==0x03)
            {
                ReadDataUnit readData;
                char id[4]={0x00,0x10,0x02,0x00};
                readData.dataID.append(QByteArray(id,4));
                readData.notRead=true;
                readData.costTime=0.0;
                readInfo_ST.dataUnitList.append(readData);
            }
            readInfoList.append(readInfo_ST);
        }
    }
}
bool Script_ReadMeter_14F1_ChangeMeter::isMeterExist(Address address)
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
int Script_ReadMeter_14F1_ChangeMeter::getReadInfo(Address address)
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
double Script_ReadMeter_14F1_ChangeMeter::calSuccessRate()
{
    double successCount=0.0;
    for(int i=0;i<readInfoList.size();i++)
    {
        if(readInfoList.at(i).readFlag==ReadSuccess)
            successCount++;
    }
    return successCount/double(readInfoList.size());
}

QString Script_ReadMeter_14F1_ChangeMeter::calCostTime()
{
    double totalConsume=0.0;
    double successCount=0.0;
    for(int i=0;i<readInfoList.size();i++)
    {
        if(readInfoList.at(i).readFlag==ReadSuccess)
        {
            successCount++;
            for(int j=0;j<readInfoList.at(i).dataUnitList.size();j++)
            {
                totalConsume+=readInfoList.at(i).dataUnitList.at(j).costTime;
            }
        }
    }
    return QString::number(totalConsume/successCount,'g',3);
}
QString Script_ReadMeter_14F1_ChangeMeter::getFailMeterNo()
{
    QString failMeterNo;
    int count=0;
    for(int i=0;i<readInfoList.size();i++)
    {
        if(readInfoList.at(i).readFlag!=ReadSuccess)
        {
            count++;
            failMeterNo+=QString(QByteArray(readInfoList.at(i).meterNo.addr,6).toHex())+";";
            if(count%8==0)
                failMeterNo+="\n";
        }

    }
    return failMeterNo;
}
void Script_ReadMeter_14F1_ChangeMeter::CalcAvrgConsumeTimeLen(uchar rdFlag)
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

Address Script_ReadMeter_14F1_ChangeMeter::extractAddressFromAfn06F2(shared_ptr<Afn06F2> p_ReportReadData_Up)
{
    Address srcAddr;
    memset(srcAddr.addr, 0, sizeof(srcAddr.addr));

    if (!p_ReportReadData_Up) {
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "错误: Afn06F2对象为空");
        return srcAddr;
    }
    uchar protocolType = (uchar)p_ReportReadData_Up->report_data_unit_.node_protocol_type_;

    p_AbstractScriptHost->updateProgress(ProcessState_Processing,
        QString("从节点序号: %1").arg(p_ReportReadData_Up->report_data_unit_.node_no_));
    p_AbstractScriptHost->updateProgress(ProcessState_Processing,
        QString("通信协议类型: %1").arg((uchar)p_ReportReadData_Up->report_data_unit_.node_protocol_type_));
    p_AbstractScriptHost->updateProgress(ProcessState_Processing,
        QString("报文长度: %1").arg(p_ReportReadData_Up->report_data_unit_.frame_length_));
    p_AbstractScriptHost->updateProgress(ProcessState_Processing,
        QString("报文内容: %1").arg(QString(p_ReportReadData_Up->report_data_unit_.frame_content_.toHex())));


    if (protocolType == 0x01 || protocolType == 0x02) {
        if (p_ReportReadData_Up->report_data_unit_.frame_content_.size() >= 7) {
            QByteArray frameContent = p_ReportReadData_Up->report_data_unit_.frame_content_;

            if ((unsigned char)frameContent[0] == 0x68) {
                QByteArray address645 = frameContent.mid(1, 6);

                QByteArray reversedAddress;
                for (int i = 5; i >= 0; i--) {
                    reversedAddress.append(address645[i]);
                }

                memcpy(srcAddr.addr, reversedAddress.constData(), 6);

                p_AbstractScriptHost->updateProgress(ProcessState_Processing,
                                                     QString("从645帧提取的地址(反转后): %1").arg(QString(reversedAddress.toHex())));
            } else {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing,
                                                     "645帧起始标志不是0x68，无法提取地址");
            }
        } else {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing,
                                                 QString("645帧内容长度不足，实际长度: %1").arg(p_ReportReadData_Up->report_data_unit_.frame_content_.size()));
        }
    } else if (protocolType == 0x03) {
        if (p_ReportReadData_Up->report_data_unit_.frame_content_.size() >= 12) {
            QByteArray frameContent = p_ReportReadData_Up->report_data_unit_.frame_content_;

            if ((unsigned char)frameContent[0] == 0x68) {
                // 提取698帧中的服务器地址SA（从第4字节开始的7字节）
                QByteArray address698 = frameContent.mid(4, 7);

                p_AbstractScriptHost->updateProgress(ProcessState_Processing,
                                                     QString("从698帧提取的服务器地址SA: %1").arg(QString(address698.toHex())));

                QByteArray extractedAddress = address698.mid(1, 6);

                QByteArray reversedAddress;
                for (int i = extractedAddress.size() - 1; i >= 0; i--) {
                    reversedAddress.append(extractedAddress[i]);
                }

                memcpy(srcAddr.addr, reversedAddress.constData(), 6);

                p_AbstractScriptHost->updateProgress(ProcessState_Processing,
                                                     QString("提取的698地址(反转后): %1").arg(QString(reversedAddress.toHex())));
            } else {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing,
                                                     "698帧起始标志不是0x68，无法提取地址");
            }
        } else {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing,
                                                 QString("698帧内容长度不足，实际长度: %1").arg(p_ReportReadData_Up->report_data_unit_.frame_content_.size()));
        }
    } else {
        p_AbstractScriptHost->updateProgress(ProcessState_Processing,
                                             QString("未知通信协议类型: %1，无法提取地址").arg(protocolType));
    }
    return srcAddr;
}

bool Script_ReadMeter_14F1_ChangeMeter::extractAndProcess3762Frame(QByteArray& buf,QByteArray& completeFrame)
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
