#include "Script_RouterState_SearchMeter_Upgrade_Beijing.h"

Script_RouterState_SearchMeter_Upgrade_Beijing::Script_RouterState_SearchMeter_Upgrade_Beijing(QObject *parent) : QObject(parent)
{
    emScriptRunState=TestInit;
    resultFlag=false;
    sendMsgOct.clear();

    p_CtrInfoList=make_shared<QList<shared_ptr<CtrInfo>>>();
    p_BuildNetwork_GW=new BuildNetwork_GW();

    p_Frame3762Helper=make_shared<Frame3762Helper>();
    p_Confirm_00F1=make_shared<Afn00F1>();
    p_ActiveRegister_11F5=make_shared<Afn11F5>();
    p_StopActiveRegister_11F6=make_shared<Afn11F6>();
    p_TransferUpgradeFile_15F1=make_shared<Afn15F1>();
    p_QueryRouterState_10F4=make_shared<Afn10F4_Beijing>();

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
Script_RouterState_SearchMeter_Upgrade_Beijing::~Script_RouterState_SearchMeter_Upgrade_Beijing()
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

void Script_RouterState_SearchMeter_Upgrade_Beijing::execute()
{
    p_AbstractScriptHost->updateProgress(ProcessState_Start, "Test start!");

    emScriptRunState=TestInit;
    resultFlag=false;
    addrList.clear();

    if(needBuildNet==true)
    {
        p_BuildNetwork_GW->execute();
        emScriptRunState=Wait_BuildNetFinish_Whole;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "等待全网组网完成");

    }
    else
    {
        LoadUpgradeFile();
        emScriptRunState=Wait_QueryRouterStateSearchMeter_Finish;
        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_ActiveRegister_11F5);
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--激活主动注册，等待--确认");
        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
    }
    p_maxAllowTimer->start((timerForReachThresld+timerAfterReachThresld)*1000);
}
void Script_RouterState_SearchMeter_Upgrade_Beijing::stop()
{
    if(p_AbstractScriptHost==nullptr)
        return;
    p_AbstractScriptHost->updateProgress(ProcessState_Error, "Test stop!");
    if(needPowerOff==true)
        powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost);
}
void Script_RouterState_SearchMeter_Upgrade_Beijing::addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq)
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
void Script_RouterState_SearchMeter_Upgrade_Beijing::setHost(AbstractScriptHost *host)
{
    p_AbstractScriptHost=host;
    p_BuildNetwork_GW->setHost(host);
}
bool Script_RouterState_SearchMeter_Upgrade_Beijing::config(const QMap<QString,QString> *paraDic)
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
void Script_RouterState_SearchMeter_Upgrade_Beijing::processMsg(DvcType dvcType, int id, uchar *data, int datalen)
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
                index=0;
                LoadUpgradeFile();
                emScriptRunState=Wait_QueryRouterStateSearchMeter_Finish;
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_ActiveRegister_11F5);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--激活主动注册，等待--确认");
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
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
void Script_RouterState_SearchMeter_Upgrade_Beijing::processCtrlDvcRes(DvcType dvcType, QList<int> idList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params)
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
void Script_RouterState_SearchMeter_Upgrade_Beijing::processMsgFromCCO(DvcType dvcType, int dvcId)
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
            case Wait_QueryRouterStateSearchMeter_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();//接收到一条完整报文
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "激活主动注册回复确认");
                }
                else if(p_Frame3762Base->afn_==0x06&&p_Frame3762Base->dt1_==0x08&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    shared_ptr<Afn06F4> p_ReportRegisterNode_Up=dynamic_pointer_cast<Afn06F4>(p_Frame3762Base);
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("上报注册从节点，地址为%1；").arg(QString(QByteArray(p_ReportRegisterNode_Up->report_node_info_unit_.report_node_address_.addr,6).toHex())));
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_Confirm_00F1);
                    if(index==0)
                    {
                        index++;
                        emScriptRunState=Wait_QueryRouterStateSearchMeter_Finish;
                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryRouterState_10F4);
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询路由运行状态，等待--回复");
                        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                    }
                }
                else if(p_Frame3762Base->afn_ == 0x10&&p_Frame3762Base->dt1_==0x08&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();//接收到一条完整报文
                    shared_ptr<Afn10F4_Beijing> p_QueryRouterState_10F4_Up=dynamic_pointer_cast<Afn10F4_Beijing>(p_Frame3762Base);
                    if(uchar(p_QueryRouterState_10F4_Up->router_operate_state_unit_.work_switch_.area_difference_flag_)==0x00
                            &&uchar(p_QueryRouterState_10F4_Up->router_operate_state_unit_.work_switch_.work_state_1)==0x01)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "查询路由运行状态，符合要求");
                        emScriptRunState=Wait_QueryRouterStateUpgrade_Finish;
                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_StopActiveRegister_11F6);
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--终止主动注册（11F6），等待--确认");
                        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "查询路由运行状态，不符合要求");
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "搜表查询路由运行状态测试失败");
                    }
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_QueryRouterStateUpgrade_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();//接收到一条完整报文
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "终止主动注册，回复确认");
                    index=0;//标记
                    emScriptRunState=Wait_QueryRouterStateUpgrade_Finish;
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_TransferUpgradeFile_15F1);
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--文件传输，等待--回复");
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                }
                else if(p_Frame3762Base->afn_ == 0x15&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    emScriptRunState=Wait_QueryRouterStateUpgrade_Finish;
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryRouterState_10F4);
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询路由运行状态，等待--回复");
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                }
                else if(p_Frame3762Base->afn_ == 0x10&&p_Frame3762Base->dt1_==0x08&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();//接收到一条完整报文
                    shared_ptr<Afn10F4_Beijing> p_QueryRouterState_10F4_Up=dynamic_pointer_cast<Afn10F4_Beijing>(p_Frame3762Base);
                    if(uchar(p_QueryRouterState_10F4_Up->router_operate_state_unit_.work_switch_.area_difference_flag_)==0x00
                            &&uchar(p_QueryRouterState_10F4_Up->router_operate_state_unit_.work_switch_.work_state_1)==0x02
                            &&uchar(p_QueryRouterState_10F4_Up->router_operate_state_unit_.operate_state_word_.router_complete_flag_)==0x01)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "升级中查询路由运行状态，符合要求");
                        emScriptRunState=Wait_QueryRouterStateAfterUpgrade_Finish;
                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_HardReset_01F1);
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--硬件复位（01F1），等待--确认");
                        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "升级中查询路由运行状态，不符合要求");
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "升级查询路由运行状态测试失败");
                    }
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_QueryRouterStateAfterUpgrade_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    break;
                }
                else if(p_Frame3762Base->afn_ == 0x03&&p_Frame3762Base->dt1_==0x02&&p_Frame3762Base->dt2_==0x01&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "硬件复位，回复确认");
                    emScriptRunState=Wait_QueryRouterStateAfterUpgrade_Finish;
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryRouterState_10F4);
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询路由运行状态，等待--回复");
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                }
                else if(p_Frame3762Base->afn_ == 0x10&&p_Frame3762Base->dt1_==0x08&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();//接收到一条完整报文
                    shared_ptr<Afn10F4_Beijing> p_QueryRouterState_10F4_Up=dynamic_pointer_cast<Afn10F4_Beijing>(p_Frame3762Base);
                    if(uchar(p_QueryRouterState_10F4_Up->router_operate_state_unit_.work_switch_.area_difference_flag_)==0x00
                            &&uchar(p_QueryRouterState_10F4_Up->router_operate_state_unit_.work_switch_.work_state_1)==0x03)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "升级结束查询路由运行状态，符合要求");
                        emScriptRunState=ScriptSuccess;
                        p_AbstractScriptHost->updateProgress(ProcessState_Success, "搜表-升级查询路由运行状态测试成功");
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "升级结束查询路由运行状态，不符合要求");
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "搜表-升级查询路由运行状态测试失败");
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
void Script_RouterState_SearchMeter_Upgrade_Beijing::processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID)
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
void Script_RouterState_SearchMeter_Upgrade_Beijing::processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID)
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

void Script_RouterState_SearchMeter_Upgrade_Beijing::sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame)
{
    if(frame==p_HardReset_01F1)
    {
        p_HardReset_01F1->ctrl_field_.dir=kDirDown;
        p_HardReset_01F1->ctrl_field_.prm=kActive;
        p_HardReset_01F1->ctrl_field_.comn_type=kHplc;

        p_HardReset_01F1->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_HardReset_01F1->info_field_.info_field_down.comu_rate=0;
        p_HardReset_01F1->info_field_.info_field_down.comu_module_ident=0;

        sendMsgOct=p_HardReset_01F1->EncodeFrame();
        sendMsgLog=QString("》》硬件复位01F1：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_ActiveRegister_11F5)
    {
        p_ActiveRegister_11F5->ctrl_field_.dir=kDirDown;
        p_ActiveRegister_11F5->ctrl_field_.prm=kActive;
        p_ActiveRegister_11F5->ctrl_field_.comn_type=kHplc;

        p_ActiveRegister_11F5->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_ActiveRegister_11F5->info_field_.info_field_down.comu_rate=0;
        p_ActiveRegister_11F5->info_field_.info_field_down.comu_module_ident=0;

        QByteArray startTime=QByteArray::fromHex(QDateTime::currentDateTime().toString("ssmmhhddMMyy").toLatin1());
        memcpy(&p_ActiveRegister_11F5->start_time_,startTime,6);
        p_ActiveRegister_11F5->last_time_=2;//2min
        p_ActiveRegister_11F5->retransmit_times_=0;
        p_ActiveRegister_11F5->wait_time_slice_num_=0;

        sendMsgOct=p_ActiveRegister_11F5->EncodeFrame();
        sendMsgLog=QString("》》激活主动注册11F5：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_StopActiveRegister_11F6)
    {
        p_StopActiveRegister_11F6->ctrl_field_.dir=kDirDown;
        p_StopActiveRegister_11F6->ctrl_field_.prm=kActive;
        p_StopActiveRegister_11F6->ctrl_field_.comn_type=kHplc;

        p_StopActiveRegister_11F6->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_StopActiveRegister_11F6->info_field_.info_field_down.comu_rate=0;
        p_StopActiveRegister_11F6->info_field_.info_field_down.comu_module_ident=0;

        sendMsgOct=p_StopActiveRegister_11F6->EncodeFrame();
        sendMsgLog=QString("》》终止主动注册11F6：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_Confirm_00F1)
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
    else if(frame==p_QueryRouterState_10F4)
    {
        p_QueryRouterState_10F4->ctrl_field_.dir=kDirDown;
        p_QueryRouterState_10F4->ctrl_field_.prm=kActive;
        p_QueryRouterState_10F4->ctrl_field_.comn_type=kHplc;

        p_QueryRouterState_10F4->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_QueryRouterState_10F4->info_field_.info_field_down.comu_rate=0;
        p_QueryRouterState_10F4->info_field_.info_field_down.comu_module_ident=0;

        sendMsgOct=p_QueryRouterState_10F4->EncodeFrame();
        sendMsgLog=QString("》》查询台区区分标志10F4：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_TransferUpgradeFile_15F1)
    {
        p_TransferUpgradeFile_15F1->ctrl_field_={kHplc,kActive,kDirDown};

        p_TransferUpgradeFile_15F1->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_TransferUpgradeFile_15F1->info_field_.info_field_down.comu_rate=0;
        p_TransferUpgradeFile_15F1->info_field_.info_field_down.comu_module_ident=0;

        p_TransferUpgradeFile_15F1->file_transfer_unit_.file_identify_=0x08;
        if(index==upgradeArrayList.size()-1)
            p_TransferUpgradeFile_15F1->file_transfer_unit_.file_property_=0x01;
        else if(index<upgradeArrayList.size()-1)
            p_TransferUpgradeFile_15F1->file_transfer_unit_.file_property_=0x00;
        p_TransferUpgradeFile_15F1->file_transfer_unit_.file_instruct_=0x00;
        p_TransferUpgradeFile_15F1->file_transfer_unit_.total_num_=ushort(upgradeArrayList.size());
        p_TransferUpgradeFile_15F1->file_transfer_unit_.this_identify_=index;
        p_TransferUpgradeFile_15F1->file_transfer_unit_.file_length_=ushort(upgradeArrayList.at(index).size());
        p_TransferUpgradeFile_15F1->file_transfer_unit_.file_content_=upgradeArrayList.at(index);

        sendMsgOct=p_TransferUpgradeFile_15F1->EncodeFrame();
        sendMsgLog=QString("文件传输15F1第%1段").arg(index);
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
void Script_RouterState_SearchMeter_Upgrade_Beijing::sendSrcMsg(DvcType dvcType, int dvcId, QByteArray msg)
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

void Script_RouterState_SearchMeter_Upgrade_Beijing::timer_timeoutProc()
{
    switch(emScriptRunState)
    {
        case Wait_BuildNetFinish_Whole:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait build whole net finish timeout!!!");
            break;
        }
        default:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==p_timer");
            break;
        }
    }
}
void Script_RouterState_SearchMeter_Upgrade_Beijing::maxAllowTimer_timeoutProc()
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
void Script_RouterState_SearchMeter_Upgrade_Beijing::delayTimer_timeoutProc()
{
    p_delayTimer->stop();

    index=0;
    p_maxAllowTimer->stop();


}

void Script_RouterState_SearchMeter_Upgrade_Beijing::LoadUpgradeFile()
{
    //加载程序文件
    QStringList fileNameFilters;
    QStringList fileList;
    QString path=tr("DataBase\\Upgrade\\北京HPLC模块");

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
    upgradeArrayList.clear();
    int currentIndex=0;
    while (currentIndex<originProgram.size())
    {
        QByteArray array;
        if(currentIndex+segmentLen<=originProgram.size())
            array=originProgram.mid(currentIndex,segmentLen);
        else
            array=originProgram.mid(currentIndex);
        upgradeArrayList.append(array);
        currentIndex+=segmentLen;
    }

}
