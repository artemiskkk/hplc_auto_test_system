#include "Script_ReadMeter_SearchMeter_Upgrade_MultiTask.h"

Script_ReadMeter_SearchMeter_Upgrade_MultiTask::Script_ReadMeter_SearchMeter_Upgrade_MultiTask(QObject *parent) : QObject(parent)
{
    p_BuildNetwork_GW=new BuildNetwork_GW();
    p_CtrInfoList=make_shared<QList<shared_ptr<CtrInfo>>>();

    p_Frame3762Helper=make_shared<Frame3762Helper>();
    p_HardReset_01F1=make_shared<Afn01F1>();
    p_SetAreaIdentityFlag_05F6=make_shared<Afn05F6>();
    p_SetBaudRate_11F4=make_shared<Afn11F4>();
    p_FileTransfer_15F1_Down=make_shared<qgdw_3762_protocol::Afn15F1>();

    p_ActiveRegister_11F5=make_shared<Afn11F5>();
    p_Confirm_00F1=make_shared<Afn00F1>();
    p_Deny_00F2=make_shared<Afn00F2>();
    p_RouterRestart_12F1=make_shared<Afn12F1>();
    p_RouterPause_12F2=make_shared<Afn12F2>();
    p_RouterRecover_12F3=make_shared<Afn12F3>();
    p_RouterRequestRead_14F1=make_shared<Afn14F1>();
    p_MonitorSlaveNode_13F1=make_shared<Afn13F1>();
    p_ParallelReadMeter_F1F1=make_shared<AfnF1F1>();
    p_CcoRunStateInfo_10F4_Down=make_shared<qgdw_3762_protocol::Afn10F4>();
    p_ChkStaInVrsnInfo_02F1_Down=make_shared<qgdw_3762_protocol::Afn02F1>();

    p_Frame645Helper=make_shared<Frame645Helper>();
    p_MeterAddrResp_93=make_shared<RspsNormal_ReadAddr_0x93>(addr,6);
    p_FrameOOPHelper=make_shared<FrameOOPHelper>();
    p_GetResponseNormal_ReadAddr=make_shared<GetResponseNormal>();

    p_timer=new QTimer(this);
    p_maxAllowTimer=new QTimer(this);
    p_delayTimer=new QTimer(this);
    p_13F1Timer = new QTimer(this);
    p_F1F1Timer = new QTimer(this);
    p_14F1Timer = new QTimer(this);
    p_meterSearchTimer = new QTimer(this);
    p_upgardeTimer= new QTimer(this);


    connect(p_timer,SIGNAL(timeout()),this,SLOT(timer_timeoutProc()));
    connect(p_maxAllowTimer,SIGNAL(timeout()),this,SLOT(maxAllowTimer_timeoutProc()));
    connect(p_delayTimer,SIGNAL(timeout()),this,SLOT(delayTimer_timeoutProc()));

    connect(p_13F1Timer,SIGNAL(timeout()),this,SLOT(timeoutProc_13F1()));
    connect(p_F1F1Timer,SIGNAL(timeout()),this,SLOT(timeoutProc_F1F1()));
    connect(p_14F1Timer,SIGNAL(timeout()),this,SLOT(timeoutProc_14F1()));

    connect(p_meterSearchTimer,SIGNAL(timeout()),this,SLOT(timeoutProc_meterSearch()));
    connect(p_upgardeTimer,SIGNAL(timeout()),this,SLOT(timeoutProc_upgarde()));

}
bool Script_ReadMeter_SearchMeter_Upgrade_MultiTask::tryLockMeter(char* meterAddr, const QString& taskName)
{
    QString addr = QString(QByteArray((char*)meterAddr, 6).toHex());

    QMutexLocker locker(&m_meterMutex);

    // 检查是否已被占用
    if(m_busyMeters.contains(addr)) {
        qint64 lockTime = m_meterLockTime.value(addr, 0);
        qint64 now = QDateTime::currentMSecsSinceEpoch();

        // 超过30秒强制解锁
        if(now - lockTime > 30000) {
            m_busyMeters.remove(addr);
            m_meterLockTime.remove(addr);
            p_AbstractScriptHost->updateProgress(ProcessState_Processing,
                QString(" [%1] 表%2锁定超时，强制解锁").arg(taskName).arg(addr));
        } else {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing,
                QString(" [%1] 表%2被占用，跳过").arg(taskName).arg(addr));
            return false;
        }
    }

    // 锁定成功
    m_busyMeters.insert(addr);
    m_meterLockTime[addr] = QDateTime::currentMSecsSinceEpoch();
    return true;
}

// 解锁电表
void Script_ReadMeter_SearchMeter_Upgrade_MultiTask::unlockMeter(char* meterAddr)
{
    QString addr = QString(QByteArray((char*)meterAddr, 6).toHex());

    QMutexLocker locker(&m_meterMutex);
    m_busyMeters.remove(addr);
    m_meterLockTime.remove(addr);
}
Script_ReadMeter_SearchMeter_Upgrade_MultiTask::~Script_ReadMeter_SearchMeter_Upgrade_MultiTask()
{
    p_BuildNetwork_GW->initBuildNetWork();
    if(needPowerOff==true)//断电处理
        powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost,true);
}


void Script_ReadMeter_SearchMeter_Upgrade_MultiTask::setHost(AbstractScriptHost *host)
{
    p_AbstractScriptHost=host;
    p_BuildNetwork_GW->setHost(host);
}

void Script_ReadMeter_SearchMeter_Upgrade_MultiTask::addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq)
{
    p_CtrInfoList->clear();
    comnAddAddrsInfo(p_CtrInfoList, p_ConcentratorInfoList, p_MeterInfoList, p_SchemeCfgInfoList);
    uchar dstFreq=freq&0x0f;

    uchar dstPrtcl=uchar(freq)>>4;
    if(dstPrtcl==0x03)
        setMeterAddrsPrtcl(p_CtrInfoList,dstPrtcl);

    p_BuildNetwork_GW->setCtrInfoListAndFreq(p_CtrInfoList, dstFreq);

}

bool Script_ReadMeter_SearchMeter_Upgrade_MultiTask::config(const QMap<QString,QString> *paraDic)
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


void Script_ReadMeter_SearchMeter_Upgrade_MultiTask::execute()
{
    QString step_desc = "流程描述: \r";
    step_desc += "1. 通用组网流程，组网场景4(需要重新组网)，Wait_BuildNetFinish_Whole \r";
    step_desc += "2. 05F6设置允许台区识别，Wait_SetAeraIdentification_05F6_Finish \r";
//    step_desc += "3. 切换波特率至115200，Wait_SetBandRate_Finish \r";
    step_desc += "3. 启动15F1在线升级流程（串口文件传输给路由），Wait_StartUpdata_15F1_Finish \r";
    step_desc += "4. 查询路由运行状态（处于升级态），Wait_StaUpgradeFinish \r";
    step_desc += "5. 下发11F5激活从节点主动注册命令，时间30Min，Wait_SetStaActiveResgister_11F5_Finish \r";
    step_desc += "6. 给路由下发13F1监控从节点命令，Wait_SendAndMoniter_13F1_Finish \r";
    step_desc += "7. 给路由下发12F1，重启路由命令，Wait_SendResetRoute_12F1_Finish \r";
    step_desc += "8. 下发F1F1并发抄表，10并发，Wait_SendMultiTaskConcurrency_F1F1_Finish \r";
    step_desc += "9. 周期10F4查询路由运行状态，超时时间1h（脚本参数可配置，超时时间从下发15F1开始），Wait_QueryRouteRunStateCycles_10F4_Finish \r";
    step_desc += "10. 下发02F1查询模块升级版本，Wait_QueryStaVersion_02F1_Finish \r";
    step_desc += "11. 对工装上的单相表拉高管脚，对事件上报报文回复确认，Wait_EventReport_06F5_Finish \n";

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
        emScriptRunState = Wait_SetAeraIdentification_05F6_Finish;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--设置允许台区识别(05F6)，等待--确认");

        sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, INSIGNIFICANCE, p_SetAreaIdentityFlag_05F6);
        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
    }
    p_maxAllowTimer->start((timerForReachThresld+timerAfterReachThresld)*1000);//此处要启动脚本的最大执行时间定时器
}

void Script_ReadMeter_SearchMeter_Upgrade_MultiTask::stop()
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

void Script_ReadMeter_SearchMeter_Upgrade_MultiTask::processMsg(DvcType dvcType, int id, uchar *data, int datalen)
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

//            timeoutProc_upgarde();

            //  先让路由复位  20240124  yang
            tryTimes = 0;
            emScriptRunState = Wait_CcoHardInit_Finish;
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送-CCO硬件初始化(01F1)，等待--确认");

            sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, INSIGNIFICANCE, p_HardReset_01F1);
            p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
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
#ifdef DIRECT_CCO_MODE
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "Direct CCO mode: ignore STA fixture slot frame.");
            return;
#endif
            if(dvcType==SingleSTA)
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到来自单通报文：%1").arg(QString(recvTempData.toHex())));
            else
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到来自三通报文：%1").arg(QString(recvTempData.toHex())));


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

void Script_ReadMeter_SearchMeter_Upgrade_MultiTask::processCtrlDvcRes(DvcType dvcType, QList<int> idList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params)
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
        case Wait_SetBandRate_Finish:
        {
//            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "工装波特率115200设置完成，等待10s！\n");
//            p_delayTimer->start(10*1000);

            break;
        }
        case Wait_QueryStaVersion_02F1_Finish:
        {
            if(dvcType == SingleSTA)
            {
                emScriptRunState = Wait_EventReport_06F5_Finish;
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("单通事件管脚已拉高"));
                QList<double> sendParams_;
                sendParams_.clear();
                idList.clear();
                idList = getDvcIdList(ThreeSTA);
                p_AbstractScriptHost->controlDvc(ThreeSTA,idList,CtrlCmd_EventPinHigh,sendParams_);
            }
            break;
        }
        case Wait_EventReport_06F5_Finish:
        {
            if(dvcType == SingleSTA)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("单通事件管脚已拉高，开始拉高三通事件管脚"));
                QList<double> sendParams_;
                sendParams_.clear();
                idList.clear();
                idList = getDvcIdList(ThreeSTA);
                p_AbstractScriptHost->controlDvc(ThreeSTA,idList,CtrlCmd_EventPinHigh,sendParams_);
            }
            else if(dvcType == ThreeSTA)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("三通事件管脚已拉高"));
            }

            break;
        }
        case ScriptSuccess:
        {
            if(dvcType == SingleSTA)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("单通事件管脚已拉低"));

                QList<double> sendParams_;
                sendParams_.clear();
                idList.clear();
                idList = getDvcIdList(ThreeSTA);
                p_AbstractScriptHost->controlDvc(ThreeSTA,idList,CtrlCmd_EventPinLow,sendParams_);
            }
            else if(dvcType == ThreeSTA)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("三通事件管脚已拉低"));

                if(recvEventNum >= 1)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Success, "脚本执行成功 【GW-CCO-F022-0004-V01】");
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("Wait_EventReport_06F5_Finish 未收到电表事件上报 【GW-CCO-F022-0004-V01】"));
                }
            }
            break;

        }
        default:
        {
            break;
        }
    }
}

QList<int> Script_ReadMeter_SearchMeter_Upgrade_MultiTask::getDvcIdList(DvcType dvcType)
{
    QList<int> dvcIdList;
    dvcIdList.clear();
    switch (dvcType)
    {
    case SingleSTA:
    {
        QList<int> tempIdList;
        QMap<int,MeterInfoForSingleNet*>::const_iterator const_it;
        for (const_it = (p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList)->constBegin(); const_it != (p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList)->constEnd(); const_it++)
        {
            if(const_it.value()->slotPosition == SingleSTA)
            {
                tempIdList.append(const_it.value()->dvcId);
            }
        }
        if(tempIdList.size() == 0)
        {
            qDebug()<<QString("单通槽位档案未找到");
        }
        foreach(int id,tempIdList)
        {
            if(!dvcIdList.contains(id))
            {
                dvcIdList.append(id);
            }
        }
        break;
    }
    case ThreeSTA:
    {
        QList<int> tempIdList;
        QMap<int,MeterInfoForSingleNet*>::const_iterator const_it;
        for (const_it = (p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList)->constBegin(); const_it != (p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList)->constEnd(); const_it++)
        {
            if(const_it.value()->slotPosition == ThreeSTA)
            {
                tempIdList.append(const_it.value()->dvcId);
            }
        }
        if(tempIdList.size() == 0)
        {
            qDebug()<<QString("三通槽位档案未找到");
        }
        foreach(int id,tempIdList)
        {
            if(!dvcIdList.contains(id))
            {
                dvcIdList.append(id);
            }
        }
        break;
    }
    case CCO_GW:
    {
        dvcIdList.append(p_CtrInfoList->at(0)->dvcId);
        break;
    }
    case CCO_NW:
    {
        dvcIdList.append(p_CtrInfoList->at(0)->dvcId);
        break;
    }
    case CJQ:
    {
        QList<int> tempIdList;
        QMap<int,MeterInfoForSingleNet*>::const_iterator const_it;
        for (const_it = (p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList)->constBegin(); const_it != (p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList)->constEnd(); const_it++)
        {
            if(const_it.value()->slotPosition == CJQ)
            {
                tempIdList.append(const_it.value()->dvcId);
            }
        }
        if(tempIdList.size() == 0)
        {
            qDebug()<<QString("采集器槽位档案未找到");
        }
        foreach(int id,tempIdList)
        {
            if(!dvcIdList.contains(id))
            {
                dvcIdList.append(id);
            }
        }
        break;
    }
    default:
    {
        break;
    }

    }

    if(dvcIdList.size()==0)
    {
        dvcIdList.append(p_CtrInfoList->at(0)->dvcId);
    }

    return dvcIdList;
}

void Script_ReadMeter_SearchMeter_Upgrade_MultiTask::processMsgFromCCO(DvcType dvcType, int dvcId)
{
    if(dvcId!=p_CtrInfoList->at(0)->ctrlID)
        return;
    QByteArray completeFrame;
    QByteArray& buf = p_CtrInfoList->at(0)->buf;
    bool haveCompleteMsg=true;
    while(extractAndProcess3762Frame(buf,completeFrame))
    {
        qInfo()<<QString("CCO-解析前 buf3762=%1").arg(QString((completeFrame.toHex())));
        shared_ptr<Frame3762Base> p_Frame3762Base = p_Frame3762Helper->DecodeLocalMsg(&(completeFrame),haveCompleteMsg);
        qInfo()<<QString("CCO-解析后 buf3762=%1").arg(QString((completeFrame.toHex())));
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
            case Wait_CcoHardInit_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "硬件初始化，路由回复确认 \n");
                    p_delayTimer->start(10*1000);
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_SetAeraIdentification_05F6_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "设置台区区分标志使能，路由回复确认 \n");

                    LoadUpdateFile();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "等待升级文件传输完成");
                    fileIndex=0;
                    emScriptRunState = Wait_StartUpdata_15F1_Finish_2;
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,0,p_FileTransfer_15F1_Down);
                    p_timer->start(10*1000);

//                    // 暂停路由
//                    tryTimes = 0;
//                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "暂停路由");
//                    sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, 0, p_RouterPause_12F2);
//                    emScriptRunState = Wait_00F1_for_12F2_UpgrdSta;
//                    p_timer->start(10*1000);

                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_00F1_for_12F2_UpgrdSta:
            {
                if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "暂停路由回复确认 \n");

                    // 清除下装文件
                    // LoadUpdateFile();

                    emScriptRunState = Wait_StartUpdata_15F1_Finish_1;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "等待清除下装文件");
                    sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, 0, p_FileTransfer_15F1_Down);
                    p_timer->start(10*1000);

    //                    tryTimes = 0;
    //                    emScriptRunState=Wait_SetBandRate_Finish;
    //                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--设置波特率115200(11F4)，等待--回复");

    //                    sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, INSIGNIFICANCE, p_SetBaudRate_11F4);
    //                    p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }

            case Wait_SetBandRate_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();//接收到一条完整报文
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "波特率115200设置--回复确认！ \n");

                    //**设置工装的波特率为115200
                    sendParams.clear();
                    idList.clear();
                    emDvcType = CCO_GW;
                    sendParams.append(115200);
                    idList = getDvcIdList(emDvcType);
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "设置工装槽位波特率为115200");

                    p_AbstractScriptHost->controlDvc(emDvcType, idList, CtrlCmd_SetBaudRate, sendParams);
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_StartUpdata_15F1_Finish_1:
            {
                uchar dtValue3762=get3762Dt(p_Frame3762Base->dt1_,p_Frame3762Base->dt2_);
                if(p_Frame3762Base->afn_ == 0x15&&dtValue3762==1&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到15F1_Up清除下装文件应答 \n");
                    p_timer->stop();

                    LoadUpdateFile();
                    tryTimes = 0;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "等待升级文件传输完成");
                    fileIndex=0;
                    emScriptRunState = Wait_StartUpdata_15F1_Finish_2;
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,0,p_FileTransfer_15F1_Down);
                    p_timer->start(10*1000);
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_StartUpdata_15F1_Finish_2:
            {
                uchar dtValue3762=get3762Dt(p_Frame3762Base->dt1_,p_Frame3762Base->dt2_);
                if(p_Frame3762Base->afn_ == 0x15&&dtValue3762==1&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();

                    shared_ptr<Afn15F1> p_FileTransfer_15F1_Up=std::dynamic_pointer_cast<Afn15F1>(p_Frame3762Base);
                    Refresh_TestResult_15F1(p_FileTransfer_15F1_Up);

                    ushort sucSegs = Refresh_SuccessCnt_15F1();

                    if(sucSegs==p_FileTransfer_15F1_Down->file_transfer_unit_.total_num_)
                    {
                        emScriptRunState = Wait_StaUpgradeFinish;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "开始等待模块升级，20s后查询10F4路由运行状态");
                        p_delayTimer->start(20*1000);
                        p_upgardeTimer->start(60*60*1000);   // 模块升级 --60min超时
                    }
                    else if(!failedSegments.isEmpty())
                    {
                        // 有失败的段，进行重传
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,
                            QString("检测到失败段，准备重传。失败段数量：%1").arg(failedSegments.size()));
                        RetransmitFailedSegments();
                    }
                    else
                    {
                        // 继续发送下一段
                        if(++fileIndex < p_FileTransfer_15F1_Down->file_transfer_unit_.total_num_)
                        {
                            sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID,0, p_FileTransfer_15F1_Down);
                            p_timer->start(1*1000);
                        }
                        else
                        {
                            // 所有段发送完毕，但不是全部成功
                            p_maxAllowTimer->stop();
                            p_AbstractScriptHost->updateProgress(ProcessState_Failed,
                                                                 QString("升级文件传输成功率：%1%; 升级文件传输耗时：%2秒;")
                                                                 .arg((double)(sucSegs) / (double)(totalSegs) * 100)
                                                                 .arg(p_CtrInfoList->at(0)->successConsume[0]));
                            emScriptRunState = ScriptSuccess;
                        }
                    }
//                    ushort sucSegs=0;
//                    if(++fileIndex==p_FileTransfer_15F1_Down->file_transfer_unit_.total_num_)
//                    {
//                        sucSegs=Refresh_SuccessCnt_15F1();
//                    }

//                    if(sucSegs==p_FileTransfer_15F1_Down->file_transfer_unit_.total_num_)
//                    {
//                        emScriptRunState = Wait_StaUpgradeFinish;
//                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "开始等待模块升级，20s后查询10F4路由运行状态");
//                        p_delayTimer->start(20*1000);
//                        p_upgardeTimer->start(60*60*1000);   // 模块升级 --60min超时
//                    }
//                    else
//                    {
//                        if(fileIndex<totalSegs)
//                        {
//                            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,0,p_FileTransfer_15F1_Down);
//                            p_timer->start(30*1000);
//                        }
//                        else
//                        {
//                            p_maxAllowTimer->stop();
//                            emScriptRunState=ScriptSuccess;
//                            p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("升级文件传输成功率：%1%").arg((double)(sucSegs)/(double)(totalSegs)*100));
//                        }
//                    }
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_StaUpgradeFinish:
            {
                uchar dtValue3762=get3762Dt(p_Frame3762Base->dt1_,p_Frame3762Base->dt2_);

                dtValue3762=get3762Dt(p_Frame3762Base->dt1_,p_Frame3762Base->dt2_);
                if(p_Frame3762Base->afn_ == 0x10&&dtValue3762==4&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    qDebug()<<"收到10F4_Up报文******"<<"状态机的值："<<emScriptRunState;
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到10F4_Up报文");

                    shared_ptr<Afn10F4> p_CcoRunStateInfo_10F4_Up=std::dynamic_pointer_cast<Afn10F4>(p_Frame3762Base);
                    QStringList rptEvntFlagList;
                    QStringList workFlagList;
                    QStringList ccoFinishFlagList;
                    QStringList crntStateList;
                    QStringList platIdentFlagList;
                    QStringList evntRptStateFlagList;
                    QStringList registerAllowStateList;
                    QStringList workStateList;
                    QStringList workStepList;

                    rptEvntFlagList.clear();
                    rptEvntFlagList<<"无从节点上报事件"<<"有从节点上报事件";
                    workFlagList.clear();
                    workFlagList<<"停止工作"<<"正在工作";
                    ccoFinishFlagList.clear();
                    ccoFinishFlagList<<"未完成"<<"路由学习完成";
                    crntStateList.clear();
                    crntStateList<<"抄表"<<"搜表"<<"升级"<<"其他";
                    platIdentFlagList.clear();
                    platIdentFlagList<<"不允许"<<"允许";
                    evntRptStateFlagList.clear();
                    evntRptStateFlagList<<"不允许"<<"允许";
                    registerAllowStateList.clear();
                    registerAllowStateList<<"不允许"<<"允许";
                    workStateList.clear();
                    workStateList<<"不允许"<<"允许";
                    workStepList.clear();
                    workStepList<<"未定义"<<"初始状态"<<"直抄"<<"中继"<<"监控状态"<<"广播状态"<<"召读电表"<<"侦听信息"<<"空闲";

                    QString res=QString("\r\n\r\n\r\n\r\n\r\n\r\n路由运行状态信息如下：\r\n\r\n\r\n");
                    res+=QString("[运行状态字]\r\n纠错编码: %1;\r\n上报事件标志: %2;\r\n工作标志: %3;\r\n路由完成标志: %4;\r\n\r\n")
                            .arg(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.operate_state_word_.error_correction_coding_)
                            .arg(rptEvntFlagList.at(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.operate_state_word_.report_event_flag_))
                            .arg(workFlagList.at(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.operate_state_word_.report_work_flag_))
                            .arg(ccoFinishFlagList.at(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.operate_state_word_.router_complete_flag_));
                    res+=QString("从节点总数量: %1;\r\n")
                            .arg(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.node_total_num_);
                    res+=QString("已抄从节点数量: %1;\r\n")
                            .arg(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.have_read_node_num_);
                    res+=QString("中继抄到从节点数量: %1;\r\n\r\n")
                            .arg(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.read_by_relay_node_num_);
                    res+=QString("[工作开关]\r\n当前状态: %1;\r\n台区识别使能标志: %2;\r\n事件上报状态标志: %3;\r\n注册允许标志: %4;\r\n工作状态: %5;\r\n\r\n")
                            .arg(crntStateList.at(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.work_switch_.current_state_))
                            .arg(platIdentFlagList.at(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.work_switch_.area_difference_flag_))
                            .arg(evntRptStateFlagList.at(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.work_switch_.event_report_flag_))
                            .arg(registerAllowStateList.at(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.work_switch_.register_allow_flag_))
                            .arg(workStateList.at(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.work_switch_.work_state));
                    res+=QString("载波通信速率: %1;\r\n")
                            .arg(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.communication_rate_);
                    res+=QString("第1相中继级别: %1;\r\n")
                            .arg(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.relay_level_phase_1_);
                    res+=QString("第2相中继级别: %1;\r\n")
                            .arg(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.relay_level_phase_2_);
                    res+=QString("第3相中继级别: %1;\r\n")
                            .arg(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.relay_level_phase_3_);
                    res+=QString("第1相工作步骤: %1;\r\n")
                            .arg(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.work_procedure_phase_1_<9?workStepList.at(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.work_procedure_phase_1_):"备用");
                    res+=QString("第2相工作步骤: %1;\r\n")
                            .arg(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.work_procedure_phase_2_<9?workStepList.at(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.work_procedure_phase_2_):"备用");
                    res+=QString("第3相工作步骤: %1;\r\n")
                            .arg(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.work_procedure_phase_3_<9?workStepList.at(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.work_procedure_phase_3_):"备用");
                    res+=QString("\r\n\r\n\r\n\r\n\r\n\r\n");

                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, res);

                    if(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.work_switch_.current_state_ != 2)
                    {
                        emScriptRunState = Wait_SetStaActiveResgister_11F5_Finish;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("升级完成，发送--11F5启动搜表（30min），等待--确认"));
                        sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, INSIGNIFICANCE, p_ActiveRegister_11F5);
                        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                        p_meterSearchTimer->start(30*65*1000);

                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "路由运行状态显示升级STA未完成，等待60秒后继续读取10F4");
                        p_delayTimer->start(60*1000);
                    }
//                    if(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.operate_state_word_.report_work_flag_==1 && p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.work_switch_.current_state_==2)
//                    {
//                        // 激活从节点主动注册
//                        emScriptRunState = Wait_SetStaActiveResgister_11F5_Finish;
//                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("路由处于升级状态，发送--11F5启动搜表（30min），等待--确认"));
//                        sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, INSIGNIFICANCE, p_ActiveRegister_11F5);
//                        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
//                        p_meterSearchTimer->start(30*65*1000);
//                    }
//                    else
//                    {
//                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("路由运行状态有误，未处于升级状态"));
//                    }
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }

            case Wait_SetStaActiveResgister_11F5_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "激活从节点主动注册--路由回复确认！\n");

                    // 给路由下发抄读任务
                    readInfoInit();
                    emScriptRunState = Wait_SendResetRoute_12F1_Finish;
                    sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, INSIGNIFICANCE, p_RouterRestart_12F1);
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--路由重启（12F1），等待--确认");
                    p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);

                }
                else if(p_Frame3762Base->afn_ == 0x06&&p_Frame3762Base->dt1_==0x08&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    shared_ptr<Afn06F4> p_ReportRegisterNode_Up=dynamic_pointer_cast<Afn06F4>(p_Frame3762Base);
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("上报注册从节点，表号%1").arg(QString(QByteArray(p_ReportRegisterNode_Up->report_node_info_unit_.report_node_address_.addr,6).toHex())));
                    bool reportTwiceFlag = false;
                    for(int i=0;i<nodeInfoList.size();i++)
                    {
                        if(memcmp(nodeInfoList.at(i).nodeAddress.addr,p_ReportRegisterNode_Up->report_node_info_unit_.report_node_address_.addr,6)==0)
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "上报表号重复");
                            reportTwiceFlag = true;
                        }
                    }
                    if(reportTwiceFlag == false)
                    {
                        NodeInfo_Struct node;
                        memcpy(node.nodeAddress.addr,p_ReportRegisterNode_Up->report_node_info_unit_.report_node_address_.addr,6);
                        node.protocol=p_ReportRegisterNode_Up->report_node_info_unit_.report_node_protocol_;
                        node.deviceType=p_ReportRegisterNode_Up->report_node_info_unit_.report_node_device_type_;
                        nodeInfoList.append(node);
                    }

                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "回复确认，当前搜表上报数:" + QString::number(nodeInfoList.size()));

                    sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, INSIGNIFICANCE, p_Confirm_00F1);
                }
                else if(p_Frame3762Base->afn_ == 0x06&&p_Frame3762Base->dt1_==0x04&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_delayTimer->stop();
                    shared_ptr<Afn06F3> p_ReportRouterState_Up=dynamic_pointer_cast<Afn06F3>(p_Frame3762Base);
                    if(p_ReportRouterState_Up->router_work_task_change_==0x02)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "路由上报工况变动，搜表结束，回复确认");

                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_Confirm_00F1);
                        search_flag = true;
                        p_meterSearchTimer->stop();

                        if(isAllExist()==true)
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "档案所有表已上报");
                        }
                        else
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "有档案中的表未上报，上报数："
                                                                 + QString::number(nodeInfoList.size()) + "，档案数："
                                                                 + QString::number(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().size()));
                        }
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "路由上报工况变动，不符合要求");
                    }
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_SendResetRoute_12F1_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到确认，等待14F1请求");
                    emScriptRunState = Wait_ReadMeter_Finish;
                    p_delayTimer->start(60*1000);
                }
                else if(p_Frame3762Base->afn_ == 0x06&&p_Frame3762Base->dt1_==0x08&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    shared_ptr<Afn06F4> p_ReportRegisterNode_Up=dynamic_pointer_cast<Afn06F4>(p_Frame3762Base);
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("上报注册从节点，表号%1").arg(QString(QByteArray(p_ReportRegisterNode_Up->report_node_info_unit_.report_node_address_.addr,6).toHex())));
                    bool reportTwiceFlag = false;
                    for(int i=0;i<nodeInfoList.size();i++)
                    {
                        if(memcmp(nodeInfoList.at(i).nodeAddress.addr,p_ReportRegisterNode_Up->report_node_info_unit_.report_node_address_.addr,6)==0)
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "上报表号重复");
                            reportTwiceFlag = true;
                        }
                    }
                    if(reportTwiceFlag == false)
                    {
                        NodeInfo_Struct node;
                        memcpy(node.nodeAddress.addr,p_ReportRegisterNode_Up->report_node_info_unit_.report_node_address_.addr,6);
                        node.protocol=p_ReportRegisterNode_Up->report_node_info_unit_.report_node_protocol_;
                        node.deviceType=p_ReportRegisterNode_Up->report_node_info_unit_.report_node_device_type_;
                        nodeInfoList.append(node);
                    }

                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "回复确认，当前搜表上报数:" + QString::number(nodeInfoList.size()));
                    sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, INSIGNIFICANCE, p_Confirm_00F1);
                }
                else if(p_Frame3762Base->afn_ == 0x06&&p_Frame3762Base->dt1_==0x04&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_delayTimer->stop();
                    shared_ptr<Afn06F3> p_ReportRouterState_Up=dynamic_pointer_cast<Afn06F3>(p_Frame3762Base);
                    if(p_ReportRouterState_Up->router_work_task_change_==0x02)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "路由上报工况变动，搜表结束，回复确认");
                        search_flag = true;
                        p_meterSearchTimer->stop();

                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_Confirm_00F1);
                        if(isAllExist()==true)
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "档案所有表已上报");
                        }
                        else
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing,"有档案中的表未上报，上报数："+QString::number(nodeInfoList.size())+"，档案数："+QString::number(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().size()));
                        }

                        bool all14F1Done = !readInfoList.isEmpty();
                        for(int i=0; i<readInfoList.size(); ++i)
                        {
                            if(readInfoList.at(i).requestFlag_14F1==false || readInfoList.at(i).readFlag_14F1==WaitRead)
                            {
                                all14F1Done = false;
                                break;
                            }
                        }
                        if(all14F1Done && endFlag_14F1==false)
                        {
                            p_14F1Timer->stop();
                            endFlag_14F1 = true;
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "搜表结束时14F1已完成，进入结果检查");
                        }
                        resultCheck();
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "路由上报工况变动，不符合要求");
                    }
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_ReadMeter_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到确认");

                    int noReq_14F1=0;
                    for(int i=0;i<readInfoList.size();i++)
                    {
                        if(readInfoList.at(i).requestFlag_14F1==false)
                        {
                            noReq_14F1++;
                        }
                    }
                    if(noReq_14F1>0)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("路由14F1请求表不全，%1个表未请求").arg(noReq_14F1));
                    }

                    endFlag_14F1=true;
                    resultCheck();
                }
                else if(p_Frame3762Base->afn_==0x14&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_delayTimer->stop();
                    p_timer->stop();
                    p_14F1Timer->stop();
                    shared_ptr<Afn14F1> p_RouterRequestRead_14F1_Up=dynamic_pointer_cast<Afn14F1>(p_Frame3762Base);
                    int currentMeterIndex=getReadInfo(p_RouterRequestRead_14F1_Up->node_address_);

                    // 检查表索引是否有效
                    if(currentMeterIndex < 0) {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,
                                                             QString("14F1请求的表在档案中未找到！"));
                        // 回复否认
                        sendMsg(dvcType, dvcId, INSIGNIFICANCE, p_Deny_00F2);
                        continue;
                    }

                    // 检查该表是否被其他任务占用
                    QString addr = QString(QByteArray((char*)readInfoList[currentMeterIndex].meterNo.addr, 6).toHex());
                    if(m_busyMeters.contains(addr)) {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,
                                                             QString("14F1请求的表%1正被占用，立即强制解锁并回复").arg(addr));

                        // 强制解锁该表，优先处理14F1
                        unlockMeter(readInfoList[currentMeterIndex].meterNo.addr);
                        
                        // 暂停其他任务，给14F1优先权
                        p_13F1Timer->stop();
                        p_F1F1Timer->stop();
                    }

                    readInfoList[currentMeterIndex].requestFlag_14F1=true;
                    requestNum_14F1++;
                    
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, 
                        QString("收到第%1个14F1请求，表号%2，立即回复抄读数据项")
                        .arg(requestNum_14F1)
                        .arg(QString(QByteArray(p_RouterRequestRead_14F1_Up->node_address_.addr,6).toHex())));
                    
                    // 立即回复14F1，不启动定时器
                    sendMsg(dvcType, dvcId, currentMeterIndex, p_RouterRequestRead_14F1);
                    
                    if(requestNum_14F1==1)
                    {
                        // 第一次收到14F1后，延迟启动其他任务，避免竞争
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, 
                            QString("第1个14F1已回复，20秒后启动13F1和F1F1任务"));

                        QTimer::singleShot(20000, this, [this]() {
                            // 启动13F1任务
                            if(requestNum_13F1 < readInfoList.size()) {
                                int meterIdx = requestNum_13F1;
                                requestNum_13F1++;

                                if(tryLockMeter(readInfoList[meterIdx].meterNo.addr, "13F1")) {
                                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,
                                        QString("发送第%1条13F1抄表").arg(requestNum_13F1));
                                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,meterIdx,p_MonitorSlaveNode_13F1);
                                    p_13F1Timer->start(100*1000);
                                } else {
                                    requestNum_13F1--;
                                }
                            }
                        });

                        QTimer::singleShot(30000, this, [this]() {
                            // 启动F1F1任务
                            if(requestNum_F1F1 < readInfoList.size()) {
                                int meterIdx = requestNum_F1F1;
                                requestNum_F1F1++;

                                if(tryLockMeter(readInfoList[meterIdx].meterNo.addr, "F1F1")) {
                                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,
                                        QString("发送第%1条F1F1抄表").arg(requestNum_F1F1));
                                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,meterIdx,p_ParallelReadMeter_F1F1);
                                    p_F1F1Timer->start(100*1000);
                                } else {
                                    requestNum_F1F1--;
                                }
                            }
                        });
                    }
                    else if(requestNum_14F1>readInfoList.size()*2)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到第%1个14F1请求，超过表数的2倍，14F1一轮抄表结束，06F2上报抄读成功报文数%2，上报数据为空报文数%3").arg(requestNum_14F1).arg(reportSucNum_14F1).arg(reportFailNum_14F1));
                        sendMsg(p_CtrInfoList->at(0)->slotPosition,p_CtrInfoList->at(0)->dvcId,INSIGNIFICANCE,p_RouterPause_12F2);
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--路由暂停（12F2），等待--确认");
                        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                    {
                        // 其他14F1请求也立即回复，不等待
                        // 但需要检查是否有其他任务占用该表
                        if(m_busyMeters.contains(addr)) {
                            // 暂停其他任务2秒，让14F1优先
                            p_13F1Timer->stop();
                            p_F1F1Timer->stop();
                            
                            QTimer::singleShot(2000, this, [this]() {
                                // 恢复其他任务
                                if(requestNum_13F1 < readInfoList.size() && endFlag_13F1 == false) {
                                    for(int i = requestNum_13F1; i < readInfoList.size(); i++) {
                                        if(tryLockMeter(readInfoList[i].meterNo.addr, "13F1")) {
                                            requestNum_13F1 = i + 1;
                                            sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, i, p_MonitorSlaveNode_13F1);
                                            p_13F1Timer->start(100*1000);
                                            break;
                                        }
                                    }
                                }
                                
                                if(requestNum_F1F1 < readInfoList.size() && endFlag_F1F1 == false) {
                                    for(int i = requestNum_F1F1; i < readInfoList.size(); i++) {
                                        if(tryLockMeter(readInfoList[i].meterNo.addr, "F1F1")) {
                                            requestNum_F1F1 = i + 1;
                                            sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, i, p_ParallelReadMeter_F1F1);
                                            p_F1F1Timer->start(100*1000);
                                            break;
                                        }
                                    }
                                }
                            });
                        }
                    }
                }
                else if(p_Frame3762Base->afn_==0x06&&p_Frame3762Base->dt1_==0x02&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    shared_ptr<Afn06F2> p_ReportReadData_Up = dynamic_pointer_cast<Afn06F2>(p_Frame3762Base);
                    Address srcAddr;
                    srcAddr = extractAddressFromAfn06F2(p_ReportReadData_Up);

                    unlockMeter(srcAddr.addr);

                    int currentMeterIndex = getReadInfo(srcAddr);

                    if(currentMeterIndex < 0) {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,
                                                             QString(" 错误：06F2地址在档案中未找到！"));
                        sendMsg(dvcType, dvcId, INSIGNIFICANCE, p_Confirm_00F1);
                        continue;  // 改为continue，继续处理下一个帧
                    }

                    //记录修改前的状态
                    ReadFlag oldFlag = readInfoList[currentMeterIndex].readFlag_14F1;

                    if(p_ReportReadData_Up->report_data_unit_.frame_length_ > 0) {
                        reportSucNum_14F1++;
                        readInfoList[currentMeterIndex].readFlag_14F1 = ReadSuccess;

                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,
                                                             QString("收到第%1条06F2成功上报，表号%2，readFlag: %3→ReadSuccess")
                                                             .arg(reportSucNum_14F1)
                                                             .arg(QString(QByteArray((char*)srcAddr.addr, 6).toHex()))
                                                             .arg((int)oldFlag));
                    } else {
                        reportFailNum_14F1++;
                        readInfoList[currentMeterIndex].readFlag_14F1 = ReadFail;

                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,
                                                             QString("收到第%1条06F2数据为空，表号%2，readFlag: %3→ReadFail")
                                                             .arg(reportFailNum_14F1)
                                                             .arg(QString(QByteArray((char*)srcAddr.addr, 6).toHex()))
                                                             .arg((int)oldFlag));
                    }

                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "回复确认");
                    sendMsg(dvcType, dvcId, INSIGNIFICANCE, p_Confirm_00F1);
                }
                else if(p_Frame3762Base->afn_==0x13&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    // 收到13F1抄读的数据
                    p_13F1Timer->stop();
                    shared_ptr<Afn13F1> p_MonitorSlaveNode_13F1_Up=dynamic_pointer_cast<Afn13F1>(p_Frame3762Base);
                    Address srcAddr;
                    memcpy(srcAddr.addr,p_MonitorSlaveNode_13F1_Up->address_field_.src_addr,6);

                    unlockMeter(srcAddr.addr);

                    int currentMeterIndex=getReadInfo(srcAddr);
                    if(p_MonitorSlaveNode_13F1_Up->data_field_up_.frame_length_>0)
                    {
                        responseSucNum_13F1++;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到第%1条13F1抄读成功回复数据，表号").arg(responseSucNum_13F1)+QByteArray(srcAddr.addr,6).toHex());
                        readInfoList[currentMeterIndex].readFlag_13F1=ReadSuccess;
                    }
                    else
                    {
                        responseFailNum_13F1++;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到第%1条13F1抄读回复数据内容为空，表号").arg(responseFailNum_13F1)+QByteArray(srcAddr.addr,6).toHex());
                        readInfoList[currentMeterIndex].readFlag_13F1=ReadFail;
                    }

                    //继续进行下一只表的13F1抄读
                    if(requestNum_13F1<readInfoList.size())
                    {
                        int meterIdx = requestNum_13F1;
                        requestNum_13F1++;

                        int randomDelay = 50 + (qrand() % 100);
                        QTimer::singleShot(randomDelay, this, [this, meterIdx]() {
                            if(tryLockMeter(readInfoList[meterIdx].meterNo.addr, "13F1")) {
                                sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, meterIdx, p_MonitorSlaveNode_13F1);
                                p_13F1Timer->start(100*1000);
                            } else {
                                requestNum_13F1--;
                                // 2秒后重试
                                QTimer::singleShot(2000, this, [this, meterIdx]() {
                                    if(requestNum_13F1 < readInfoList.size() &&
                                            tryLockMeter(readInfoList[meterIdx].meterNo.addr, "13F1")) {
                                        requestNum_13F1++;
                                        sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, meterIdx, p_MonitorSlaveNode_13F1);
                                        p_13F1Timer->start(100*1000);
                                    }
                                });
                            }
                        });
//                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("发送第%1条13F1继续抄下一只表").arg(++requestNum_13F1));
//                        sendMsg(dvcType,dvcId,++readNo_13F1,p_MonitorSlaveNode_13F1);
//                        p_13F1Timer->start(100*1000);
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("13F1一轮抄表结束，发送报文数%1，抄读成功报文数%2，回复数据为空报文数%3，回复超时数%4").arg(requestNum_13F1).arg(responseSucNum_13F1).arg(responseFailNum_13F1).arg(timeoutNum_13F1));
                        endFlag_13F1=true;
                        resultCheck();
                    }
                }
                else if(p_Frame3762Base->afn_==char(0xF1)&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    // 收到F1F1抄读的数据
                    p_F1F1Timer->stop();
                    shared_ptr<AfnF1F1> p_ParallelReadMeter_F1F1_Up=dynamic_pointer_cast<AfnF1F1>(p_Frame3762Base);
                    Address srcAddr;
                    memcpy(srcAddr.addr,p_ParallelReadMeter_F1F1_Up->address_field_.src_addr,6);

                    unlockMeter(srcAddr.addr);

                    int currentMeterIndex=getReadInfo(srcAddr);
                    if(p_ParallelReadMeter_F1F1_Up->unit_up_.frame_length_>0)
                    {
                        responseSucNum_F1F1++;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到第%1条F1F1抄读成功回复数据，表号").arg(responseSucNum_F1F1)+QByteArray(srcAddr.addr,6).toHex());
                        readInfoList[currentMeterIndex].readFlag_F1F1=ReadSuccess;
                    }
                    else
                    {
                        responseFailNum_F1F1++;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到第%1条F1F1抄读回复数据内容为空，表号").arg(responseFailNum_F1F1)+QByteArray(srcAddr.addr,6).toHex());
                        readInfoList[currentMeterIndex].readFlag_F1F1=ReadFail;
                    }

                    QTimer::singleShot(10000, this, [this]() {
                        if(requestNum_F1F1 < readInfoList.size()) {
                            bool foundAvailable = false;
                            int attempts = 0;
                            int startIdx = requestNum_F1F1;

                            // 尝试找到一个未被占用的表
                            while(requestNum_F1F1 < readInfoList.size() && !foundAvailable && attempts < readInfoList.size()) {
                                int meterIdx = requestNum_F1F1;
                                QString addr = QString(QByteArray((char*)readInfoList[meterIdx].meterNo.addr, 6).toHex());

                                if(tryLockMeter(readInfoList[meterIdx].meterNo.addr, "F1F1")) {
                                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,
                                                                         QString("发送第%1条F1F1抄表，表号%2").arg(requestNum_F1F1).arg(addr));

                                    sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, meterIdx, p_ParallelReadMeter_F1F1);
                                    p_F1F1Timer->start(100*1000);
                                    requestNum_F1F1++;
                                    foundAvailable = true;
                                } else {
                                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,
                                                                         QString(" [F1F1] 表%1被占用，尝试下一个").arg(addr));
                                    requestNum_F1F1++;
                                    attempts++;
                                }
                            }

                            if(!foundAvailable) {
                                p_AbstractScriptHost->updateProgress(ProcessState_Processing,
                                                                     " [F1F1] 所有表都被占用，5秒后重试");

                                // 5秒后从头开始重试
                                QTimer::singleShot(5000, this, [this, startIdx]() {
                                    requestNum_F1F1 = startIdx;

                                    // 再次尝试
                                    if(requestNum_F1F1 < readInfoList.size()) {
                                        int meterIdx = requestNum_F1F1;
                                        if(tryLockMeter(readInfoList[meterIdx].meterNo.addr, "F1F1")) {
                                            p_AbstractScriptHost->updateProgress(ProcessState_Processing,
                                                                                 QString("F1F1重试成功，发送第%1条").arg(requestNum_F1F1));
                                            sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, meterIdx, p_ParallelReadMeter_F1F1);
                                            p_F1F1Timer->start(100*1000);
                                            requestNum_F1F1++;
                                        } else {
                                            p_AbstractScriptHost->updateProgress(ProcessState_Processing,
                                                                                 " [F1F1] 重试失败，F1F1功能跳过");
                                            // 标记F1F1已结束
                                            endFlag_F1F1 = true;
                                            resultCheck();
                                        }
                                    }
                                });
                            }
                        }
                        else
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("F1F1一轮抄表结束，发送报文数%1，抄读成功报文数%2，回复数据为空报文数%3，回复超时数%4").arg(requestNum_F1F1).arg(responseSucNum_F1F1).arg(responseFailNum_F1F1).arg(timeoutNum_F1F1));
                            endFlag_F1F1=true;
                            resultCheck();
                        }
                    });
                }
                else if(p_Frame3762Base->afn_ == 0x06&&p_Frame3762Base->dt1_==0x08&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    shared_ptr<Afn06F4> p_ReportRegisterNode_Up=dynamic_pointer_cast<Afn06F4>(p_Frame3762Base);
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("上报注册从节点，表号%1").arg(QString(QByteArray(p_ReportRegisterNode_Up->report_node_info_unit_.report_node_address_.addr,6).toHex())));
                    bool reportTwiceFlag = false;
                    for(int i=0;i<nodeInfoList.size();i++)
                    {
                        if(memcmp(nodeInfoList.at(i).nodeAddress.addr,p_ReportRegisterNode_Up->report_node_info_unit_.report_node_address_.addr,6)==0)
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "上报表号重复");
                            reportTwiceFlag = true;
                        }
                    }
                    if(reportTwiceFlag == false)
                    {
                        NodeInfo_Struct node;
                        memcpy(node.nodeAddress.addr,p_ReportRegisterNode_Up->report_node_info_unit_.report_node_address_.addr,6);
                        node.protocol=p_ReportRegisterNode_Up->report_node_info_unit_.report_node_protocol_;
                        node.deviceType=p_ReportRegisterNode_Up->report_node_info_unit_.report_node_device_type_;
                        nodeInfoList.append(node);
                    }

                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "回复确认，当前搜表上报数:" + QString::number(nodeInfoList.size()));
                    sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, INSIGNIFICANCE, p_Confirm_00F1);
                }
                else if(p_Frame3762Base->afn_==0x06&&p_Frame3762Base->dt1_==0x04&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {

                    shared_ptr<Afn06F3> p_ReportMissionState_Up=dynamic_pointer_cast<Afn06F3>(p_Frame3762Base);
                    // 收到06F3上报路由工况变动--抄表结束
                    if(p_ReportMissionState_Up->router_work_task_change_==0x01)
                    {
                        p_14F1Timer->stop();
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到06F3上报路由工况变动：抄表任务结束，回复确认"));
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("共收到%1个14F1请求，06F2上报抄读成功报文数%1，上报数据为空报文数%2").arg(requestNum_14F1).arg(reportSucNum_14F1).arg(reportFailNum_14F1));
                        endFlag_14F1=true;
                        resultCheck();
                    }
                    // 收到06F3上报路由工况变动--搜表结束
                    else if(p_ReportMissionState_Up->router_work_task_change_==0x02)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "路由上报工况变动，搜表结束，回复确认");
                        search_flag = true;
                        p_meterSearchTimer->stop();

                        if(isAllExist()==true)
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "档案所有表已上报");
                        }
                        else
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing,"有档案中的表未上报，上报数："+QString::number(nodeInfoList.size())+"，档案数："+QString::number(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().size()));
                        }
                        bool all14F1Done = !readInfoList.isEmpty();
                        for(int i=0; i<readInfoList.size(); ++i)
                        {
                            if(readInfoList.at(i).requestFlag_14F1==false || readInfoList.at(i).readFlag_14F1==WaitRead)
                            {
                                all14F1Done = false;
                                break;
                            }
                        }
                        if(all14F1Done && endFlag_14F1==false)
                        {
                            p_14F1Timer->stop();
                            endFlag_14F1 = true;
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "搜表结束时14F1已完成，进入结果检查");
                        }
                        resultCheck();
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到06F3上报路由工况变动，非抄表/搜表任务结束："+QString::number(p_ReportMissionState_Up->router_work_task_change_));
                    }
                    sendMsg(dvcType,dvcId,INSIGNIFICANCE,p_Confirm_00F1);
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }

            case Wait_QueryRouteRunStateCycles_10F4_Finish:
            {
                uchar dtValue3762=get3762Dt(p_Frame3762Base->dt1_,p_Frame3762Base->dt2_);
                if(p_Frame3762Base->afn_ == 0x10&&dtValue3762==4&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();

                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到10F4_Up报文");
                    shared_ptr<Afn10F4> p_CcoRunStateInfo_10F4_Up=std::dynamic_pointer_cast<Afn10F4>(p_Frame3762Base);

                    QStringList rptEvntFlagList;
                    QStringList workFlagList;
                    QStringList ccoFinishFlagList;
                    QStringList crntStateList;
                    QStringList platIdentFlagList;
                    QStringList evntRptStateFlagList;
                    QStringList registerAllowStateList;
                    QStringList workStateList;
                    QStringList workStepList;

                    rptEvntFlagList.clear();
                    rptEvntFlagList<<"无从节点上报事件"<<"有从节点上报事件";
                    workFlagList.clear();
                    workFlagList<<"停止工作"<<"正在工作";
                    ccoFinishFlagList.clear();
                    ccoFinishFlagList<<"未完成"<<"路由学习完成";
                    crntStateList.clear();
                    crntStateList<<"抄表"<<"搜表"<<"升级"<<"其他";
                    platIdentFlagList.clear();
                    platIdentFlagList<<"不允许"<<"允许";
                    evntRptStateFlagList.clear();
                    evntRptStateFlagList<<"不允许"<<"允许";
                    registerAllowStateList.clear();
                    registerAllowStateList<<"不允许"<<"允许";
                    workStateList.clear();
                    workStateList<<"不允许"<<"允许";
                    workStepList.clear();
                    workStepList<<"未定义"<<"初始状态"<<"直抄"<<"中继"<<"监控状态"<<"广播状态"<<"召读电表"<<"侦听信息"<<"空闲";

                    QString res=QString("\r\n\r\n\r\n\r\n\r\n\r\n路由运行状态信息如下：\r\n\r\n\r\n");
                    res+=QString("[运行状态字]\r\n纠错编码: %1;\r\n上报事件标志: %2;\r\n工作标志: %3;\r\n路由完成标志: %4;\r\n\r\n")
                            .arg(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.operate_state_word_.error_correction_coding_)
                            .arg(rptEvntFlagList.at(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.operate_state_word_.report_event_flag_))
                            .arg(workFlagList.at(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.operate_state_word_.report_work_flag_))
                            .arg(ccoFinishFlagList.at(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.operate_state_word_.router_complete_flag_));
                    res+=QString("从节点总数量: %1;\r\n")
                            .arg(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.node_total_num_);
                    res+=QString("已抄从节点数量: %1;\r\n")
                            .arg(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.have_read_node_num_);
                    res+=QString("中继抄到从节点数量: %1;\r\n\r\n")
                            .arg(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.read_by_relay_node_num_);
                    res+=QString("[工作开关]\r\n当前状态: %1;\r\n台区识别使能标志: %2;\r\n事件上报状态标志: %3;\r\n注册允许标志: %4;\r\n工作状态: %5;\r\n\r\n")
                            .arg(crntStateList.at(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.work_switch_.current_state_))
                            .arg(platIdentFlagList.at(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.work_switch_.area_difference_flag_))
                            .arg(evntRptStateFlagList.at(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.work_switch_.event_report_flag_))
                            .arg(registerAllowStateList.at(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.work_switch_.register_allow_flag_))
                            .arg(workStateList.at(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.work_switch_.work_state));
                    res+=QString("载波通信速率: %1;\r\n")
                            .arg(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.communication_rate_);
                    res+=QString("第1相中继级别: %1;\r\n")
                            .arg(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.relay_level_phase_1_);
                    res+=QString("第2相中继级别: %1;\r\n")
                            .arg(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.relay_level_phase_2_);
                    res+=QString("第3相中继级别: %1;\r\n")
                            .arg(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.relay_level_phase_3_);
                    res+=QString("第1相工作步骤: %1;\r\n")
                            .arg(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.work_procedure_phase_1_<9?workStepList.at(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.work_procedure_phase_1_):"备用");
                    res+=QString("第2相工作步骤: %1;\r\n")
                            .arg(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.work_procedure_phase_2_<9?workStepList.at(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.work_procedure_phase_2_):"备用");
                    res+=QString("第3相工作步骤: %1;\r\n")
                            .arg(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.work_procedure_phase_3_<9?workStepList.at(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.work_procedure_phase_3_):"备用");
                    res+=QString("\r\n\r\n\r\n\r\n\r\n\r\n");

                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, res);
                    p_timer->stop();

                    if(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.work_switch_.current_state_ != 2)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Success, QString("路由运行状态告知升级结束（标准流程）  升级文件传输成功率：%1%; 升级文件传输耗时：%2秒;").arg(100).arg(p_CtrInfoList->at(0)->successConsume[0]));
                        p_maxAllowTimer->stop();
#ifdef DIRECT_CCO_MODE
                        emScriptRunState = ScriptSuccess;
                        p_AbstractScriptHost->updateProgress(ProcessState_Success, "Direct CCO mode: skip fixture event report.");
                        return;
#endif

                        tryTimes = 0;
                        emScriptRunState = Wait_EventReport_06F5_Finish;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送-CCO硬件初始化(01F1)，等待--确认");

                        sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, INSIGNIFICANCE, p_HardReset_01F1);
                        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
//                        // 开始查询版本（不支持查内部版本 暂时跳过）
//                        tryTimes = 0;
//                        meterIndex_02F1 = 0;
//                        p_timer->start(10*1000);
//                        emScriptRunState = Wait_QueryStaVersion_02F1_Finish;
//                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "开始查询模块内部版本信息");
//                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,0,p_ChkStaInVrsnInfo_02F1_Down);
//                        p_maxAllowTimer->start(timerForReachThresld_QueryNodeVrsnInfo02F1*1000);
//                        failCnt_InVrsn=0;
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "路由运行状态显示升级STA未完成，等待60秒后继续读取10F4");
                        p_delayTimer->start(60*1000);
                    }
                }
                else if(p_Frame3762Base->afn_ == 0x06&&p_Frame3762Base->dt1_==0x08&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    shared_ptr<Afn06F4> p_ReportRegisterNode_Up=dynamic_pointer_cast<Afn06F4>(p_Frame3762Base);
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("上报注册从节点，表号%1").arg(QString(QByteArray(p_ReportRegisterNode_Up->report_node_info_unit_.report_node_address_.addr,6).toHex())));
                    bool reportTwiceFlag = false;
                    for(int i=0;i<nodeInfoList.size();i++)
                    {
                        if(memcmp(nodeInfoList.at(i).nodeAddress.addr,p_ReportRegisterNode_Up->report_node_info_unit_.report_node_address_.addr,6)==0)
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "上报表号重复");
                            reportTwiceFlag = true;
                        }
                    }
                    if(reportTwiceFlag == false)
                    {
                        NodeInfo_Struct node;
                        memcpy(node.nodeAddress.addr,p_ReportRegisterNode_Up->report_node_info_unit_.report_node_address_.addr,6);
                        node.protocol=p_ReportRegisterNode_Up->report_node_info_unit_.report_node_protocol_;
                        node.deviceType=p_ReportRegisterNode_Up->report_node_info_unit_.report_node_device_type_;
                        nodeInfoList.append(node);
                    }

                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "回复确认，当前搜表上报数:" + QString::number(nodeInfoList.size()));
                    sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, INSIGNIFICANCE, p_Confirm_00F1);
                }
                else if(p_Frame3762Base->afn_ == 0x06&&p_Frame3762Base->dt1_==0x04&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    shared_ptr<Afn06F3> p_ReportRouterState_Up=dynamic_pointer_cast<Afn06F3>(p_Frame3762Base);
                    if(p_ReportRouterState_Up->router_work_task_change_==0x02)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "路由上报工况变动，搜表结束，回复确认");
                        search_flag = true;
                        p_meterSearchTimer->stop();

                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_Confirm_00F1);
                        if(isAllExist()==true)
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "档案所有表已上报");
                        }
                        else
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing,"有档案中的表未上报，上报数："+QString::number(nodeInfoList.size())+"，档案数："+QString::number(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().size()));
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
            case Wait_QueryStaVersion_02F1_Finish:
            {
                uchar dtValue3762=get3762Dt(p_Frame3762Base->dt1_,p_Frame3762Base->dt2_);
                if(p_Frame3762Base->afn_ == 0x02&&dtValue3762==1&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();

                    shared_ptr<Afn02F1> p_ChkNodeVrsnInfo_02F1_Up=std::dynamic_pointer_cast<Afn02F1>(p_Frame3762Base);
                    Refresh_TestResult_02F1(p_ChkNodeVrsnInfo_02F1_Up);

                    if(++meterIndex_02F1==p_CtrInfoList->at(0)->totalNodeCnt)
                    {
                        meterIndex_02F1=0;
                    }

                    Refresh_SuccessCnt_02F1();

                    if(p_CtrInfoList->at(0)->successRate[0]>=1)
                    {
                        resultFlag=true;
                        p_AbstractScriptHost->updateProgress(ProcessState_Success, QString("查询内部版本全部成功"));
                        p_maxAllowTimer->stop();
#ifdef DIRECT_CCO_MODE
                        emScriptRunState = ScriptSuccess;
                        p_AbstractScriptHost->updateProgress(ProcessState_Success, "Direct CCO mode: skip fixture event report.");
                        return;
#endif

                        tryTimes = 0;
                        emScriptRunState = Wait_EventReport_06F5_Finish;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送-CCO硬件初始化(01F1)，等待--确认");

                        sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, INSIGNIFICANCE, p_HardReset_01F1);
                        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                    {
                        while(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(meterIndex_02F1)->testResultList[0] == true)
                        {
                            if(++meterIndex_02F1==p_CtrInfoList->at(0)->totalNodeCnt)
                            {
                                meterIndex_02F1=0;
                            }
                        }
                        sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, 0, p_ChkStaInVrsnInfo_02F1_Down);
                        p_timer->start(10*1000);
                    }
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_EventReport_06F5_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x06&&p_Frame3762Base->dt1_==0x10&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    shared_ptr<Afn06F5> p_EventReport_06F5_Up=dynamic_pointer_cast<Afn06F5>(p_Frame3762Base);

                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到06F5电表事件上报，回复确认"));

                    this->msgSeq = uchar(p_EventReport_06F5_Up->info_field_.info_field_up.msg_seq);
                    sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, INSIGNIFICANCE, p_Confirm_00F1);

                    recvEventNum += 1;
                }
                else if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "硬件初始化，路由回复确认 \n");

                    recvEventNum = 0;

                    // 拉高单通事件管脚
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "开始拉高单通事件管脚");
                    sendParams.clear();
                    idList.clear();
                    idList = getDvcIdList(SingleSTA);
                    p_AbstractScriptHost->controlDvc(SingleSTA, idList, CtrlCmd_EventPinHigh, sendParams);
                }
                else if(p_Frame3762Base->afn_ == 0x03&&p_Frame3762Base->dt1_==0x02&&p_Frame3762Base->dt2_==0x01&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到03F10上报，开始组网等待事件上报");
                    p_delayTimer->start(60*1000);
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


void Script_ReadMeter_SearchMeter_Upgrade_MultiTask::processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID)
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

void Script_ReadMeter_SearchMeter_Upgrade_MultiTask::processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID)
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

void Script_ReadMeter_SearchMeter_Upgrade_MultiTask::sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame)
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
        sendMsgLog=QString("》》路由硬件复位01F1：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_SetAreaIdentityFlag_05F6)
    {
        p_SetAreaIdentityFlag_05F6->ctrl_field_.dir=kDirDown;
        p_SetAreaIdentityFlag_05F6->ctrl_field_.prm=kActive;
        p_SetAreaIdentityFlag_05F6->ctrl_field_.comn_type=kHplc;

        p_SetAreaIdentityFlag_05F6->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_SetAreaIdentityFlag_05F6->info_field_.info_field_down.comu_rate=0;
        p_SetAreaIdentityFlag_05F6->info_field_.info_field_down.comu_module_ident=0;

        // 开启
        p_SetAreaIdentityFlag_05F6->area_identify_enable_flag_=0x01;

        sendMsgOct=p_SetAreaIdentityFlag_05F6->EncodeFrame();
        sendMsgLog=QString("》》设置台区区分标志05F6：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_SetBaudRate_11F4)
    {
        p_SetBaudRate_11F4->ctrl_field_.dir=kDirDown;
        p_SetBaudRate_11F4->ctrl_field_.prm=kActive;
        p_SetBaudRate_11F4->ctrl_field_.comn_type=kHplc;

        p_SetBaudRate_11F4->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_SetBaudRate_11F4->info_field_.info_field_down.comu_rate=0;
        p_SetBaudRate_11F4->info_field_.info_field_down.comu_module_ident=0;

        p_SetBaudRate_11F4->com_rate_unit_.com_rate_=115;
        p_SetBaudRate_11F4->com_rate_unit_.rate_unit_identift_=1;

        sendMsgOct=p_SetBaudRate_11F4->EncodeFrame();
        sendMsgLog=QString("》》设置波特率115200(11F4)：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_FileTransfer_15F1_Down)
    {
        if(emScriptRunState==Wait_StartUpdata_15F1_Finish_2)
        {
            p_FileTransfer_15F1_Down->file_transfer_unit_.file_identify_=0x08;
            p_FileTransfer_15F1_Down->file_transfer_unit_.file_instruct_=0x00;
            if(fileIndex<totalSegs-1)
            {
                p_FileTransfer_15F1_Down->file_transfer_unit_.file_property_=0x00;
                p_FileTransfer_15F1_Down->file_transfer_unit_.file_length_=SEG_LEN_STA;
            }
            else
            {
                p_FileTransfer_15F1_Down->file_transfer_unit_.file_property_=0x01;
                p_FileTransfer_15F1_Down->file_transfer_unit_.file_length_=((dataLen%SEG_LEN_STA==0)?SEG_LEN_STA:(dataLen%SEG_LEN_STA));
            }
            p_FileTransfer_15F1_Down->file_transfer_unit_.this_identify_=fileIndex;
            p_FileTransfer_15F1_Down->file_transfer_unit_.file_content_=rawUpdateFile.mid(fileIndex*SEG_LEN_STA,p_FileTransfer_15F1_Down->file_transfer_unit_.file_length_);

            p_FileTransfer_15F1_Down->ctrl_field_={kHplc,kActive,kDirDown};
            p_FileTransfer_15F1_Down->info_field_.info_field_down.msg_seq=char(msgSeq++);

            sendMsgOct=p_FileTransfer_15F1_Down->EncodeFrame();

            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("总段数=(%1); 该段文件长度=(%2); 该段文件索引=(%3); ").arg(p_FileTransfer_15F1_Down->file_transfer_unit_.total_num_).arg(p_FileTransfer_15F1_Down->file_transfer_unit_.file_length_).arg(p_FileTransfer_15F1_Down->file_transfer_unit_.this_identify_));

            sendMsgLog=QString("》》文件传输：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
        }
        else
        {
            p_FileTransfer_15F1_Down->file_transfer_unit_.file_identify_=0x00;
            p_FileTransfer_15F1_Down->file_transfer_unit_.file_property_=0x00;
            p_FileTransfer_15F1_Down->file_transfer_unit_.file_length_=0;

            p_FileTransfer_15F1_Down->file_transfer_unit_.this_identify_=0;
            p_FileTransfer_15F1_Down->file_transfer_unit_.file_content_="";

            p_FileTransfer_15F1_Down->ctrl_field_={kHplc,kActive,kDirDown};
            p_FileTransfer_15F1_Down->info_field_.info_field_down.msg_seq=char(msgSeq++);

            sendMsgOct=p_FileTransfer_15F1_Down->EncodeFrame();
            sendMsgLog=QString("》》清除下装文件15F1：%1\n").arg(QString(sendMsgOct.toHex()));
        }
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
        p_ActiveRegister_11F5->last_time_ = search_meter_duration;
        p_ActiveRegister_11F5->retransmit_times_=0;
        p_ActiveRegister_11F5->wait_time_slice_num_=0;

        sendMsgOct=p_ActiveRegister_11F5->EncodeFrame();
        sendMsgLog=QString("》》激活主动注册11F5：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
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
            if(readInfoList.at(meterID).readFlag_14F1==WaitRead)
            {
                for(int i=0;i<readInfoList.at(meterID).dataUnitList.size();i++)
                {
                    shared_ptr<Rqst_ReadData_0x11> p_ReadData_0x11=make_shared<Rqst_ReadData_0x11>(addr,4,0);
                    memcpy(p_ReadData_0x11->addr_,readInfoList.at(meterID).meterNo.addr,6);
                    memcpy(p_ReadData_0x11->di,readInfoList.at(meterID).dataUnitList.at(i).dataID,4);
                    msg645=p_ReadData_0x11->EncodeFrame();
                    break;
                }
            }
            p_RouterRequestRead_14F1->router_request_read_unit_.read_flag_=char(readInfoList.at(meterID).readFlag_14F1);
            p_RouterRequestRead_14F1->router_request_read_unit_.delay_related_flag_=0x00;
            p_RouterRequestRead_14F1->router_request_read_unit_.subsidiary_node_num_=0x00;
            p_RouterRequestRead_14F1->router_request_read_unit_.frame_length_=uchar(msg645.size());
            p_RouterRequestRead_14F1->router_request_read_unit_.frame_content_=msg645;

            sendMsgOct.clear();
            sendMsgOct=p_RouterRequestRead_14F1->EncodeFrame();
            if(readInfoList.at(meterID).readFlag_14F1==WaitRead)
            {
                sendMsgLog=QString("》》14F1回复，可以抄读（645）：%1\n").arg(QString(sendMsgOct.toHex()));
            }
            else if(readInfoList.at(meterID).readFlag_14F1==ReadSuccess)
            {
                sendMsgLog=QString("》》14F1回复，抄读成功（645）：%1\n").arg(QString(sendMsgOct.toHex()));
            }
            else if(readInfoList.at(meterID).readFlag_14F1==ReadFail)
            {
                sendMsgLog=QString("》》14F1回复，抄读失败（645）：%1\n").arg(QString(sendMsgOct.toHex()));
            }
       //     startTime=QDateTime::currentDateTime().toString(CURRENT_TIME_FORMAT);
        }
        else if(readInfoList.at(meterID).protocolType==OOP)
        {
            QByteArray msgOOP;
            if(readInfoList.at(meterID).readFlag_14F1==WaitRead)
            {
                for(int i=0;i<readInfoList.at(meterID).dataUnitList.size();i++)
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
                    break;
                }
            }
            p_RouterRequestRead_14F1->router_request_read_unit_.read_flag_=char(readInfoList.at(meterID).readFlag_14F1);
            p_RouterRequestRead_14F1->router_request_read_unit_.delay_related_flag_=0x00;
            p_RouterRequestRead_14F1->router_request_read_unit_.subsidiary_node_num_=0x00;
            p_RouterRequestRead_14F1->router_request_read_unit_.frame_length_=uchar(msgOOP.size());
            p_RouterRequestRead_14F1->router_request_read_unit_.frame_content_=msgOOP;

            sendMsgOct.clear();
            sendMsgOct=p_RouterRequestRead_14F1->EncodeFrame();
            if(readInfoList.at(meterID).readFlag_14F1==WaitRead)
            {
                sendMsgLog=QString("》》14F1回复，可以抄读（OOP）：%1\n").arg(QString(sendMsgOct.toHex()));
            }
            else if(readInfoList.at(meterID).readFlag_14F1==ReadSuccess)
            {
                sendMsgLog=QString("》》14F1回复，抄读成功（OOP）：%1\n").arg(QString(sendMsgOct.toHex()));
            }
            else if(readInfoList.at(meterID).readFlag_14F1==ReadFail)
            {
                sendMsgLog=QString("》》14F1回复，抄读失败（OOP）：%1\n").arg(QString(sendMsgOct.toHex()));
            }
       //     startTime=QDateTime::currentDateTime().toString(CURRENT_TIME_FORMAT);
        }
        else
            return;
    }
    else if(frame==p_MonitorSlaveNode_13F1)
    {
        uchar tmpAddr[6];
        memcpy(tmpAddr,readInfoList.at(meterID).meterNo.addr,6);
        uchar comPrtclType=readInfoList.at(meterID).protocolType;

        if(comPrtclType==DLT645_2007)
        {
            shared_ptr<Rqst_ReadData_0x11> p_ReadData_0x11=make_shared<Rqst_ReadData_0x11>(addr,4,0);
            //reverseAddr(tmpAddr, 6);
            memcpy(p_ReadData_0x11->addr_,tmpAddr,6);
            memcpy(p_ReadData_0x11->di,readInfoList.at(meterID).dataUnitList.at(0).dataID,4);
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
            sendMsgLog=QString("》》监控从节点13F1,抄读645电表%1：%1\n").arg(QString((QByteArray(reinterpret_cast<char*>(tmpAddr),6).toHex()))).arg(QString(sendMsgOct.toHex()));

        //    startTime=QDateTime::currentDateTime().toString(CURRENT_TIME_FORMAT);
        }
        else if(comPrtclType==OOP)
        {
            uchar tmpAddr[6];
            memcpy(tmpAddr,readInfoList.at(meterID).meterNo.addr,6);

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

            p_MonitorSlaveNode_13F1->data_field_down_.delay_tag_=0x00;
            p_MonitorSlaveNode_13F1->data_field_down_.sub_node_num_=0x00;
            p_MonitorSlaveNode_13F1->data_field_down_.protocol_type_=0x03;
            p_MonitorSlaveNode_13F1->data_field_down_.frame_content_=msg698;
            p_MonitorSlaveNode_13F1->data_field_down_.frame_length_=uchar(msg698.size());

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
            sendMsgLog=QString("》》监控从节点13F1,抄读OOP电表%1：%1\n").arg(QString((QByteArray(reinterpret_cast<char*>(tmpAddr),6).toHex()))).arg(QString(sendMsgOct.toHex()));

         //   startTime=QDateTime::currentDateTime().toString(CURRENT_TIME_FORMAT);
        }
        else
            return;
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
            memcpy(p_ReadData_0x11->addr_,tmpAddr,6);
            memcpy(p_ReadData_0x11->di,readInfoList.at(meterID).dataUnitList.at(0).dataID,4);
            QByteArray msg645=p_ReadData_0x11->EncodeFrame();

            p_ParallelReadMeter_F1F1->unit_down_.protocol_type_=0x02;
            p_ParallelReadMeter_F1F1->unit_down_.subsidiary_node_num_=0x00;
            p_ParallelReadMeter_F1F1->unit_down_.frame_content_=msg645;
            p_ParallelReadMeter_F1F1->unit_down_.frame_length_=ushort(msg645.size());

            sendMsgOct=p_ParallelReadMeter_F1F1->EncodeFrame();
            sendMsgLog=QString("》》并发抄表F1F1,抄读645电表%1：%1\n").arg(QString((QByteArray(reinterpret_cast<char*>(tmpAddr),6).toHex()))).arg(QString(sendMsgOct.toHex()));

       //     startTime=QDateTime::currentDateTime().toString(CURRENT_TIME_FORMAT);
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

            p_ParallelReadMeter_F1F1->unit_down_.subsidiary_node_num_=0x00;
            p_ParallelReadMeter_F1F1->unit_down_.protocol_type_=0x03;
            p_ParallelReadMeter_F1F1->unit_down_.frame_content_=msg698;
            p_ParallelReadMeter_F1F1->unit_down_.frame_length_=uchar(msg698.size());

            sendMsgOct=p_ParallelReadMeter_F1F1->EncodeFrame();
            sendMsgLog=QString("》》并发抄表F1F1,抄读OOP电表%1：%1\n").arg(QString((QByteArray(reinterpret_cast<char*>(tmpAddr),6).toHex()))).arg(QString(sendMsgOct.toHex()));

      //      startTime=QDateTime::currentDateTime().toString(CURRENT_TIME_FORMAT);
        }
        else
            return;
    }
    else if(frame==p_CcoRunStateInfo_10F4_Down)
    {
        p_CcoRunStateInfo_10F4_Down->ctrl_field_={kHplc,kActive,kDirDown};
        p_CcoRunStateInfo_10F4_Down->info_field_.info_field_down.comu_module_ident=0;
        p_CcoRunStateInfo_10F4_Down->info_field_.info_field_down.msg_seq=char(msgSeq++);
       // p_CcoRunStateInfo_10F4_Down->ucAFN=0x10;
       // sendMsgOct=p_CcoRunStateInfo_10F4_Down->encode_3762_MsgDown(p_MsgBase_1376_2,p_CcoRunStateInfo_10F4_Down);
        sendMsgOct=p_CcoRunStateInfo_10F4_Down->EncodeFrame();

        sendMsgLog=QString("》》查询路由工作状态10F4：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_ChkStaInVrsnInfo_02F1_Down)
    {
        p_ChkStaInVrsnInfo_02F1_Down->protocol_type_ = 0x04;
        p_ChkStaInVrsnInfo_02F1_Down->frame_content_ = QByteArray::fromHex("fefe0f00010500031a000000");
        p_ChkStaInVrsnInfo_02F1_Down->frame_content_[8]=char(msgSeq++);
        for(int i=0;i<6;i++)
        {
            p_ChkStaInVrsnInfo_02F1_Down->frame_content_.append(p_CtrInfoList->at(0)->ccoAddr[i]);
        }
        for(int i=0;i<6;i++)
        {
            p_ChkStaInVrsnInfo_02F1_Down->frame_content_.append(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(meterIndex_02F1)->mtrAddr[i]);
        }
        p_ChkStaInVrsnInfo_02F1_Down->frame_length_ = p_ChkStaInVrsnInfo_02F1_Down->frame_content_.length();

        p_ChkStaInVrsnInfo_02F1_Down->ctrl_field_={kHplc,kActive,kDirDown};
        p_ChkStaInVrsnInfo_02F1_Down->info_field_.info_field_down.comu_module_ident=1;
        p_ChkStaInVrsnInfo_02F1_Down->info_field_.info_field_down.msg_seq=char(msgSeq);
        memcpy(&p_ChkStaInVrsnInfo_02F1_Down->address_field_.src_addr,p_CtrInfoList->at(0)->ccoAddr,6);
        memcpy(&p_ChkStaInVrsnInfo_02F1_Down->address_field_.dst_addr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(meterIndex_02F1)->mtrAddr,6);
        sendMsgOct=p_ChkStaInVrsnInfo_02F1_Down->EncodeFrame();

        sendMsgLog=QString("》》查询STA版本信息02F1：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
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

void Script_ReadMeter_SearchMeter_Upgrade_MultiTask::sendSrcMsg(DvcType dvcType, int dvcId, QByteArray msg)
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


void Script_ReadMeter_SearchMeter_Upgrade_MultiTask::timer_timeoutProc()
{
    p_timer->stop();
    switch(emScriptRunState)
    {
        case Wait_BuildNetFinish_Whole:
        {
            p_CtrInfoList->at(0)->inNetResult=false;
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("Wait_BuildNetFinish_Whole 全网组网成功率：%1% 【GW-CCO-F022-0004-V01】").arg(p_CtrInfoList->at(0)->inNetSuccessRate*100));
            break;
        }
        case Wait_CcoHardInit_Finish:
        {
            if(++tryTimes>=3)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_CcoHardInit_Finish timeout 【GW-CCO-F022-0004-V01】");
            }
            else
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "再次发送--硬件初始化，等待--确认");
                sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, INSIGNIFICANCE, p_HardReset_01F1);
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            }
            break;
        }
        case Wait_SetAeraIdentification_05F6_Finish:
        {
            if(++tryTimes>=3)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_SetAeraIdentification_05F6_Finish timeout 【GW-CCO-F022-0004-V01】");
            }
            else
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "再次发送--设置允许台区识别(05F6)，等待--确认");
                sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, INSIGNIFICANCE, p_SetAreaIdentityFlag_05F6);
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            }
            break;
        }
        case Wait_StaUpgradeFinish:
        {
            if(++tryTimes>=3)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_StaUpgradeFinish timeout 【GW-CCO-F022-0004-V01】");
            }
            else
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "再次发送--查询CCO运行状态，等待回复");
                sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, INSIGNIFICANCE, p_CcoRunStateInfo_10F4_Down);
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            }
            break;
        }
        case Wait_SetStaActiveResgister_11F5_Finish:
        {
            if(++tryTimes>=3)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_SetStaActiveResgister_11F5_Finish timeout 【GW-CCO-F022-0004-V01】");
            }
            else
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "再次发送--启动搜表，等待--确认");
                sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, INSIGNIFICANCE, p_ActiveRegister_11F5);
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            }
            break;
        }
        case Wait_SendResetRoute_12F1_Finish:
        {
            if(++tryTimes>=3)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_SendResetRoute_12F1_Finish timeout 【GW-CCO-F022-0004-V01】");
            }
            else
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "再次发送--路由重启，等待--确认");
                sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, INSIGNIFICANCE, p_RouterRestart_12F1);
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            }
            break;
        }
        case Wait_StartUpdata_15F1_Finish_2:
        {
//            if(++tryTimes>=3)
//            {
//                p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_StartUpdata_15F1_Finish_2 timeout 【GW-CCO-F022-0004-V01】");
//            }
//            else
//            {
//                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "再次发送--文件传输，等待确认");
//                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,0,p_FileTransfer_15F1_Down);
//                p_timer->start(10*1000);
//            }
//            break;
            if(!failedSegments.contains(fileIndex))
            {
                failedSegments.append(fileIndex);
            }

            p_AbstractScriptHost->updateProgress(ProcessState_Processing,
                                                 QString("第(%1)段传输超时，准备重传").arg(fileIndex));

            RetransmitFailedSegments();
            break;
        }
        case Wait_QueryRouteRunStateCycles_10F4_Finish:
        {
            if(++tryTimes>=3)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_QueryRouteRunStateCycles_10F4_Finish timeout 【GW-CCO-F022-0004-V01】");
            }
            else
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "再次发送--发送10F4查询路由运行状态，等待回复");
                sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, INSIGNIFICANCE, p_CcoRunStateInfo_10F4_Down);
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            }
            break;
        }
        case Wait_QueryStaVersion_02F1_Finish:
        {
            if(++tryTimes>=3)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_QueryStaVersion_02F1_Finish timeout 【GW-CCO-F022-0004-V01】");
            }
            else
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "再次发送--查询模块内部版本信息，等待回复");
                sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, INSIGNIFICANCE, p_ChkStaInVrsnInfo_02F1_Down);
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            }
            break;
        }
        case Wait_EventReport_06F5_Finish:
        {
            if(++tryTimes>=3)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_EventReport_06F5_Finish timeout 【GW-CCO-F022-0004-V01】");
            }
            else
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "再次发送--硬件初始化，等待--确认");
                sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, INSIGNIFICANCE, p_HardReset_01F1);
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

void Script_ReadMeter_SearchMeter_Upgrade_MultiTask::maxAllowTimer_timeoutProc()
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

void Script_ReadMeter_SearchMeter_Upgrade_MultiTask::delayTimer_timeoutProc()
{
    p_delayTimer->stop();
    switch(emScriptRunState)
    {
        case Wait_BuildNetFinish_Whole:
        {
            break;
        }
        case Wait_CcoHardInit_Finish:
        {
            tryTimes = 0;
            emScriptRunState = Wait_SetAeraIdentification_05F6_Finish;
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--设置允许台区识别(05F6)，等待--确认");

            sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, INSIGNIFICANCE, p_SetAreaIdentityFlag_05F6);
            p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);

            break;
        }
        case Wait_SetBandRate_Finish:
        {
            // 开始传输文件---让CCO进入升级状态
            /****调入升级包并计算每段长度和总段数*****/
//            LoadUpdateFile();

//            emScriptRunState = Wait_StartUpdata_15F1_Finish_1;
//            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "等待清除下装文件");
//            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,0,p_FileTransfer_15F1_Down);
//            p_timer->start(10*1000);


            LoadUpdateFile();
            fileIndex=0;
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "等待升级文件传输完成");

            emScriptRunState = Wait_StartUpdata_15F1_Finish_2;
            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,0,p_FileTransfer_15F1_Down);
            p_timer->start(10*1000);

            break;
        }
        case Wait_ReadMeter_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_ReadMeter_Finish timeout 【GW-CCO-F022-0004-V01】");
            break;
        }
        case Wait_StaUpgradeFinish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("发送10F4查询路由运行状态"));
            sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, 0, p_CcoRunStateInfo_10F4_Down);
            p_timer->start(10*1000);
            break;
        }
        case Wait_EventReport_06F5_Finish:
        {
            emScriptRunState = ScriptSuccess;
            sendParams.clear();
            idList.clear();
            idList = getDvcIdList(SingleSTA);
            p_AbstractScriptHost->controlDvc(SingleSTA,idList,CtrlCmd_EventPinLow,sendParams);
            p_maxAllowTimer->stop();
            break;
        }
        case Wait_QueryRouteRunStateCycles_10F4_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("发送10F4查询路由运行状态"));
            sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, 0, p_CcoRunStateInfo_10F4_Down);
            p_timer->start(5*1000);
            break;
        }

        default:
        {
            break;
        }
    }
}

void Script_ReadMeter_SearchMeter_Upgrade_MultiTask::timeoutProc_13F1()
{
    p_13F1Timer->stop();
    
    // 超时时强制解锁当前表
    if(readNo_13F1 < readInfoList.size()) {
        unlockMeter(readInfoList[readNo_13F1].meterNo.addr);
    }
    
    switch(emScriptRunState)
    {
        case Wait_ReadMeter_Finish:
        {
            timeoutNum_13F1++;
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("发送13F1后第%1次100s内未回复").arg(timeoutNum_13F1));
            readInfoList[readNo_13F1].readFlag_13F1=ReadFail;
            if(requestNum_13F1<readInfoList.size())
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("发送第%1条13F1继续抄下一只表").arg(++requestNum_13F1));
                sendMsg(p_CtrInfoList->at(0)->slotPosition,p_CtrInfoList->at(0)->dvcId,++readNo_13F1,p_MonitorSlaveNode_13F1);
                p_13F1Timer->start(100*1000);
            }
            else
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("13F1一轮抄表结束，发送报文数%1，抄读成功报文数%2，回复数据为空报文数%3，回复超时数%4").arg(requestNum_13F1).arg(responseSucNum_13F1).arg(responseFailNum_13F1).arg(timeoutNum_13F1));
                endFlag_13F1=true;
                resultCheck();
            }
            break;
        }
        default:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==p_13F1timer");
            break;
        }
    }

}

void Script_ReadMeter_SearchMeter_Upgrade_MultiTask::timeoutProc_F1F1()
{
    p_F1F1Timer->stop();
    
    // 超时时强制解锁当前表
    if(readNo_F1F1 < readInfoList.size()) {
        unlockMeter(readInfoList[readNo_F1F1].meterNo.addr);
    }
    
    if(emScriptRunState==Wait_ReadMeter_Finish)
    {
        timeoutNum_F1F1++;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("发送F1F1后第%1次100s内未回复").arg(timeoutNum_F1F1));
        readInfoList[readNo_F1F1].readFlag_F1F1=ReadFail;
        if(requestNum_F1F1<readInfoList.size())
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("发送第%1条F1F1继续抄下一只表").arg(++requestNum_F1F1));
            sendMsg(p_CtrInfoList->at(0)->slotPosition,p_CtrInfoList->at(0)->dvcId,++readNo_F1F1,p_ParallelReadMeter_F1F1);
            p_F1F1Timer->start(100*1000);
        }
        else
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("F1F1一轮抄表结束，发送报文数%1，抄读成功报文数%2，回复数据为空报文数%3，回复超时数%4").arg(requestNum_F1F1).arg(responseSucNum_F1F1).arg(responseFailNum_F1F1).arg(timeoutNum_F1F1));
            endFlag_F1F1=true;
            resultCheck();
        }
    }
    else
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Error, "F1F1Timer状态机错误");
    }
}

void Script_ReadMeter_SearchMeter_Upgrade_MultiTask::timeoutProc_14F1()
{
    p_14F1Timer->stop();

    if(emScriptRunState == Wait_ReadMeter_Finish)
    {
        timeoutNum_14F1++;

        p_AbstractScriptHost->updateProgress(ProcessState_Processing,
                                             QString("14F1处理超时(第%1次)！requestNum=%2, reportSuc=%3, reportFail=%4")
                                             .arg(timeoutNum_14F1)
                                             .arg(requestNum_14F1)
                                             .arg(reportSucNum_14F1)
                                             .arg(reportFailNum_14F1));

        // 强制解锁所有可能被锁定的表
        QMutexLocker locker(&m_meterMutex);
        qint64 now = QDateTime::currentMSecsSinceEpoch();
        QStringList toRemove;
        for(auto it = m_meterLockTime.begin(); it != m_meterLockTime.end(); ++it) {
            if(now - it.value() > 20000) {  // 超过20秒强制解锁
                toRemove.append(it.key());
            }
        }
        for(const QString& addr : toRemove) {
            m_busyMeters.remove(addr);
            m_meterLockTime.remove(addr);
        }
        locker.unlock();

        if(timeoutNum_14F1 >= 3) {  // 减少到3次重试
            p_AbstractScriptHost->updateProgress(ProcessState_Failed,
                                                 QString("Wait_ReadMeter_Finish 14F1处理多次异常，已重试%1次").arg(timeoutNum_14F1));
        } else {
            // 暂停所有其他任务10秒，清理环境
            p_13F1Timer->stop();
            p_F1F1Timer->stop();

            p_AbstractScriptHost->updateProgress(ProcessState_Processing,
                                                 QString("暂停所有任务10秒，清理通信环境"));

            // 10秒后恢复其他任务
            QTimer::singleShot(10000, this, [this]() {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing,
                                                     "恢复13F1和F1F1任务");

                // 恢复13F1
                if(requestNum_13F1 < readInfoList.size() && endFlag_13F1 == false) {
                    for(int i = requestNum_13F1; i < readInfoList.size(); i++) {
                        if(tryLockMeter(readInfoList[i].meterNo.addr, "13F1")) {
                            requestNum_13F1 = i + 1;
                            sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, i, p_MonitorSlaveNode_13F1);
                            p_13F1Timer->start(100*1000);
                            break;
                        }
                    }
                }

                // 恢复F1F1
                if(requestNum_F1F1 < readInfoList.size() && endFlag_F1F1 == false) {
                    for(int i = requestNum_F1F1; i < readInfoList.size(); i++) {
                        if(tryLockMeter(readInfoList[i].meterNo.addr, "F1F1")) {
                            requestNum_F1F1 = i + 1;
                            sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, i, p_ParallelReadMeter_F1F1);
                            p_F1F1Timer->start(100*1000);
                            break;
                        }
                    }
                }
            });
        }
    }
}

void Script_ReadMeter_SearchMeter_Upgrade_MultiTask::timeoutProc_meterSearch()
{
    p_meterSearchTimer->stop();
    if(search_flag == false)
    {
        p_upgardeTimer->stop();
        p_delayTimer->stop();
        p_timer->stop();
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("搜表超时未完成【GW-CCO-F022-0004-V01】"));
    }
}

void Script_ReadMeter_SearchMeter_Upgrade_MultiTask::timeoutProc_upgarde()
{
    p_upgardeTimer->stop();
    p_delayTimer->stop();
    p_timer->stop();
#ifdef DIRECT_CCO_MODE
    emScriptRunState = ScriptSuccess;
    p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Direct CCO mode: upgrade timeout, skip fixture event report.");
    return;
#endif

    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "升级超时时间到（60min），进行事件上报测试");

    emScriptRunState = Wait_EventReport_06F5_Finish;
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送-CCO硬件初始化(01F1)，等待--确认");
    sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, INSIGNIFICANCE, p_HardReset_01F1);
    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);

//    emScriptRunState = Wait_QueryStaVersion_02F1_Finish;
//    // 先拉低单通事件管脚
//    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "开始拉低单通事件管脚");
//    sendParams.clear();
//    idList.clear();
//    idList = getDvcIdList(SingleSTA);
//    p_AbstractScriptHost->controlDvc(SingleSTA, idList, CtrlCmd_EventPinHigh, sendParams);
}

void Script_ReadMeter_SearchMeter_Upgrade_MultiTask::LoadUpdateFile()
{
    QStringList fileNameFilters;
    QStringList fileList;
    QString path;
    QString error;
    if(1==dstUpgradeDvc)
    {
        path=tr("DataBase\\Upgrade\\模块程序(旧-新)");
    }
    else if(2==dstUpgradeDvc)
    {
        path=tr("DataBase\\Upgrade\\模块程序(新-新)");
    }
    else
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Error, "dstUpgradeDvc配置有误");
        return;
    }

    QDir *updateFileDir=new QDir(path);
    fileNameFilters << "*" ;
    QList<QFileInfo> *fileInfo=new QList<QFileInfo>(updateFileDir->entryInfoList(fileNameFilters,QDir::Files,QDir::NoSort));
    if(fileInfo->count() != 1)
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "获取到的STA升级文件数不为1："+QString::number(fileInfo->count()));
        return;
    }

    QString fileName = fileInfo->at(0).fileName();
    QString fileNameForParsing = fileName;

    if(staOutVrsn.isEmpty() || staInVrsn.isEmpty())
    {
        if(fileName.left(6)!="GY2301")
        {
            error= "获取到的文件名头不是GY2301；实际是："+fileName.left(6);
            p_AbstractScriptHost->updateProgress(ProcessState_Error,error);
            return;
        }
        if(fileName.right(3)!="bin" && fileName.right(3)!="dat")
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Error, "获取到的文件后缀名不是dat或bin："+fileName.right(3));
            return;
        }

        QString outVer,innerVer;
        QStringList list=fileNameForParsing.remove(".bin").remove(".dat").split("_");
        if(list.size()<2)
        {
            error="获取到的文件名格式不对："+fileName;
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, error);
            return;
        }
        // 新格式应该按 "_" 分割后取时间戳后面的部分
        QString versionPart = list.at(1); // 获取 "202511181520V00.10(V00.10).bin"
        QStringList verList = versionPart.split("V");
        if(verList.size() < 3)  // 需要至少3个元素：时间戳、外部版本、内部版本
        {
            error = "获取到的文件名版本号格式不对：" + fileName;
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, error);
            return;
        }

        // 从时间戳中提取日期部分(YYMMDD)：202511181520 -> 251118
        QString dateStr = verList.at(0);  // "202511181520"
        QString datePart = dateStr.mid(2, 6);  // 从位置2开始取6位 -> "251118"

        // 外部版本号：V00.10 -> 0010
        QString outVerNum = verList[1];
        outVerNum.remove('.').remove('(').remove(')');
        outVer = datePart + outVerNum.rightJustified(4, '0');  // "2511180010"

        // 内部版本号：(V00.10) -> 0010
        QString innerVerNum = verList[2];
        innerVerNum.remove('.').remove('(').remove(')');
        innerVer = datePart + innerVerNum.rightJustified(4, '0');  // "2511180010"

        staOutVrsn = outVer;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "获取到的文件名中STA外部版本："+staOutVrsn);
        staInVrsn  = innerVer;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "获取到的文件名中STA内部版本："+staInVrsn);

    }

    QString filePath=path+"\\"+fileName;

    delete fileInfo;
    delete updateFileDir;

    char upgradeFileBuf[1024*1024];
    QFile file;
    file.setFileName(filePath);
    if(!file.open(QIODevice::ReadOnly))
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "打开STA升级文件失败!!!");
        return;
    }

    QDataStream in(&file);
    dataLen=in.readRawData(upgradeFileBuf,1024*1024);
    file.close();

    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("升级包大小=(%1);").arg(dataLen));
    rawUpdateFile=QByteArray::fromRawData(upgradeFileBuf,dataLen);

    totalSegs = (dataLen%SEG_LEN_STA==0)?dataLen/SEG_LEN_STA:(dataLen/SEG_LEN_STA+1);

    p_FileTransfer_15F1_Down->file_transfer_unit_.total_num_=totalSegs;
    for(int i=0; i<p_FileTransfer_15F1_Down->file_transfer_unit_.total_num_; i++)
    {
        transResList.append(false);
    }
    getVendorChipCode();
}

void Script_ReadMeter_SearchMeter_Upgrade_MultiTask::getVendorChipCode()
{
    QSettings property("PropertyConfig.ini",QSettings::IniFormat);
    QString vendor;
    QString chip;
    property.beginGroup("SYSTEM_PROPERTY");
    vendor=property.value("STAVendorCoder").toString();
    chip=property.value("STAChipCode").toString();
    property.endGroup();
    if(!vendor.isEmpty()&&!chip.isEmpty())
    {
        staVendorChipCode=vendor+chip;
    }
    else
    {
        if(staVendorChipCode.isEmpty())
            staVendorChipCode="GYY1";
    }
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, staVendorChipCode);

}

void Script_ReadMeter_SearchMeter_Upgrade_MultiTask::Refresh_TestResult_15F1(shared_ptr<Afn15F1> p_FileTransfer_15F1_Up)
{
//    if(p_FileTransfer_15F1_Up->current_identify_==fileIndex)
//    {
//        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("第(%1)段传输完成;").arg(fileIndex));
//        transResList[fileIndex]=true;
//    }
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到确认：段标志%1，fileIndex%2").arg(p_FileTransfer_15F1_Up->current_identify_).arg(fileIndex));
    if(p_FileTransfer_15F1_Up->current_identify_==fileIndex)
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("第(%1)段传输完成;").arg(fileIndex));
        transResList[fileIndex] = true;

        // 清除该段的重传计数
        if(segmentRetryCount.contains(fileIndex))
        {
            segmentRetryCount.remove(fileIndex);
        }

        // 从失败列表移除（如果存在）
        if(failedSegments.contains(fileIndex))
        {
            failedSegments.removeAll(fileIndex);
        }
    }
    else
    {
        // 收到的段标志与期望不符，标记为失败
        p_AbstractScriptHost->updateProgress(ProcessState_Processing,
                                             QString("段标志不匹配：期望%1，收到%2").arg(fileIndex).arg(p_FileTransfer_15F1_Up->current_identify_));

        if(!failedSegments.contains(fileIndex))
        {
            failedSegments.append(fileIndex);
        }
    }
}

ushort Script_ReadMeter_SearchMeter_Upgrade_MultiTask::Refresh_SuccessCnt_15F1()
{
    ushort succSefgs=0;
    for(int i=0; i<p_FileTransfer_15F1_Down->file_transfer_unit_.total_num_; i++)
    {
        if(transResList[i]==true)
        {
            succSefgs+=1;
        }
    }
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("TotalCnt=%1; SuccessCnt=%2; SuccessRate=%3%").arg(totalSegs).arg(succSefgs).arg(((double)succSefgs/(double)totalSegs)*100));
    return succSefgs;
}

bool Script_ReadMeter_SearchMeter_Upgrade_MultiTask::isAllExist()
{
  //  bool result=true;
    for(int j=0;j<p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().size();j++)
    {
        p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(j)->testResultList[0]=false;
    }
    for(int i=0;i<nodeInfoList.size();i++)
    {
        for(int j=0;j<p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().size();j++)
        {
            if(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(j)->prtcl==uchar(nodeInfoList.at(i).protocol)
                    &&memcmp(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(j)->mtrAddr,nodeInfoList.at(i).nodeAddress.addr,6)==0)
            {
                nodeInfoList[i].isExist=true;
                p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(j)->testResultList[0]=true;
            }
        }
    }

    QString meters_OutOfRecord;
    num_reportOutOfRecord=0;
    for(int i=0;i<nodeInfoList.size();i++)
    {
        if(nodeInfoList.at(i).isExist==false)
        {
            meters_OutOfRecord.append("表号"+QByteArray(nodeInfoList.at(i).nodeAddress.addr,6).toHex()+"  协议0").append(QString::number(int(nodeInfoList.at(i).protocol))).append("\n");
            num_reportOutOfRecord++;
        }
    }
    if(!meters_OutOfRecord.isEmpty())
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "上报了档案外的表：\n"+meters_OutOfRecord);
    }

    QString meters_NotReport;
    for(int j=0;j<p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().size();j++)
    {
        if(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(j)->testResultList[0]==false)
        {
            meters_NotReport.append(QByteArray(reinterpret_cast<char*>(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(j)->mtrAddr),6).toHex()).append("\n");
        }
    }
    if(!meters_NotReport.isEmpty())
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "档案中未上报的表：\n"+meters_NotReport);
        return false;
    }
    else
    {
        return true;
    }
}

void Script_ReadMeter_SearchMeter_Upgrade_MultiTask::readInfoInit()
{
    readInfoList.clear();
    for(int i=0;i<p_CtrInfoList->size();i++)
    {
        QList<MeterInfoForSingleNet*> meterInfoList=p_CtrInfoList->at(i)->p_MeterInfoForSingleNetList->values();
        for(int j=0;j<meterInfoList.size();j++)
        {
            ReadInfo readInfo_ST;
       //     readInfo_ST.phase=meterInfoList.at(j)->realPhase;
            memcpy(readInfo_ST.meterNo.addr,meterInfoList.at(j)->mtrAddr,6);
            readInfo_ST.protocolType=meterInfoList.at(j)->prtcl;
            readInfo_ST.requestFlag_14F1=false;
            readInfo_ST.readFlag_13F1=WaitRead;
            readInfo_ST.readFlag_14F1=WaitRead;
            readInfo_ST.readFlag_F1F1=WaitRead;
            if(readInfo_ST.protocolType==0x02)
            {
                ReadDataUnit readData;
                char id[4]={0x00,char(0xFF),0x01,0x00};
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

int Script_ReadMeter_SearchMeter_Upgrade_MultiTask::getReadInfo(Address address)
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
void Script_ReadMeter_SearchMeter_Upgrade_MultiTask::resultCheck()
{
    if(endFlag_13F1&&endFlag_14F1&&endFlag_F1F1&&search_flag)
    {
        int suc_13F1=0,fail_13F1=0,not_13F1=0;
        int suc_14F1=0,fail_14F1=0,not_14F1=0;
        int suc_F1F1=0,fail_F1F1=0,not_F1F1=0;
        QString failAddr_13F1,notReadAddr_13F1;
        QString failAddr_14F1,notReadAddr_14F1;
        QString failAddr_F1F1,notReadAddr_F1F1;
        for(int i=0;i<readInfoList.size();i++)
        {
            QString addr = QByteArray(readInfoList.at(i).meterNo.addr,6).toHex();
            if(readInfoList.at(i).readFlag_13F1==ReadSuccess)
            {
                suc_13F1++;
            }
            else if(readInfoList.at(i).readFlag_13F1==ReadFail)
            {
                fail_13F1++;
                failAddr_13F1 += addr + " ";
            }
            else if(readInfoList.at(i).readFlag_13F1==WaitRead)
            {
                not_13F1++;
                notReadAddr_13F1 += addr + " ";
            }
            else
            {}
            if(readInfoList.at(i).readFlag_14F1==ReadSuccess)
            {
                suc_14F1++;
            }
            else if(readInfoList.at(i).requestFlag_14F1==true && readInfoList.at(i).readFlag_14F1!=ReadSuccess)
            {
                fail_14F1++;
                failAddr_14F1 += addr + " ";
            }
            else if(readInfoList.at(i).requestFlag_14F1==false)
            {
                not_14F1++;
                notReadAddr_14F1 += addr + " ";
            }
            else
            {}
            if(readInfoList.at(i).readFlag_F1F1==ReadSuccess)
            {
                suc_F1F1++;
            }
            else if(readInfoList.at(i).readFlag_F1F1==ReadFail)
            {
                fail_F1F1++;
                failAddr_F1F1 += addr + " ";
            }
            else if(readInfoList.at(i).readFlag_F1F1==WaitRead)
            {
                not_F1F1++;
                notReadAddr_F1F1 += addr + " ";
            }
            else
            {}
        }
        int meterSum = readInfoList.size();
    //    fail_14F1 = meterSum-suc_14F1-not_14F1;
        if(double(suc_13F1)/double(meterSum)>=0.98 && double(suc_14F1)/double(meterSum)>=0.98 && double(suc_F1F1)/double(meterSum)>=0.98)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("13F1、14F1、F1F1全部抄读成功(成功率大于98%)！"));
            emScriptRunState = Wait_QueryRouteRunStateCycles_10F4_Finish;
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "20s后查询10F4路由运行状态 \n");
            p_delayTimer->start(20*1000);
        }
        else
        {
            QString result = QString("13F1成功数%1  失败数%2  未抄数%3  失败表%4  未抄表%5\n14F1成功数%6  失败数%7  未抄数%8  失败表%9  未抄表%10\nF1F1成功数%11  失败数%12  未抄数%13  失败表%14  未抄表%15\n")
                    .arg(suc_13F1).arg(fail_13F1).arg(not_13F1).arg(failAddr_13F1).arg(notReadAddr_13F1).arg(suc_14F1).arg(fail_14F1).arg(not_14F1).arg(failAddr_14F1).arg(notReadAddr_14F1).arg(suc_F1F1).arg(fail_F1F1).arg(not_F1F1).arg(failAddr_F1F1).arg(notReadAddr_F1F1);
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("13F1、14F1、F1F1未全部抄读成功，统计（失败数包含回复数据空和超时未回复）：\n")+result);
        }
    }
}


void Script_ReadMeter_SearchMeter_Upgrade_MultiTask::Refresh_TestResult_02F1(shared_ptr<Afn02F1> p_ChkNodeVrsnInfo_02F1_Up)
{
    if(p_ChkNodeVrsnInfo_02F1_Up->frame_length_==0)    //p_ChkNodeVrsnInfo_02F1_Up->protocol_type_!=0x04||
    {
        return;
    }

    QByteArray rptVrsnMnfcCode;
    rptVrsnMnfcCode.append(p_ChkNodeVrsnInfo_02F1_Up->frame_content_[24])
            .append(p_ChkNodeVrsnInfo_02F1_Up->frame_content_[25]);

    QByteArray rptVrsnChipCode;
    rptVrsnChipCode.append(p_ChkNodeVrsnInfo_02F1_Up->frame_content_[26])
            .append(p_ChkNodeVrsnInfo_02F1_Up->frame_content_[27]);

    QByteArray rptVrsn;
    rptVrsn.append(p_ChkNodeVrsnInfo_02F1_Up->frame_content_[29])
            .append(p_ChkNodeVrsnInfo_02F1_Up->frame_content_[30]);

    QByteArray rptVrsnDate;
    rptVrsnDate.append(p_ChkNodeVrsnInfo_02F1_Up->frame_content_[34])
               .append(p_ChkNodeVrsnInfo_02F1_Up->frame_content_[35])
               .append(p_ChkNodeVrsnInfo_02F1_Up->frame_content_[36]);

    QByteArray rptVrsnTime;
    rptVrsnTime.append(p_ChkNodeVrsnInfo_02F1_Up->frame_content_[37])
               .append(p_ChkNodeVrsnInfo_02F1_Up->frame_content_[38])
               .append(p_ChkNodeVrsnInfo_02F1_Up->frame_content_[39]);

    uchar nodeAddr[6];
    for(int j=0;j<6;j++)
    {
        nodeAddr[j]=uchar(p_ChkNodeVrsnInfo_02F1_Up->frame_content_[12+j]);
    }


    for(int i=0; i<p_CtrInfoList->at(0)->totalNodeCnt; i++)
    {
        if(true==isArrayEqual(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(i)->mtrAddr, nodeAddr, 6))
        {

            QString res="节点地址"+QString(QByteArray(reinterpret_cast<char*>(nodeAddr),6).toHex())
                    + "\n目标厂商芯片代码"+this->staVendorChipCode + "  回复厂商芯片代码"+QString(rptVrsnMnfcCode+rptVrsnChipCode)
                    + "\n目标内部版本"+this->staInVrsn + "  回复内部版本"+ QString((rptVrsnDate+rptVrsnTime+rptVrsn).toHex());

            if(staVendorChipCode==QString(rptVrsnMnfcCode+rptVrsnChipCode)
                    && staInVrsn.toLatin1()==(rptVrsnDate+rptVrsnTime+rptVrsn))
            {
                res="Success  " + res;
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, res);
                p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(i)->testResultList[0] = true;
            }
            else
            {
                res="Fail  " + res;
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, res);
            }
            break;
        }
    }
}

void Script_ReadMeter_SearchMeter_Upgrade_MultiTask::Refresh_SuccessCnt_02F1()
{
    p_CtrInfoList->at(0)->successCnt[0]=0;

    for(int i=0; i<p_CtrInfoList->at(0)->totalNodeCnt; i++)
    {
        if(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(i)->testResultList[0]==true)
            ++p_CtrInfoList->at(0)->successCnt[0];
    }

    p_CtrInfoList->at(0)->successRate[0]=(double)(p_CtrInfoList->at(0)->successCnt[0])/(double)(p_CtrInfoList->at(0)->totalNodeCnt);
    failCnt_InVrsn = p_CtrInfoList->at(0)->totalNodeCnt - p_CtrInfoList->at(0)->successCnt[0];
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("TotalCnt=%1; SuccessCnt=%2; SuccessRate=%3%").arg(p_CtrInfoList->at(0)->totalNodeCnt).arg(p_CtrInfoList->at(0)->successCnt[0]).arg(p_CtrInfoList->at(0)->successRate[0]*100));
}

void Script_ReadMeter_SearchMeter_Upgrade_MultiTask::RetransmitFailedSegments()
{
    if(failedSegments.isEmpty())
    {
        // 没有失败的段，检查是否所有段都成功
        ushort successCnt = Refresh_SuccessCnt_15F1();
        if(successCnt == p_FileTransfer_15F1_Down->file_transfer_unit_.total_num_)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "所有段传输完成");
            return;
        }
    }

    isRetransmitting = true;

    // 获取第一个失败的段
    ushort segToRetransmit = failedSegments.first();

    // 增加该段的重传计数
    if(!segmentRetryCount.contains(segToRetransmit))
    {
        segmentRetryCount[segToRetransmit] = 1;
    }
    else
    {
        segmentRetryCount[segToRetransmit]++;
    }

    // 检查是否超过最大重传次数
    if(segmentRetryCount[segToRetransmit] > MAX_RETRY_TIMES)
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Failed,
            QString("第(%1)段重传超过最大次数，升级失败").arg(segToRetransmit));
        failedSegments.removeAll(segToRetransmit);
        emScriptRunState = ScriptSuccess;
        return;
    }

    // 重新发送该段
    fileIndex = segToRetransmit;
    p_AbstractScriptHost->updateProgress(ProcessState_Processing,
        QString("重传第(%1)段，重试次数: %2/%3").arg(segToRetransmit)
        .arg(segmentRetryCount[segToRetransmit]).arg(MAX_RETRY_TIMES));

    sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID,0, p_FileTransfer_15F1_Down);
    p_timer->start(30*1000);  // 重传超时时间
}


Address Script_ReadMeter_SearchMeter_Upgrade_MultiTask::extractAddressFromAfn06F2(shared_ptr<Afn06F2> p_ReportReadData_Up)
{
    Address srcAddr;
    memset(srcAddr.addr, 0, sizeof(srcAddr.addr));

    if (!p_ReportReadData_Up) {
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "错误: Afn06F2对象为空");
        return srcAddr;
    }

    // 检查报文长度是否为0
    if (p_ReportReadData_Up->report_data_unit_.frame_length_ == 0) {
        p_AbstractScriptHost->updateProgress(ProcessState_Processing,
                                             QString("06F2报文内容为空，从节点序号: %1").arg(p_ReportReadData_Up->report_data_unit_.node_no_));

        // 尝试从从节点序号查找地址
        int nodeNo = p_ReportReadData_Up->report_data_unit_.node_no_;
        if (nodeNo > 0 && nodeNo <= readInfoList.size()) {
            memcpy(srcAddr.addr, readInfoList[nodeNo-1].meterNo.addr, 6);
            p_AbstractScriptHost->updateProgress(ProcessState_Processing,
                                                 QString("从readInfoList获取地址: %1").arg(QString(QByteArray((char*)srcAddr.addr, 6).toHex())));
        }
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

bool Script_ReadMeter_SearchMeter_Upgrade_MultiTask::extractAndProcess3762Frame(QByteArray& buf, QByteArray& completeFrame)
{
    completeFrame.clear();

    p_AbstractScriptHost->updateProgress(ProcessState_Processing,
        QString("当前缓冲区数据: %1, 长度: %2").arg(QString(buf.toHex())).arg(buf.size()));

    // 至少需要：起始符(1) + 长度字段(2) + 结束符(1) = 4字节
    if (buf.size() < 4) {
        return false;
    }

    // 查找帧起始标志
    int frameStart = -1;
    for (int i = 0; i < buf.size(); i++) {
        if ((unsigned char)buf[i] == 0x68) {
            frameStart = i;
            break;
        }
    }

    if (frameStart == -1) {
        buf.clear();
        return false;
    }

    // 移除帧起始之前的数据
    if (frameStart > 0) {
        buf.remove(0, frameStart);
    }

    // 确保至少有起始符+长度字段
    if (buf.size() < 3) {
        return false;
    }

    // 读取帧长度（小端序）
    uint16_t frameLength = ((unsigned char)buf[2] << 8) | (unsigned char)buf[1];

    // 帧长度异常检查（根据3762协议，合理范围应该在某个区间）
    if (frameLength < 4 || frameLength > 1024) {  // 根据实际协议调整上限
        p_AbstractScriptHost->updateProgress(ProcessState_Processing,
            QString("帧长度异常: %1，丢弃此字节").arg(frameLength));
        buf.remove(0, 1);
        return true;
    }

    // 等待完整帧
    if (buf.size() < frameLength) {
        p_AbstractScriptHost->updateProgress(ProcessState_Processing,
            QString("等待完整帧，需要%1字节，当前%2字节").arg(frameLength).arg(buf.size()));
        return false;
    }

    // 检查帧结束标志
    if ((unsigned char)buf[frameLength - 1] != 0x16) {
        p_AbstractScriptHost->updateProgress(ProcessState_Processing,
            QString("帧结束标志错误(位置%1: 0x%2)，跳过此帧")
                .arg(frameLength - 1)
                .arg((unsigned char)buf[frameLength - 1], 2, 16, QChar('0')));
        buf.remove(0, 1);
        return true;
    }

    // 提取完整帧
    completeFrame = buf.left(frameLength);

    // 从缓冲区移除已处理的数据
    buf.remove(0, frameLength);

    p_AbstractScriptHost->updateProgress(ProcessState_Processing,
        QString("提取到完整3762帧: %1").arg(QString(completeFrame.toHex())));


    return true;
}
