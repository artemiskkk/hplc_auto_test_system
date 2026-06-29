#include "Script_ProgramComparison_STA_Hunan.h"

Script_ProgramComparison_STA_Hunan::Script_ProgramComparison_STA_Hunan(QObject *parent) : QObject(parent)
{
    emScriptRunState=TestInit;
    resultFlag=false;
    sendMsgOct.clear();

    p_CtrInfoList=make_shared<QList<shared_ptr<CtrInfo>>>();
    p_BuildNetwork_GW=new BuildNetwork_GW();

    p_Frame3762Helper=make_shared<Frame3762Helper>();

    p_Frame645Helper=make_shared<Frame645Helper>();
    p_MeterAddrResp_93=make_shared<RspsNormal_ReadAddr_0x93>(addr,6);
    p_Rqst_ReadData_0x11=make_shared<Rqst_ReadData_0x11>(addr,4,0);
//    p_RouterRecover_12F3=make_shared<Afn12F3>();
    p_MonitorSlaveNode_13F1=make_shared<Afn13F1>();
    ForwardCommProtocolDataFrame_02F1=make_shared<Afn02F1>();
    p_AddSlaveNode_11F1=make_shared<Afn11F1>();
    p_QueryNetScale_10F9=make_shared<Afn10F9>();

    p_ParameterInit_01F2=make_shared<Afn01F2>();
    p_HardReset_01F1=make_shared<Afn01F1>();

    p_FrameOOPHelper=make_shared<FrameOOPHelper>();
    p_GetResponseNormal_ReadAddr=make_shared<GetResponseNormal>();

    p_timer=new QTimer();
    p_maxAllowTimer=new QTimer();
    p_delayTimer=new QTimer();
    connect(p_timer,SIGNAL(timeout()),this,SLOT(timer_timeoutProc()));
    connect(p_maxAllowTimer,SIGNAL(timeout()),this,SLOT(maxAllowTimer_timeoutProc()));
    connect(p_delayTimer,SIGNAL(timeout()),this,SLOT(delayTimer_timeoutProc()));
}
Script_ProgramComparison_STA_Hunan::~Script_ProgramComparison_STA_Hunan()
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
void Script_ProgramComparison_STA_Hunan::execute()
{
    p_AbstractScriptHost->updateProgress(ProcessState_Start, "Test start!");

    emScriptRunState=TestInit;
    resultFlag=false;
    addrList.clear();
    softwareInfoInit();

    //p_BuildNetwork_GW->flagStaHighComBaud=true;

//    needBuildNet=false;
    if(needBuildNet==true)
    {
        p_BuildNetwork_GW->execute();
        emScriptRunState=Wait_BuildNetFinish_Whole;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "等待全网组网完成");
        p_timer->start((timerForReachThresld+timerAfterReachThresld)*1000);
    }
    else
    {
//        powerOn12V_STA(p_CtrInfoList,p_AbstractScriptHost);

        emScriptRunState=Wait_QuerySoftWareInfo_645_Finish;
        sendMsg(SingleSTA,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_Rqst_ReadData_0x11);
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询STA软件信息（645），等待--回复");
        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
    }
    p_maxAllowTimer->start((timerForReachThresld+timerAfterReachThresld)*1000);

}
void Script_ProgramComparison_STA_Hunan::stop()
{
    if(p_AbstractScriptHost==nullptr)
        return;
    p_AbstractScriptHost->updateProgress(ProcessState_Error, "Test stop!");
    if(needPowerOff==true)
        powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost);
}
void Script_ProgramComparison_STA_Hunan::addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq)
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
void Script_ProgramComparison_STA_Hunan::setHost(AbstractScriptHost *host)
{
    p_AbstractScriptHost=host;
    p_BuildNetwork_GW->setHost(host);
}
bool Script_ProgramComparison_STA_Hunan::config(const QMap<QString,QString> *paraDic)
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
void Script_ProgramComparison_STA_Hunan::processMsg(DvcType dvcType, int id, uchar *data, int datalen)
{
    if(p_CtrInfoList->size()==0)
        return;
    if(emScriptRunState==Wait_BuildNetFinish_Whole)
    {
//        if(!p_BuildNetwork_GW->startBuildNetFlag)
        if(!p_BuildNetwork_GW->buildNetworkResultFlag)
        {
            p_BuildNetwork_GW->processMsg(dvcType,id,data,datalen);
        }
        else
        {
            if(p_BuildNetwork_GW->emScriptRunState==BuildNetFinish&&p_BuildNetwork_GW->buildNetworkResultFlag==true)
            {
//                p_BuildNetwork_GW->initBuildNetWork();
                emScriptRunState=Wait_QuerySoftWareInfo_645_Finish;
                sendMsg(SingleSTA,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_Rqst_ReadData_0x11);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询STA软件信息（645），等待--回复");
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
void Script_ProgramComparison_STA_Hunan::processCtrlDvcRes(DvcType dvcType, QList<int> idList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params)
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
        case ScriptSuccess:
        {
            break;
        }
        default:
            break;
    }
}

void Script_ProgramComparison_STA_Hunan::processMsgFromCCO(DvcType dvcType, int dvcId)
{
    Q_UNUSED(dvcType)
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
        case Wait_QuerySoftWareInfo_645_Finish:
        {
            break;
        }
        case Wait_QuerySoftWareCompare_645_Finish:
        {
            break;
        }
        case Wait_QuerySoftWareCompare_645_2_Finish:
        {
            break;
        }
        case Wait_QuerySoftWareCompare_645_3_Finish:
        {
            break;
        }

        case Wait_ParameterInit_Finish:
        {
            if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();//接收到一条完整报文
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "参数初始化，回复确认");
                emScriptRunState=Wait_HardReset_Finish;
                sendMsg(CCO_GW,dvcId,INSIGNIFICANCE,p_HardReset_01F1);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--硬件复位（01F1），等待--确认");
                p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case Wait_HardReset_Finish:
        {
            if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                break;
            }
            else if(p_Frame3762Base->afn_ == 0x03&&p_Frame3762Base->dt1_==0x02&&p_Frame3762Base->dt2_==0x01&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                index=0;

                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_AddSlaveNode_11F1);
                emScriptRunState=Wait_AddSlaveNode_11F1_Finish;
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--添加从节点（11F1），等待--确认");
                p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);

            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case Wait_AddSlaveNode_11F1_Finish:
        {
            if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();//接收到一条完整报文
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("添加从节点收到确认！"));
                index+=num;
                if(index<p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size())
                {
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_AddSlaveNode_11F1);
                    emScriptRunState=Wait_AddSlaveNode_11F1_Finish;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--添加从节点（11F1），等待--确认");
                    p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                }
                else
                {
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryNetScale_10F9);
                    emScriptRunState=Wait_QueryNetScale_10F9_Finish;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询网络规模（10F9），等待--回复");
                    p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                }
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case Wait_QueryNetScale_10F9_Finish:
        {
            if(p_Frame3762Base->afn_ == 0x10&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x01&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();//接收到一条完整报文
                shared_ptr<Afn10F9> p_QueryNetScale_10F9_Up=dynamic_pointer_cast<Afn10F9>(p_Frame3762Base);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("当前网络规模为%1").arg(p_QueryNetScale_10F9_Up->network_scale_));
                if(p_QueryNetScale_10F9_Up->network_scale_<p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size()+1)
                {
                    p_delayTimer->start(STA_CMD_TIMEOUT_TIME*1000);
//                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryNetScale_10F9);
//                    emScriptRunState=Wait_QueryNetScale_10F9_Finish;
//                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询从节点数量（10F9），等待--回复");
//                    p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                }
                else
                {
                    softwareInfoInit(65536,2048);
                    times=ushort(programCompareList.size());
                    index=0;

                    emScriptRunState=Wait_QuerySoftWareInfo_13F1_Finish;
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_MonitorSlaveNode_13F1);
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询STA软件信息（13F1），等待--回复11");
                    p_timer->start(maxMonitorTime*1000);

//                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("当前网络规模为%1，不符合要求").arg(p_QueryNetScale_10F9_Up->network_scale_));
                }
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case Wait_QuerySoftWareInfo_13F1_Finish:
        {
            if(p_Frame3762Base->afn_ == 0x13&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                shared_ptr<Afn13F1> p_MonitorSlaveNode_13F1_Up=dynamic_pointer_cast<Afn13F1>(p_Frame3762Base);
                if(p_MonitorSlaveNode_13F1_Up->data_field_up_.frame_content_.size()==0)
                    continue;
                p_timer->stop();
                QByteArray msgBuf=p_MonitorSlaveNode_13F1_Up->data_field_up_.frame_content_;
                if(p_MonitorSlaveNode_13F1_Up->data_field_up_.protocol_type_==0x02)
                {
                    bool res=true;
                    shared_ptr<Frame645Base> MsgBase_645_ptr = Frame645Helper::DecodeLocalMsg(&msgBuf,res);
                    if(MsgBase_645_ptr==nullptr)
                        continue;
                    if(MsgBase_645_ptr->ctrlCode_!=NORMAL_RESP)
                        continue;
                    shared_ptr<RspsNormal_ReadData_0x91> p_RspsNormal_ReadData_0x91=dynamic_pointer_cast<RspsNormal_ReadData_0x91>(MsgBase_645_ptr);
                    if(memcmp(SoftwareInfo,p_RspsNormal_ReadData_0x91->di,4)==0)
                    {
                        if(p_RspsNormal_ReadData_0x91->data==softwareInfoArray)
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("回复信息与初始信息一致"));
                            //开始比较程序段
                            times=ushort(programCompareList.size());
                            index=0;
                            emScriptRunState=Wait_QuerySoftWareCompare_13F1_Finish;
                            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_MonitorSlaveNode_13F1);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--STA程序比对（13F1），等待--回复");
                            p_timer->start(maxMonitorTime*1000);
                        }
                        else
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("回复信息与初始信息不一致"));
                        }
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("回复信息DI不一致"));
                    }
//                        continue;
                }
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }

        case Wait_QuerySoftWareCompare_13F1_Finish:
        {
            if(p_Frame3762Base->afn_ == 0x13&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                shared_ptr<Afn13F1> p_MonitorSlaveNode_13F1_Up=dynamic_pointer_cast<Afn13F1>(p_Frame3762Base);
                if(p_MonitorSlaveNode_13F1_Up->data_field_up_.frame_content_.size()==0)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("数据内容为空！！！！"));
                    continue;
                }
                p_timer->stop();
                QByteArray msgBuf=p_MonitorSlaveNode_13F1_Up->data_field_up_.frame_content_;

                if(p_MonitorSlaveNode_13F1_Up->data_field_up_.protocol_type_==0x02)
                {
                    tryTimes=0;
                    bool res=true;
                    shared_ptr<Frame645Base> MsgBase_645_ptr = Frame645Helper::DecodeLocalMsg(&msgBuf,res);
                    if(MsgBase_645_ptr==nullptr)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("数据内容为空2！！！！"));
                        continue;
                    }
                    if(MsgBase_645_ptr->ctrlCode_!=NORMAL_RESP)
                        continue;
                    shared_ptr<dlt_645_Protocol::RspsNormal_ReadData_0x91> p_RspsNormal_ReadData_0x91 = std::dynamic_pointer_cast<dlt_645_Protocol::RspsNormal_ReadData_0x91>(MsgBase_645_ptr);
                    if(memcmp(ProgramCompare,p_RspsNormal_ReadData_0x91->di,4)==0)
                    {
                        if(p_RspsNormal_ReadData_0x91->data==programCompareList.at(index).compareData)//判断条件
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("第%1段程序比较一致").arg(++index));
                            //开始比对下一段
                            if(index<times)
                            {
                                p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("第%1段").arg(index));
                                p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("共%1段").arg(times));
                                emScriptRunState=Wait_QuerySoftWareCompare_13F1_Finish;
                                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_MonitorSlaveNode_13F1);
                                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--STA程序比对（13F1），等待--回复");
                                p_timer->start(maxMonitorTime*1000);
                            }
                            else
                            {
                                softwareInfoInit(65536,2048);
                                times=ushort(programCompareList.size());
                                index=0;
                                emScriptRunState=Wait_QuerySoftWareCompare_02F1_Finish;
                                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,ForwardCommProtocolDataFrame_02F1);
                                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--程序比对（02F1），等待--回复");
                                p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                            }
                        }
                        else
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("第%1段程序比较不一致").arg(index+1));
                        }
                    }
                    else
                        continue;
                }
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case Wait_QuerySoftWareCompare_02F1_Finish:
        {
            if(p_Frame3762Base->afn_ == 0x02&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                shared_ptr<Afn02F1> p_ForwardCommProtocolDataFrame_02F1_Up=dynamic_pointer_cast<Afn02F1>(p_Frame3762Base);
                if(p_ForwardCommProtocolDataFrame_02F1_Up->frame_content_.size()==0)
                    continue;
                p_timer->stop();
                QByteArray msgBuf=p_ForwardCommProtocolDataFrame_02F1_Up->frame_content_;
                if(p_ForwardCommProtocolDataFrame_02F1_Up->protocol_type_==0x02)
                {
                    tryTimes=0;
                    bool res=true;
                    shared_ptr<Frame645Base> MsgBase_645_ptr = Frame645Helper::DecodeLocalMsg(&msgBuf,res);
                    if(MsgBase_645_ptr==nullptr)
                        continue;
                    if(MsgBase_645_ptr->ctrlCode_!=NORMAL_RESP)
                        continue;
                    shared_ptr<dlt_645_Protocol::RspsNormal_ReadData_0x91> p_RspsNormal_ReadData_0x91 = std::dynamic_pointer_cast<dlt_645_Protocol::RspsNormal_ReadData_0x91>(MsgBase_645_ptr);
                    if(memcmp(ProgramCompare,p_RspsNormal_ReadData_0x91->di,4)==0)
                    {
                        if(p_RspsNormal_ReadData_0x91->data==programCompareList.at(index).compareData)//判断条件
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("第%1段程序比较一致").arg(++index));
                            //开始比对下一段
                            if(index<times)
                            {
                                emScriptRunState=Wait_QuerySoftWareCompare_02F1_Finish;
                                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,ForwardCommProtocolDataFrame_02F1);
                                p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("发送--STA程序比对（02F1），第%1段，等待--回复").arg(times));
                                p_timer->start(maxMonitorTime*1000);
                            }
                            else
                            {
                                emScriptRunState=ScriptSuccess;
                                p_AbstractScriptHost->updateProgress(ProcessState_Success, QString("STA程序比对,测试成功"));
                            }
                        }
                        else
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("第%1段程序比较不一致").arg(index+1));
                        }
                    }
                    else
                        continue;
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
void Script_ProgramComparison_STA_Hunan::processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID)
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
            case Wait_QuerySoftWareInfo_645_Finish:
            {
                if(MsgBase_645_ptr->ctrlCode_==NORMAL_RESP)
                {
                    p_timer->stop();
                    shared_ptr<dlt_645_Protocol::RspsNormal_ReadData_0x91> p_RspsNormal_ReadData_0x91 = std::dynamic_pointer_cast<dlt_645_Protocol::RspsNormal_ReadData_0x91>(MsgBase_645_ptr);
                    if(memcmp(SoftwareInfo,p_RspsNormal_ReadData_0x91->di,4)==0)
                    {
                        if(p_RspsNormal_ReadData_0x91->data==softwareInfoArray)
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("回复信息与初始信息一致"));
                            //开始比较程序段
                            times=ushort(programCompareList.size());
                            index=0;
                            emScriptRunState=Wait_QuerySoftWareCompare_645_Finish;
                            sendMsg(SingleSTA,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_Rqst_ReadData_0x11);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--STA程序比对（645），等待--回复");
                            p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
                        }
                        else
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("回复信息与初始信息不一致"));
                        }
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("数据标识回复异常！！！"));
                    }
                }
                else if(MsgBase_645_ptr->ctrlCode_==READ_ADDR)
                {
                    sendMsg(dvcType,dvcId,mtrlID,p_MeterAddrResp_93);
                }
                else
                {
                    uchar di[4]={0x00};
                    QByteArray tmpSendMsg=prcsOther645Msg(MsgBase_645_ptr->ctrlCode_,MsgBase_645_ptr->dataLen_,di,MsgBase_645_ptr->addr_,(*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[mtrlID]->mtrAddr,p_CtrInfoList);
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_QuerySoftWareCompare_645_Finish:
            {
                if(MsgBase_645_ptr->ctrlCode_==NORMAL_RESP)
                {
                    p_timer->stop();
                    shared_ptr<dlt_645_Protocol::RspsNormal_ReadData_0x91> p_RspsNormal_ReadData_0x91 = std::dynamic_pointer_cast<dlt_645_Protocol::RspsNormal_ReadData_0x91>(MsgBase_645_ptr);
                    if(memcmp(ProgramCompare,p_RspsNormal_ReadData_0x91->di,4)==0)
                    {
                        tryTimes=0;
                        if(p_RspsNormal_ReadData_0x91->data==programCompareList.at(index).compareData)//判断条件
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("第%1段程序比较一致").arg(++index));
                            //开始比对下一段
                            if(index<times)
                            {
                                emScriptRunState=Wait_QuerySoftWareCompare_645_Finish;
                                sendMsg(SingleSTA,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_Rqst_ReadData_0x11);
                                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--STA程序比对（645），等待--回复");
                                p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                            }
                            else
                            {
                                softwareInfoInit(65536,512);
                                times=ushort(programCompareList.size());
                                index=0;
                                emScriptRunState=Wait_QuerySoftWareCompare_645_2_Finish;
                                sendMsg(SingleSTA,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_Rqst_ReadData_0x11);
                                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--STA程序比对（645），等待--回复");
                                p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                            }
                        }
                        else
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("第%1段程序比较不一致").arg(index+1));
                        }
                    }

                }
                else if(MsgBase_645_ptr->ctrlCode_==READ_ADDR)
                {
                    sendMsg(dvcType,dvcId,mtrlID,p_MeterAddrResp_93);
                }
                else
                {
                    uchar di[4]={0x00};
                    QByteArray tmpSendMsg=prcsOther645Msg(MsgBase_645_ptr->ctrlCode_,MsgBase_645_ptr->dataLen_,di,MsgBase_645_ptr->addr_,(*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[mtrlID]->mtrAddr,p_CtrInfoList);
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
        case Wait_QuerySoftWareCompare_645_2_Finish:
        {
            if(MsgBase_645_ptr->ctrlCode_==NORMAL_RESP)
            {
                p_timer->stop();
                shared_ptr<dlt_645_Protocol::RspsNormal_ReadData_0x91> p_RspsNormal_ReadData_0x91 = std::dynamic_pointer_cast<dlt_645_Protocol::RspsNormal_ReadData_0x91>(MsgBase_645_ptr);
                if(memcmp(ProgramCompare,p_RspsNormal_ReadData_0x91->di,4)==0)
                {
                    tryTimes=0;
                    if(p_RspsNormal_ReadData_0x91->data==programCompareList.at(index).compareData)//判断条件
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("第%1段程序比较一致").arg(++index));
                        //开始比对下一段
                        if(index<times)
                        {
                            emScriptRunState=Wait_QuerySoftWareCompare_645_2_Finish;
                            sendMsg(SingleSTA,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_Rqst_ReadData_0x11);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--STA程序比对（645），等待--回复");
                            p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                        }
                        else
                        {
                            softwareInfoInit(65536,2048);
                            times=ushort(programCompareList.size());
                            index=0;
                            emScriptRunState=Wait_QuerySoftWareCompare_645_3_Finish;
                            sendMsg(SingleSTA,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_Rqst_ReadData_0x11);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--STA程序比对（645），等待--回复");
                            p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                        }
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("第%1段程序比较不一致").arg(index+1));
                    }
                }

            }
            else if(MsgBase_645_ptr->ctrlCode_==READ_ADDR)
            {
                sendMsg(dvcType,dvcId,mtrlID,p_MeterAddrResp_93);
            }
            else
            {
                uchar di[4]={0x00};
                QByteArray tmpSendMsg=prcsOther645Msg(MsgBase_645_ptr->ctrlCode_,MsgBase_645_ptr->dataLen_,di,MsgBase_645_ptr->addr_,(*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[mtrlID]->mtrAddr,p_CtrInfoList);
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case Wait_QuerySoftWareCompare_645_3_Finish:
        {
            if(MsgBase_645_ptr->ctrlCode_==NORMAL_RESP)
            {
                p_timer->stop();
                shared_ptr<dlt_645_Protocol::RspsNormal_ReadData_0x91> p_RspsNormal_ReadData_0x91 = std::dynamic_pointer_cast<dlt_645_Protocol::RspsNormal_ReadData_0x91>(MsgBase_645_ptr);
                if(memcmp(ProgramCompare,p_RspsNormal_ReadData_0x91->di,4)==0)
                {
                    tryTimes=0;
                    if(p_RspsNormal_ReadData_0x91->data==programCompareList.at(index).compareData)//判断条件
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("第%1段程序比较一致").arg(++index));
                        //开始比对下一段
                        if(index<times)
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("程序共%1段").arg(times));
                            emScriptRunState=Wait_QuerySoftWareCompare_645_3_Finish;
                            sendMsg(SingleSTA,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_Rqst_ReadData_0x11);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--STA程序比对（645），等待--回复");
                            p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                        }
                        else
                        {
                            emScriptRunState=Wait_ParameterInit_Finish;
                            sendMsg(CCO_GW,dvcId,INSIGNIFICANCE,p_ParameterInit_01F2);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--参数初始化（01F2），等待--确认");
                            p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);


////                            resultFlag=false;
////                            needBuildNet=true;
////                            if(needBuildNet==true)
////                            {
////                                p_BuildNetwork_GW->execute();
////                                emScriptRunState=Wait_BuildNetFinish_2_Whole;
////                                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "等待全网组网完成");
////                            }


//                            softwareInfoInit(65536,2048);
//                            times=ushort(programCompareList.size());
//                            index=0;


////                            emScriptRunState=Wait_QuerySoftWareCompare_13F1_Finish;
////                            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_MonitorSlaveNode_13F1);
////                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询STA软件信息（13F1），等待--回复");
////                            p_timer->start(maxMonitorTime*1000);

//                            emScriptRunState=Wait_QuerySoftWareInfo_13F1_Finish;
//                            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_MonitorSlaveNode_13F1);
//                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询STA软件信息（13F1），等待--回复11");
//                            p_timer->start(maxMonitorTime*1000);
                        }
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("第%1段程序比较不一致").arg(index+1));
                    }
                }

            }
            else if(MsgBase_645_ptr->ctrlCode_==READ_ADDR)
            {
                sendMsg(dvcType,dvcId,mtrlID,p_MeterAddrResp_93);
            }
            else
            {
                uchar di[4]={0x00};
                QByteArray tmpSendMsg=prcsOther645Msg(MsgBase_645_ptr->ctrlCode_,MsgBase_645_ptr->dataLen_,di,MsgBase_645_ptr->addr_,(*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[mtrlID]->mtrAddr,p_CtrInfoList);
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case Wait_QuerySoftWareCompare_13F1_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "################");
            break;
        }
        case Wait_QuerySoftWareCompare_02F1_Finish:
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
void Script_ProgramComparison_STA_Hunan::processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID)
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
void Script_ProgramComparison_STA_Hunan::sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame)
{
    if(frame==p_Rqst_ReadData_0x11)
    {
        uchar tmpAddr[6];
        memcpy(tmpAddr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(0)->mtrAddr,6);
        memcpy(p_Rqst_ReadData_0x11->addr_,tmpAddr,6);
        if(emScriptRunState==Wait_QuerySoftWareInfo_645_Finish)
        {
            memcpy(p_Rqst_ReadData_0x11->di,SoftwareInfo,4);
        }
        else
//            if(emScriptRunState==Wait_QuerySoftWareCompare_645_Finish)
        {
            memcpy(p_Rqst_ReadData_0x11->di,ProgramCompare,4);

            p_Rqst_ReadData_0x11->m_DataLen=7;
            p_Rqst_ReadData_0x11->programCompareData.clear();
            p_Rqst_ReadData_0x11->programCompareData.append(softwareInfoData.cpu_no_);
            p_Rqst_ReadData_0x11->programCompareData.append(programCompareList.at(index).startAddress,4);
            p_Rqst_ReadData_0x11->programCompareData.append(char(programCompareList.at(index).programSegment.size()));
            p_Rqst_ReadData_0x11->programCompareData.append(char(programCompareList.at(index).programSegment.size()>>8));
        }

        sendMsgOct=p_Rqst_ReadData_0x11->EncodeFrame();
        sendMsgLog=QString("》》查询软件信息：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==ForwardCommProtocolDataFrame_02F1)
    {
        ForwardCommProtocolDataFrame_02F1->ctrl_field_.dir=kDirDown;
        ForwardCommProtocolDataFrame_02F1->ctrl_field_.prm=kActive;
        ForwardCommProtocolDataFrame_02F1->ctrl_field_.comn_type=kHplc;

        ForwardCommProtocolDataFrame_02F1->info_field_.info_field_down.msg_seq=char(msgSeq++);
        ForwardCommProtocolDataFrame_02F1->info_field_.info_field_down.comu_rate=0;
        ForwardCommProtocolDataFrame_02F1->info_field_.info_field_down.comu_module_ident=1;

        uchar tmpAddr[6];
        memcpy(tmpAddr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(0)->mtrAddr,6);
        uchar comPrtclType=0x02;

        if(comPrtclType==0x02)
        {
            shared_ptr<Rqst_ReadData_0x11> p_Rqst_ReadData_0x11=make_shared<Rqst_ReadData_0x11>(addr,4,0);
            memcpy(p_Rqst_ReadData_0x11->addr_,tmpAddr,6);

            if(emScriptRunState==Wait_QuerySoftWareCompare_02F1_Finish)
            {
                memcpy(p_Rqst_ReadData_0x11->di,ProgramCompare,4);

                p_Rqst_ReadData_0x11->m_DataLen=7;
                p_Rqst_ReadData_0x11->programCompareData.clear();
                p_Rqst_ReadData_0x11->programCompareData.append(softwareInfoData.cpu_no_);
                p_Rqst_ReadData_0x11->programCompareData.append(programCompareList.at(index).startAddress,4);
                p_Rqst_ReadData_0x11->programCompareData.append(char(programCompareList.at(index).programSegment.size()));
                p_Rqst_ReadData_0x11->programCompareData.append(char(programCompareList.at(index).programSegment.size()>>8));
            }
            QByteArray msg645=p_Rqst_ReadData_0x11->EncodeFrame();

            ForwardCommProtocolDataFrame_02F1->protocol_type_=0x02;
            ForwardCommProtocolDataFrame_02F1->frame_content_=msg645;
            ForwardCommProtocolDataFrame_02F1->frame_length_=uchar(msg645.size());
            uchar tmpCcoAddr[6];
            memcpy(tmpCcoAddr,p_CtrInfoList->at(0)->ccoAddr,6);
            memcpy(ForwardCommProtocolDataFrame_02F1->address_field_.src_addr,tmpCcoAddr,6);
            memcpy(ForwardCommProtocolDataFrame_02F1->address_field_.dst_addr,tmpAddr,6);

            sendMsgOct=ForwardCommProtocolDataFrame_02F1->EncodeFrame();
            sendMsgLog=QString("》》转发通信协议数据帧（02F1）：%1\n").arg(QString(sendMsgOct.toHex()));
        }
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
    else if(frame==p_ParameterInit_01F2)
    {
        p_ParameterInit_01F2->ctrl_field_.dir=kDirDown;
        p_ParameterInit_01F2->ctrl_field_.prm=kActive;
        p_ParameterInit_01F2->ctrl_field_.comn_type=kHplc;

        p_ParameterInit_01F2->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_ParameterInit_01F2->info_field_.info_field_down.comu_rate=0;
        p_ParameterInit_01F2->info_field_.info_field_down.comu_module_ident=0;

        sendMsgOct=p_ParameterInit_01F2->EncodeFrame();
        sendMsgLog=QString("》》参数初始化01F1：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
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
        if(index+num<p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().size())//每次添加表数num，
        {
            for(int i=0;i<num;i++)
            {
                NodeParameter nodePara;
                memcpy(nodePara.node_address_.addr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(index+i)->mtrAddr,6);
                nodePara.protocol_type_=char(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(index+i)->prtcl);
                p_AddSlaveNode_11F1->node_parameter_list_.append(nodePara);
            }
        }
        else
        {
            for(int i=0;i<p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().size()-index;i++)
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
    else if(frame==p_MonitorSlaveNode_13F1)
    {
        uchar tmpAddr[6];
        memcpy(tmpAddr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(0)->mtrAddr,6);
        uchar comPrtclType=0x02;

        if(comPrtclType==0x02)
        {
            shared_ptr<Rqst_ReadData_0x11> p_Rqst_ReadData_0x11=make_shared<Rqst_ReadData_0x11>(addr,4,0);
            memcpy(p_Rqst_ReadData_0x11->addr_,tmpAddr,6);
            if(emScriptRunState==Wait_QuerySoftWareInfo_13F1_Finish)
            {
                memcpy(p_Rqst_ReadData_0x11->di,SoftwareInfo,4);
            }
            else if(emScriptRunState==Wait_QuerySoftWareCompare_13F1_Finish)
            {
                memcpy(p_Rqst_ReadData_0x11->di,ProgramCompare,4);

                p_Rqst_ReadData_0x11->m_DataLen=0x07;
                p_Rqst_ReadData_0x11->programCompareData.clear();
                p_Rqst_ReadData_0x11->programCompareData.append(softwareInfoData.cpu_no_);
                p_Rqst_ReadData_0x11->programCompareData.append(programCompareList.at(index).startAddress,4);
                p_Rqst_ReadData_0x11->programCompareData.append(char(programCompareList.at(index).programSegment.size()));
                p_Rqst_ReadData_0x11->programCompareData.append(char(programCompareList.at(index).programSegment.size()>>8));
            }

            QByteArray msg645=p_Rqst_ReadData_0x11->EncodeFrame();

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
void Script_ProgramComparison_STA_Hunan::sendSrcMsg(DvcType dvcType, int dvcId, QByteArray msg)
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

void Script_ProgramComparison_STA_Hunan::timer_timeoutProc()
{
    switch(emScriptRunState)
    {
        case Wait_BuildNetFinish_Whole:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_BuildNetFinish_Whole timeout!!!");
            break;
        }
        case Wait_QuerySoftWareCompare_645_Finish:
        {
            if(++tryTimes<=3)
            {
                emScriptRunState=Wait_QuerySoftWareCompare_645_Finish;
                sendMsg(SingleSTA,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_Rqst_ReadData_0x11);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("第%1次重新发送--STA程序比对（645），等待--回复").arg(tryTimes));
                p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
            }
            else
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Failed, "达到最大尝试次数1");
            }
            break;
        }
        case Wait_QuerySoftWareCompare_645_2_Finish:
        {
            if(++tryTimes<=3)
            {
                emScriptRunState=Wait_QuerySoftWareCompare_645_2_Finish;
                sendMsg(SingleSTA,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_Rqst_ReadData_0x11);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("第%1次重新发送--STA程序比对（645），等待--回复").arg(tryTimes));
                p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
            }
            else
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Failed, "达到最大尝试次数2");
            }
            break;
        }
        case Wait_QuerySoftWareCompare_645_3_Finish:
        {
            if(++tryTimes<=3)
            {
                emScriptRunState=Wait_QuerySoftWareCompare_645_3_Finish;
                sendMsg(SingleSTA,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_Rqst_ReadData_0x11);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("第%1次重新发送--STA程序比对（645），等待--回复").arg(tryTimes));
                p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
            }
            else
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Failed, "达到最大尝试次数3");
            }
            break;
        }
        case Wait_QuerySoftWareCompare_13F1_Finish:
        {
            if(++tryTimes<=3)
            {
                emScriptRunState=Wait_QuerySoftWareCompare_13F1_Finish;
                sendMsg(SingleSTA,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_MonitorSlaveNode_13F1);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("第%1次重新发送--STA程序比对（13F1），等待--回复").arg(tryTimes));
                p_timer->start(maxMonitorTime*1000);
            }
            else
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("程序共%1段").arg(times));
                p_AbstractScriptHost->updateProgress(ProcessState_Failed, "达到最大尝试次数4");
            }
            break;
        }
        case Wait_QuerySoftWareCompare_02F1_Finish:
        {
            if(++tryTimes<=3)
            {
                emScriptRunState=Wait_QuerySoftWareCompare_02F1_Finish;
                sendMsg(SingleSTA,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,ForwardCommProtocolDataFrame_02F1);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("第%1次重新发送--STA程序比对（02F1），等待--回复").arg(tryTimes));
                p_timer->start(maxMonitorTime*1000);
            }
            else
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("程序共%1段").arg(times));
                p_AbstractScriptHost->updateProgress(ProcessState_Failed, "达到最大尝试次数5");
            }
            break;
        }
    case Wait_QuerySoftWareInfo_13F1_Finish:
    {
        if(++tryTimes<=3)
        {
            emScriptRunState=Wait_QuerySoftWareInfo_13F1_Finish;
            sendMsg(SingleSTA,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_MonitorSlaveNode_13F1);
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("第%1次重新发送--STA程序比对（13F1），等待--回复").arg(tryTimes));
            p_timer->start(maxMonitorTime*1000);
        }
        else
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("程序共%1段").arg(times));
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "达到最大尝试次数6");
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
void Script_ProgramComparison_STA_Hunan::maxAllowTimer_timeoutProc()
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
void Script_ProgramComparison_STA_Hunan::delayTimer_timeoutProc()
{
    p_delayTimer->stop();
    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryNetScale_10F9);
    emScriptRunState=Wait_QueryNetScale_10F9_Finish;
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询从节点数量（10F9），等待--回复");
    p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
}

void Script_ProgramComparison_STA_Hunan::softwareInfoInit()
{
    //加载程序文件
    QStringList fileNameFilters;
    QStringList fileList;
    QString path=tr("DataBase\\Upgrade\\湖南HPLC模块");
//    QString path=tr("DataBase\\Upgrade\\国网HPLC模块(旧-新)");

    QDir *updateFileDir=new QDir(path);
    fileNameFilters << "*" ;
    QList<QFileInfo> *fileInfo=new QList<QFileInfo>(updateFileDir->entryInfoList(fileNameFilters,QDir::Files,QDir::NoSort));

    if(fileInfo->count() != 1)
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "没有STA升级文件或STA升级文件多于1个!!!");
    }
    QString filePath=path+"\\"+fileInfo->at(0).fileName();

    delete fileInfo;
    delete updateFileDir;

    QFile file(filePath);
    if(!file.open(QIODevice::ReadOnly))
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "打开STA升级文件失败!!!");
        return;
    }
    //
    originProgram=file.readAll();
    //获取软件信息,根据程序名
    softwareInfoData.cpu_no_=0x00;
    softwareInfoData.vendor_code_[1]='T';
    softwareInfoData.vendor_code_[0]='C';
    char date[3]={0x18,0x0C,0x15};//241221
    memcpy(softwareInfoData.version_date_,date,3);
    softwareInfoData.hardware_version_[0]=0x01;
    softwareInfoData.hardware_version_[1]=0x02;
    softwareInfoData.software_version_[0]=0x02;
    softwareInfoData.software_version_[1]=0x43;
    softwareInfoData.mcu_type_len_=2;
    softwareInfoData.mcu_type_.append('A');
    softwareInfoData.mcu_type_.append('C');

    softwareInfoArray.append(this->softwareInfoData.cpu_no_);
    softwareInfoArray.append(this->softwareInfoData.vendor_code_,2);
    softwareInfoArray.append(char(this->softwareInfoData.version_date_[0]<<3));
    ushort temp=ushort(this->softwareInfoData.version_date_[2])+2000;
    softwareInfoArray.append(char(uchar(this->softwareInfoData.version_date_[1]&0x0F)+uchar((temp&0x0F)<<4)));
    softwareInfoArray.append(char(temp>>4));
    softwareInfoArray.append(this->softwareInfoData.hardware_version_,2);
    softwareInfoArray.append(this->softwareInfoData.software_version_,2);
    softwareInfoArray.append(char(this->softwareInfoData.mcu_type_len_));
    softwareInfoArray.append(this->softwareInfoData.mcu_type_);
    //获取compare信息
    int currentIndex=0;
    while (currentIndex<originProgram.size())
    {
        ProgramCompareStruct programCompareST;
        programCompareST.startAddress[0]=char(currentIndex);
        programCompareST.startAddress[1]=char(currentIndex>>8);
        programCompareST.startAddress[2]=char(currentIndex>>16);
        programCompareST.startAddress[3]=char(currentIndex>>24);
        if(currentIndex+segmentLen<=originProgram.size())
            programCompareST.programSegment=originProgram.mid(currentIndex,segmentLen);
        else
            programCompareST.programSegment=originProgram.mid(currentIndex);
        programCompareST.compareData=QCryptographicHash::hash(programCompareST.programSegment, QCryptographicHash::Sha1);
        programCompareList.append(programCompareST);
        currentIndex+=segmentLen;
    }
}
void Script_ProgramComparison_STA_Hunan::softwareInfoInit(int currentIndex,int segmentLen)
{

    programCompareList.clear();
    while (currentIndex<originProgram.size())
    {
        ProgramCompareStruct programCompareST;

        programCompareST.startAddress[0]=char(currentIndex);
        programCompareST.startAddress[1]=char(currentIndex>>8);
        programCompareST.startAddress[2]=char(currentIndex>>16);
        programCompareST.startAddress[3]=char(currentIndex>>24);
        if(currentIndex+segmentLen<=originProgram.size())
            programCompareST.programSegment=originProgram.mid(currentIndex,segmentLen);
        else
            programCompareST.programSegment=originProgram.mid(currentIndex);
        programCompareST.compareData=QCryptographicHash::hash(programCompareST.programSegment, QCryptographicHash::Sha1);
        programCompareList.append(programCompareST);
        currentIndex+=segmentLen;
    }
}
