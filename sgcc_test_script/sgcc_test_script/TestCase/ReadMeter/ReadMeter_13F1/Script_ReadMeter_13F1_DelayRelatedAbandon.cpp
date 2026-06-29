#include "Script_ReadMeter_13F1_DelayRelatedAbandon.h"

Script_ReadMeter_13F1_DelayRelatedAbandon::Script_ReadMeter_13F1_DelayRelatedAbandon(QObject *parent) : QObject(parent)
{
    p_BuildNetwork_GW=new BuildNetwork_GW();
    p_CtrInfoList=make_shared<QList<shared_ptr<CtrInfo>>>();
    p_Frame3762Helper=make_shared<Frame3762Helper>();
    p_Afn12F2=make_shared<Afn12F2>();
    p_Afn13F1=make_shared<Afn13F1>();
    p_Afn14F3=make_shared<Afn14F3>();
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
Script_ReadMeter_13F1_DelayRelatedAbandon::~Script_ReadMeter_13F1_DelayRelatedAbandon()
{
    p_BuildNetwork_GW->initBuildNetWork();
    if(needPowerOff==true)//断电处理
        powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost);
}
void Script_ReadMeter_13F1_DelayRelatedAbandon::setHost(AbstractScriptHost *host)
{
    p_AbstractScriptHost=host;
    p_BuildNetwork_GW->setHost(host);
}
void Script_ReadMeter_13F1_DelayRelatedAbandon::addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq)
{
    p_CtrInfoList->clear();
    comnAddAddrsInfo(p_CtrInfoList, p_ConcentratorInfoList, p_MeterInfoList, p_SchemeCfgInfoList);
    uchar dstFreq=freq&0x0f;

    uchar dstPrtcl=uchar(freq)>>4;
    if(dstPrtcl==0x03)
        setMeterAddrsPrtcl(p_CtrInfoList,dstPrtcl);

    p_BuildNetwork_GW->setCtrInfoListAndFreq(p_CtrInfoList, dstFreq);

}
bool Script_ReadMeter_13F1_DelayRelatedAbandon::config(const QMap<QString,QString> *paraDic)
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
void Script_ReadMeter_13F1_DelayRelatedAbandon::execute()
{
    p_AbstractScriptHost->updateProgress(ProcessState_Start, "Test start!");

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
        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,index,p_Afn12F2);
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送暂停路由命令，等待--回复");
        emScriptRunState=Wait_PauseRouter_12F2_Finish;
        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
    }
    p_maxAllowTimer->start((timerForReachThresld+timerAfterReachThresld)*1000);//此处要启动脚本的最大执行时间定时器
}
void Script_ReadMeter_13F1_DelayRelatedAbandon::stop()
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
void Script_ReadMeter_13F1_DelayRelatedAbandon::processMsg(DvcType dvcType, int id, uchar *data, int datalen)
{
    if(p_CtrInfoList->size()==0)
        return;
    if(emScriptRunState==Wait_BuildNetFinish_Whole)//场景2-4
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
            tryTimes=0;
            index=0;
            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,index,p_Afn12F2);
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送暂停路由命令，等待--回复");
            emScriptRunState=Wait_PauseRouter_12F2_Finish;
            p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
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
void Script_ReadMeter_13F1_DelayRelatedAbandon::processCtrlDvcRes(DvcType dvcType, QList<int> idList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params)
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

void Script_ReadMeter_13F1_DelayRelatedAbandon::processMsgFromCCO(DvcType dvcType, int dvcId)
{
    if(dvcId!=p_CtrInfoList->at(0)->ctrlID)
        return;
    
    QByteArray completeFrame;
    QByteArray& buf = p_CtrInfoList->at(0)->buf;
    bool haveCompleteMsg=extractAndProcess3762Frame(buf,completeFrame);

    while(haveCompleteMsg)
    {
        shared_ptr<Frame3762Base> p_Frame3762Base = nullptr;
        
        try {
            p_Frame3762Base = p_Frame3762Helper->DecodeLocalMsg(&completeFrame,haveCompleteMsg);
        }
        catch(...) {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, 
                QString("3762帧解析异常，跳过此帧: %1").arg(QString(completeFrame.toHex())));
            // 继续处理下一帧
            haveCompleteMsg = extractAndProcess3762Frame(buf, completeFrame);
            continue;
        }
        
        if(p_Frame3762Base==nullptr)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, 
                QString("3762帧解析失败，跳过此帧: %1").arg(QString(completeFrame.toHex())));
            // 继续处理下一帧
            haveCompleteMsg = extractAndProcess3762Frame(buf, completeFrame);
            continue;
        }
        //检查点处理
        if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x02&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp&&uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq)==msgSeq)
        {
            p_timer->stop();
            p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("收到CCO回复否认，运行状态为%1").arg(runStateMetaEnum.valueToKey(emScriptRunState)));
            p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("收到CCO回复否认，运行状态为%1").arg(runStateMetaEnum.valueToKey(emScriptRunState)));
            return;
        }
        else if(p_Frame3762Base->afn_ == 0x03&&p_Frame3762Base->dt1_==0x02&&p_Frame3762Base->dt2_==0x01&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("收到CCO上报03F10，疑似复位，运行状态为%1").arg(runStateMetaEnum.valueToKey(emScriptRunState)));
            p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("收到CCO上报03F10，疑似复位，运行状态为%1").arg(runStateMetaEnum.valueToKey(emScriptRunState)));
            return;
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
            case Wait_PauseRouter_12F2_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp&&uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq)==msgSeq)
                {
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("收到暂停路由回复确认"));
                    emScriptRunState=Wait_13F1_NoReply14F3_Finish;
                    index=0;
                    tryTimes=0;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("抄表13F1，等待回复，运行状态%1").arg(runStateMetaEnum.valueToKey(emScriptRunState)));
                    sendMsg(dvcType,p_CtrInfoList->at(0)->ctrlID,index,p_Afn13F1);
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                    t.start();
                }
                else//收到其它报文的处理
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_13F1_NoReply14F3_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x13&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp&&uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq)==msgSeq)
                {
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("收到抄表13F1回复"));
                    shared_ptr<Afn13F1> ptr=dynamic_pointer_cast<Afn13F1>(p_Frame3762Base);
                    if(ptr->data_field_up_.frame_length_==0)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("收到抄表13F1回复数据为空，运行状态%1").arg(runStateMetaEnum.valueToKey(emScriptRunState)));
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("收到抄表13F1回复数据为空，运行状态%1"+testCase).arg(runStateMetaEnum.valueToKey(emScriptRunState)));
                        return;
                    }
                    else
                    {
                        if(t.elapsed()/1000<10)
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("14F3在未收到下行报文时，10s之后会载波抄读，因此收到13F1的回复时间是超过10s的"));
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("收到抄表13F1回复数据时间在10s内，不符合要求，运行状态%1").arg(runStateMetaEnum.valueToKey(emScriptRunState)));
                            p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("收到抄表13F1回复数据时间在10s内，不符合要求，运行状态%1"+testCase).arg(runStateMetaEnum.valueToKey(emScriptRunState)));
                            return;
                        }
                        if(++index<p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().size())
                        {
                            emScriptRunState=Wait_13F1_NoReply14F3_Finish;
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("抄表13F1，等待回复，运行状态%1").arg(runStateMetaEnum.valueToKey(emScriptRunState)));
                            sendMsg(dvcType,p_CtrInfoList->at(0)->ctrlID,index,p_Afn13F1);
                            p_timer->start(MAX_13F1_TIMEOUT_TIME*1000);
                            t.start();
                        }
                        else
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("14F3不回复可以正常13F1抄读"));
                            emScriptRunState=Wait_13F1_Wrong14F3_Finish;
                            index=0;
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("抄表13F1，等待回复，运行状态%1").arg(runStateMetaEnum.valueToKey(emScriptRunState)));
                            sendMsg(dvcType,p_CtrInfoList->at(0)->ctrlID,index,p_Afn13F1);
                            p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                        }
                    }
                }
                else if(p_Frame3762Base->afn_==0x14&&p_Frame3762Base->dt1_==0x04&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    shared_ptr<Afn14F3> p_DelayCorrect_14F3_Up=dynamic_pointer_cast<Afn14F3>(p_Frame3762Base);
                    if(readMeterMsg!=p_DelayCorrect_14F3_Up->request_unit_up_.frame_content_)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("14F3的抄读报文错误，运行状态%1").arg(runStateMetaEnum.valueToKey(emScriptRunState)));
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("14F3的抄读报文错误，运行状态%1"+testCase).arg(runStateMetaEnum.valueToKey(emScriptRunState)));
                    }
                    else
                    {
                        p_timer->start(MAX_13F1_TIMEOUT_TIME*1000);
                    }
                }
                else//收到其它报文的处理
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_13F1_Wrong14F3_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x13&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp&&uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq)==msgSeq)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("收到抄表13F1回复，运行状态%1").arg(runStateMetaEnum.valueToKey(emScriptRunState)));
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("收到抄表13F1回复，不符合要求，运行状态%1"+testCase).arg(runStateMetaEnum.valueToKey(emScriptRunState)));
                    return;
//                    p_timer->stop();
//                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("收到抄表13F1回复"));
//                    shared_ptr<Afn13F1> ptr=dynamic_pointer_cast<Afn13F1>(p_Frame3762Base);
//                    if(ptr->data_field_up_.frame_length_!=0)
//                    {
//                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("收到抄表13F1回复数据不为空，运行状态%1").arg(runStateMetaEnum.valueToKey(emScriptRunState)));
//                        p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("收到抄表13F1回复数据不为空，不符合要求，运行状态%1").arg(runStateMetaEnum.valueToKey(emScriptRunState)));
//                        return;
//                    }
//                    else
//                    {
//                        if(++index<p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().size())
//                        {
//                            emScriptRunState=Wait_13F1_Wrong14F3_Finish;
//                            p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("抄表13F1，等待回复，运行状态%1").arg(runStateMetaEnum.valueToKey(emScriptRunState)));
//                            sendMsg(dvcType,p_CtrInfoList->at(0)->ctrlID,index,p_Afn13F1);
//                            p_timer->start(MAX_13F1_TIMEOUT_TIME*1000);
//                        }
//                        else
//                        {
//                            p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("13F1抄读回复数据为空-符合要求"));
//                            p_AbstractScriptHost->updateProgress(ProcessState_Success,QString("13F1抄读-通信延时放弃测试成功"));
//                        }
//                    }
                }
                else if(p_Frame3762Base->afn_==0x14&&p_Frame3762Base->dt1_==0x04&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    shared_ptr<Afn14F3> p_DelayCorrect_14F3_Up=dynamic_pointer_cast<Afn14F3>(p_Frame3762Base);
                    if(readMeterMsg!=p_DelayCorrect_14F3_Up->request_unit_up_.frame_content_)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("14F3的抄读报文错误，运行状态%1").arg(runStateMetaEnum.valueToKey(emScriptRunState)));
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("14F3的抄读报文错误，运行状态%1"+testCase).arg(runStateMetaEnum.valueToKey(emScriptRunState)));
                    }
                    else
                    {
                        p_Afn14F3->info_field_.info_field_down.msg_seq=p_DelayCorrect_14F3_Up->info_field_.info_field_up.msg_seq;
                        sendMsg(dvcType,p_CtrInfoList->at(0)->ctrlID,index,p_Afn14F3);
                        p_delayTimer->start(MAX_13F1_TIMEOUT_TIME*1000);
                    }
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
        
        // 继续处理下一帧
        haveCompleteMsg = extractAndProcess3762Frame(buf, completeFrame);
    }
}
void Script_ReadMeter_13F1_DelayRelatedAbandon::processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID)
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
void Script_ReadMeter_13F1_DelayRelatedAbandon::processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID)
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
void Script_ReadMeter_13F1_DelayRelatedAbandon::sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame)
{
    if(frame==p_Afn12F2)
    {
        p_Afn12F2->ctrl_field_.dir=kDirDown;
        p_Afn12F2->ctrl_field_.prm=kActive;
        p_Afn12F2->ctrl_field_.comn_type=kHplc;
        msgSeq++;
        p_Afn12F2->info_field_.info_field_down.msg_seq=char(msgSeq);
        p_Afn12F2->info_field_.info_field_down.comu_rate=0;
        p_Afn12F2->info_field_.info_field_down.comu_module_ident=0;

        sendMsgOct=p_Afn12F2->EncodeFrame();
        sendMsgLog=QString("》》暂停路由12F2：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_Afn13F1)
    {
        if(index>=p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size())
        {
            index=0;
            return;
        }
        uchar tmpAddr[6];
        memcpy(tmpAddr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(index)->mtrAddr,6);
        uchar comPrtclType=p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(index)->prtcl;
        msgSeq++;
        if(comPrtclType==0x02)
        {
            uchar CrntPosEneTotal[4]={0x00,0x00,0x01,0x00}; //DI0_DI3
            shared_ptr<Rqst_ReadData_0x11> p_ReadData_0x11=make_shared<Rqst_ReadData_0x11>(addr,4,0);
            //reverseAddr(tmpAddr, 6);
            memcpy(p_ReadData_0x11->addr_,tmpAddr,6);
            memcpy(p_ReadData_0x11->di,CrntPosEneTotal,4);
            QByteArray msg645=p_ReadData_0x11->EncodeFrame();

            p_Afn13F1->data_field_down_.delay_tag_=0x01;//精度1s
            p_Afn13F1->data_field_down_.sub_node_num_=0x00;
            p_Afn13F1->data_field_down_.protocol_type_=0x02;
            p_Afn13F1->data_field_down_.frame_content_=msg645;
            p_Afn13F1->data_field_down_.frame_length_=uchar(msg645.size());
            readMeterMsg=msg645;
            p_Afn13F1->ctrl_field_.dir=kDirDown;
            p_Afn13F1->ctrl_field_.prm=kActive;
            p_Afn13F1->ctrl_field_.comn_type=kHplc;

            p_Afn13F1->info_field_.info_field_down.msg_seq=char(msgSeq);
            p_Afn13F1->info_field_.info_field_down.comu_rate=0;
            p_Afn13F1->info_field_.info_field_down.comu_module_ident=1;

            uchar tmpCcoAddr[6];
            memcpy(tmpCcoAddr,p_CtrInfoList->at(0)->ccoAddr,6);
            memcpy(p_Afn13F1->address_field_.src_addr,tmpCcoAddr,6);
            memcpy(p_Afn13F1->address_field_.dst_addr,tmpAddr,6);

            sendMsgOct=p_Afn13F1->EncodeFrame();
            sendMsgLog=QString("》》监控从节点13F1,抄读645电表：%1\n").arg(QString(sendMsgOct.toHex()));
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

            p_Afn13F1->data_field_down_.delay_tag_=0x01;//精度1s
            p_Afn13F1->data_field_down_.sub_node_num_=0x00;
            p_Afn13F1->data_field_down_.protocol_type_=0x03;
            p_Afn13F1->data_field_down_.frame_content_=msg698;
            p_Afn13F1->data_field_down_.frame_length_=uchar(msg698.size());
            readMeterMsg=msg698;
            p_Afn13F1->ctrl_field_.dir=kDirDown;
            p_Afn13F1->ctrl_field_.prm=kActive;
            p_Afn13F1->ctrl_field_.comn_type=kHplc;

            p_Afn13F1->info_field_.info_field_down.msg_seq=char(msgSeq);
            p_Afn13F1->info_field_.info_field_down.comu_rate=0;
            p_Afn13F1->info_field_.info_field_down.comu_module_ident=1;

            uchar tmpCcoAddr[6];
            memcpy(tmpCcoAddr,p_CtrInfoList->at(0)->ccoAddr,6);
            memcpy(p_Afn13F1->address_field_.src_addr,tmpCcoAddr,6);
            memcpy(p_Afn13F1->address_field_.dst_addr,tmpAddr,6);

            sendMsgOct=p_Afn13F1->EncodeFrame();
            sendMsgLog=QString("》》监控从节点13F1,抄读OOP电表：%1\n").arg(QString(sendMsgOct.toHex()));
        }
        else
            return;
    }
    else if(frame==p_Afn14F3)
    {
        p_Afn14F3->ctrl_field_.dir=kDirDown;
        p_Afn14F3->ctrl_field_.prm=kActive;
        p_Afn14F3->ctrl_field_.comn_type=kHplc;

        p_Afn14F3->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_Afn14F3->info_field_.info_field_down.comu_rate=0;
        p_Afn14F3->info_field_.info_field_down.comu_module_ident=0;

        p_Afn14F3->request_unit_down_.frame_length_=0;

        sendMsgOct=p_Afn14F3->EncodeFrame();
        sendMsgLog=QString("》》请求依通信延时修正通信数据14F3：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
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
void Script_ReadMeter_13F1_DelayRelatedAbandon::sendSrcMsg(DvcType dvcType, int dvcId, QByteArray msg)
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

void Script_ReadMeter_13F1_DelayRelatedAbandon::timer_timeoutProc()
{
    switch(emScriptRunState)
    {
        case Wait_BuildNetFinish_Whole:
        {
            p_CtrInfoList->at(0)->inNetResult=false;
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("全网组网成功率：%1%").arg(p_CtrInfoList->at(0)->inNetSuccessRate*100));
            break;
        }
        case Wait_PauseRouter_12F2_Finish:
        {
            p_timer->stop();
            if(++tryTimes>=3)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("%1 timeout!!!").arg(runStateMetaEnum.valueToKey(emScriptRunState)));
            }
            else
            {
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,nullptr);
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
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

void Script_ReadMeter_13F1_DelayRelatedAbandon::maxAllowTimer_timeoutProc()
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

void Script_ReadMeter_13F1_DelayRelatedAbandon::delayTimer_timeoutProc()
{
    switch(emScriptRunState)
    {
        case Wait_BuildNetFinish_Whole:
        {
            break;
        }
        case Wait_13F1_Wrong14F3_Finish:
        {
            p_delayTimer->stop();
            p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("未收到抄表13F1回复，符合要求"));
            if(++index<p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().size())
            {
                emScriptRunState=Wait_13F1_Wrong14F3_Finish;
                p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("抄表13F1，等待回复，运行状态%1").arg(runStateMetaEnum.valueToKey(emScriptRunState)));
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,index,p_Afn13F1);
                p_delayTimer->start(MAX_13F1_TIMEOUT_TIME*1000);
            }
            else
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("14F3报文错误时，13F1抄读不回复-符合要求"));
                if(p_maxAllowTimer!=nullptr)
                    p_maxAllowTimer->stop();
                emScriptRunState = ScriptSuccess;
                p_AbstractScriptHost->updateProgress(ProcessState_Success,QString("脚本执行成功"+testCase));
            }
            break;
        }
        default:
        {
            break;
        }
    }
}
bool Script_ReadMeter_13F1_DelayRelatedAbandon::extractAndProcess3762Frame(QByteArray& buf,QByteArray& completeFrame)
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