#include "Script_BroadcastTime_LocalProtocolTest.h"


Script_BroadcastTime_LocalProtocolTest::Script_BroadcastTime_LocalProtocolTest(QObject *parent) : QObject(parent)
{
    p_BuildNetwork_GW=new BuildNetwork_GW();
    p_CtrInfoList=make_shared<QList<shared_ptr<CtrInfo>>>();
    p_Frame3762Helper=make_shared<Frame3762Helper>();
    p_Broadcast_05F3 = make_shared<Afn05F3>();

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

Script_BroadcastTime_LocalProtocolTest::~Script_BroadcastTime_LocalProtocolTest()
{
    p_BuildNetwork_GW->initBuildNetWork();
    if(needPowerOff==true)//断电处理
        powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost,true);
}

double Script_BroadcastTime_LocalProtocolTest::calSuccessRate()
{
    // 计算成功率
    double successCount=0.0;
    for(int i=0;i<meterInfoList.size();i++)
    {
        if(meterInfoList.at(i).readFlag==true)
            successCount++;
    }
    return successCount/double(meterInfoList.size());
}

int Script_BroadcastTime_LocalProtocolTest::getMeterInfo(Address address)
{
    for(int i=0; i<meterInfoList.size(); i++)
    {
        if(address == meterInfoList.at(i).meterNo)
        {
            return i;
        }
    }
    return -1;
}

QString Script_BroadcastTime_LocalProtocolTest::getFailMeterNo()
{
    QString failMeterNo;
    for(int i=0;i<meterInfoList.size();i++)
    {
        if(meterInfoList.at(i).readFlag==false)
            failMeterNo+=QString(QByteArray(meterInfoList.at(i).meterNo.addr,6).toHex())+";";
    }
    return failMeterNo;
}

void Script_BroadcastTime_LocalProtocolTest::setHost(AbstractScriptHost *host)
{
    p_AbstractScriptHost=host;
    p_BuildNetwork_GW->setHost(host);
}

void Script_BroadcastTime_LocalProtocolTest::addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq)
{
    p_CtrInfoList->clear();
    comnAddAddrsInfo(p_CtrInfoList, p_ConcentratorInfoList, p_MeterInfoList, p_SchemeCfgInfoList);
    uchar dstFreq=freq&0x0f;

    uchar dstPrtcl=uchar(freq)>>4;
    if(dstPrtcl==0x03)
        setMeterAddrsPrtcl(p_CtrInfoList,dstPrtcl);

    p_BuildNetwork_GW->setCtrInfoListAndFreq(p_CtrInfoList, dstFreq);

}

bool Script_BroadcastTime_LocalProtocolTest::config(const QMap<QString,QString> *paraDic)
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


void Script_BroadcastTime_LocalProtocolTest::execute()
{
    QString step_desc = "流程描述: \r";
    step_desc += "0.仅使用工装单通+三通\r";
    step_desc += "1.组网通用流程，SENCE4(需要重新组网) \r";
    step_desc += "2.下发05F3广播校时报文，通信协议0x04，数据长度为18，广播报文为645报文（长度18），广播地址全9，校时时间为当前时间 Wait_SendBroadcast_05F3_Finish_1 \r";
    step_desc += "3.下发05F3广播校时报文，通信协议0x02，数据长度为17，广播报文为645报文（长度18），广播地址全9，校时时间为当前时间 Wait_SendBroadcast_05F3_Finish_2 \r";
    step_desc += "4.下发05F3广播校时报文，通信协议0x02，数据长度为19，广播报文为645报文（长度18），广播地址全9，校时时间为当前时间 Wait_SendBroadcast_05F3_Finish_3 \r";
    step_desc += "5.下发05F3广播校时报文，通信协议0x02，数据长度为255，广播报文为14条645报文（长度18字节）及补零（长度255），广播地址全9，校时时间为当前时间 Wait_SendBroadcast_05F3_Finish_4 \r";
    step_desc += "6.下发05F3广播校时报文，通信协议0x02，数据长度为0，广播报文为645报文（长度18），广播地址全9，校时时间为当前时间 Wait_SendBroadcast_05F3_Finish_5 \n";

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
        tryTimes=0;
        index=0;
//        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,index,make_shared<void>());//make_shared<void>()应该替换成实际的376.2命令
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--命令，等待--回复");
        emScriptRunState = Wait_SendBroadcast_05F3_Finish_1;
        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
    }
    p_maxAllowTimer->start((timerForReachThresld+timerAfterReachThresld)*1000);//此处要启动脚本的最大执行时间定时器
}

void Script_BroadcastTime_LocalProtocolTest::stop()
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

void Script_BroadcastTime_LocalProtocolTest::processMsg(DvcType dvcType, int id, uchar *data, int datalen)
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
            if(p_BuildNetwork_GW->emScriptRunState==BuildNetFinish && p_BuildNetwork_GW->buildNetworkResultFlag==true)
            {
                emScriptRunState = Wait_SendBroadcast_05F3_Finish_1;
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "进行第1轮广播校时测试--下发05F3广播校时报文，通信协议0x04，数据长度为18，广播报文为645报文（长度18），广播地址全9，校时时间为当前时间");
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--广播校时（05F3），等待--确认");

                tryTimes=0;
                meterInfoInit();
                sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, INSIGNIFICANCE, p_Broadcast_05F3);
                p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
            }
        }
    }
    else//当测试脚本开始执行脚本自己的操作时，均从此进入
    {
        QByteArray tmpRecvTempData=QByteArray::fromRawData(reinterpret_cast<char*>(data),datalen);
        QByteArray recvTempData;
        recvTempData.append(tmpRecvTempData);
        delete[] data;

        if(dvcType==CCO_GW || dvcType==CCO_NW)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到来自CCO报文：%1").arg(QString(recvTempData.toHex())));

            if(dvcType == p_CtrInfoList->at(0)->slotPosition && id == p_CtrInfoList->at(0)->dvcId)
            {
                p_CtrInfoList->at(0)->buf.append(recvTempData);
                processMsgFromCCO(dvcType,id);
            }
        }
        else if(dvcType==SingleSTA || dvcType==ThreeSTA)
        {
            if(dvcType==SingleSTA)
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到来自单通报文：%1").arg(QString(recvTempData.toHex())));
            else
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到来自三通报文：%1").arg(QString(recvTempData.toHex())));

                if(emScriptRunState == Wait_SendBroadcast_05F3_Finish_1 || emScriptRunState == Wait_SendBroadcast_05F3_Finish_4)
                {
                    for(int i=0; i<p_CtrInfoList->at(0)->keyList.size(); i++)
                    {
                        if(dvcType == p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->value(p_CtrInfoList->at(0)->keyList.at(i))->slotPosition
                                && id == p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->value(p_CtrInfoList->at(0)->keyList.at(i))->dvcId)
                        {
                            (*p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList)[p_CtrInfoList->at(0)->keyList.at(i)]->buf645.append(recvTempData);
                            processMsgFromMeter645(dvcType,id,p_CtrInfoList->at(0)->keyList.at(i));
                            break;
                        }
                    }
                    return;
                }
            }

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

void Script_BroadcastTime_LocalProtocolTest::processCtrlDvcRes(DvcType dvcType, QList<int> idList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params)
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
        case BroadcastInit:
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


void Script_BroadcastTime_LocalProtocolTest::processMsgFromCCO(DvcType dvcType, int dvcId)
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

        uchar dtValue3762;
        switch(emScriptRunState)
        {
            case BroadcastInit:
            {
                break;
            }
            case Wait_BuildNetFinish_Whole:
            {
                break;
            }
            case Wait_SendBroadcast_05F3_Finish_1:
            {
                dtValue3762 = get3762Dt(p_Frame3762Base->dt1_, p_Frame3762Base->dt2_);
                if(p_Frame3762Base->afn_ == 0x00 && p_Frame3762Base->dt1_==0x01 && p_Frame3762Base->dt2_==0x00 && p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    // 路由上行回复确认
                    p_timer->stop();//接收到一条完整报文
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到路由上行回复确认，等待电表校时...");

                    // 等待校时完成，虚拟表收到校时报文
                    p_delayTimer->start(70*1000);

                }
                else if(p_Frame3762Base->afn_ == 0x00&& dtValue3762 == 2 &&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_SendBroadcast_05F3_Finish_1 广播校时路由回复否认 【GW-CCO-F008-0002-V01】");
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_SendBroadcast_05F3_Finish_2:
            {
                dtValue3762 = get3762Dt(p_Frame3762Base->dt1_, p_Frame3762Base->dt2_);
                if(p_Frame3762Base->afn_ == 0x00 && p_Frame3762Base->dt1_==0x01 && p_Frame3762Base->dt2_==0x00 && p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    // 路由上行回复确认
                    p_timer->stop();//接收到一条完整报文
                    p_AbstractScriptHost->updateProgress(ProcessState_Success, "cco 不支持判断广播校时异常帧 测试通过");
//                  p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_SendBroadcast_05F3_Finish_2 发送异常广播校时,路由回复确认 【GW-CCO-F008-0002-V01】");

                }
                else if(p_Frame3762Base->afn_ == 0x00&& dtValue3762 == 2 &&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    // 路由上行回复否认
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到路由上行回复确否认，等待70s \n");

                    // 需要超时器，判断虚拟表是否收到校时报文
                    p_delayTimer->start(70*1000);
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_SendBroadcast_05F3_Finish_3:
            {
                dtValue3762 = get3762Dt(p_Frame3762Base->dt1_, p_Frame3762Base->dt2_);
                if(p_Frame3762Base->afn_ == 0x00 && p_Frame3762Base->dt1_==0x01 && p_Frame3762Base->dt2_==0x00 && p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    // 路由上行回复确认
                    p_timer->stop();//接收到一条完整报文
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_SendBroadcast_05F3_Finish_3 发送异常广播校时,路由回复确认 【GW-CCO-F008-0002-V01】");

                }
                else if(p_Frame3762Base->afn_ == 0x00&& dtValue3762 == 2 &&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    // 路由上行回复否认
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到路由上行回复确否认，等待70s  \n");

                    // 需要超时器，判断虚拟表是否收到校时报文
                    p_delayTimer->start(70*1000);
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_SendBroadcast_05F3_Finish_4:
            {
                dtValue3762 = get3762Dt(p_Frame3762Base->dt1_, p_Frame3762Base->dt2_);
                if(p_Frame3762Base->afn_ == 0x00 && p_Frame3762Base->dt1_==0x01 && p_Frame3762Base->dt2_==0x00 && p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    // 路由上行回复确认
                    p_timer->stop();//接收到一条完整报文
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送广播校时报文数据长度255，报文实际长度255，路由回复确认，等待1min \n");
                    // 需要超时器，判断虚拟表是否收到校时报文
                    p_delayTimer->start(70*1000);

                }
                else if(p_Frame3762Base->afn_ == 0x00&& dtValue3762 == 2 &&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    // 路由上行回复否认
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_SendBroadcast_05F3_Finish_4 发送广播校时报文数据长度255，报文实际长度255，路由回复否认 【GW-CCO-F008-0002-V01】");
                    break;
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_SendBroadcast_05F3_Finish_5:
            {
                dtValue3762 = get3762Dt(p_Frame3762Base->dt1_, p_Frame3762Base->dt2_);
                if(p_Frame3762Base->afn_ == 0x00 && p_Frame3762Base->dt1_==0x01 && p_Frame3762Base->dt2_==0x00 && p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    // 路由上行回复确认
                    p_timer->stop();//接收到一条完整报文
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_SendBroadcast_05F3_Finish_5 发送异常广播校时,路由回复确认 【GW-CCO-F008-0002-V01】");

                }
                else if(p_Frame3762Base->afn_ == 0x00&& dtValue3762 == 2 &&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    // 路由上行回复否认
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到路由上行回复确否认，等待70s \n");

                    // 需要超时器，判断虚拟表是否收到校时报文
                    p_delayTimer->start(70*1000);

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


void Script_BroadcastTime_LocalProtocolTest::processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID)
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
            case BroadcastInit:
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
                    if(MsgBase_645_ptr->ctrlCode_ == BROADCAST)
                    {
                        // 645广播校时报文
                        Address address;
                        memcpy(address.addr,(*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[mtrlID]->mtrAddr, 6);

                        int meterIndex = getMeterInfo(address);
                        if(meterIndex != -1)
                        {
                            meterInfoList[meterIndex].readFlag = true;
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("电表%1，收到0x08广播校时报文")
                                                                 .arg(QString(QByteArray(meterInfoList.at(meterIndex).meterNo.addr,6).toHex())));
                        }

                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("当前广播校时成功率：%1% \n").arg(calSuccessRate()*100));
                    }
                    else if(MsgBase_645_ptr->ctrlCode_==READ_DATA)
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

void Script_BroadcastTime_LocalProtocolTest::processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID)
{
    bool haveCompleteMsg=true;
    while(haveCompleteMsg)
    {
        QByteArray temp = (*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[mtrlID]->buf698;
        shared_ptr<FrameOOPBase> MsgBase_OOP_ptr = FrameOOPHelper::DecodeMsg(&((*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[mtrlID]->buf698),haveCompleteMsg);

        if(MsgBase_OOP_ptr==nullptr)
        {
            break;
        }

        switch(emScriptRunState)
        {
            case BroadcastInit:
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

                break;
            }
        }
    }
}

void Script_BroadcastTime_LocalProtocolTest::sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame)
{
    if(frame == p_Broadcast_05F3)
    {
        p_Broadcast_05F3->ctrl_field_.dir=kDirDown;
        p_Broadcast_05F3->ctrl_field_.prm=kActive;
        p_Broadcast_05F3->ctrl_field_.comn_type=kHplc;

        p_Broadcast_05F3->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_Broadcast_05F3->info_field_.info_field_down.comu_rate=0;
        p_Broadcast_05F3->info_field_.info_field_down.comu_module_ident=0;

        // 645校时报文
        QByteArray msg645;
        shared_ptr<Rqst_BroadCastTiming_0x08> p_BroadCastTiming_0x08 = make_shared<Rqst_BroadCastTiming_0x08>(addr,4);
        memset(p_BroadCastTiming_0x08->addr_,char(0x99),6);  // 地址域全9
        QByteArray setTime=QByteArray::fromHex(QDateTime::currentDateTime().addSecs(checkTime).toString("yyMMddhhmmss").toLatin1());
        memcpy(p_BroadCastTiming_0x08->dateTime,setTime,6);
        msg645.append(p_BroadCastTiming_0x08->EncodeFrame());

        p_Broadcast_05F3->broadcast_data_unit_.ctrl_word_=DLT645_2007;
        p_Broadcast_05F3->broadcast_data_unit_.frame_content_=msg645;

        p_Broadcast_05F3->broadcast_data_unit_.frame_length_ = uchar(msg645.size());

        // 645校时报文固定18字节
        if(emScriptRunState == Wait_SendBroadcast_05F3_Finish_1)
        {
            p_Broadcast_05F3->broadcast_data_unit_.ctrl_word_=0x04;
        }

        if(emScriptRunState == Wait_SendBroadcast_05F3_Finish_4)
        {
            p_Broadcast_05F3->broadcast_data_unit_.frame_content_.clear();
            for(int i=0; i<14; i++)
            {
                // 14*18 = 252
                p_Broadcast_05F3->broadcast_data_unit_.frame_content_.append(msg645);
            }
            p_Broadcast_05F3->broadcast_data_unit_.frame_content_.append(char(0x00));
            p_Broadcast_05F3->broadcast_data_unit_.frame_content_.append(char(0x00));
            p_Broadcast_05F3->broadcast_data_unit_.frame_content_.append(char(0x00));

            // 补零保证报文长度为255
            p_Broadcast_05F3->broadcast_data_unit_.frame_length_ = 255;

        }


        sendMsgOct=p_Broadcast_05F3->EncodeFrame();

        if(emScriptRunState == Wait_SendBroadcast_05F3_Finish_2)
        {
            // 将长度字段改为17字节
            int index = sendMsgOct.size()-21;
            sendMsgOct[index] = sendMsgOct[index] -1;
            sendMsgOct[sendMsgOct.size()-2] = sendMsgOct[sendMsgOct.size()-2]-1;
        }
        else if(emScriptRunState == Wait_SendBroadcast_05F3_Finish_3)
        {
            // 将长度字段改为19字节
            int index = sendMsgOct.size()-21;
            sendMsgOct[index] = sendMsgOct[index] + 1;
            sendMsgOct[sendMsgOct.size()-2] = sendMsgOct[sendMsgOct.size()-2]+1;
        }
        else if(emScriptRunState == Wait_SendBroadcast_05F3_Finish_5)
        {
            // 将长度字段改为0字节
            int index = sendMsgOct.size()-21;
            sendMsgOct[index] = 0;
            sendMsgOct[sendMsgOct.size()-2] = sendMsgOct[sendMsgOct.size()-2]-18;
        }

        sendMsgLog=QString("》》广播校时05F3：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
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

void Script_BroadcastTime_LocalProtocolTest::sendSrcMsg(DvcType dvcType, int dvcId, QByteArray msg)
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

void Script_BroadcastTime_LocalProtocolTest::meterInfoInit()
{
    meterInfoList.clear();
    for(int i=0;i<p_CtrInfoList->size();i++)
    {
        QList<MeterInfoForSingleNet*> meterList = p_CtrInfoList->at(i)->p_MeterInfoForSingleNetList->values();
        for(int j=0; j<meterList.size(); j++)
        {
            if(meterList.at(j)->slotPosition==SingleSTA || meterList.at(j)->slotPosition==ThreeSTA)
            {
                MeterInfoBroadcast_Struct meterInfo_ST;
                memcpy(meterInfo_ST.meterNo.addr, meterList.at(j)->mtrAddr, 6);
                meterInfo_ST.protocolType = meterList.at(j)->prtcl;
                meterInfo_ST.readFlag = false;

                meterInfoList.append(meterInfo_ST);
            }
        }
    }
}


void Script_BroadcastTime_LocalProtocolTest::timer_timeoutProc()
{
    p_timer->stop();
    switch(emScriptRunState)
    {
        case Wait_BuildNetFinish_Whole:
        {
            p_CtrInfoList->at(0)->inNetResult=false;
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("全网组网成功率：%1%").arg(p_CtrInfoList->at(0)->inNetSuccessRate*100));
            break;
        }
        case Wait_SendBroadcast_05F3_Finish_1:
        {
            p_timer->stop();
            if(++tryTimes>=3)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_SendBroadcast_05F3_Finish_1 timeout 【GW-CCO-F008-0002-V01】");
            }
            else
            {
                meterInfoInit();
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "再次发送--广播校时（05F3），等待--确认");

                sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, INSIGNIFICANCE, p_Broadcast_05F3);
                p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
            }
            break;
        }
        case Wait_SendBroadcast_05F3_Finish_2:
        {
            p_timer->stop();
            if(++tryTimes>=3)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_SendBroadcast_05F3_Finish_2 timeout 【GW-CCO-F008-0002-V01】");
            }
            else
            {
                meterInfoInit();
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "再次发送--广播校时（05F3），等待--确认");

                sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, INSIGNIFICANCE, p_Broadcast_05F3);
                p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
            }
            break;
        }
        case Wait_SendBroadcast_05F3_Finish_3:
        {
            p_timer->stop();
            if(++tryTimes>=3)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_SendBroadcast_05F3_Finish_3 timeout 【GW-CCO-F008-0002-V01】");
            }
            else
            {
                meterInfoInit();
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "再次发送--广播校时（05F3），等待--确认");

                sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, INSIGNIFICANCE, p_Broadcast_05F3);
                p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
            }
            break;
        }
        case Wait_SendBroadcast_05F3_Finish_4:
        {
            p_timer->stop();
            if(++tryTimes>=3)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_SendBroadcast_05F3_Finish_4 timeout 【GW-CCO-F008-0002-V01】");
            }
            else
            {
                meterInfoInit();
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "再次发送--广播校时（05F3），等待--确认");

                sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, INSIGNIFICANCE, p_Broadcast_05F3);
                p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
            }
            break;
        }
        case Wait_SendBroadcast_05F3_Finish_5:
        {
            p_timer->stop();
            if(++tryTimes>=3)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_SendBroadcast_05F3_Finish_5 timeout 【GW-CCO-F008-0002-V01】");
            }
            else
            {
                meterInfoInit();
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "再次发送--广播校时（05F3），等待--确认");

                sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, INSIGNIFICANCE, p_Broadcast_05F3);
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

void Script_BroadcastTime_LocalProtocolTest::maxAllowTimer_timeoutProc()
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

void Script_BroadcastTime_LocalProtocolTest::delayTimer_timeoutProc()
{
    p_delayTimer->stop();
    switch(emScriptRunState)
    {
        case Wait_BuildNetFinish_Whole:
        {
            break;
        }
        case Wait_SendBroadcast_05F3_Finish_1:
        {
            if(calSuccessRate()<1)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "校时超时，失败表:" + getFailMeterNo());
                p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("Wait_SendBroadcast_05F3_Finish_1 timeout 广播校时成功率：%1% 【GW-CCO-F008-0002-V01】").arg(calSuccessRate()*100));
                break;
            }
            else
            {
                // 进行下一次校时
                tryTimes=0;
                meterInfoInit();
                emScriptRunState = Wait_SendBroadcast_05F3_Finish_2;

                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "进行第2轮广播校时测试--下发05F3广播校时报文，通信协议0x02，数据长度为17，广播报文为645报文（长度18），广播地址全9，校时时间为当前时间");
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--广播校时（05F3），等待--确认");

                sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, INSIGNIFICANCE, p_Broadcast_05F3);
                p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
            }
            break;
        }
        case Wait_SendBroadcast_05F3_Finish_2:
        {
            if(calSuccessRate()>0)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("Wait_SendBroadcast_05F3_Finish_2  长度不匹配时不应收到校时报文 【GW-CCO-F008-0002-V01】"));
                break;
            }
            else
            { // 进行下一次校时
                tryTimes=0;
                meterInfoInit();
                emScriptRunState = Wait_SendBroadcast_05F3_Finish_3;
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "进行第3轮广播校时测试--下发05F3广播校时报文，通信协议0x02，数据长度为19，广播报文为645报文（长度18），广播地址全9，校时时间为当前时间");

                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--广播校时（05F3），等待--确认");

                sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, INSIGNIFICANCE, p_Broadcast_05F3);
                p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
            }
            break;
        }
        case Wait_SendBroadcast_05F3_Finish_3:
        {
            if(calSuccessRate()>0)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("Wait_SendBroadcast_05F3_Finish_3  长度不匹配时不应收到校时报文 【GW-CCO-F008-0002-V01】"));
            }
            else
            { // 进行下一次校时
                tryTimes=0;
                meterInfoInit();
                emScriptRunState = Wait_SendBroadcast_05F3_Finish_4;
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "进行第4轮广播校时测试--通信协议0x02，数据长度为255，广播报文为14条645报文（长度18字节）及补零（长度255），广播地址全9，校时时间为当前时间");
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--广播校时（05F3），等待--确认");

                sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, INSIGNIFICANCE, p_Broadcast_05F3);
                p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
            }
            break;
        }
        case Wait_SendBroadcast_05F3_Finish_4:
        {
            if(calSuccessRate()<1)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "校时超时，失败表:" + getFailMeterNo());
                p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("Wait_SendBroadcast_05F3_Finish_4 timeout 广播校时成功率：%1% 【GW-CCO-F008-0002-V01】").arg(calSuccessRate()*100));
            }
            else
            {
                // 进行下一次校时
                tryTimes=0;
                meterInfoInit();
                emScriptRunState = Wait_SendBroadcast_05F3_Finish_5;

                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "进行第5轮广播校时测试--下发05F3广播校时报文，通信协议0x02，数据长度为0，广播报文为645报文（长度18），广播地址全9，校时时间为当前时间");
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--广播校时（05F3），等待--确认");

                sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, INSIGNIFICANCE, p_Broadcast_05F3);
                p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
            }
            break;
        }
        case Wait_SendBroadcast_05F3_Finish_5:
        {
            if(calSuccessRate()>0)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("Wait_SendBroadcast_05F3_Finish_5  长度不匹配时不应收到校时报文 【GW-CCO-F008-0002-V01】"));
            }
            else
            { // 测试完成
                p_AbstractScriptHost->updateProgress(ProcessState_Success, QString("脚本执行成功 【GW-CCO-F008-0002-V01】"));
                emScriptRunState = ScriptSuccess;
            }
            break;
        }
        default:
        {
            break;
        }
    }
}

