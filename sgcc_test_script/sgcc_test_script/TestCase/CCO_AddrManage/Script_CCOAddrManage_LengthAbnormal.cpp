#include "Script_CCOAddrManage_LengthAbnormal.h"

Script_CCOAddrManage_LengthAbnormal::Script_CCOAddrManage_LengthAbnormal(QObject *parent) : QObject(parent)
{
    p_BuildNetwork_GW=new BuildNetwork_GW();
    p_CtrInfoList=make_shared<QList<shared_ptr<CtrInfo>>>();
    p_Frame3762Helper=make_shared<Frame3762Helper>();
    p_QueryCcoAddr_03F4_Down=make_shared<Afn03F4>();
    p_SetCcoAddr_05F1_Down=make_shared<Afn05F1>();
    p_HardInit_01F1_Down=make_shared<Afn01F1>();

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

Script_CCOAddrManage_LengthAbnormal::~Script_CCOAddrManage_LengthAbnormal()
{
    p_BuildNetwork_GW->initBuildNetWork();
    if(needPowerOff==true)//断电处理
        powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost,true);
}

void Script_CCOAddrManage_LengthAbnormal::setHost(AbstractScriptHost *host)
{
    p_AbstractScriptHost=host;
    p_BuildNetwork_GW->setHost(host);
}

void Script_CCOAddrManage_LengthAbnormal::addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq)
{
    p_CtrInfoList->clear();
    comnAddAddrsInfo(p_CtrInfoList, p_ConcentratorInfoList, p_MeterInfoList, p_SchemeCfgInfoList);
    uchar dstFreq=freq&0x0f;

    uchar dstPrtcl=uchar(freq)>>4;
    if(dstPrtcl==0x03)
        setMeterAddrsPrtcl(p_CtrInfoList,dstPrtcl);

    p_BuildNetwork_GW->setCtrInfoListAndFreq(p_CtrInfoList, dstFreq);

}

bool Script_CCOAddrManage_LengthAbnormal::config(const QMap<QString,QString> *paraDic)
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


void Script_CCOAddrManage_LengthAbnormal::execute()
{
    QString step_desc = "流程描述: \r";
    step_desc += "1.组网通用流程，SENCE2(需要配置CCO，但不需要组网)\r";
    step_desc += "2.设置CCO地址123456abcdef，查询CCO地址  Wait_SetCCOAddress_05F1_Finish\r";
    step_desc += "3.设置CCO地址非A主节点地址112233445566，查询CCO地址  Wait_SetCCOAddress1_05F1_Finish\r";
    step_desc += "4.设置CCO地址长度为4字节的主节点地址11223344，查询CCO地址  Wait_SetCCOAddress2_05F1_Finish\r";
    step_desc += "5.设置CCO地址设置长度为2字节的主节点地址1122，查询CCO地址  Wait_SetCCOAddress3_05F1_Finish\r";
    step_desc += "6.设置CCO地址设置长度为0的主节点地址，查询CCO地址  Wait_SetCCOAddress4_05F1_Finish\r";
    step_desc += "7.设置CCO地址长度大于6字节的主节点地址11223344556677，查询CCO地址  Wait_SetCCOAddress5_05F1_Finish\r";
    step_desc += "8.路由01F1硬件初始化，查询CCO地址  Wait_SetCCOAddress6_05F1_Finish\n";

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
        emScriptRunState = Wait_SetCCOAddress_05F1_Finish;
        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
    }
    p_maxAllowTimer->start((timerForReachThresld+timerAfterReachThresld)*1000);//此处要启动脚本的最大执行时间定时器
}

void Script_CCOAddrManage_LengthAbnormal::stop()
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

void Script_CCOAddrManage_LengthAbnormal::processMsg(DvcType dvcType, int id, uchar *data, int datalen)
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
            // 设置CCO地址
            p_BuildNetwork_GW->initBuildNetWork();

            this->ccoNewAddr.clear();
            QString addr = "123456abcdef";
            this->ccoNewAddr = QByteArray::fromHex(addr.toLatin1());

            tryTimes=0;
            index=0;
            emScriptRunState = Wait_SetCCOAddress_05F1_Finish;
            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_SetCcoAddr_05F1_Down);
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--设置主节点地址 123456abcdef，等待--回复");
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

void Script_CCOAddrManage_LengthAbnormal::processCtrlDvcRes(DvcType dvcType, QList<int> idList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params)
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


void Script_CCOAddrManage_LengthAbnormal::processMsgFromCCO(DvcType dvcType, int dvcId)
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
            case ScriptInit:
            {
                break;
            }
            case Wait_BuildNetFinish_Whole:
            {
                break;
            }
            case Wait_SetCCOAddress_05F1_Finish:
            {
                dtValue3762 = get3762Dt(p_Frame3762Base->dt1_,p_Frame3762Base->dt2_);
                if(p_Frame3762Base->afn_ == 0x00 && dtValue3762==1 &&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到设置主节点地址确认，等待10s...");
                    p_delayTimer->start(10*1000);
                }
                else if(p_Frame3762Base->afn_ == 0x00&& dtValue3762==2 &&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_SetCCOAddress_05F1_Finish 设置主节点地址否认【GW-CCO-F015-0002-V01】");
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_QueryCCOAddress_Finish:
            {
                dtValue3762=get3762Dt(p_Frame3762Base->dt1_,p_Frame3762Base->dt2_);
                if(p_Frame3762Base->afn_ == 0x03&&dtValue3762==4&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    shared_ptr<Afn03F4> p_QueryCcoInitAddr_03F4_Up=dynamic_pointer_cast<Afn03F4>(p_Frame3762Base);

                    if(isArrayEqual(reinterpret_cast<uchar*>(this->ccoNewAddr.data()),reinterpret_cast<uchar*>(p_QueryCcoInitAddr_03F4_Up->master_node_address_.addr),6))
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "CCO地址为目标地址 123456abcdef \n");
                        this->ccoNewAddr.clear();
                        QString addr = "112233445566";
                        this->ccoNewAddr = QByteArray::fromHex(addr.toLatin1());

                        sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, INSIGNIFICANCE, p_SetCcoAddr_05F1_Down);
                        emScriptRunState=Wait_SetCCOAddress1_05F1_Finish;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--设置主节点地址112233445566，等待--确认");
                        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                    {
                        QByteArray real_addr;
                        for(int i=0;i<6;i++)
                        {
                            real_addr.append(p_QueryCcoInitAddr_03F4_Up->master_node_address_.addr[i]);
                        }
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("Wait_QueryCCOAddress_Finish CCO地址不为目标地址%1 实际地址%2【GW-CCO-F015-0002-V01】")
                                                             .arg(QString(ccoNewAddr.toHex())).arg(QString(real_addr.toHex())));
                    }
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_SetCCOAddress1_05F1_Finish:
            {
                dtValue3762 = get3762Dt(p_Frame3762Base->dt1_,p_Frame3762Base->dt2_);
                if(p_Frame3762Base->afn_ == 0x00 && dtValue3762==1 &&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到设置主节点地址确认，等待10s...");
                    p_delayTimer->start(10*1000);
                }
                else if(p_Frame3762Base->afn_ == 0x00&& dtValue3762==2 &&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_SetCCOAddress1_05F1_Finish 设置主节点地址否认【GW-CCO-F015-0002-V01】");
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_QueryCCOAddress1_Finish:
            {
                dtValue3762=get3762Dt(p_Frame3762Base->dt1_,p_Frame3762Base->dt2_);
                if(p_Frame3762Base->afn_ == 0x03&&dtValue3762==4&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    shared_ptr<Afn03F4> p_QueryCcoInitAddr_03F4_Up=dynamic_pointer_cast<Afn03F4>(p_Frame3762Base);

                    if(isArrayEqual(reinterpret_cast<uchar*>(this->ccoNewAddr.data()),reinterpret_cast<uchar*>(p_QueryCcoInitAddr_03F4_Up->master_node_address_.addr),6))
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "CCO地址为目标地址 112233445566 \n");
                        this->ccoNewAddr.clear();
                        QString addr = "112233445566";
                        this->ccoNewAddr = QByteArray::fromHex(addr.toLatin1());

//                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--设置异常主节点地址11223344，等待路由回复");
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--设置异常主节点地址为0，等待路由回复");

//                        emScriptRunState = Wait_SetCCOAddress2_05F1_Finish;
                        emScriptRunState = Wait_SetCCOAddress4_05F1_Finish;
                        sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, INSIGNIFICANCE, p_SetCcoAddr_05F1_Down);
                        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                    {
                        QByteArray real_addr;
                        for(int i=0;i<6;i++)
                        {
                            real_addr.append(p_QueryCcoInitAddr_03F4_Up->master_node_address_.addr[i]);
                        }
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("Wait_QueryCCOAddress1_Finish CCO地址不为目标地址%1 实际地址%2【GW-CCO-F015-0002-V01】")
                                                             .arg(QString(ccoNewAddr.toHex())).arg(QString(real_addr.toHex())));
                    }
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_SetCCOAddress2_05F1_Finish:
            {
                dtValue3762 = get3762Dt(p_Frame3762Base->dt1_,p_Frame3762Base->dt2_);
                if(p_Frame3762Base->afn_ == 0x00 && dtValue3762==1 &&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
//                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_SetCCOAddress2_05F1_Finish 设置异常主节点地址11223344，路由回复确认 【GW-CCO-F015-0002-V01】");
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "Wait_SetCCOAddress2_05F1_Finish 设置异常主节点地址11223344，路由回复确认 路由不支持对地址长度判断");
                    p_delayTimer->start(10*1000);
                    break;
                }
                else if(p_Frame3762Base->afn_ == 0x00&& dtValue3762==2 &&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    // 实际会回复否认
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "设置主节点地址11223344，路由回复否认 \n");
                    p_delayTimer->start(10*1000);
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_QueryCCOAddress2_Finish:
            {
                dtValue3762=get3762Dt(p_Frame3762Base->dt1_,p_Frame3762Base->dt2_);
                if(p_Frame3762Base->afn_ == 0x03&&dtValue3762==4&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    shared_ptr<Afn03F4> p_QueryCcoInitAddr_03F4_Up = dynamic_pointer_cast<Afn03F4>(p_Frame3762Base);

                    uchar rev_addr[6] = {0x11,0x22,0x33,0x44,0x55,0x66};
                    if(isArrayEqual(reinterpret_cast<uchar*>(p_QueryCcoInitAddr_03F4_Up->master_node_address_.addr), rev_addr, 6))
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "CCO地址为目标地址 12233445566 \n");

                        this->ccoNewAddr.clear();
                        QString addr = "112200000000";
                        this->ccoNewAddr = QByteArray::fromHex(addr.toLatin1());

                        emScriptRunState=Wait_SetCCOAddress3_05F1_Finish;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--设置异常主节点地址1122，等待路由回复");
                        sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, INSIGNIFICANCE, p_SetCcoAddr_05F1_Down);
                        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                    {
                        QByteArray real_addr;
                        for(int i=0;i<6;i++)
                        {
                            real_addr.append(p_QueryCcoInitAddr_03F4_Up->master_node_address_.addr[i]);
                        }
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("Wait_QueryCCOAddress2_Finish CCO地址不为目标地址112233445566 实际地址%1【GW-CCO-F015-0002-V01】")
                                                             .arg(QString(real_addr.toHex())));
                    }
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_SetCCOAddress3_05F1_Finish:
            {
                dtValue3762 = get3762Dt(p_Frame3762Base->dt1_,p_Frame3762Base->dt2_);
                if(p_Frame3762Base->afn_ == 0x00 && dtValue3762==1 &&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
//                  p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_SetCCOAddress3_05F1_Finish 设置异常主节点地址1122，路由回复确认 【GW-CCO-F015-0002-V01】");
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "Wait_SetCCOAddress3_05F1_Finish 设置异常主节点地址1122，路由回复确认 路由不支持地址长度判断");
                    p_delayTimer->start(10*1000);
                    break;
                }
                else if(p_Frame3762Base->afn_ == 0x00&& dtValue3762==2 &&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    // 实际会回复否认
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "设置异常主节点地址1122，路由回复否认 \n");
                    p_delayTimer->start(10*1000);
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_QueryCCOAddress3_Finish:
            {
                dtValue3762=get3762Dt(p_Frame3762Base->dt1_,p_Frame3762Base->dt2_);
                if(p_Frame3762Base->afn_ == 0x03&&dtValue3762==4&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    shared_ptr<Afn03F4> p_QueryCcoInitAddr_03F4_Up=dynamic_pointer_cast<Afn03F4>(p_Frame3762Base);

                    uchar rev_addr[6] = {0x11,0x22,0x33,0x44,0x55,0x66};
                    if(isArrayEqual(reinterpret_cast<uchar*>(p_QueryCcoInitAddr_03F4_Up->master_node_address_.addr), rev_addr, 6))
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "CCO地址为目标地址1112233445566 \n");

                        this->ccoNewAddr.clear();
                        QString addr = "000000000000";
                        this->ccoNewAddr = QByteArray::fromHex(addr.toLatin1());

                        emScriptRunState=Wait_SetCCOAddress4_05F1_Finish;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--设置主节点地址长度为0，等待路由回复");
                        sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, INSIGNIFICANCE, p_SetCcoAddr_05F1_Down);
                        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                    {
                        QByteArray real_addr;
                        for(int i=0;i<6;i++)
                        {
                            real_addr.append(p_QueryCcoInitAddr_03F4_Up->master_node_address_.addr[i]);
                        }
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("Wait_QueryCCOAddress3_Finish CCO地址不为目标地址112233445566 实际地址%1【GW-CCO-F015-0002-V01】")
                                                             .arg(QString(real_addr.toHex())));
                    }
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_SetCCOAddress4_05F1_Finish:
            {
                dtValue3762 = get3762Dt(p_Frame3762Base->dt1_,p_Frame3762Base->dt2_);
                if(p_Frame3762Base->afn_ == 0x00 && dtValue3762==1 &&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_SetCCOAddress4_05F1_Finish 设置主节点地址长度为0，路由回复确认 【GW-CCO-F015-0002-V01】");
                    break;
                }
                else if(p_Frame3762Base->afn_ == 0x00&& dtValue3762==2 &&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    // 实际会回复否认
                    p_timer->stop();
//                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "设置主节点地址长度为0，路由回复否认 \n");
//                    p_delayTimer->start(10*1000);
                    sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, INSIGNIFICANCE, p_HardInit_01F1_Down);
                    emScriptRunState=Wait_SetCCOAddress6_05F1_Finish;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--路由01F1硬件初始化，等待--确认");

                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_QueryCCOAddress4_Finish:
            {
                dtValue3762=get3762Dt(p_Frame3762Base->dt1_,p_Frame3762Base->dt2_);
                if(p_Frame3762Base->afn_ == 0x03&&dtValue3762==4&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    shared_ptr<Afn03F4> p_QueryCcoInitAddr_03F4_Up=dynamic_pointer_cast<Afn03F4>(p_Frame3762Base);

                    uchar rev_addr[6] = {0x11,0x22,0x33,0x44,0x55,0x66};
                    if(isArrayEqual(reinterpret_cast<uchar*>(p_QueryCcoInitAddr_03F4_Up->master_node_address_.addr), rev_addr, 6))
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "CCO地址为目标地址112233445566 \n");
                        this->ccoNewAddr.clear();
                        QString addr = "112233445566";               // 11223344556677
                        this->ccoNewAddr = QByteArray::fromHex(addr.toLatin1());

                        emScriptRunState = Wait_SetCCOAddress5_05F1_Finish;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--设置主节点地址11223344556677，等待路由回复");
                        sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, INSIGNIFICANCE, p_SetCcoAddr_05F1_Down);
                        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                    {
                        QByteArray real_addr;
                        for(int i=0;i<6;i++)
                        {
                            real_addr.append(p_QueryCcoInitAddr_03F4_Up->master_node_address_.addr[i]);
                        }
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("Wait_QueryCCOAddress3_Finish CCO地址不为目标地址 实际地址%1【GW-CCO-F015-0002-V01】")
                                                             .arg(QString(real_addr.toHex())));
                    }

                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_SetCCOAddress5_05F1_Finish:
            {
                dtValue3762 = get3762Dt(p_Frame3762Base->dt1_,p_Frame3762Base->dt2_);
                if(p_Frame3762Base->afn_ == 0x00 && dtValue3762==1 &&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_SetCCOAddress45_05F1_Finish 设置异常主节点地址11223344556677，路由回复确认 【GW-CCO-F015-0002-V01】");
                    break;
                }
                else if(p_Frame3762Base->afn_ == 0x00&& dtValue3762==2 &&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    // 实际会回复否认
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "设置异常主节点地址11223344556677，路由回复否认");
                    p_delayTimer->start(10*1000);
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_QueryCCOAddress5_Finish:
            {
                dtValue3762=get3762Dt(p_Frame3762Base->dt1_,p_Frame3762Base->dt2_);
                if(p_Frame3762Base->afn_ == 0x03&&dtValue3762==4&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    shared_ptr<Afn03F4> p_QueryCcoInitAddr_03F4_Up=dynamic_pointer_cast<Afn03F4>(p_Frame3762Base);

                    QString addr = "112233445566";
                    this->ccoNewAddr = QByteArray::fromHex(addr.toLatin1());

                    if(isArrayEqual(reinterpret_cast<uchar*>(this->ccoNewAddr.data()),reinterpret_cast<uchar*>(p_QueryCcoInitAddr_03F4_Up->master_node_address_.addr),6))
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "CCO地址为目标地址 112233445566 \n");

                        sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, INSIGNIFICANCE, p_HardInit_01F1_Down);
                        emScriptRunState=Wait_SetCCOAddress6_05F1_Finish;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--路由01F1硬件初始化，等待--确认");

                        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                    {
                        QByteArray real_addr;
                        for(int i=0;i<6;i++)
                        {
                            real_addr.append(p_QueryCcoInitAddr_03F4_Up->master_node_address_.addr[i]);
                        }
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("Wait_QueryCCOAddress5_Finish CCO地址不为目标地址112233445566 实际地址%1【GW-CCO-F015-0002-V01】")
                                                             .arg(QString(real_addr.toHex())));
                    }
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_SetCCOAddress6_05F1_Finish:
            {
                dtValue3762 = get3762Dt(p_Frame3762Base->dt1_,p_Frame3762Base->dt2_);
                if(p_Frame3762Base->afn_ == 0x00 && dtValue3762==1 &&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到硬件初始化确认，等待10s...");
                    p_delayTimer->start(10*1000);
                }
                else if(p_Frame3762Base->afn_ == 0x00&& dtValue3762==2 &&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_SetCCOAddress6_05F1_Finish 05F1硬件初始化回复异常【GW-CCO-F015-0002-V01】");
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_QueryCCOAddress6_Finish:
            {
                dtValue3762=get3762Dt(p_Frame3762Base->dt1_,p_Frame3762Base->dt2_);
                if(p_Frame3762Base->afn_ == 0x03&&dtValue3762==4&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    shared_ptr<Afn03F4> p_QueryCcoInitAddr_03F4_Up=dynamic_pointer_cast<Afn03F4>(p_Frame3762Base);

//                    // 强制设置期望的CCO地址为112233445566
//                    this->ccoNewAddr.clear();
//                    QString addr = "112233445566";
//                    this->ccoNewAddr = QByteArray::fromHex(addr.toLatin1());

                    if(isArrayEqual(reinterpret_cast<uchar*>(this->ccoNewAddr.data()),reinterpret_cast<uchar*>(p_QueryCcoInitAddr_03F4_Up->master_node_address_.addr),6))
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "CCO地址为目标地址 112233445566 \n");
                        p_AbstractScriptHost->updateProgress(ProcessState_Success, "脚本执行成功【GW-CCO-F015-0002-V01】");
                        break;
                    }
                    else
                    {
                        QByteArray real_addr;
                        for(int i=0;i<6;i++)
                        {
                            real_addr.append(p_QueryCcoInitAddr_03F4_Up->master_node_address_.addr[i]);
                        }
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("Wait_QueryCCOAddress6_Finish CCO地址不为目标地址%1 实际地址%2【GW-CCO-F015-0002-V01】")
                                                             .arg(QString(ccoNewAddr.toHex())).arg(QString(real_addr.toHex())));
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


void Script_CCOAddrManage_LengthAbnormal::processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID)
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

void Script_CCOAddrManage_LengthAbnormal::processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID)
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

void Script_CCOAddrManage_LengthAbnormal::sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame)
{
    if(frame==p_HardInit_01F1_Down)
    {
        p_HardInit_01F1_Down->ctrl_field_.dir=kDirDown;
        p_HardInit_01F1_Down->ctrl_field_.prm=kActive;
        p_HardInit_01F1_Down->ctrl_field_.comn_type=kHplc;

        p_HardInit_01F1_Down->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_HardInit_01F1_Down->info_field_.info_field_down.comu_rate=0;
        p_HardInit_01F1_Down->info_field_.info_field_down.comu_module_ident=0;

        sendMsgOct=p_HardInit_01F1_Down->EncodeFrame();
        sendMsgLog=QString("》》路由硬件初始化01F1：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_QueryCcoAddr_03F4_Down)
    {
        p_QueryCcoAddr_03F4_Down->ctrl_field_={kHplc,kActive,kDirDown};
        p_QueryCcoAddr_03F4_Down->info_field_.info_field_down.comu_module_ident=0;
        p_QueryCcoAddr_03F4_Down->info_field_.info_field_down.msg_seq=msgSeq++;

        sendMsgOct=p_QueryCcoAddr_03F4_Down->EncodeFrame();
        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("》》查询主节点地址03F4：%1\n").arg(QString(sendMsgOct.toHex())));

    }
    else if(frame==p_SetCcoAddr_05F1_Down)
    {
        p_SetCcoAddr_05F1_Down->ctrl_field_={kHplc,kActive,kDirDown};
        p_SetCcoAddr_05F1_Down->info_field_.info_field_down.comu_module_ident=0;
        p_SetCcoAddr_05F1_Down->info_field_.info_field_down.msg_seq=msgSeq++;

        memcpy(p_SetCcoAddr_05F1_Down->primary_node_address_.addr, ccoNewAddr, 6);
        sendMsgOct=p_SetCcoAddr_05F1_Down->EncodeFrame();

        if(emScriptRunState == Wait_SetCCOAddress2_05F1_Finish)
        {
            // 11223344
            sendMsgOct[1] = char(0x13);
            QByteArray new_frame = sendMsgOct.mid(0, 13);
            new_frame.append(sendMsgOct.mid(15));
            cs_null =  sendMsgOct.at(19);
            sendMsgOct = new_frame;      
        }
        else if(emScriptRunState == Wait_SetCCOAddress3_05F1_Finish)
        {
            //1122
            sendMsgOct[1] = char(0x11);
            QByteArray new_frame = sendMsgOct.mid(0, 13);
            new_frame.append(sendMsgOct.mid(17));
            cs_null =  sendMsgOct.at(19);
            sendMsgOct = new_frame;


        }
        else if(emScriptRunState == Wait_SetCCOAddress4_05F1_Finish)
        {
            // 空
            sendMsgOct[1] = char(0x0f);
            sendMsgOct[2] = char(0x00);

            QByteArray new_frame = sendMsgOct.mid(0, 13);
            new_frame.append(sendMsgOct.mid(19));
            cs_null =  sendMsgOct.at(19);
            sendMsgOct = new_frame;

        }
        else if(emScriptRunState == Wait_SetCCOAddress5_05F1_Finish)
        {
            // 11223344556677
            sendMsgOct[1] = char(0x16);
            sendMsgOct[19] = sendMsgOct.at(19) + char(0x77);
            QByteArray new_frame = sendMsgOct.mid(0, 13);
            new_frame.append(char(0x77));
            new_frame.append(sendMsgOct.mid(13));
            cs_null =  sendMsgOct.at(19);
            sendMsgOct = new_frame;

        }
        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("》》设置主节点地址05F1：%1\n").arg(QString(sendMsgOct.toHex())));
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

void Script_CCOAddrManage_LengthAbnormal::sendSrcMsg(DvcType dvcType, int dvcId, QByteArray msg)
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


void Script_CCOAddrManage_LengthAbnormal::timer_timeoutProc()
{
    p_timer->stop();
    switch(emScriptRunState)
    {
        case Wait_BuildNetFinish_Whole:
        {
            p_CtrInfoList->at(0)->inNetResult=false;
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("Wait_BuildNetFinish_Whole 全网组网成功率：%1% 【GW-CCO-F015-0002-V01】").arg(p_CtrInfoList->at(0)->inNetSuccessRate*100));
            break;
        }
        case Wait_SetCCOAddress_05F1_Finish:
        {
            p_timer->stop();
            if(++tryTimes>=3)
            {
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_SetCCOAddress_05F1_Finish timeout 【GW-CCO-F015-0002-V01】");
            }
            else
            {
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_SetCcoAddr_05F1_Down);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--设置主节点地址123456abcdef，等待--回复");
                p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
            }
            break;
        }
        case Wait_QueryCCOAddress_Finish:
        {
            p_timer->stop();
            if(++tryTimes>=3)
            {
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_QueryCCOAddress_Finish timeout 【GW-CCO-F015-0002-V01】");
            }
            else
            {
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryCcoAddr_03F4_Down);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询主节点地址，等待--回复");
                p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
            }
            break;
        }

        case Wait_SetCCOAddress1_05F1_Finish:
        {
            p_timer->stop();
            if(++tryTimes>=3)
            {
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_SetCCOAddress1_05F1_Finish timeout 【GW-CCO-F015-0002-V01】");
            }
            else
            {
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_SetCcoAddr_05F1_Down);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--设置主节点地址112233445566，等待--回复");
                p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
            }
            break;
        }
        case Wait_QueryCCOAddress1_Finish:
        {
            p_timer->stop();
            if(++tryTimes>=3)
            {
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_QueryCCOAddress1_Finish timeout 【GW-CCO-F015-0002-V01】");
            }
            else
            {
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryCcoAddr_03F4_Down);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询主节点地址，等待--回复");
                p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
            }
            break;
        }
        case Wait_SetCCOAddress2_05F1_Finish:
        {
            p_timer->stop();
            if(++tryTimes>=3)
            {
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_SetCCOAddress2_05F1_Finish timeout 【GW-CCO-F015-0002-V01】");
            }
            else
            {
                sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, INSIGNIFICANCE, p_SetCcoAddr_05F1_Down);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--设置主节点地址11223344，等待--回复");
                p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
            }
            break;
        }
        case Wait_QueryCCOAddress2_Finish:
        {
            p_timer->stop();
            if(++tryTimes>=3)
            {
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_QueryCCOAddress2_Finish timeout 【GW-CCO-F015-0002-V01】");
            }
            else
            {
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryCcoAddr_03F4_Down);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询主节点地址，等待--回复");
                p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
            }
            break;
        }
        case Wait_SetCCOAddress3_05F1_Finish:
        {
            p_timer->stop();
            if(++tryTimes>=3)
            {
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_SetCCOAddress3_05F1_Finish timeout 【GW-CCO-F015-0002-V01】");
            }
            else
            {
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_SetCcoAddr_05F1_Down);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--设置主节点地址1122，等待--回复");
                p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
            }
            break;
        }
        case Wait_QueryCCOAddress3_Finish:
        {
            p_timer->stop();
            if(++tryTimes>=3)
            {
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_QueryCCOAddress3_Finish timeout 【GW-CCO-F015-0002-V01】");
            }
            else
            {
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryCcoAddr_03F4_Down);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询主节点地址，等待--回复");
                p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
            }
            break;
        }
        case Wait_SetCCOAddress4_05F1_Finish:
        {
            p_timer->stop();
            if(++tryTimes>=3)
            {
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_SetCCOAddress4_05F1_Finish timeout 【GW-CCO-F015-0002-V01】");
            }
            else
            {
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_SetCcoAddr_05F1_Down);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--设置主节点地址为空，等待--回复");
                p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
            }
            break;
        }
        case Wait_QueryCCOAddress4_Finish:
        {
            p_timer->stop();
            if(++tryTimes>=3)
            {
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_QueryCCOAddress4_Finish timeout 【GW-CCO-F015-0002-V01】");
            }
            else
            {
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryCcoAddr_03F4_Down);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询主节点地址，等待--回复");
                p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
            }
            break;
        }
        case Wait_SetCCOAddress5_05F1_Finish:
        {
            p_timer->stop();
            if(++tryTimes>=3)
            {
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_SetCCOAddress5_05F1_Finish timeout 【GW-CCO-F015-0002-V01】");
            }
            else
            {
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_SetCcoAddr_05F1_Down);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--设置主节点地址为11223344556677，等待--回复");
                p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
            }
            break;
        }
        case Wait_QueryCCOAddress5_Finish:
        {
            p_timer->stop();
            if(++tryTimes>=3)
            {
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_QueryCCOAddress5_Finish timeout 【GW-CCO-F015-0002-V01】");
            }
            else
            {
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryCcoAddr_03F4_Down);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询主节点地址，等待--回复");
                p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
            }
            break;
        }
        case Wait_SetCCOAddress6_05F1_Finish:
        {
            p_timer->stop();
            if(++tryTimes>=3)
            {
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_SetCCOAddress6_05F1_Finish timeout 【GW-CCO-F015-0002-V01】");
            }
            else
            {
                sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, INSIGNIFICANCE, p_HardInit_01F1_Down);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--路由01F1硬件初始化，等待--回复");
                p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
            }
            break;
        }
        case Wait_QueryCCOAddress6_Finish:
        {
            p_timer->stop();
            if(++tryTimes>=3)
            {
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_QueryCCOAddress6_Finish timeout 【GW-CCO-F015-0002-V01】");
            }
            else
            {
                sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, INSIGNIFICANCE, p_QueryCcoAddr_03F4_Down);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询主节点地址，等待--回复");
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

void Script_CCOAddrManage_LengthAbnormal::maxAllowTimer_timeoutProc()
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

void Script_CCOAddrManage_LengthAbnormal::delayTimer_timeoutProc()
{
    p_delayTimer->stop();
    switch(emScriptRunState)
    {
        case Wait_BuildNetFinish_Whole:
        {
            break;
        }
        case Wait_SetCCOAddress_05F1_Finish:
        {
            sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, INSIGNIFICANCE, p_QueryCcoAddr_03F4_Down);
            emScriptRunState = Wait_QueryCCOAddress_Finish;
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询主节点地址，等待--确认");
            p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            break;
        }
        case Wait_SetCCOAddress1_05F1_Finish:
        {
            sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, INSIGNIFICANCE, p_QueryCcoAddr_03F4_Down);
            emScriptRunState = Wait_QueryCCOAddress1_Finish;
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询主节点地址，等待--确认");
            p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            break;
        }
        case Wait_SetCCOAddress2_05F1_Finish:
        {
            sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, INSIGNIFICANCE, p_QueryCcoAddr_03F4_Down);
            emScriptRunState = Wait_QueryCCOAddress2_Finish;
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询主节点地址，等待--确认");
            p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            break;
        }
        case Wait_SetCCOAddress3_05F1_Finish:
        {
            sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, INSIGNIFICANCE, p_QueryCcoAddr_03F4_Down);
            emScriptRunState = Wait_QueryCCOAddress3_Finish;
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询主节点地址，等待--确认");
            p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            break;
        }
        case Wait_SetCCOAddress4_05F1_Finish:
        {
            sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, INSIGNIFICANCE, p_QueryCcoAddr_03F4_Down);
            emScriptRunState = Wait_QueryCCOAddress4_Finish;
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询主节点地址，等待--确认");
            p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            break;
        }
        case Wait_SetCCOAddress5_05F1_Finish:
        {
            sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, INSIGNIFICANCE, p_QueryCcoAddr_03F4_Down);
            emScriptRunState = Wait_QueryCCOAddress5_Finish;
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询主节点地址，等待--确认");
            p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            break;
        }
        case Wait_SetCCOAddress6_05F1_Finish:
        {
            sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, INSIGNIFICANCE, p_QueryCcoAddr_03F4_Down);
            emScriptRunState = Wait_QueryCCOAddress6_Finish;
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询主节点地址，等待--确认");
            p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            break;
        }
        default:
        {
            break;
        }
    }
}

