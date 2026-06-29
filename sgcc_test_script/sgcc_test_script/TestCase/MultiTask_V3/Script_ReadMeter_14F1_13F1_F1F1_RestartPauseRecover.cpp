#include "Script_ReadMeter_14F1_13F1_F1F1_RestartPauseRecover.h"

Script_ReadMeter_14F1_13F1_F1F1_RestartPauseRecover::Script_ReadMeter_14F1_13F1_F1F1_RestartPauseRecover(QObject *parent) : QObject(parent)
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
    p_MonitorSlaveNode_13F1=make_shared<Afn13F1>();
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
Script_ReadMeter_14F1_13F1_F1F1_RestartPauseRecover::~Script_ReadMeter_14F1_13F1_F1F1_RestartPauseRecover()
{
    // 先停止并清理 BuildNetwork_GW
    if(p_BuildNetwork_GW)
    {
        p_BuildNetwork_GW->initBuildNetWork();
        if(needPowerOff==true)
            powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost);
        delete p_BuildNetwork_GW;
        p_BuildNetwork_GW=nullptr;
    }

    // 清理所有定时器 - 不要有早期返回
    if(p_timer)
    {
        p_timer->stop();
        delete p_timer;
        p_timer=nullptr;
    }

    if(p_maxAllowTimer)
    {
        p_maxAllowTimer->stop();
        delete p_maxAllowTimer;
        p_maxAllowTimer=nullptr;
    }

    if(p_delayTimer)
    {
        p_delayTimer->stop();
        delete p_delayTimer;
        p_delayTimer=nullptr;
    }
}

void Script_ReadMeter_14F1_13F1_F1F1_RestartPauseRecover::execute()
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
void Script_ReadMeter_14F1_13F1_F1F1_RestartPauseRecover::stop()
{
    if(p_AbstractScriptHost==nullptr)
        return;
    p_AbstractScriptHost->updateProgress(ProcessState_Error, "Test stop!");
    if(needPowerOff==true)
        powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost);
}
void Script_ReadMeter_14F1_13F1_F1F1_RestartPauseRecover::addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq)
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
void Script_ReadMeter_14F1_13F1_F1F1_RestartPauseRecover::setHost(AbstractScriptHost *host)
{
    p_AbstractScriptHost=host;
    p_BuildNetwork_GW->setHost(host);
}
bool Script_ReadMeter_14F1_13F1_F1F1_RestartPauseRecover::config(const QMap<QString,QString> *paraDic)
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
void Script_ReadMeter_14F1_13F1_F1F1_RestartPauseRecover::processMsg(DvcType dvcType, int id, uchar *data, int datalen)
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
                emScriptRunState=Wait_ReadMeter_Restart;
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_RouterRestart_12F1);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--路由重启（12F1），等待--确认");
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
void Script_ReadMeter_14F1_13F1_F1F1_RestartPauseRecover::processCtrlDvcRes(DvcType dvcType, QList<int> idList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params)
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
void Script_ReadMeter_14F1_13F1_F1F1_RestartPauseRecover::processMsgFromCCO(DvcType dvcType, int dvcId)
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
            case Wait_ReadMeter_Restart:
            {
                if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到确认，等待14F1请求");
                    No_14F1=0;
                    p_timer->start(10*1000);
                }
                else if(p_Frame3762Base->afn_==0x14&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    shared_ptr<Afn14F1> p_RouterRequestRead_14F1_Up=dynamic_pointer_cast<Afn14F1>(p_Frame3762Base);
                    currentMeterIndex=getReadInfo(p_RouterRequestRead_14F1_Up->node_address_);
                    if(++No_14F1==1)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("路由重启后收到第%0个14F1请求，回复抄读数据项，同时下发13F1、F1F1，表号").arg(No_14F1)+QByteArray(p_RouterRequestRead_14F1_Up->node_address_.addr,6).toHex());
                        sendMsg(dvcType,dvcId,currentMeterIndex,p_RouterRequestRead_14F1);
                        sendMsg(dvcType,dvcId,currentMeterIndex,p_MonitorSlaveNode_13F1);
                        sendMsg(dvcType,dvcId,currentMeterIndex,p_ParallelReadMeter_F1F1);
                        p_delayTimer->start(30*1000);
                        flag_13F1=false;
                        flag_14F1=false;
                        flag_F1F1=false;
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("路由重启后收到第%0个14F1请求，表号").arg(No_14F1)+QByteArray(p_RouterRequestRead_14F1_Up->node_address_.addr,6).toHex());
                    }
                }
                else if(p_Frame3762Base->afn_==0x06&&p_Frame3762Base->dt1_==0x02&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    shared_ptr<Afn06F2> p_ReportReadData_Up=dynamic_pointer_cast<Afn06F2>(p_Frame3762Base);
                    Address srcAddr;
                    memcpy(srcAddr.addr,p_ReportReadData_Up->address_field_.src_addr,6);
                    if(p_ReportReadData_Up->report_data_unit_.frame_content_.length()>16)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到06F2上报抄读数据，表号"+QByteArray(srcAddr.addr,6).toHex());
                        flag_14F1=true;
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "06F2上报抄读数据内容为空："+p_ReportReadData_Up->report_data_unit_.frame_content_.toHex());
                    }
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "回复确认");
                    sendMsg(dvcType,dvcId,INSIGNIFICANCE,p_Confirm_00F1);

                    if(flag_13F1&&flag_14F1&&flag_F1F1)
                    {
                        p_delayTimer->stop();
                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_RouterPause_12F2);
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "13F1、14F1、F1F1全部抄回，发送--路由暂停（12F2），等待--确认");
                        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                        emScriptRunState=Wait_ReadMeter_Pause;
                    }
                }
                else if(p_Frame3762Base->afn_==0x13&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    shared_ptr<Afn13F1> p_MonitorSlaveNode_13F1_Up=dynamic_pointer_cast<Afn13F1>(p_Frame3762Base);
                    if(p_MonitorSlaveNode_13F1_Up->data_field_up_.frame_content_.size()>16)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到13F1抄读回复数据，表号"+QByteArray(p_MonitorSlaveNode_13F1_Up->address_field_.src_addr,6).toHex());
                        flag_13F1=true;
                        if(flag_13F1&&flag_14F1&&flag_F1F1)
                        {
                            p_delayTimer->stop();
                            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_RouterPause_12F2);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "13F1、14F1、F1F1全部抄回，发送--路由暂停（12F2），等待--确认");
                            p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                            emScriptRunState=Wait_ReadMeter_Pause;
                        }
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "13F1抄读回复数据内容为空："+p_MonitorSlaveNode_13F1_Up->data_field_up_.frame_content_.toHex());
                    }
                }
                else if(p_Frame3762Base->afn_==char(0xF1)&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    shared_ptr<AfnF1F1> p_ParallelReadMeter_F1F1_Up=dynamic_pointer_cast<AfnF1F1>(p_Frame3762Base);
                    Address srcAddr;
                    memcpy(srcAddr.addr,p_ParallelReadMeter_F1F1_Up->address_field_.src_addr,6);
                    if(p_ParallelReadMeter_F1F1_Up->unit_up_.frame_length_>16)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到F1F1抄读回复数据，表号"+QByteArray(srcAddr.addr,6).toHex());
                        flag_F1F1=true;
                        if(flag_13F1&&flag_14F1&&flag_F1F1)
                        {
                            p_delayTimer->stop();
                            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_RouterPause_12F2);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "13F1、14F1、F1F1全部抄回，发送--路由暂停（12F2），等待--确认");
                            p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                            emScriptRunState=Wait_ReadMeter_Pause;
                        }
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "F1F1抄读回复数据内容为空："+p_ParallelReadMeter_F1F1_Up->unit_up_.frame_content_.toHex());
                    }
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_ReadMeter_Pause:
            {
            if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到确认，下发13F1、F1F1");
                sendMsg(dvcType,dvcId,currentMeterIndex,p_MonitorSlaveNode_13F1);
                sendMsg(dvcType,dvcId,currentMeterIndex,p_ParallelReadMeter_F1F1);
                flag_13F1=false;
                flag_F1F1=false;
                flag_pause_ack=true;//当收到确认之后才不能有14F1请求
                p_delayTimer->start(30*1000);
            }
            else if(flag_pause_ack==true&&p_Frame3762Base->afn_==0x14&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Failed, "路由暂停后又收到14F1请求");
            }
            else if(p_Frame3762Base->afn_==0x13&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                shared_ptr<Afn13F1> p_MonitorSlaveNode_13F1_Up=dynamic_pointer_cast<Afn13F1>(p_Frame3762Base);
                if(p_MonitorSlaveNode_13F1_Up->data_field_up_.frame_content_.size()>16)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到13F1抄读回复数据，表号"+QByteArray(p_MonitorSlaveNode_13F1_Up->address_field_.src_addr,6).toHex());
                    flag_13F1=true;
                    if(flag_13F1&flag_F1F1)
                    {
                        p_delayTimer->stop();
                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_RouterRecover_12F3);
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "13F1、F1F1全部抄回，发送--路由恢复（12F3），等待--确认");
                        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                        emScriptRunState=Wait_ReadMeter_Recover;
                    }
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, "13F1抄读回复数据内容为空："+p_MonitorSlaveNode_13F1_Up->data_field_up_.frame_content_.toHex());
                }
            }
            else if(p_Frame3762Base->afn_==char(0xF1)&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                shared_ptr<AfnF1F1> p_ParallelReadMeter_F1F1_Up=dynamic_pointer_cast<AfnF1F1>(p_Frame3762Base);
                Address srcAddr;
                memcpy(srcAddr.addr,p_ParallelReadMeter_F1F1_Up->address_field_.src_addr,6);
                if(p_ParallelReadMeter_F1F1_Up->unit_up_.frame_length_>16)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到F1F1抄读回复数据，表号"+QByteArray(srcAddr.addr,6).toHex());
                    flag_F1F1=true;
                    if(flag_13F1&flag_F1F1)
                    {
                        p_delayTimer->stop();
                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_RouterRecover_12F3);
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "13F1、F1F1全部抄回，发送--路由恢复（12F3），等待--确认");
                        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                        emScriptRunState=Wait_ReadMeter_Recover;
                    }
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, "F1F1抄读回复数据内容为空："+p_ParallelReadMeter_F1F1_Up->unit_up_.frame_content_.toHex());
                }
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
            }
            case Wait_ReadMeter_Recover:
            {
            if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到确认，等待14F1请求");
                No_14F1=0;
                p_timer->start(10*1000);
            }
            else if(p_Frame3762Base->afn_==0x14&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                shared_ptr<Afn14F1> p_RouterRequestRead_14F1_Up=dynamic_pointer_cast<Afn14F1>(p_Frame3762Base);
                currentMeterIndex=getReadInfo(p_RouterRequestRead_14F1_Up->node_address_);
                if(++No_14F1==1)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("路由恢复后收到第%0个14F1请求，回复抄读数据项，同时下发13F1、F1F1，表号").arg(No_14F1)+QByteArray(p_RouterRequestRead_14F1_Up->node_address_.addr,6).toHex());
                    sendMsg(dvcType,dvcId,currentMeterIndex,p_RouterRequestRead_14F1);
                    sendMsg(dvcType,dvcId,currentMeterIndex,p_MonitorSlaveNode_13F1);
                    sendMsg(dvcType,dvcId,currentMeterIndex,p_ParallelReadMeter_F1F1);
                    p_delayTimer->start(30*1000);
                    flag_13F1=false;
                    flag_14F1=false;
                    flag_F1F1=false;
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("路由恢复后收到第%0个14F1请求，表号").arg(No_14F1)+QByteArray(p_RouterRequestRead_14F1_Up->node_address_.addr,6).toHex());
                }
            }
            else if(p_Frame3762Base->afn_==0x06&&p_Frame3762Base->dt1_==0x02&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                shared_ptr<Afn06F2> p_ReportReadData_Up=dynamic_pointer_cast<Afn06F2>(p_Frame3762Base);
                Address srcAddr;
                memcpy(srcAddr.addr,p_ReportReadData_Up->address_field_.src_addr,6);
                if(p_ReportReadData_Up->report_data_unit_.frame_content_.length()>16)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到06F2上报抄读数据，表号"+QByteArray(srcAddr.addr,6).toHex());
                    flag_14F1=true;
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, "06F2上报抄读数据内容为空："+p_ReportReadData_Up->report_data_unit_.frame_content_.toHex());
                }
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "回复确认");
                sendMsg(dvcType,dvcId,INSIGNIFICANCE,p_Confirm_00F1);

                if(flag_13F1&&flag_14F1&&flag_F1F1)
                {
                    p_delayTimer->stop();
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_RouterPause_12F2);
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "13F1、14F1、F1F1全部抄回，发送--路由暂停（12F2），等待--确认");
                    p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                    emScriptRunState=RouterPause;
                }
            }
            else if(p_Frame3762Base->afn_==0x13&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                shared_ptr<Afn13F1> p_MonitorSlaveNode_13F1_Up=dynamic_pointer_cast<Afn13F1>(p_Frame3762Base);
                if(p_MonitorSlaveNode_13F1_Up->data_field_up_.frame_content_.size()>16)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到13F1抄读回复数据，表号"+QByteArray(p_MonitorSlaveNode_13F1_Up->address_field_.src_addr,6).toHex());
                    flag_13F1=true;
                    if(flag_13F1&&flag_14F1&&flag_F1F1)
                    {
                        p_delayTimer->stop();
                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_RouterPause_12F2);
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "13F1、14F1、F1F1全部抄回，发送--路由暂停（12F2），等待--确认");
                        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                        emScriptRunState=RouterPause;
                    }
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, "13F1抄读回复数据内容为空："+p_MonitorSlaveNode_13F1_Up->data_field_up_.frame_content_.toHex());
                }
            }
            else if(p_Frame3762Base->afn_==char(0xF1)&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                shared_ptr<AfnF1F1> p_ParallelReadMeter_F1F1_Up=dynamic_pointer_cast<AfnF1F1>(p_Frame3762Base);
                Address srcAddr;
                memcpy(srcAddr.addr,p_ParallelReadMeter_F1F1_Up->address_field_.src_addr,6);
                if(p_ParallelReadMeter_F1F1_Up->unit_up_.frame_length_>16)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到F1F1抄读回复数据，表号"+QByteArray(srcAddr.addr,6).toHex());
                    flag_F1F1=true;
                    if(flag_13F1&&flag_14F1&&flag_F1F1)
                    {
                        p_delayTimer->stop();
                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_RouterPause_12F2);
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "13F1、14F1、F1F1全部抄回，发送--路由暂停（12F2），等待--确认");
                        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                        emScriptRunState=RouterPause;
                    }
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, "F1F1抄读回复数据内容为空："+p_ParallelReadMeter_F1F1_Up->unit_up_.frame_content_.toHex());
                }
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case RouterPause:
        {
            if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                p_AbstractScriptHost->updateProgress(ProcessState_Success, "收到确认，3种抄读方式并行测试成功");
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
//            default:
//            {
//                p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==emScriptRunState");
//                break;
//            }
        }
    }
}
void Script_ReadMeter_14F1_13F1_F1F1_RestartPauseRecover::processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID)
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
void Script_ReadMeter_14F1_13F1_F1F1_RestartPauseRecover::processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID)
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

void Script_ReadMeter_14F1_13F1_F1F1_RestartPauseRecover::sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame)
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

        p_RouterRequestRead_14F1->info_field_.info_field_down.msg_seq=char(msgSeq);
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
    }
    else if(frame==p_MonitorSlaveNode_13F1)
    {
        uchar tmpAddr[6];
        memcpy(tmpAddr,readInfoList.at(meterID).meterNo.addr,6);
        uchar comPrtclType=readInfoList.at(meterID).protocolType;

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
            shared_ptr<Rqst_ReadData_0x11> p_ReadData_0x11=make_shared<Rqst_ReadData_0x11>(addr,4,0);
            uchar CrntPosEneTotal[4]={0x00,0x00,0x01,0x00};
            memcpy(p_ReadData_0x11->addr_,tmpAddr,6);
            memcpy(p_ReadData_0x11->di,CrntPosEneTotal,4);
            QByteArray msg645=p_ReadData_0x11->EncodeFrame();

            p_ParallelReadMeter_F1F1->unit_down_.protocol_type_=0x02;
            p_ParallelReadMeter_F1F1->unit_down_.subsidiary_node_num_=0x00;
            p_ParallelReadMeter_F1F1->unit_down_.frame_content_=msg645;
            p_ParallelReadMeter_F1F1->unit_down_.frame_length_=ushort(msg645.size());

            sendMsgOct=p_ParallelReadMeter_F1F1->EncodeFrame();
            sendMsgLog=QString("》》并发抄表F1F1,抄读645电表%0：%1\n").arg(QString((QByteArray(reinterpret_cast<char*>(tmpAddr),6).toHex()))).arg(QString(sendMsgOct.toHex()));

            startTime=QDateTime::currentDateTime().toString(CURRENT_TIME_FORMAT);
        }
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
void Script_ReadMeter_14F1_13F1_F1F1_RestartPauseRecover::sendSrcMsg(DvcType dvcType, int dvcId, QByteArray msg)
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

void Script_ReadMeter_14F1_13F1_F1F1_RestartPauseRecover::timer_timeoutProc()
{
    p_timer->stop();
    switch(emScriptRunState)
    {
        case Wait_BuildNetFinish_Whole:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait build whole net finish timeout!!!");
            break;
        }
        case Wait_ReadMeter_Restart:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "路由重启后10s内未收到14F1请求");
            break;
        }
        case Wait_ReadMeter_Recover:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "路由恢复后10s内未收到14F1请求");
            break;
        }
        default:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==p_timer");
            break;
        }
    }
}
void Script_ReadMeter_14F1_13F1_F1F1_RestartPauseRecover::maxAllowTimer_timeoutProc()
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
void Script_ReadMeter_14F1_13F1_F1F1_RestartPauseRecover::delayTimer_timeoutProc()
{
    p_delayTimer->stop();
    if(emScriptRunState==Wait_ReadMeter_Restart)
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "30s内未收全13F1、14F1、F1F1抄读回复数据");
    }
    else if(emScriptRunState==Wait_ReadMeter_Pause)
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "30s内未收全13F1、F1F1抄读回复数据");
    }
    else if(emScriptRunState==Wait_ReadMeter_Recover)
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "30s内未收全13F1、14F1、F1F1抄读回复数据");
    }
    else
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Error, "delayTimer状态机错误");
    }
}

void Script_ReadMeter_14F1_13F1_F1F1_RestartPauseRecover::readInfoInit()
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
bool Script_ReadMeter_14F1_13F1_F1F1_RestartPauseRecover::isMeterExist(Address address)
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
int Script_ReadMeter_14F1_13F1_F1F1_RestartPauseRecover::getReadInfo(Address address)
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
double Script_ReadMeter_14F1_13F1_F1F1_RestartPauseRecover::calSuccessRate()
{
    double successCount=0.0;
    for(int i=0;i<readInfoList.size();i++)
    {
        if(readInfoList.at(i).readFlag==ReadSuccess)
            successCount++;
    }
    return successCount/double(readInfoList.size());
}

QString Script_ReadMeter_14F1_13F1_F1F1_RestartPauseRecover::calCostTime()
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
QString Script_ReadMeter_14F1_13F1_F1F1_RestartPauseRecover::getFailMeterNo()
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
void Script_ReadMeter_14F1_13F1_F1F1_RestartPauseRecover::CalcAvrgConsumeTimeLen(uchar rdFlag)
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

