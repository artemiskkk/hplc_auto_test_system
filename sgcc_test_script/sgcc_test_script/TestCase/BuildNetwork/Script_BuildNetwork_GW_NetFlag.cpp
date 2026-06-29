#include "Script_BuildNetwork_GW_NetFlag.h"

Script_BuildNetwork_GW_NetFlag::Script_BuildNetwork_GW_NetFlag(QObject *parent) : QObject(parent)
{
    emScriptRunState=ScriptInit;
    p_CtrInfoList=make_shared<QList<shared_ptr<CtrInfo>>>();
    p_BuildNetwork_GW=new BuildNetwork_GW();

    p_Frame3762Helper=make_shared<Frame3762Helper>();
    p_Frame645Helper=make_shared<Frame645Helper>();
    p_MeterAddrResp_93=make_shared<RspsNormal_ReadAddr_0x93>(addr,6);
    p_FrameOOPHelper=make_shared<FrameOOPHelper>();
    p_GetResponseNormal_ReadAddr=make_shared<GetResponseNormal>();

    p_Afn01F2=make_shared<qgdw_3762_protocol::Afn01F2>();
    p_Afn11F1=make_shared<qgdw_3762_protocol::Afn11F1>();
    p_Afn10F4=make_shared<qgdw_3762_protocol::Afn10F4>();


    p_timer=new QTimer(this);
    p_maxAllowTimer=new QTimer(this);
    p_delayTimer=new QTimer(this);
    connect(p_timer,SIGNAL(timeout()),this,SLOT(timer_timeoutProc()));
    connect(p_maxAllowTimer,SIGNAL(timeout()),this,SLOT(maxAllowTimer_timeoutProc()));
    connect(p_delayTimer,SIGNAL(timeout()),this,SLOT(delayTimer_timeoutProc()));
}
Script_BuildNetwork_GW_NetFlag::~Script_BuildNetwork_GW_NetFlag()
{
    p_BuildNetwork_GW->initBuildNetWork();
    if(needPowerOff==true)//断电处理
        powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost,true);
}
void Script_BuildNetwork_GW_NetFlag::setHost(AbstractScriptHost *host)
{
    p_AbstractScriptHost=host;
    p_BuildNetwork_GW->setHost(host);
}
void Script_BuildNetwork_GW_NetFlag::addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq)
{
    p_CtrInfoList->clear();
    comnAddAddrsInfo(p_CtrInfoList, p_ConcentratorInfoList, p_MeterInfoList, p_SchemeCfgInfoList);
    uchar dstFreq=freq&0x0f;

    uchar dstPrtcl=uchar(freq)>>4;
    if(dstPrtcl==0x03)
        setMeterAddrsPrtcl(p_CtrInfoList,dstPrtcl);

    p_BuildNetwork_GW->setCtrInfoListAndFreq(p_CtrInfoList, dstFreq);

}
bool Script_BuildNetwork_GW_NetFlag::config(const QMap<QString,QString> *paraDic)
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
        if(paraDic->keys().contains("loopTimes"))
        {
            this->testCircleTimes = (*paraDic)["loopTimes"].toUShort();
        }
        else
        {
            this->testCircleTimes=10;
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
void Script_BuildNetwork_GW_NetFlag::execute()
{

    p_AbstractScriptHost->updateProgress(ProcessState_Start, "Test start!");
    p_AbstractScriptHost->updateProgress(ProcessState_Start, "******************************************************");

    userCaseIds=QString("[GW-CCO-F002-0008-V01],");

    QString script_ID,script_Desc,script_CheckKeyPoint;
    script_ID=QString("用例ID：GW-CCO-F002-0008-V01 有档案但无节点入网，不置组网完成标志 \n\n");

    script_Desc+=QString("等待组网完成 Wait_BuildNetFinish_Whole \n");
    script_Desc+=QString("设置01F2参数初始化路由 Wait_RouterParaInit_01F2_Finish \n");
    script_Desc+=QString("设置11F1命令，添加从节点信息(随机节点 实际环境不存在) Wait_AddNodes_11F1_Finish \n");
    script_Desc+=QString("下发10F4命令，查询路由运行状态信息(周期性查询-期望组网完成标志始终为未完成) Wait_QueryRouterRunMode_10F4_Finish \n\n");

    script_CheckKeyPoint=QString("检查点1: 有节点但始终不入网时，路由组网完成状态：未完成 \n");

    p_AbstractScriptHost->updateProgress(ProcessState_Processing, script_ID+script_Desc+script_CheckKeyPoint);
    p_AbstractScriptHost->updateProgress(ProcessState_Start, "******************************************************");

    if(needBuildNet==true)//场景2-4
    {
#ifdef SENCE4
        p_BuildNetwork_GW->needRebuildNetwork=true;
#endif
        p_BuildNetwork_GW->execute();//执行组网通用脚本
        emScriptRunState=Wait_BuildNetFinish_Whole;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "等待全网组网完成");
    }
    else//场景1
    {
        tryTimes=0;
        index=0;
        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,index,p_Afn01F2);//make_shared<void>()应该替换成实际的376.2命令
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--参数初始化命令，等待--回复");
        emScriptRunState=Wait_RouterParaInit_01F2_Finish;
        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
    }
    p_maxAllowTimer->start((timerForReachThresld+timerAfterReachThresld)*1000);//此处要启动脚本的最大执行时间定时器
}
void Script_BuildNetwork_GW_NetFlag::stop()
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
void Script_BuildNetwork_GW_NetFlag::processMsg(DvcType dvcType, int id, uchar *data, int datalen)
{
    if(p_CtrInfoList->size()==0)
        return;
    if(emScriptRunState==Wait_BuildNetFinish_Whole)//场景2-4
    {
#ifdef SENCE2
        if(p_BuildNetwork_GW->emScriptRunState<=Wait_00F1_for_05F16_SetFreq)//场景2 关注p_BuildNetwork_GW->startBuildNetFlag标志,开始组网标志
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
            tryTimes=0;
            index=0;
            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,index,p_Afn01F2);//make_shared<void>()应该替换成实际的376.2命令
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--命令，等待--回复");
            emScriptRunState=Wait_RouterParaInit_01F2_Finish;
            p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
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
void Script_BuildNetwork_GW_NetFlag::processCtrlDvcRes(DvcType dvcType, QList<int> idList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params)
{
    //处理工装操作命令，如12V断上电，复位，设置波特率之类的
    //    Q_UNUSED(dvcType)
    //    Q_UNUSED(idList)
    //    Q_UNUSED(ctrlCmdType)
    //    Q_UNUSED(isSucs)
    //    Q_UNUSED(params)

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
    default:
    {
        break;
    }
    }
}

void Script_BuildNetwork_GW_NetFlag::processMsgFromCCO(DvcType dvcType, int dvcId)
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
        case Wait_RouterParaInit_01F2_Finish:
        {
            //示例
            if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "参数初始化回复确认！");

                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_Afn11F1);
                emScriptRunState=Wait_AddNodes_11F1_Finish;
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--添加从节点地址（11F1），等待--回复");
                p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
            }
            else  if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x02&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                emScriptRunState=ScriptSuccess;
                p_AbstractScriptHost->updateProgress(ProcessState_Failed,"Wait_RouterParaInit_01F2_Finish ,期望[回复确认]，实际[回复否认]，"+userCaseIds);

            }
            else  if(p_Frame3762Base->afn_ == 0x03&&p_Frame3762Base->dt1_==0x02&&p_Frame3762Base->dt2_==0x01&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                emScriptRunState=ScriptSuccess;
                p_AbstractScriptHost->updateProgress(ProcessState_Failed,"Wait_RouterParaInit_01F2_Finish，期望[回复确认]，实际[回复03F10]，"+userCaseIds);

            }
            else//收到其它报文的处理
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case Wait_AddNodes_11F1_Finish:
        {
            //示例
            if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "添加从节点回复确认！");

                p_delayTimer->start(120*1000);
                emScriptRunState=Wait_QueryRouterRunMode_10F4_Finish;

            }
            else  if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x02&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
//                p_timer->stop();
//                emScriptRunState=ScriptSuccess;
//                p_AbstractScriptHost->updateProgress(ProcessState_Failed,"Wait_AddNodes_11F1_Finish 期望[回复确认]，实际[11F1回复否认]，"+userCaseIds);
                p_timer->stop();
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "添加从节点回复确认！");

                p_delayTimer->start(120*1000);
                emScriptRunState=Wait_QueryRouterRunMode_10F4_Finish;

            }
            else  if(p_Frame3762Base->afn_ == 0x03&&p_Frame3762Base->dt1_==0x02&&p_Frame3762Base->dt2_==0x01&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
//                p_timer->stop();
//                emScriptRunState=ScriptSuccess;
//                p_AbstractScriptHost->updateProgress(ProcessState_Failed,"Wait_AddNodes_11F1_Finish 期望[回复确认]，实际[11F1回复03F10]，"+userCaseIds);
                p_timer->stop();
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "添加从节点回复确认！");

                p_delayTimer->start(120*1000);
                emScriptRunState=Wait_QueryRouterRunMode_10F4_Finish;

            }
            else//收到其它报文的处理
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case Wait_QueryRouterRunMode_10F4_Finish:
        {
            if(p_Frame3762Base->afn_ == 0x10&&p_Frame3762Base->dt1_==0x08&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();

                shared_ptr<qgdw_3762_protocol::Afn10F4> p_Afn10F4=dynamic_pointer_cast<qgdw_3762_protocol::Afn10F4>(p_Frame3762Base);
                if(p_Afn10F4->router_operate_state_unit_.operate_state_word_.router_complete_flag_==0x00)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("第%1轮:").arg(++runCircleTimes)+"10F4回复正常，期望回复组网标志位未完成，实际回复组网标志位未完成");
                }
                else if(p_Afn10F4->router_operate_state_unit_.operate_state_word_.router_complete_flag_==0x01)
                {
                    p_delayTimer->stop();
                    emScriptRunState=ScriptSuccess;
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("第%1轮:").arg(++runCircleTimes)+"Wait_QueryRouterRunMode_10F4_Finish ,期望[回复组网标志位未完成]，实际[回复组网标志位完成]"+userCaseIds);
                    break;
                }
                else
                {
                    break;
                }
                if(runCircleTimes>=testCircleTimes)
                {
                    p_delayTimer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Success, "脚本执行成功,"+userCaseIds);
                }

            }
            else  if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x02&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                emScriptRunState=ScriptSuccess;
                p_AbstractScriptHost->updateProgress(ProcessState_Failed,"Wait_QueryRouterRunMode_10F4_Finish,期望[回复10F4],实际[回复否认],"+userCaseIds);

            }
            else  if(p_Frame3762Base->afn_ == 0x03&&p_Frame3762Base->dt1_==0x02&&p_Frame3762Base->dt2_==0x01&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                emScriptRunState=ScriptSuccess;
                p_AbstractScriptHost->updateProgress(ProcessState_Failed,"Wait_QueryRouterRunMode_10F4_Finish ,期望[回复10F4]，实际[回复03F10],"+userCaseIds);
            }
            else//收到其它报文的处理
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
void Script_BuildNetwork_GW_NetFlag::processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID)
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

        default:
        {
            if(MsgBase_645_ptr->ctrlCode_==READ_ADDR)
            {
                sendMsg(dvcType,dvcId,mtrlID,p_MeterAddrResp_93);
                if(emScriptRunState>Wait_BuildNetFinish_Whole)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed,"分配地址结束且成功后，模块不应重复请求645地址");
                }
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
void Script_BuildNetwork_GW_NetFlag::processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID)
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
                    if(emScriptRunState>Wait_BuildNetFinish_Whole)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed,"分配地址结束且成功后，模块不应重复请求oop地址");
                    }
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
void Script_BuildNetwork_GW_NetFlag::sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame)
{
    /**if(frame==make_shared<void>())//举例
    {
//        p_QueryAfn_03F11->ctrl_field_.dir=kDirDown;
//        p_QueryAfn_03F11->ctrl_field_.prm=kActive;
//        p_QueryAfn_03F11->ctrl_field_.comn_type=kHplc;

//        p_QueryAfn_03F11->info_field_.info_field_down.msg_seq=char(msgSeq++);
//        p_QueryAfn_03F11->info_field_.info_field_down.comu_rate=0;
//        p_QueryAfn_03F11->info_field_.info_field_down.comu_module_ident=0;

//        p_QueryAfn_03F11->afn_code_=afnCodeArray[meterID];

//        sendMsgOct=p_QueryAfn_03F11->EncodeFrame();
        sendMsgLog=QString("》》查询Afn索引03F11：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else **/if(frame==p_MeterAddrResp_93)
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
    else if(frame==p_Afn01F2)
    {
        p_Afn01F2->ctrl_field_.dir=kDirDown;
        p_Afn01F2->ctrl_field_.prm=kActive;
        p_Afn01F2->ctrl_field_.comn_type=kHplc;

        p_Afn01F2->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_Afn01F2->info_field_.info_field_down.comu_rate=0;
        p_Afn01F2->info_field_.info_field_down.comu_module_ident=0;

        sendMsgOct=p_Afn01F2->EncodeFrame();
        sendMsgLog=QString("》》路由参数初始化01F2：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_Afn11F1)
    {
        p_Afn11F1->ctrl_field_.dir=kDirDown;
        p_Afn11F1->ctrl_field_.prm=kActive;
        p_Afn11F1->ctrl_field_.comn_type=kHplc;

        p_Afn11F1->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_Afn11F1->info_field_.info_field_down.comu_rate=0;
        p_Afn11F1->info_field_.info_field_down.comu_module_ident=0;

        NodeParameter node;
        node.protocol_type_=0x02;
        memset(node.node_address_.addr,char(0xDD),6);
        p_Afn11F1->node_num_=1;
        p_Afn11F1->node_parameter_list_.clear();
        p_Afn11F1->node_parameter_list_.append(node);
        sendMsgOct=p_Afn11F1->EncodeFrame();
        sendMsgLog=QString("》》添加从节点11F1：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_Afn10F4)
    {
        p_Afn10F4->ctrl_field_.dir=kDirDown;
        p_Afn10F4->ctrl_field_.prm=kActive;
        p_Afn10F4->ctrl_field_.comn_type=kHplc;

        p_Afn10F4->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_Afn10F4->info_field_.info_field_down.comu_rate=0;
        p_Afn10F4->info_field_.info_field_down.comu_module_ident=0;

        sendMsgOct=p_Afn10F4->EncodeFrame();
        sendMsgLog=QString("》》查询路由运行状态10F4：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }

















    p_AbstractScriptHost->updateProgress(ProcessState_Processing, sendMsgLog);

    uchar *sendMsg=new uchar[uint(sendMsgOct.size())];
    memcpy(sendMsg,reinterpret_cast<uchar*>(sendMsgOct.data()),uint(sendMsgOct.size()));
    p_AbstractScriptHost->sendMsg2Dvc(dvcType,dvcId,sendMsg,sendMsgOct.size());
}
void Script_BuildNetwork_GW_NetFlag::sendSrcMsg(DvcType dvcType, int dvcId, QByteArray msg)
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


void Script_BuildNetwork_GW_NetFlag::timer_timeoutProc()
{
    switch(emScriptRunState)
    {
    case Wait_BuildNetFinish_Whole:
    {
        p_CtrInfoList->at(0)->inNetResult=false;
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("全网组网成功率：%1%").arg(p_CtrInfoList->at(0)->inNetSuccessRate*100));
        break;
    }
    case Wait_RouterParaInit_01F2_Finish:
    {
        p_timer->stop();
        if(++tryTimes>=3)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_RouterParaInit_01F2_Finish timeout!!!");
        }
        else
        {
            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,index,p_Afn01F2);
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--参数初始化命令（01F2），等待--确认");
        }
        break;
    }
    case Wait_AddNodes_11F1_Finish:
    {
        p_timer->stop();
        if(++tryTimes>=3)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_AddNodes_11F1_Finish timeout!!!");
        }
        else
        {
            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,index,p_Afn11F1);
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--添加从节点命令（11F1），等待--确认");
        }
        break;
    }
    case Wait_QueryRouterRunMode_10F4_Finish:
    {
        p_timer->stop();
        if(++tryTimes>=3)
        {
            p_delayTimer->stop();
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_QueryRouterRunMode_10F4_Finish timeout!!!");
        }
        else
        {
            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,index,p_Afn10F4);
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询路由运行状态（10F4），等待--确认");
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

void Script_BuildNetwork_GW_NetFlag::maxAllowTimer_timeoutProc()
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

void Script_BuildNetwork_GW_NetFlag::delayTimer_timeoutProc()
{
    switch(emScriptRunState)
    {
    case Wait_QueryRouterRunMode_10F4_Finish:
    {
        tryTimes=0;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询路由运行状态（10F4），等待--回复");
        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_Afn10F4);
        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
        break;
    }
    default:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==p_delayTimer");
        break;
    }
    }
}
