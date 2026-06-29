#include "Script_RefuseListReport_DefaultClose.h"


Script_RefuseListReport_DefaultClose::Script_RefuseListReport_DefaultClose(QObject *parent) : QObject(parent)
{
    p_BuildNetwork_GW=new BuildNetwork_GW();
    p_CtrInfoList=make_shared<QList<shared_ptr<CtrInfo>>>();

    p_Frame3762Helper=make_shared<Frame3762Helper>();
    p_ParaInit_01F2=make_shared<Afn01F2>();
    p_RefuseNodeReport_05F200=make_shared<Afn05F200>();
    p_Confirm_00F1=make_shared<Afn00F1>();

    p_Frame645Helper=make_shared<Frame645Helper>();
    p_MeterAddrResp_93=make_shared<RspsNormal_ReadAddr_0x93>(addr,6);
    p_FrameOOPHelper=make_shared<FrameOOPHelper>();
    p_GetResponseNormal_ReadAddr=make_shared<GetResponseNormal>();

    p_timer=new QTimer(this);
    p_maxAllowTimer=new QTimer(this);
    p_delayTimer=new QTimer(this);
    connect(p_timer,SIGNAL(timeout()),this,SLOT(timer_timeoutProc()));
    connect(p_maxAllowTimer,SIGNAL(timeout()),this,SLOT(maxAllowTimer_timeoutProc()));
    connect(p_delayTimer,SIGNAL(timeout()),this,SLOT(delayTimer_timeoutProc()));
}

Script_RefuseListReport_DefaultClose::~Script_RefuseListReport_DefaultClose()
{
    p_BuildNetwork_GW->initBuildNetWork();
    if(needPowerOff==true)//断电处理
        powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost);
}


void Script_RefuseListReport_DefaultClose::setHost(AbstractScriptHost *host)
{
    p_AbstractScriptHost=host;
    p_BuildNetwork_GW->setHost(host);
}

void Script_RefuseListReport_DefaultClose::addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq)
{
    p_CtrInfoList->clear();
    comnAddAddrsInfo(p_CtrInfoList, p_ConcentratorInfoList, p_MeterInfoList, p_SchemeCfgInfoList);
    uchar dstFreq=freq&0x0f;

    uchar dstPrtcl=uchar(freq)>>4;
    if(dstPrtcl==0x03)
        setMeterAddrsPrtcl(p_CtrInfoList,dstPrtcl);

    p_BuildNetwork_GW->setCtrInfoListAndFreq(p_CtrInfoList, dstFreq);

}

bool Script_RefuseListReport_DefaultClose::config(const QMap<QString,QString> *paraDic)
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
            (*paraDic)["needBuildNet"].toLower()=="true"?this->needBuildNet=true:this->needBuildNet=false;
        }
        if(paraDic->keys().contains("needPowerOff"))
        {
            (*paraDic)["needPowerOff"].toLower()=="true"?this->needPowerOff=true:this->needPowerOff=false;
        }
        result = true;
    }
    return result;
}

///几种测试场景
/// ***********************************************************************************
/// 场景1.不需要组网，且不需要添加档案或重新设置CCO，这种一般像抄控器升级，不涉及CCO的操作
/// 直接在execute()中执行测试脚本的第一步操作即可
/// ***********************************************************************************
/// 场景2.不需要组网，但是需要添加档案或重新设置CCO，设置needBuildNet=true;
/// 结合p_BuildNetwork_GW->startBuildNetFlag标志，startBuildNetFlag初始是false
/// 当组网通用脚本执行完添加档案之后，startBuildNetFlag标志置为true
/// 据此判断执行测试脚本的新操作即可
/// ***********************************************************************************
/// 场景3.需要组网，但如果CCO已组网完成且档案也全部匹配则不需要重新组网，如抄表脚本，
/// 设置needBuildNet=true;
/// 组网通用脚本默认会首先执行组网探测脚本，具体可查看组网通用脚本代码
/// 如果探测已组网完成且拓扑档案与设定档案一样，会提示组网完成，否则继续执行组网通用脚本，完成组网
/// 组网完成后，p_BuildNetwork_GW->buildNetworkResultFlag标志为true
/// 执行测试脚本的新操作即可
/// ***********************************************************************************
/// 场景4.需要组网，需要重新组网，如组网脚本，设置needBuildNet=true;
/// 设置p_BuildNetwork_GW->needRebuildNetwork=true;
/// 组网通用脚本默认会跳过组网探测脚本，直接执行组网通用脚本后续操作，完成组网
/// 组网完成后，p_BuildNetwork_GW->buildNetworkResultFlag标志为true
/// 执行测试脚本的新操作即可


void Script_RefuseListReport_DefaultClose::execute()
{
    QString step_desc = "流程描述: \r";
    step_desc += "0. 用例前提：需要环境中电表数大于100 \r";
    step_desc += "1. 通用组网流程，组网场景2(需要重新添加档案)，Wait_BuildNetFinish_Whole \r";
    step_desc += "2. 01F2参数初始化，等待5Min，Wait_SetParaInit_01F2_Finish \r";
    step_desc += "3. 05F200设置拒绝节点信息上报打开，等待5min，Wait_SetRefuseStaInforReportOpen_05F200_Finish \r";
    step_desc += "4. 05F200设置拒绝节点信息上报关闭，Wait_SetRefuseStaInforReportClose_05F200_Finish \n";

    p_AbstractScriptHost->updateProgress(ProcessState_Processing, step_desc);
    p_AbstractScriptHost->updateProgress(ProcessState_Start, "Test start!");

    if(needBuildNet==true) //场景2-4
    {
#ifdef SENCE4
        p_BuildNetwork_GW->needRebuildNetwork=true;
#endif
        p_BuildNetwork_GW->execute();//执行组网通用脚本
        emScriptRunState = Wait_BuildNetFinish_Whole;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "等待全网组网完成");
    }
    else //场景1
    {
        tryTimes = 0;
        sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, INSIGNIFICANCE, p_ParaInit_01F2);
        emScriptRunState = Wait_SetParaInit_01F2_Finish;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--参数初始化，等待--确认");
        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
    }
    p_maxAllowTimer->start((timerForReachThresld+timerAfterReachThresld)*1000);//此处要启动脚本的最大执行时间定时器
}

void Script_RefuseListReport_DefaultClose::stop()
{
    p_timer->stop();
    p_delayTimer->stop();
    p_maxAllowTimer->stop();
    if(p_AbstractScriptHost==nullptr)
        return;
    p_AbstractScriptHost->updateProgress(ProcessState_Error, "Test stop!");
    if(needPowerOff==true)
        powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost);
}

void Script_RefuseListReport_DefaultClose::processMsg(DvcType dvcType, int id, uchar *data, int datalen)
{
    if(p_CtrInfoList->size()==0)
        return;
    if(emScriptRunState == Wait_BuildNetFinish_Whole)//场景2-4
    {
#ifdef SENCE2
        if(!p_BuildNetwork_GW->startBuildNetFlag)//场景2 关注p_BuildNetwork_GW->startBuildNetFlag标志,开始组网标志
            p_BuildNetwork_GW->processMsg(dvcType,id,data,datalen);
#elif defined(SENCE3)||defined(SENCE4)
        if(!p_BuildNetwork_GW->buildNetworkResultFlag)//场景3、4 关注p_BuildNetwork_GW->buildNetworkResultFlag标志，组网完成标志
            p_BuildNetwork_GW->processMsg(dvcType,id,data,datalen);
#elif defined SENCE1
        if(true){}
#endif
        else
        {
            p_BuildNetwork_GW->initBuildNetWork();

//            if(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().size() < 100)
//            {
//                p_AbstractScriptHost->updateProgress(ProcessState_Error, "Wait_SetParaInit_01F2_Finish 测试异常，档案数不足100 【GW-CCO-F019-0001-V01】");
//            }

            emScriptRunState = Wait_SetParaInit_01F2_Finish;
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "添加1个档案完成，等待1min \n");
            p_delayTimer->start(60*1000);

//            tryTimes = 0;
//            emScriptRunState = Wait_SetParaInit_01F2_Finish;
//            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--参数初始化，等待--确认");

//            sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, INSIGNIFICANCE, p_ParaInit_01F2);
//            p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
        }
    }
    else//当测试脚本开始执行脚本自己的操作时，均从此进入
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

void Script_RefuseListReport_DefaultClose::processCtrlDvcRes(DvcType dvcType, QList<int> idList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params)
{
    //处理工装操作命令，如12V断上电，复位，设置波特率之类的
    Q_UNUSED(dvcType)
    Q_UNUSED(idList)
    Q_UNUSED(ctrlCmdType)
    Q_UNUSED(isSucs)
    Q_UNUSED(params)

    if(isSucs==false)
        return;
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到控制命令回复，参数个数=%1").arg(QString::number(params.size())));
    QList<int> sendParams;
    switch(emScriptRunState)
    {
        case ScriptInit:
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
        {
            break;
        }
    }
}


void Script_RefuseListReport_DefaultClose::processMsgFromCCO(DvcType dvcType, int dvcId)
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
            case ScriptInit:
            {
                break;
            }
            case Wait_BuildNetFinish_Whole:
            {
                break;
            }
            case Wait_SetParaInit_01F2_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x00 && p_Frame3762Base->dt1_==0x01 && p_Frame3762Base->dt2_==0x00 && p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "参数初始化，回复确认！");

                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "等待1min \n");
                    p_delayTimer->start(60*1000);
                }
                else if(p_Frame3762Base->afn_ == 0x06&&p_Frame3762Base->dt1_==0x10&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    shared_ptr<Afn06F5> p_EventReport_06F5_Up=dynamic_pointer_cast<Afn06F5>(p_Frame3762Base);
                    if(p_EventReport_06F5_Up->report_event_unit_.node_protocol_type_==0x05)
                    {
                        p_delayTimer->stop();
                        p_timer->stop();
                        uchar reportNum = uchar(p_EventReport_06F5_Up->report_event_unit_.frame_content_.at(0));
                        totalReportNum += reportNum;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到06F5拒绝列表事件上报，本次上报数%1，合计上报数%2")
                                                             .arg(reportNum).arg(totalReportNum));
                        report_flag_06F5 = true;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "回复确认");
                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_Confirm_00F1);

                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_SetParaInit_01F2_Finish 拒绝节点信息上报，默认关闭状态下收到06F5上报 【GW-CCO-F019-0001-V01】");
                        break;
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到06F5其他事件上报"));
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "回复确认");
                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_Confirm_00F1);
                    }

                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_SetRefuseStaInforReportOpen_05F200_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "允许拒绝节点信息上报，路由回复确认，等待上报");
                    totalReportNum = 0;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "等待5min \n");

                    p_delayTimer->start(5*60*1000);
                }
                else if(p_Frame3762Base->afn_ == 0x06&&p_Frame3762Base->dt1_==0x10&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    shared_ptr<Afn06F5> p_EventReport_06F5_Up=dynamic_pointer_cast<Afn06F5>(p_Frame3762Base);
                    if(p_EventReport_06F5_Up->report_event_unit_.node_protocol_type_==0x05)
                    {
                        p_timer->stop();
                        uchar reportNum = uchar(p_EventReport_06F5_Up->report_event_unit_.frame_content_.at(0));
                        totalReportNum += reportNum;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到06F5拒绝列表事件上报，本次上报数%1，合计上报数%2")
                                                             .arg(reportNum).arg(totalReportNum));
                        report_flag_06F5 = true;
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到06F5其他事件上报"));
                    }
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "回复确认");
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_Confirm_00F1);
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_SetRefuseStaInforReportClose_05F200_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "设置拒绝节点信息上报为关闭，路由回复确认");
                    p_AbstractScriptHost->updateProgress(ProcessState_Success, "脚本执行成功 【GW-CCO-F019-0001-V01】");
                    emScriptRunState = ScriptSuccess;
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


void Script_RefuseListReport_DefaultClose::processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID)
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
            case ScriptInit:
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

void Script_RefuseListReport_DefaultClose::processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID)
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
            case ScriptInit:
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

void Script_RefuseListReport_DefaultClose::sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame)
{
    if(frame==p_ParaInit_01F2)
    {
        p_ParaInit_01F2->ctrl_field_.dir=kDirDown;
        p_ParaInit_01F2->ctrl_field_.prm=kActive;
        p_ParaInit_01F2->ctrl_field_.comn_type=kHplc;

        p_ParaInit_01F2->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_ParaInit_01F2->info_field_.info_field_down.comu_rate=0;
        p_ParaInit_01F2->info_field_.info_field_down.comu_module_ident=0;

        sendMsgOct=p_ParaInit_01F2->EncodeFrame();
        sendMsgLog=QString("》》参数初始化01F2：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_RefuseNodeReport_05F200)
    {
        p_RefuseNodeReport_05F200->ctrl_field_.dir=kDirDown;
        p_RefuseNodeReport_05F200->ctrl_field_.prm=kActive;
        p_RefuseNodeReport_05F200->ctrl_field_.comn_type=kHplc;

        p_RefuseNodeReport_05F200->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_RefuseNodeReport_05F200->info_field_.info_field_down.comu_rate=0;
        p_RefuseNodeReport_05F200->info_field_.info_field_down.comu_module_ident=0;
        if(emScriptRunState==Wait_SetRefuseStaInforReportOpen_05F200_Finish)
        {
            p_RefuseNodeReport_05F200->refuse_sta_report_flag_=1;
            sendMsgOct=p_RefuseNodeReport_05F200->EncodeFrame();
            sendMsgLog=QString("》》允许拒绝节点信息上报05F200：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
        }
        else if(emScriptRunState==Wait_SetRefuseStaInforReportClose_05F200_Finish)
        {
            p_RefuseNodeReport_05F200->refuse_sta_report_flag_=0;
            sendMsgOct=p_RefuseNodeReport_05F200->EncodeFrame();
            sendMsgLog=QString("》》禁止拒绝节点信息上报05F200：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
        }
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

void Script_RefuseListReport_DefaultClose::sendSrcMsg(DvcType dvcType, int dvcId, QByteArray msg)
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


void Script_RefuseListReport_DefaultClose::timer_timeoutProc()
{
    p_timer->stop();
    switch(emScriptRunState)
    {
        case Wait_BuildNetFinish_Whole:
        {
            p_CtrInfoList->at(0)->inNetResult=false;
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("Wait_BuildNetFinish_Whole 全网组网成功率：%1% 【GW-CCO-F019-0001-V01】").arg(p_CtrInfoList->at(0)->inNetSuccessRate*100));
            break;
        }
        case Wait_SetParaInit_01F2_Finish:
        {
            if(++tryTimes>=3)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_SetParaInit_01F2_Finish timeout 【GW-CCO-F019-0001-V01】");
            }
            else
            {
                sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, INSIGNIFICANCE, p_ParaInit_01F2);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "再次发送--参数初始化，等待--确认");
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            }
            break;
        }
        case Wait_SetRefuseStaInforReportOpen_05F200_Finish:
        {
            if(++tryTimes>=3)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_SetRefuseStaInforReportOpen_05F200_Finish timeout 【GW-CCO-F019-0001-V01】");
            }
            else
            {
                totalReportNum = 0;
                emScriptRunState = Wait_SetRefuseStaInforReportOpen_05F200_Finish;
                sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, INSIGNIFICANCE, p_RefuseNodeReport_05F200);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "再次发送--设置拒绝节点信息上报打开（05F200），等待--回复");
                p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
            }
            break;
        }
        case Wait_SetRefuseStaInforReportClose_05F200_Finish:
        {
            if(++tryTimes>=3)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_SetRefuseStaInforReportClose_05F200_Finish timeout 【GW-CCO-F019-0001-V01】");
            }
            else
            {
                totalReportNum = 0;
                emScriptRunState = Wait_SetRefuseStaInforReportClose_05F200_Finish;
                sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, INSIGNIFICANCE, p_RefuseNodeReport_05F200);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "再次发送--设置拒绝节点信息上报打开（05F200），等待--回复");
                p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
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

void Script_RefuseListReport_DefaultClose::maxAllowTimer_timeoutProc()
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

void Script_RefuseListReport_DefaultClose::delayTimer_timeoutProc()
{
    p_delayTimer->stop();
    switch(emScriptRunState)
    {
        case Wait_BuildNetFinish_Whole:
        {
            break;
        }
        case Wait_SetParaInit_01F2_Finish:
        {
            if(report_flag_06F5 == false)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "1min内未收到拒绝节点信息上报 \n");

                tryTimes = 0;
                totalReportNum = 0;
                emScriptRunState = Wait_SetRefuseStaInforReportOpen_05F200_Finish;
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--允许拒绝节点信息上报（05F200），等待--回复");
                sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, INSIGNIFICANCE, p_RefuseNodeReport_05F200);
                p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
            }
            else
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--禁止拒绝节点信息上报（05F200），等待--回复");
                sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, INSIGNIFICANCE, p_RefuseNodeReport_05F200);

                p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_SetParaInit_01F2_Finish 拒绝节点信息上报，默认关闭状态下收到06F5上报 【GW-CCO-F019-0001-V01】");
            }
            break;
        }
        case Wait_SetRefuseStaInforReportOpen_05F200_Finish:
        {
            if(report_flag_06F5 == false)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--禁止拒绝节点信息上报（05F200），等待--回复");
                sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, INSIGNIFICANCE, p_RefuseNodeReport_05F200);

                p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_SetParaInit_01F2_Finish 拒绝节点信息上报，开启状态下未收到06F5上报 【GW-CCO-F019-0001-V01】");
            }
            else
            {
                tryTimes = 0;
                totalReportNum = 0;
                emScriptRunState = Wait_SetRefuseStaInforReportClose_05F200_Finish;
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--禁止拒绝节点信息上报（05F200），等待--回复");

                sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, INSIGNIFICANCE, p_RefuseNodeReport_05F200);
                p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
            }
            break;
        }
        default:
        {
            break;
        }
    }
}

