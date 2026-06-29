#include "Script_Upgrade_STA_normal_2_MultiTask.h"
#include <QDataStream>
#include <QDir>

Script_Upgrade_STA_normal_2_MultiTask::Script_Upgrade_STA_normal_2_MultiTask(QObject *parent) : QObject(parent)
{
    emScriptRunState=UpgradeInit;
    resultFlag=false;
    p_CtrInfoList = make_shared<QList<shared_ptr<CtrInfo>>>();

    p_BuildNetwork=make_shared<BuildNetwork_GW>();
    p_MsgBase_1376_2=make_shared<qgdw_3762_protocol::Frame3762Helper>();
    p_FileTransfer_15F1_Down=make_shared<qgdw_3762_protocol::Afn15F1>();
    p_CcoRunStateInfo_10F4_Down=make_shared<qgdw_3762_protocol::Afn10F4>();
    p_CcoRunModeInfo_03F10_Down=make_shared<qgdw_3762_protocol::Afn03F10>();
    p_CcoCtrlPause_12F2=make_shared<qgdw_3762_protocol::Afn12F2>();
    p_StopSlaveNodeReg_11F6=make_shared<qgdw_3762_protocol::Afn11F6>();
    p_ChkStaInVrsnInfo_02F1_Down=make_shared<qgdw_3762_protocol::Afn02F1>();
    p_ChkStaOutVrsnInfo_10F104_Down=make_shared<qgdw_3762_protocol::Afn10F104>();
    p_ActiveRegister_11F5=make_shared<Afn11F5>();
    p_Confirm_00F1=make_shared<Afn00F1>();
    p_RouterRestart_12F1=make_shared<Afn12F1>();
    p_RouterRequestRead_14F1=make_shared<Afn14F1>();
    p_MonitorSlaveNode_13F1=make_shared<Afn13F1>();
    p_ParallelReadMeter_F1F1=make_shared<AfnF1F1>();
    sendMsgOct.clear();

    p_MsgBase_645=make_shared<dlt_645_Protocol::Frame645Helper>();
    p_MeterAddrResp_93=make_shared<dlt_645_Protocol::RspsNormal_ReadAddr_0x93>(addr,6);
    p_MsgBase_698_45=make_shared<object_oriented_electic_data_exchange_protocol::FrameOOPHelper>();
    p_GetResponseNormal_ReadAddr=make_shared<object_oriented_electic_data_exchange_protocol::GetResponseNormal>();

    dataLen=0;
    totalSegs=0;
    transResList.clear();
    cfgCntPerTime=10;
    msgSeq=0;

    havePassedTimeLen=0.0;
    haveStartContinueTimer=false;
    /***可配置参数***/
    timerForReachThresld=1800; //单位:s
    netSucRateThresld=1.0;
    timerAfterReachThresld=300; //单位:s

    timerForReachThresld_Upgrade=1200; //单位:s
    sucRateThresld_Upgrade=1.0;
    timerAfterTransferFinished=3600; //单位:s

    timerForReachThresld_QueryNodeVrsnInfo02F1=1200; //单位:s
    sucRateThresld_QueryNodeVrsnInfo02F1=1.0;
    timerAfterReachThresld_QueryNodeVrsnInfo02F1=180; //单位:s

    needBuildNet=true;
    dstUpgradeDvc=1;

    staOutVrsn.clear();
    staInVrsn.clear();

    p_timer=make_shared<QTimer>(this);
    p_delayTimer=make_shared<QTimer>(this);
    p_maxAllowTimer=make_shared<QTimer>(this);
    p_14F1Timer=make_shared<QTimer>(this);
    p_13F1Timer=make_shared<QTimer>(this);
    p_F1F1Timer=make_shared<QTimer>(this);
    connect(p_timer.get(),SIGNAL(timeout()),this,SLOT(timer_timeoutProc()));
    connect(p_delayTimer.get(),SIGNAL(timeout()),this,SLOT(delayTimer_timeoutProc()));
    connect(p_maxAllowTimer.get(),SIGNAL(timeout()),this,SLOT(maxAllowTimer_timeoutProc()));
    connect(p_14F1Timer.get(),SIGNAL(timeout()),this,SLOT(timeoutProc_14F1()));
    connect(p_13F1Timer.get(),SIGNAL(timeout()),this,SLOT(timeoutProc_13F1()));
    connect(p_F1F1Timer.get(),SIGNAL(timeout()),this,SLOT(timeoutProc_F1F1()));
}

bool Script_Upgrade_STA_normal_2_MultiTask::tryLockMeter(char* meterAddr, const QString& taskName)
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
void Script_Upgrade_STA_normal_2_MultiTask::unlockMeter(char* meterAddr)
{
    QString addr = QString(QByteArray((char*)meterAddr, 6).toHex());

    QMutexLocker locker(&m_meterMutex);
    m_busyMeters.remove(addr);
    m_meterLockTime.remove(addr);
}

// 尝试启动13F1抄表，失败时2秒后自动重试
void Script_Upgrade_STA_normal_2_MultiTask::tryStart13F1()
{
    if(emScriptRunState != Wait_StaUpgradeFinish) {
        return;  // 测试已结束，不再重试
    }

    if(readNo_13F1 >= readInfoList.size()) {
        readNum_13F1 = ushort(readInfoList.size());
        if(!end13F1Flag) {
            end13F1Flag = true;
            p_AbstractScriptHost->updateProgress(ProcessState_Processing,
                QString("13F1按当前表档案数量%1结束").arg(readNum_13F1));
            checkResult();
        }
        return;
    }
    
    if(readNo_13F1 >= readNum_13F1) {
        return;  // 已完成所有抄表
    }
    
    int meterIdx = readNo_13F1;
    
    if(tryLockMeter(readInfoList[meterIdx].meterNo.addr, "13F1")) {
        readNo_13F1++;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing,
            QString("发送第%1条13F1抄表").arg(readNo_13F1));
        sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, meterIdx, p_MonitorSlaveNode_13F1);
        p_13F1Timer->start(100*1000);
    } else {
        // 锁定失败，2秒后重试
        p_AbstractScriptHost->updateProgress(ProcessState_Processing,
            QString("[13F1] 表%1被占用，2秒后重试").arg(
                QString(QByteArray((char*)readInfoList[meterIdx].meterNo.addr, 6).toHex())));
        QTimer::singleShot(2000, this, [this]() { tryStart13F1(); });
    }
}

// 尝试启动F1F1抄表，失败时2秒后自动重试
void Script_Upgrade_STA_normal_2_MultiTask::tryStartF1F1()
{
    if(emScriptRunState != Wait_StaUpgradeFinish) {
        return;  // 测试已结束，不再重试
    }

    if(readNo_F1F1 >= readInfoList.size()) {
        readNum_F1F1 = ushort(readInfoList.size());
        if(!endF1F1Flag) {
            endF1F1Flag = true;
            p_AbstractScriptHost->updateProgress(ProcessState_Processing,
                QString("F1F1按当前表档案数量%1结束").arg(readNum_F1F1));
            checkResult();
        }
        return;
    }
    
    if(readNo_F1F1 >= readNum_F1F1) {
        return;  // 已完成所有抄表
    }
    
    int meterIdx = readNo_F1F1;
    
    if(tryLockMeter(readInfoList[meterIdx].meterNo.addr, "F1F1")) {
        readNo_F1F1++;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing,
            QString("发送第%1条F1F1抄表").arg(readNo_F1F1));
        sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, meterIdx, p_ParallelReadMeter_F1F1);
        p_F1F1Timer->start(100*1000);
    } else {
        // 锁定失败，2秒后重试
        p_AbstractScriptHost->updateProgress(ProcessState_Processing,
            QString("[F1F1] 表%1被占用，2秒后重试").arg(
                QString(QByteArray((char*)readInfoList[meterIdx].meterNo.addr, 6).toHex())));
        QTimer::singleShot(2000, this, [this]() { tryStartF1F1(); });
    }
}

Script_Upgrade_STA_normal_2_MultiTask::~Script_Upgrade_STA_normal_2_MultiTask()
{
    p_timer->stop();
    p_maxAllowTimer->stop();
    p_delayTimer->stop();
    if (p_14F1Timer) p_14F1Timer->stop();
    if (p_13F1Timer) p_13F1Timer->stop();
    if (p_F1F1Timer) p_F1F1Timer->stop();
    // 清理缓冲区
    if (p_CtrInfoList && p_CtrInfoList->size() > 0) {
        p_CtrInfoList->at(0)->buf.clear();
    }
    powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost);
}

void  Script_Upgrade_STA_normal_2_MultiTask::execute()
{
    p_AbstractScriptHost->updateProgress(ProcessState_Start, "Test start!");

    emScriptRunState=UpgradeInit;
    resultFlag=false;

    if(needBuildNet==true)
    {
        p_BuildNetwork->execute();
        emScriptRunState=Wait_BuildNetFinish_Whole;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("等待全网组网完成"));
        p_timer->start((timerForReachThresld+timerAfterReachThresld)*1000);
    }
    else
    {
        if(isStdPrcs)
        {
            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,0,p_CcoCtrlPause_12F2);
            emScriptRunState=Wait_00F1_for_12F2_UpgrdSta;
            p_timer->start(10*1000);
        }
        else
        {
            /****调入升级包并计算每段长度和总段数*****/
            LoadUpdateFile();
            fileIndex=0;
            emScriptRunState=Wait_FileTransferFinish;
            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,0,p_FileTransfer_15F1_Down);
            qDebug()<<"发送文件传输报文"<<"段值索引："<<fileIndex;
            p_timer->start(10*1000);
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "等待升级文件传输完成");
            p_maxAllowTimer->start(timerForReachThresld_Upgrade*1000);
        }
    }
}

void  Script_Upgrade_STA_normal_2_MultiTask::stop()
{
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "Test stop!");
}

void Script_Upgrade_STA_normal_2_MultiTask::addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq)
{
    p_CtrInfoList->clear();
    comnAddAddrsInfo(p_CtrInfoList, p_ConcentratorInfoList, p_MeterInfoList, p_SchemeCfgInfoList);
    concentratorCnt=p_CtrInfoList->size();

    uchar dstFreq=freq&0x0f;
    uchar dstPrtcl=(freq>>4)&0x0f;
    if(dstPrtcl==0x03)
        setMeterAddrsPrtcl(p_CtrInfoList,dstPrtcl);

    p_BuildNetwork->setCtrInfoListAndFreq(p_CtrInfoList, dstFreq);
}

void  Script_Upgrade_STA_normal_2_MultiTask::setHost(AbstractScriptHost *host)
{
    p_AbstractScriptHost=host;
    p_BuildNetwork->setHost(host);
}

bool  Script_Upgrade_STA_normal_2_MultiTask::config(const QMap<QString,QString> *paraDic)
{
    bool result = false;
    if(paraDic!=nullptr)
    {
        p_BuildNetwork->config(paraDic);

        if(paraDic->keys().contains("timerForReachThresld"))
        {
            this->timerForReachThresld = (*paraDic)["timerForReachThresld"].toUShort();
        }
        if(paraDic->keys().contains("netSucRateThresld"))
        {
            this->netSucRateThresld = (*paraDic)["netSucRateThresld"].toDouble();
        }
        if(paraDic->keys().contains("timerAfterReachThresld"))
        {
            this->timerAfterReachThresld = (*paraDic)["timerAfterReachThresld"].toUShort();
        }
        if(paraDic->keys().contains("timerForReachThresld_Upgrade"))
        {
            this->timerForReachThresld_Upgrade = (*paraDic)["timerForReachThresld_Upgrade"].toUShort();
        }
        if(paraDic->keys().contains("sucRateThresld_Upgrade"))
        {
            this->sucRateThresld_Upgrade = (*paraDic)["sucRateThresld_Upgrade"].toDouble();
        }
        if(paraDic->keys().contains("timerAfterTransferFinished"))
        {
            this->timerAfterTransferFinished = (*paraDic)["timerAfterTransferFinished"].toUShort();
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
        if(paraDic->keys().contains("isStdPrcs"))
        {
            if((*paraDic)["isStdPrcs"].toLower()=="false")
            {
                this->isStdPrcs=false;
            }
            else
            {
                this->isStdPrcs=true;
            }
        }
        if(paraDic->keys().contains("dstUpgradeDvc"))
        {
            this->dstUpgradeDvc=(*paraDic)["dstUpgradeDvc"].toUShort();
        }
        if(paraDic->keys().contains("timerForReachThresld_QueryNodeVrsnInfo02F1"))
        {
            this->timerForReachThresld_QueryNodeVrsnInfo02F1 = (*paraDic)["timerForReachThresld_QueryNodeVrsnInfo02F1"].toUShort();
        }
        if(paraDic->keys().contains("sucRateThresld_QueryNodeVrsnInfo02F1"))
        {
            this->sucRateThresld_QueryNodeVrsnInfo02F1 = (*paraDic)["sucRateThresld_QueryNodeVrsnInfo02F1"].toDouble();
        }
        if(paraDic->keys().contains("timerAfterReachThresld_QueryNodeVrsnInfo02F1"))
        {
            this->timerAfterReachThresld_QueryNodeVrsnInfo02F1 = (*paraDic)["timerAfterReachThresld_QueryNodeVrsnInfo02F1"].toUShort();
        }
        if(paraDic->keys().contains("staVendorChipCode"))
        {
            staVendorChipCode.clear();
            this->staVendorChipCode=(*paraDic)["staVendorChipCode"];
        }
        if(paraDic->keys().contains("staOutVrsn"))
        {
            staOutVrsn.clear();
            this->staOutVrsn=(*paraDic)["staOutVrsn"];
        }
        if(paraDic->keys().contains("staInVrsn"))
        {
            staInVrsn.clear();
            this->staInVrsn=(*paraDic)["staInVrsn"];
        }
        if(paraDic->keys().contains("activeTime"))
        {
            this->activeTime = (*paraDic)["activeTime"].toUShort();
        }
        if(paraDic->keys().contains("readNum_13F1"))
        {
            this->readNum_13F1 = (*paraDic)["readNum_13F1"].toUShort();
        }
        if(paraDic->keys().contains("requestMeterNum_14F1"))
        {
            this->requestMeterNum_14F1 = (*paraDic)["requestMeterNum_14F1"].toUShort();
        }
        if(paraDic->keys().contains("readNum_F1F1"))
        {
            this->readNum_F1F1 = (*paraDic)["readNum_F1F1"].toUShort();
        }
        result = true;
    }
    return result;
}

void Script_Upgrade_STA_normal_2_MultiTask::processMsg(DvcType dvcType, int id, uchar *data, int datalen)
{
    if(emScriptRunState==Wait_BuildNetFinish_Whole)
    {
        if(!p_BuildNetwork->buildNetworkResultFlag)
            p_BuildNetwork->processMsg(dvcType,id,data,datalen);
        else
        {
            p_timer->stop();
            if(isStdPrcs)
            {
                sendTimes=0;
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,0,p_CcoCtrlPause_12F2);
                emScriptRunState=Wait_00F1_for_12F2_UpgrdSta;
                p_timer->start(5*1000);
            }
            else
            {
                /****调入升级包并计算每段长度和总段数*****/
                LoadUpdateFile();
                fileIndex=0;
                emScriptRunState=Wait_FileTransferFinish;
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,0,p_FileTransfer_15F1_Down);
                qDebug()<<"发送文件传输报文"<<"段值索引："<<fileIndex;
                p_timer->start(10*1000);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "等待升级文件传输完成");
                p_maxAllowTimer->start(timerForReachThresld_Upgrade*1000);
            }
        }
    }
    else
    {
        QByteArray tmpRecvTempData=QByteArray::fromRawData(reinterpret_cast<char*>(data),datalen);
        QByteArray recvTempData;
        recvTempData.append(tmpRecvTempData);
        delete[] data;

        if(p_CtrInfoList->size()==0)
            return;

        if(dvcType==CCO_GW)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到路由报文：%1").arg(QString(recvTempData.toHex())));
            p_CtrInfoList->at(0)->buf.append(recvTempData);
            processMsgFromCco(id);
        }
        else if(dvcType==SingleSTA || dvcType==ThreeSTA)
        {
            if(dvcType==SingleSTA)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到单通报文：%1").arg(QString(recvTempData.toHex())));
            }
            else
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到三通报文：%1").arg(QString(recvTempData.toHex())));
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
                        processMsgFromMeter698(dvcType,id,p_CtrInfoList->at(0)->keyList.at(i));
                        break;
                    }
                }
            }
        }
        else
        {
            return;
        }
    }
}

void Script_Upgrade_STA_normal_2_MultiTask::processCtrlDvcRes(DvcType dvcType, QList<int> idList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params)
{
    if(isSucs==false)
        return;
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到控制命令回复，参数个数=%1").arg(QString::number(params.size())));

    QList<int> sendParams;
    switch(emScriptRunState)
    {
    case UpgradeInit:
    {
        break;
    }
    case Wait_BuildNetFinish_Whole:
    {
        p_BuildNetwork->processCtrlDvcRes(dvcType,idList,ctrlCmdType,isSucs,params);
        break;
    }
    case Wait_00F1_for_12F2_UpgrdSta:
    {
        break;
    }
    case Wait_00F1_for_11F6_UpgrdSta:
    {
        break;
    }
    case Wait_15F1_for_15F1_BeforeUpgrdSta:
    {
        break;
    }
    case Wait_Res_for_10F4_BeforeUpgrdSta:
    {
        break;
    }
    case Wait_FileTransferFinish:
    {
        break;
    }
    case Wait_Res_for_03F10_WaitTimeLen:
    {
        break;
    }
    case Wait_StaUpgradeFinish:
    {
        break;
    }
    case Wait_QueryInVrsnInfo02F1Finish:
    {
        break;
    }
    case ScriptSuccess:
    {
        break;
    }
    default:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error:processCtrlDvcRes()");
        emScriptRunState=ScriptSuccess;
        break;
    }
    }
}

void  Script_Upgrade_STA_normal_2_MultiTask::timer_timeoutProc()
{
    p_timer->stop();
    switch(emScriptRunState)
    {
    case Wait_BuildNetFinish_Whole:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait build whole net finish timeout!!!");
        emScriptRunState=ScriptSuccess;
        break;
    }
    case Wait_00F1_for_12F2_UpgrdSta:
    {
        if(++sendTimes>=3)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_00F1_for_12F2 timeout!!!");
            emScriptRunState=ScriptSuccess;
        }
        else
        {
            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,0,p_CcoCtrlPause_12F2);
            p_timer->start(5*1000);
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("超时，重发12F2暂停路由，等待--确认"));
        }
        break;
    }
    case Wait_00F1_for_11F6_UpgrdSta:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_11F6_UpgrdSta timeout!!!");
        emScriptRunState=ScriptSuccess;
        break;
    }
    case Wait_15F1_for_15F1_BeforeUpgrdSta:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_15F1_BeforeUpgrdSta timeout!!!");
        emScriptRunState=ScriptSuccess;

        break;

    }
    case Wait_Res_for_10F4_BeforeUpgrdSta:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_Res_for_10F4_BeforeUpgrdSta timeout!!!");
        emScriptRunState=ScriptSuccess;
        break;
    }
    case Wait_FileTransferFinish:
    {
//        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait Upgrade timeout!!!"+QString("文件索引%1").arg(fileIndex));
//        emScriptRunState=ScriptSuccess;
//        break;
        if(!failedSegments.contains(fileIndex))
        {
            failedSegments.append(fileIndex);
        }

        p_AbstractScriptHost->updateProgress(ProcessState_Processing,
                                             QString("第(%1)段传输超时，准备重传").arg(fileIndex));

        RetransmitFailedSegments();
        break;
    }
    case Wait_Res_for_03F10_WaitTimeLen:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_Res_for_03F10_WaitTimeLen timeout!!!");
        emScriptRunState=ScriptSuccess;
        break;
    }
    case Wait_StaUpgradeFinish:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Timeout during Wait_StaUpgradeFinish:p_timer");
        emScriptRunState=ScriptSuccess;
        break;
    }
    case Wait_QueryOutVrsnInfo10F104Finish:
    {
        if(++sendTimes>=5)  // Increase retry count from 3 to 5
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("10F104查询节点%1超时，跳过该节点继续查询下一个").arg(meterIndex_10F104));
            
            // Add current node to failed list
            QString currentAddr = QString("节点%1").arg(meterIndex_10F104);
            if(meterIndex_10F104 <= p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().size())
            {
                currentAddr = QString(QByteArray(reinterpret_cast<char*>(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(meterIndex_10F104-1)->mtrAddr),6).toHex());
            }
            failAddr_OutVrsn.append(currentAddr);
            
            // Continue with next node
            if(++meterIndex_10F104>p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().size())
            {
                // All nodes processed, generate final result
                if(failAddr_OutVrsn.size()>0)
                {
                    QString failAddrs_OutVrsn = "查询外部版本失败的模块列表：\n";
                    for(int i=0;i<failAddr_OutVrsn.size();i++)
                    {
                        failAddrs_OutVrsn.append(failAddr_OutVrsn.at(i)+"\n");
                    }
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("STA外部版本查询完毕，失败数%0，%1").arg(failAddr_OutVrsn.size()).arg(failAddrs_OutVrsn));
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("查询外部版本失败数%0，升级过程中13F1成功数%1，失败数%2，14F1成功数%3，失败数%4，F1F1成功数%5，失败数%6，搜表上报数%7")
                                                         .arg(failAddr_OutVrsn.size()).arg(sucNum_13F1).arg(readNum_13F1-sucNum_13F1).arg(sucNum_14F1).arg(requestMeterNum_14F1-sucNum_14F1).arg(sucNum_F1F1).arg(readNum_F1F1-sucNum_F1F1).arg(searchNum));
                    emScriptRunState=ScriptSuccess;
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("STA外部版本全部正确"));
                    resultFlag=true;
                    p_AbstractScriptHost->updateProgress(ProcessState_Success, QString("查询外部版本失败数%0，升级过程中13F1成功数%1，失败数%2，14F1成功数%3，失败数%4，F1F1成功数%5，失败数%6，搜表上报数%7")
                                                         .arg(failAddr_OutVrsn.size()).arg(sucNum_13F1).arg(readNum_13F1-sucNum_13F1).arg(sucNum_14F1).arg(requestMeterNum_14F1-sucNum_14F1).arg(sucNum_F1F1).arg(readNum_F1F1-sucNum_F1F1).arg(searchNum));
                    emScriptRunState=ScriptSuccess;
                }
            }
            else
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("继续查询下一个STA外部版本"));
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,0,p_ChkStaOutVrsnInfo_10F104_Down);
                sendTimes=0;
                p_timer->start(10*1000);
            }
        }
        else
        {
            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,0,p_ChkStaOutVrsnInfo_10F104_Down);
            qDebug()<<"定时器超时，重发10F104_Down******"<<sendMsgOct.toHex()<<"状态机的值："<<emScriptRunState;
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("超时重发10F104 (第%1次重试)").arg(sendTimes));
            p_timer->start(10*1000);
        }
        break;
    }
    case Wait_QueryInVrsnInfo02F1Finish:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "Wait_02F1_for Query InVrsnInfo timeout!!!"+QString("表序号%1").arg(meterIndex_02F1));

        ++meterIndex_02F1;
        if(meterIndex_02F1==p_CtrInfoList->at(0)->totalNodeCnt+1)
        {
            meterIndex_02F1=0;
        }

        Refresh_SuccessCnt_02F1();
        if(p_CtrInfoList->at(0)->successRate[0]==1.0)
        {
            resultFlag=true;
            p_CtrInfoList->at(0)->successConsume[0]=(double)(timerForReachThresld_QueryNodeVrsnInfo02F1-p_maxAllowTimer->remainingTime())/1000.0/p_CtrInfoList->at(0)->successCnt[0];
            p_AbstractScriptHost->updateProgress(ProcessState_Success, QString("查询外部版本失败数%0，查询内部版本失败数%1；升级过程中13F1成功数%2，失败数%3，14F1成功数%4，失败数%5，F1F1成功数%6，失败数%7，搜表上报数%8")
                                                 .arg(failAddr_OutVrsn.size()).arg(failCnt_InVrsn).arg(sucNum_13F1).arg(readNum_13F1-sucNum_13F1).arg(sucNum_14F1).arg(requestMeterNum_14F1-sucNum_14F1).arg(sucNum_F1F1).arg(readNum_F1F1-sucNum_F1F1).arg(searchNum));
            p_maxAllowTimer->stop();
            emScriptRunState=ScriptSuccess;
            break;
        }
        else
        {
            while(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(meterIndex_02F1)->testResultList[0] == true)
            {
                meterIndex_02F1++;
                if(meterIndex_02F1==p_CtrInfoList->at(0)->totalNodeCnt)
                {
                    meterIndex_02F1=0;
                }
            }
            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,0,p_ChkStaInVrsnInfo_02F1_Down);
            qDebug()<<"定时器超时后，发送02F1_Down******"<<sendMsgOct.toHex()<<"状态机的值："<<emScriptRunState;
            p_timer->start(30*1000);
        }
        break;
    }
    default:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==p_timer");
        emScriptRunState=ScriptSuccess;
        break;
    }
    }
}

void Script_Upgrade_STA_normal_2_MultiTask::delayTimer_timeoutProc()
{
    p_delayTimer->stop();
    switch(emScriptRunState)
    {
    case Wait_StaUpgradeFinish:
    {
        if(startSearchMeterFlag == false)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("发送10F4查询路由运行状态"));
            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,0,p_CcoRunStateInfo_10F4_Down);
            p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
        }
        else
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "搜表持续时间到后未上报结束");
        }
        break;
    }
    default:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error:delayTimer_timeoutProc()");
        emScriptRunState=ScriptSuccess;
        break;
    }
    }
}

void Script_Upgrade_STA_normal_2_MultiTask::maxAllowTimer_timeoutProc()
{
    p_maxAllowTimer->stop();
    switch(emScriptRunState)
    {
    case Wait_FileTransferFinish:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait Upgrade Finish timeout!!!");
        emScriptRunState=ScriptSuccess;
        break;
    }
    case Wait_StaUpgradeFinish:
    {
        resultFlag=true;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("等待STA升级最大时间到，升级文件传输成功率：%1%; 升级文件传输耗时：%2秒;").arg(100).arg(p_CtrInfoList->at(0)->successConsume[0]));
        p_delayTimer->stop();
        p_timer->stop();

        meterIndex_10F104 = 1;
        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,0,p_ChkStaOutVrsnInfo_10F104_Down);
        qDebug()<<"发送10F104_Down******"<<sendMsgOct.toHex()<<"状态机的值："<<emScriptRunState;
        sendTimes=0;
        p_timer->start(5*1000);
        emScriptRunState=Wait_QueryOutVrsnInfo10F104Finish;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "开始查询模块外部版本信息");
        failAddr_OutVrsn.clear();
        break;
    }
    case Wait_QueryInVrsnInfo02F1Finish:
        {
            if(haveStartContinueTimer==false)
            {
                QString failedMeter = GenerateFailedMeterStr_Others(p_CtrInfoList->at(0));
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "查询内部版本"+failedMeter);

                p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("查询外部版本失败数%0，查询内部版本失败数%1；升级过程中13F1成功数%2，失败数%3，14F1成功数%4，失败数%5，F1F1成功数%6，失败数%7，搜表上报数%8")
                    .arg(failAddr_OutVrsn.size()).arg(failCnt_InVrsn).arg(sucNum_13F1).arg(readNum_13F1-sucNum_13F1).arg(sucNum_14F1).arg(requestMeterNum_14F1-sucNum_14F1).arg(sucNum_F1F1).arg(readNum_F1F1-sucNum_F1F1).arg(searchNum));
                emScriptRunState=ScriptSuccess;
            }
            else
            {
                p_timer->stop();
                if(p_CtrInfoList->at(0)->successRate[0]>=sucRateThresld_QueryNodeVrsnInfo02F1)
                {
                    QString failedMeter = GenerateFailedMeterStr_Others(p_CtrInfoList->at(0));
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, failedMeter);

                    p_CtrInfoList->at(0)->successConsume[0]=(double)((havePassedTimeLen+timerAfterReachThresld_QueryNodeVrsnInfo02F1)*1000)/1000.0;

                    resultFlag=true;
                    p_AbstractScriptHost->updateProgress(ProcessState_Success, QString("查询外部版本失败数%0，查询内部版本失败数%1；升级过程中13F1成功数%2，失败数%3，14F1成功数%4，失败数%5，F1F1成功数%6，失败数%7，搜表上报数%8")
                                                         .arg(failAddr_OutVrsn.size()).arg(failCnt_InVrsn).arg(sucNum_13F1).arg(readNum_13F1-sucNum_13F1).arg(sucNum_14F1).arg(requestMeterNum_14F1-sucNum_14F1).arg(sucNum_F1F1).arg(readNum_F1F1-sucNum_F1F1).arg(searchNum));
                    emScriptRunState=ScriptSuccess;
                }
                else
                {
                    QString failedMeter = GenerateFailedMeterStr_Others(p_CtrInfoList->at(0));
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "查询内部版本"+failedMeter);

                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("查询外部版本失败数%0，查询内部版本失败数%1；升级过程中13F1成功数%2，失败数%3，14F1成功数%4，失败数%5，F1F1成功数%6，失败数%7，搜表上报数%8")
                                                         .arg(failAddr_OutVrsn.size()).arg(failCnt_InVrsn).arg(sucNum_13F1).arg(readNum_13F1-sucNum_13F1).arg(sucNum_14F1).arg(requestMeterNum_14F1-sucNum_14F1).arg(sucNum_F1F1).arg(readNum_F1F1-sucNum_F1F1).arg(searchNum));
                    emScriptRunState=ScriptSuccess;
                }
            }
            break;
        }
    case ScriptSuccess:
    {
        resultFlag=true;
        p_AbstractScriptHost->updateProgress(ProcessState_Success, QString("定时时间到（非标准流程）  升级文件传输成功率：%1%; 升级文件传输耗时：%2秒;").arg(100).arg(p_CtrInfoList->at(0)->successConsume[0]));
        break;
    }
    default:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==p_maxAllowTimer");
        emScriptRunState=ScriptSuccess;
        break;
    }
    }
}

void Script_Upgrade_STA_normal_2_MultiTask::timeoutProc_14F1()
{
    p_14F1Timer->stop();
    if(start14F1Flag==false)
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "路由重启后10s内未收到14F1请求");
    }
    else
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("第%0次14F1请求后65s内未收到下一次请求").arg(requestNo_14F1));
    }
}

void Script_Upgrade_STA_normal_2_MultiTask::timeoutProc_13F1()
{
    p_13F1Timer->stop();
    p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("第%0次发送13F1后100s内未回复").arg(readNo_13F1));
}

void Script_Upgrade_STA_normal_2_MultiTask::timeoutProc_F1F1()
{
    p_F1F1Timer->stop();
    p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("第%0次发送F1F1后100s内未回复").arg(readNo_F1F1));
}

void Script_Upgrade_STA_normal_2_MultiTask::Refresh_TestResult_15F1(shared_ptr<Afn15F1> p_FileTransfer_15F1_Up)
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

ushort Script_Upgrade_STA_normal_2_MultiTask::Refresh_SuccessCnt_15F1()
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

void Script_Upgrade_STA_normal_2_MultiTask::Check_StaOutVrsnInfo(shared_ptr<Afn10F104> p_ChkStaOutVrsnInfo_10F104_Up)
{
    if(p_ChkStaOutVrsnInfo_10F104_Up->network_node_info_unit_.node_info_group_list_.size()==0
            || p_ChkStaOutVrsnInfo_10F104_Up->network_node_info_unit_.this_node_num_==0)
    {
        qDebug()<<"10F104回复报文信息为空";
        return;
    }

    QByteArray rptVrsnDate;
    rptVrsnDate.append(p_ChkStaOutVrsnInfo_10F104_Up->network_node_info_unit_.node_info_group_list_[0].node_info_.ver_year_)
               .append(p_ChkStaOutVrsnInfo_10F104_Up->network_node_info_unit_.node_info_group_list_[0].node_info_.ver_month_)
               .append(p_ChkStaOutVrsnInfo_10F104_Up->network_node_info_unit_.node_info_group_list_[0].node_info_.ver_day_);

    QByteArray rptVrsn;
    rptVrsn.append(p_ChkStaOutVrsnInfo_10F104_Up->network_node_info_unit_.node_info_group_list_[0].node_info_.software_version_[0])
            .append(p_ChkStaOutVrsnInfo_10F104_Up->network_node_info_unit_.node_info_group_list_[0].node_info_.software_version_[1]);

    QByteArray rptVrsnMnfcCode;
    rptVrsnMnfcCode.append(p_ChkStaOutVrsnInfo_10F104_Up->network_node_info_unit_.node_info_group_list_[0].node_info_.vendor_code_[0])
            .append(p_ChkStaOutVrsnInfo_10F104_Up->network_node_info_unit_.node_info_group_list_[0].node_info_.vendor_code_[1]);

    QByteArray rptVrsnChipCode;
    rptVrsnChipCode.append(p_ChkStaOutVrsnInfo_10F104_Up->network_node_info_unit_.node_info_group_list_[0].node_info_.chip_code_[0])
            .append(p_ChkStaOutVrsnInfo_10F104_Up->network_node_info_unit_.node_info_group_list_[0].node_info_.chip_code_[1]);

    QString addr = QString(QByteArray(p_ChkStaOutVrsnInfo_10F104_Up->network_node_info_unit_.node_info_group_list_[0].node_address_.addr,6).toHex());
    bool addrExist = false;
    for(int i=0;i<p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().size();i++)
    {
        if(addr==QByteArray(reinterpret_cast<char*>(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(i)->mtrAddr),6).toHex())
        {
            addrExist = true;
            break;
        }
    }
    if(addrExist == false)
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "10F104回复节点地址在档案中不存在："+addr);
    }

    QString res="节点地址"+addr
            + "\n目标厂商芯片代码"+this->staVendorChipCode + "  回复厂商芯片代码"+QString(rptVrsnMnfcCode+rptVrsnChipCode)
            + "\n目标外部版本"+this->staOutVrsn + "  回复外部版本"+ QString((rptVrsnDate+rptVrsn).toHex());

    // 支持多个厂商代码验证：GYY1, G1Y1
    QString deviceVendorCode = QString(rptVrsnMnfcCode+rptVrsnChipCode);
    QStringList supportedVendorCodes = {"GYY1", "G1GY"};
    bool vendorCodeMatch = supportedVendorCodes.contains(deviceVendorCode);
    
    if(vendorCodeMatch && this->staOutVrsn==QString((rptVrsnDate+rptVrsn).toHex()))
    {
        res="STA外部版本正确  " + res;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, res);
    }
    else
    {
        if(!vendorCodeMatch) {
            res="STA厂商代码不匹配  " + res;
        } else {
            res="STA外部版本有误  " + res;
        }
        failAddr_OutVrsn.append(addr);
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, res);
    }
}

void Script_Upgrade_STA_normal_2_MultiTask::Refresh_TestResult_02F1(shared_ptr<Afn02F1> p_ChkNodeVrsnInfo_02F1_Up)
{
    if(p_ChkNodeVrsnInfo_02F1_Up->frame_length_==0)    //p_ChkNodeVrsnInfo_02F1_Up->protocol_type_!=0x04||
    {
        qDebug()<<"02F1回复报文透传内容为空";
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

            // 支持多个厂商代码验证：GYY1, G1Y1
            QString deviceVendorCode = QString(rptVrsnMnfcCode+rptVrsnChipCode);
            QStringList supportedVendorCodes = {"GYY1", "G1Y1"};
            bool vendorCodeMatch = supportedVendorCodes.contains(deviceVendorCode);

            if(vendorCodeMatch && staInVrsn==(rptVrsnDate+rptVrsnTime+rptVrsn).toHex())
            {
                res="Success  " + res;
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, res);
                p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(i)->testResultList[0] = true;
            }
            else
            {
                if(!vendorCodeMatch) {
                    res="Fail(厂商代码不匹配)  " + res;
                } else {
                    res="Fail(内部版本不匹配)  " + res;
                }
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, res);
            }
            break;
        }
    }
}

void Script_Upgrade_STA_normal_2_MultiTask::Refresh_SuccessCnt_02F1()
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

void Script_Upgrade_STA_normal_2_MultiTask::processMsgFromCco(int id)
{
    if(id!=p_CtrInfoList->at(0)->ctrlID)
        return;

    QByteArray completeFrame;
    QByteArray& buf = p_CtrInfoList->at(0)->buf;
    bool haveCompleteMsg=true;
    while(extractAndProcess3762Frame(buf,completeFrame))
    {
        qInfo()<<QString("id=%1; 解析前 buf3762=%2").arg(id).arg(QString((completeFrame.toHex())));
        shared_ptr<Frame3762Base> p_Frame3762Base;
        try {
            p_Frame3762Base = p_MsgBase_1376_2->DecodeLocalMsg(&(completeFrame),haveCompleteMsg);
        } catch (...) {
            p_AbstractScriptHost->updateProgress(ProcessState_Error,
                QString("3762帧解析异常，跳过此帧：%1").arg(QString(completeFrame.toHex())));
            continue;
        }
        qInfo()<<QString("id=%1; 解析后 buf3762=%2").arg(id).arg(QString((completeFrame.toHex())));
        if(p_Frame3762Base==nullptr)
        {
            continue;
        }
        msgSeq=uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq);
        uchar dtValue3762;
        switch(emScriptRunState)
        {
        case UpgradeInit:
        {
            break;
        }
        case Wait_BuildNetFinish_Whole:
        {
            break;
        }
        case Wait_00F1_for_12F2_UpgrdSta:
        {
            dtValue3762=get3762Dt(p_Frame3762Base->dt1_,p_Frame3762Base->dt2_);
            if(p_Frame3762Base->afn_ == 0x00&&dtValue3762==1&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                qDebug()<<"收到00F1_Up报文******"<<"状态机的值："<<emScriptRunState;
                p_timer->stop();

                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,0,p_FileTransfer_15F1_Down);
                emScriptRunState=Wait_15F1_for_15F1_BeforeUpgrdSta;
                p_timer->start(20*1000);
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendMsg(CCO_GW,id,tmpSendMsg);
            }
            break;
        }
        case Wait_00F1_for_11F6_UpgrdSta:
        {
            dtValue3762=get3762Dt(p_Frame3762Base->dt1_,p_Frame3762Base->dt2_);
            if(p_Frame3762Base->afn_ == 0x00&&dtValue3762==1&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                qDebug()<<"收到00F1_Up报文******"<<"状态机的值："<<emScriptRunState;
                p_timer->stop();

                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,0,p_FileTransfer_15F1_Down);
                emScriptRunState=Wait_15F1_for_15F1_BeforeUpgrdSta;
                p_timer->start(10*1000);
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendMsg(CCO_GW,id,tmpSendMsg);
            }
            break;
        }
        case Wait_15F1_for_15F1_BeforeUpgrdSta:
        {
            dtValue3762=get3762Dt(p_Frame3762Base->dt1_,p_Frame3762Base->dt2_);
            if(p_Frame3762Base->afn_ == 0x15&&dtValue3762==1&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                qDebug()<<"收到15F1_Up清除下装文件回复******"<<"状态机的值："<<emScriptRunState;

                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到15F1_Up清除下装文件应答");
                p_timer->stop();

                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,0,p_CcoRunStateInfo_10F4_Down);
                emScriptRunState=Wait_Res_for_10F4_BeforeUpgrdSta;
                p_timer->start(10*1000);
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendMsg(CCO_GW,id,tmpSendMsg);
            }
            break;
        }
        case Wait_Res_for_10F4_BeforeUpgrdSta:
        {
            dtValue3762=get3762Dt(p_Frame3762Base->dt1_,p_Frame3762Base->dt2_);
            if(p_Frame3762Base->afn_ == 0x10&&dtValue3762==4&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                qDebug()<<"收到10F4_Up报文******"<<"状态机的值："<<emScriptRunState;
                p_timer->stop();

                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到10F4_Up报文");
                shared_ptr<Afn10F4> p_CcoRunStateInfo_10F4_Up=std::dynamic_pointer_cast<Afn10F4>(p_Frame3762Base);
                if(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.operate_state_word_.report_work_flag_==1 || p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.work_switch_.current_state_!=3)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("路由工作状态错误：工作标志%1，当前状态%2").arg(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.operate_state_word_.report_work_flag_).arg(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.work_switch_.current_state_));
                }
                /****调入升级包并计算每段长度和总段数*****/
                LoadUpdateFile();
                fileIndex=0;
                emScriptRunState=Wait_FileTransferFinish;
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,0,p_FileTransfer_15F1_Down);
                p_timer->start(30*1000);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "等待升级文件传输完成");
                p_maxAllowTimer->start(timerForReachThresld_Upgrade*1000);
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendMsg(CCO_GW,id,tmpSendMsg);
            }
            break;
        }
        case Wait_FileTransferFinish:
        {
            dtValue3762=get3762Dt(p_Frame3762Base->dt1_,p_Frame3762Base->dt2_);
            if(p_Frame3762Base->afn_ == 0x15&&dtValue3762==1&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                qDebug()<<"收到15F1_Up报文******"<<"状态机的值："<<emScriptRunState;
                p_timer->stop();

                shared_ptr<Afn15F1> p_FileTransfer_15F1_Up=std::dynamic_pointer_cast<Afn15F1>(p_Frame3762Base);
                Refresh_TestResult_15F1(p_FileTransfer_15F1_Up);

                ushort sucSegs = Refresh_SuccessCnt_15F1();

                if(sucSegs==p_FileTransfer_15F1_Down->file_transfer_unit_.total_num_)
                {
                    // 所有段都成功
                    p_CtrInfoList->at(0)->successConsume[0] =
                            double(timerForReachThresld_Upgrade*1000 - p_maxAllowTimer->remainingTime()) / 1000.0;

                    if(isStdPrcs)
                    {
                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,0,p_CcoRunModeInfo_03F10_Down);
                        emScriptRunState=Wait_Res_for_03F10_WaitTimeLen;
                        p_timer->start(10*1000);
                    }
                    else
                    {
                        emScriptRunState=ScriptSuccess;
                    //    p_maxAllowTimer->start(timerAfterTransferFinished*1000);
                    }
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
//                if(++fileIndex==p_FileTransfer_15F1_Down->file_transfer_unit_.total_num_)
//                {
//                    sucSegs=Refresh_SuccessCnt_15F1();
//                }

//                if(sucSegs==p_FileTransfer_15F1_Down->file_transfer_unit_.total_num_)
//                {
//                    p_CtrInfoList->at(0)->successConsume[0]=(double)(timerForReachThresld_Upgrade*1000-p_maxAllowTimer->remainingTime())/1000.0;

//                    if(isStdPrcs)
//                    {
//                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,0,p_CcoRunModeInfo_03F10_Down);
//                        emScriptRunState=Wait_Res_for_03F10_WaitTimeLen;
//                        p_timer->start(10*1000);
//                    }
//                    else
//                    {
//                        emScriptRunState=ScriptSuccess;
//                        p_maxAllowTimer->start(timerAfterTransferFinished*1000);
//                    }
//                }
//                else
//                {
//                    if(fileIndex<totalSegs)
//                    {
//                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,0,p_FileTransfer_15F1_Down);
//                        p_timer->start(30*1000);
//                    }
//                    else
//                    {
//                        p_maxAllowTimer->stop();
//                        emScriptRunState=ScriptSuccess;
//                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("升级文件传输成功率：%1%; 升级文件传输耗时：%2秒;").arg((double)(sucSegs)/(double)(totalSegs)*100).arg(p_CtrInfoList->at(0)->successConsume[0]));
//                    }
//                }
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendMsg(CCO_GW,id,tmpSendMsg);
            }
            break;
        }
        case Wait_Res_for_03F10_WaitTimeLen:
        {
            dtValue3762=get3762Dt(p_Frame3762Base->dt1_,p_Frame3762Base->dt2_);
            if(p_Frame3762Base->afn_ == 0x03&&dtValue3762==10&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                qDebug()<<"收到03F10_Up报文******"<<"状态机的值："<<emScriptRunState;
                p_timer->stop();
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到03F10_Up报文");

                emScriptRunState=Wait_StaUpgradeFinish;
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "开始等待模块升级，20s后查询10F4路由运行状态");
                p_delayTimer->start(20*1000);
                p_maxAllowTimer->start(timerAfterTransferFinished*1000);
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendMsg(CCO_GW,id,tmpSendMsg);
            }
            break;
        }
        case Wait_StaUpgradeFinish:
        {
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

                if(startSearchMeterFlag == false)
                {
                    if(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.work_switch_.current_state_ != 2)
                    {
                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_ActiveRegister_11F5);
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("路由处于升级状态，发送--11F5启动搜表（%0min），等待--确认").arg(activeTime));
                        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                        startSearchMeterFlag = true;
                        p_delayTimer->start(activeTime*65*1000);
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "路由运行状态显示升级STA未完成，等待60秒后继续读取10F4");
                        p_delayTimer->start(60*1000);
                    }
                }
                else
                {
                    QString state;
                    if(uchar(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.work_switch_.current_state_) == 0)
                    {
                        state = "抄表";
                    }
                    else if(uchar(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.work_switch_.current_state_) == 1)
                    {
                        state = "搜表";
                    }
                    else if(uchar(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.work_switch_.current_state_) == 2)
                    {
                        state = "升级";
                    }
                    else if(uchar(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.work_switch_.current_state_) == 3)
                    {
                        state = "其他";
                    }
                    else
                    {
                        state = "ERROR";
                    }

                    p_AbstractScriptHost->updateProgress(ProcessState_Success, QString("多任务结束后路由处于 %0 状态").arg(state));
                    emScriptRunState = ScriptSuccess;

//                    if(state == "升级")
//                    {
//                        p_AbstractScriptHost->updateProgress(ProcessState_Success, QString("多任务结束后路由仍处于 升级 状态"));
//                        emScriptRunState = ScriptSuccess;
//                    }
//                    else
//                    {
//                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("多任务结束后路由处于 %0 状态").arg(state));
//                    }
                }
            }
            else if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                ackNo++;
                if(ackNo==1)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "启动搜表收到确认");
                    readInfoInit();
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_RouterRestart_12F1);
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--路由重启（12F1），等待--确认");
                    p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                }
                else if(ackNo==2)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "路由重启收到确认，等待14F1请求");
                    p_14F1Timer->start(10*1000);
                }
                else if(ackNo==3)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "路由暂停收到确认");
                    end14F1Flag = true;
                    checkResult();
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Error, "收到确认，出现错误");
                }
            }
            else if(p_Frame3762Base->afn_==0x14&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_14F1Timer->stop();
                shared_ptr<Afn14F1> p_RouterRequestRead_14F1_Up=dynamic_pointer_cast<Afn14F1>(p_Frame3762Base);
                if(!p_RouterRequestRead_14F1_Up)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Error, "14F1解析结果为空，跳过该帧");
                    break;
                }
                int currentMeterIndex=getReadInfo(p_RouterRequestRead_14F1_Up->node_address_);
                if(currentMeterIndex < 0 || currentMeterIndex >= readInfoList.size())
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Error,
                        QString("14F1请求表地址不在当前档案中，跳过：%1")
                            .arg(QString(QByteArray(p_RouterRequestRead_14F1_Up->node_address_.addr,6).toHex())));
                    break;
                }
                readInfoList[currentMeterIndex].requestFlag_14F1=true;
                requestNo_14F1++;
                if(requestNo_14F1==1)
                {
                    start14F1Flag=true;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("路由重启后收到第%1个14F1请求，回复抄读数据项，表号%2，同时开始下发13F1、F1F1").arg(requestNo_14F1).arg(QString(QByteArray(p_RouterRequestRead_14F1_Up->node_address_.addr,6).toHex())));
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,currentMeterIndex,p_RouterRequestRead_14F1);
                    p_14F1Timer->start(65*1000);

//                    readNo_13F1++;
//                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("发送第%1条13F1抄表").arg(readNo_13F1));
//                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,readNo_13F1-1,p_MonitorSlaveNode_13F1);
//                    p_13F1Timer->start(100*1000);

//                    readNo_F1F1++;
//                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("发送第%1条F1F1抄表").arg(readNo_F1F1));
//                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,readNo_F1F1-1,p_ParallelReadMeter_F1F1);
//                    p_F1F1Timer->start(100*1000);
                    // 5秒后启动13F1抄表
                    QTimer::singleShot(5000, this, [this]() { tryStart13F1(); });

                    // 10秒后启动F1F1抄表
                    QTimer::singleShot(10000, this, [this]() { tryStartF1F1(); });
                }
                else
                {
                    if(requestNo_14F1>requestMeterNum_14F1)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到第%0个14F1请求，已经抄了%1个表，14F1抄表结束，抄读成功数%2").arg(requestNo_14F1).arg(requestMeterNum_14F1).arg(sucNum_14F1));
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--路由暂停（12F2），等待--确认");
                        sendMsg(p_CtrInfoList->at(0)->slotPosition,p_CtrInfoList->at(0)->dvcId,INSIGNIFICANCE,p_CcoCtrlPause_12F2);
                        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到第%0个14F1请求，回复抄读数据项，表号").arg(requestNo_14F1)+QByteArray(p_RouterRequestRead_14F1_Up->node_address_.addr,6).toHex());
                        sendMsg(p_CtrInfoList->at(0)->slotPosition,p_CtrInfoList->at(0)->dvcId,currentMeterIndex,p_RouterRequestRead_14F1);
                        p_14F1Timer->start(65*1000);
                    }
                }
            }
            else if(p_Frame3762Base->afn_==0x06&&p_Frame3762Base->dt1_==0x02&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                shared_ptr<Afn06F2> p_ReportReadData_Up=dynamic_pointer_cast<Afn06F2>(p_Frame3762Base);
                Address srcAddr = extractAddressFromAfn06F2(p_ReportReadData_Up);

                unlockMeter(srcAddr.addr);

                int currentMeterIndex=getReadInfo(srcAddr);

                if(currentMeterIndex < 0)
                {
                   break;
                }

                if(p_ReportReadData_Up->report_data_unit_.frame_length_>0)
                {
                    sucNum_14F1++;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到第%0条06F2成功上报抄读数据，表号").arg(sucNum_14F1)+QByteArray(srcAddr.addr,6).toHex());
                    readInfoList[currentMeterIndex].readFlag_14F1=ReadSuccess;
                }
                else
                {
                 //   reportFailNum_14F1++;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到06F2上报抄读数据内容为空，表号")+QByteArray(srcAddr.addr,6).toHex());
                    readInfoList[currentMeterIndex].readFlag_14F1=ReadFail;
                }
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "回复确认");
                sendMsg(p_CtrInfoList->at(0)->slotPosition,p_CtrInfoList->at(0)->dvcId,INSIGNIFICANCE,p_Confirm_00F1);
            }
            else if(p_Frame3762Base->afn_==0x13&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_13F1Timer->stop();
                shared_ptr<Afn13F1> p_MonitorSlaveNode_13F1_Up=dynamic_pointer_cast<Afn13F1>(p_Frame3762Base);
                Address srcAddr;
                memcpy(srcAddr.addr,p_MonitorSlaveNode_13F1_Up->address_field_.src_addr,6);
            //    int currentMeterIndex=getReadInfo(srcAddr);

                unlockMeter(srcAddr.addr);

                if(p_MonitorSlaveNode_13F1_Up->data_field_up_.frame_length_>0)
                {
                    sucNum_13F1++;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到第%0条13F1抄读成功回复数据，表号").arg(sucNum_13F1)+QByteArray(srcAddr.addr,6).toHex());
                  //  readInfoList[currentMeterIndex].readFlag_13F1=ReadSuccess;
                }
                else
                {
                   // responseFailNum_13F1++;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到13F1抄读回复数据内容为空，表号")+QByteArray(srcAddr.addr,6).toHex());
                  //  readInfoList[currentMeterIndex].readFlag_13F1=ReadFail;
                }
                if(readNo_13F1<readNum_13F1)
                {
                    // 使用统一的重试函数，失败时自动重试
                    tryStart13F1();
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("13F1已经抄了%1个表，13F1抄表结束，抄读成功数%2").arg(readNum_13F1).arg(sucNum_13F1));
                    end13F1Flag = true;
                    checkResult();
                }
            }
            else if(p_Frame3762Base->afn_==char(0xF1)&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_F1F1Timer->stop();
                shared_ptr<AfnF1F1> p_ParallelReadMeter_F1F1_Up=dynamic_pointer_cast<AfnF1F1>(p_Frame3762Base);
                Address srcAddr;
                memcpy(srcAddr.addr,p_ParallelReadMeter_F1F1_Up->address_field_.src_addr,6);

                unlockMeter(srcAddr.addr);

                if(p_ParallelReadMeter_F1F1_Up->unit_up_.frame_length_>0)
                {
                    sucNum_F1F1++;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,
                                                         QString("收到第%0条F1F1抄读成功回复数据，表号").arg(sucNum_F1F1)
                                                         + QByteArray(srcAddr.addr,6).toHex());
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,
                                                         QString("收到F1F1抄读回复数据内容为空，表号")+QByteArray(srcAddr.addr,6).toHex());
                }

                if(readNo_F1F1<readNum_F1F1)
                {
                    // 使用统一的重试函数，失败时自动重试
                    tryStartF1F1();
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,
                                                         QString("F1F1已经抄了%1个表，F1F1抄表结束，抄读成功数%2").arg(readNum_F1F1).arg(sucNum_F1F1));
                    endF1F1Flag = true;
                    checkResult();
                }
//                p_F1F1Timer->stop();
//                shared_ptr<AfnF1F1> p_ParallelReadMeter_F1F1_Up=dynamic_pointer_cast<AfnF1F1>(p_Frame3762Base);
//                Address srcAddr;
//                memcpy(srcAddr.addr,p_ParallelReadMeter_F1F1_Up->address_field_.src_addr,6);
//              //  int currentMeterIndex=getReadInfo(srcAddr);

//                unlockMeter(srcAddr.addr);

//                if(p_ParallelReadMeter_F1F1_Up->unit_up_.frame_length_>0)
//                {
//                    sucNum_F1F1++;
//                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到第%0条F1F1抄读成功回复数据，表号").arg(sucNum_F1F1)+QByteArray(srcAddr.addr,6).toHex());
//                //    readInfoList[currentMeterIndex].readFlag_F1F1=ReadSuccess;
//                }
//                else
//                {
//                 //   responseFailNum_F1F1++;
//                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到F1F1抄读回复数据内容为空，表号")+QByteArray(srcAddr.addr,6).toHex());
//                 //   readInfoList[currentMeterIndex].readFlag_F1F1=ReadFail;
//                }
//                if(readNo_F1F1<readNum_F1F1)
//                {
//                    int meterIdx = readNo_F1F1;
//                    readNo_F1F1++;

//                    // 加锁检查
//                    if(tryLockMeter(readInfoList[meterIdx].meterNo.addr, "F1F1"))
//                    {
//                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,
//                                                             QString("发送第%1条F1F1继续抄下一只表").arg(readNo_F1F1));
//                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,meterIdx,p_ParallelReadMeter_F1F1);
//                        p_F1F1Timer->start(100*1000);
//                    }
//                    else
//                    {
//                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,
//                                                             QString("表被占用，跳过第%1条F1F1").arg(readNo_F1F1));
//                        readNo_F1F1--;
//                        QTimer::singleShot(2000, this, [this, meterIdx]()
//                        {
//                            if(readNo_F1F1 < readNum_F1F1 && tryLockMeter(readInfoList[meterIdx].meterNo.addr, "F1F1"))
//                            {
//                                readNo_F1F1++;
//                                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,meterIdx,p_ParallelReadMeter_F1F1);
//                                p_F1F1Timer->start(100*1000);
//                            }
//                        });
//                    }
////                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("发送第%1条F1F1继续抄下一只表").arg(readNo_F1F1));
////                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,readNo_F1F1-1,p_ParallelReadMeter_F1F1);
////                    p_F1F1Timer->start(100*1000);
//                }
//                else
//                {
//                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("F1F1已经抄了%1个表，F1F1抄表结束，抄读成功数%2").arg(readNum_F1F1).arg(sucNum_F1F1));
//                    endF1F1Flag = true;
//                    checkResult();
//                }
            }
            else if(p_Frame3762Base->afn_ == 0x06&&p_Frame3762Base->dt1_==0x08&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                shared_ptr<Afn06F4> p_ReportRegisterNode_Up=dynamic_pointer_cast<Afn06F4>(p_Frame3762Base);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("上报注册从节点第%0个，表号%1，回复确认").arg(++searchNum).arg(QString(QByteArray(p_ReportRegisterNode_Up->report_node_info_unit_.report_node_address_.addr,6).toHex())));
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_Confirm_00F1);
              //  reportFlag=true;
            }
            else if(p_Frame3762Base->afn_ == 0x06&&p_Frame3762Base->dt1_==0x04&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                shared_ptr<Afn06F3> p_ReportRouterState_Up=dynamic_pointer_cast<Afn06F3>(p_Frame3762Base);
                if(p_ReportRouterState_Up->router_work_task_change_==0x02)
                {
                    p_delayTimer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "06F3路由上报工况变动：搜表任务结束，回复确认");
                    endSearchMeterFlag = true;
                }
                else if(p_ReportRouterState_Up->router_work_task_change_==0x01)
                {
                    p_14F1Timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "06F3路由上报工况变动：抄表任务结束，回复确认");
                    end14F1Flag = true;
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, "06F3路由上报工况变动，类型未知："+QString::number(p_ReportRouterState_Up->router_work_task_change_));
                    return;
                }
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_Confirm_00F1);
                checkResult();
            }
            else if(p_Frame3762Base->afn_ == 0x06&&p_Frame3762Base->dt1_==0x10&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                // 06F16 搜表从节点侧信息上报，需要回复确认
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到06F16搜表从节点侧信息上报，回复确认");
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_Confirm_00F1);
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendMsg(CCO_GW,id,tmpSendMsg);
            }
            break;
        }
        case Wait_QueryOutVrsnInfo10F104Finish:
        {
            dtValue3762=get3762Dt(p_Frame3762Base->dt1_,p_Frame3762Base->dt2_);
            if(p_Frame3762Base->afn_ == 0x10&&dtValue3762==104&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                shared_ptr<Afn10F104> p_ChkStaOutVrsnInfo_10F104_Up=std::dynamic_pointer_cast<Afn10F104>(p_Frame3762Base);
                Check_StaOutVrsnInfo(p_ChkStaOutVrsnInfo_10F104_Up);

                if(++meterIndex_10F104>p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().size())
                {
                    if(failAddr_OutVrsn.size()>0)
                    {
                        QString failAddrs_OutVrsn = "查询外部版本失败的模块列表：\n";
                        for(int i=0;i<failAddr_OutVrsn.size();i++)
                        {
                            failAddrs_OutVrsn.append(failAddr_OutVrsn.at(i)+"\n");
                        }
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("STA外部版本查询完毕，失败数%0，%1").arg(failAddr_OutVrsn.size()).arg(failAddrs_OutVrsn));
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("查询外部版本失败数%0，升级过程中13F1成功数%1，失败数%2，14F1成功数%3，失败数%4，F1F1成功数%5，失败数%6，搜表上报数%7")
                                                             .arg(failAddr_OutVrsn.size()).arg(sucNum_13F1).arg(readNum_13F1-sucNum_13F1).arg(sucNum_14F1).arg(requestMeterNum_14F1-sucNum_14F1).arg(sucNum_F1F1).arg(readNum_F1F1-sucNum_F1F1).arg(searchNum));
                        emScriptRunState=ScriptSuccess;
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("STA外部版本全部正确"));
                        resultFlag=true;
                        p_AbstractScriptHost->updateProgress(ProcessState_Success, QString("查询外部版本失败数%0，升级过程中13F1成功数%1，失败数%2，14F1成功数%3，失败数%4，F1F1成功数%5，失败数%6，搜表上报数%7")
                                                             .arg(failAddr_OutVrsn.size()).arg(sucNum_13F1).arg(readNum_13F1-sucNum_13F1).arg(sucNum_14F1).arg(requestMeterNum_14F1-sucNum_14F1).arg(sucNum_F1F1).arg(readNum_F1F1-sucNum_F1F1).arg(searchNum));
                        emScriptRunState=ScriptSuccess;
                    }
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("继续查询下一个STA外部版本"));
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,0,p_ChkStaOutVrsnInfo_10F104_Down);
                    sendTimes=0;
                    p_timer->start(10*1000);
                }
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendMsg(CCO_GW,id,tmpSendMsg);
            }
            break;
        }
        case ScriptSuccess:
        {
            break;
        }
        default:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error:processMsgFromCco()");
            emScriptRunState=ScriptSuccess;
            break;
        }
        }
    }
}

void Script_Upgrade_STA_normal_2_MultiTask::checkResult()
{
    if((end13F1Flag&&end14F1Flag&&endF1F1Flag&&endSearchMeterFlag) == false)
    {
        return;
    }

    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("13F1抄%1成功%2,14F1抄%3成功%4，F1F1抄%5成功%6，搜表上报%7个")
                                         .arg(readNum_13F1).arg(sucNum_13F1).arg(requestMeterNum_14F1).arg(sucNum_14F1).arg(readNum_F1F1).arg(sucNum_F1F1).arg(searchNum));
    QString failResult = nullptr;
    if(sucNum_13F1==0)
    {
        failResult.append(QString("13F1抄%0个表全部失败；").arg(readNum_13F1));
    }
    if(sucNum_14F1==0)
    {
        failResult.append(QString("14F1抄%0个表全部失败；").arg(requestMeterNum_14F1));
    }
//    if(sucNum_F1F1==0)
//    {
//        failResult.append(QString("F1F1抄%0个表全部失败；").arg(readNum_F1F1));
//    }
    if(searchNum==0)
    {
        failResult.append("未搜到表");
    }

    if(failResult == nullptr)
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("多任务结束，发送10F4查询路由运行状态"));
        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,0,p_CcoRunStateInfo_10F4_Down);
        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
    }
    else
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, failResult);
    }
}

void Script_Upgrade_STA_normal_2_MultiTask::processMsgFromMeter645(DvcType dvcType,int id, int mtrlID)
{
    bool haveCompleteMsg=true;
    while(haveCompleteMsg)
    {
        qInfo()<<QString("id=%1; 解析前 buf645=%2").arg(id).arg(QString((*p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList)[mtrlID]->buf645.toHex()));
        shared_ptr<Frame645Base> MsgBase_645_ptr = p_MsgBase_645->DecodeLocalMsg(&((*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[mtrlID]->buf645),haveCompleteMsg);
        qInfo()<<QString("id=%1; 解析后 buf645=%2").arg(id).arg(QString((*p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList)[mtrlID]->buf645.toHex()));
        if(MsgBase_645_ptr==nullptr)
        {
            break;
        }
        if(MsgBase_645_ptr->ctrlCode_==READ_ADDR)
        {
            sendMsg(dvcType,id,mtrlID,p_MeterAddrResp_93);
        }
        else
        {
            uchar di[4]={0x00};
            if(MsgBase_645_ptr->ctrlCode_==READ_DATA)
            {
                shared_ptr<Rqst_ReadData_0x11> Rqst_ReadData_0x11_ptr = std::dynamic_pointer_cast<Rqst_ReadData_0x11>(MsgBase_645_ptr);
                memcpy(di,Rqst_ReadData_0x11_ptr->di,4);

                QByteArray tmpSendMsg=prcsOther645Msg(MsgBase_645_ptr->ctrlCode_,MsgBase_645_ptr->dataLen_,di,MsgBase_645_ptr->addr_,(*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[id]->mtrAddr,p_CtrInfoList);
                sendMsg(dvcType,id,tmpSendMsg);
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther645Msg(MsgBase_645_ptr->ctrlCode_,MsgBase_645_ptr->dataLen_,di,MsgBase_645_ptr->addr_,(*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[id]->mtrAddr,p_CtrInfoList);
                sendMsg(dvcType,id,tmpSendMsg);
            }
        }
    }
}

void Script_Upgrade_STA_normal_2_MultiTask::processMsgFromMeter698(DvcType dvcType,int id, int mtrlID)
{
    bool haveCompleteMsg=true;
    while(haveCompleteMsg)
    {
        qInfo()<<QString("id=%1; 解析前 buf698=%2").arg(id).arg(QString((*p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList)[mtrlID]->buf698.toHex()));
        shared_ptr<FrameOOPBase> MsgBase_698_ptr=p_MsgBase_698_45->DecodeMsg(&((*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[mtrlID]->buf698),haveCompleteMsg);
        qInfo()<<QString("id=%1; 解析后 buf698=%2").arg(id).arg(QString((*p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList)[mtrlID]->buf698.toHex()));
        if(MsgBase_698_ptr==nullptr)
        {
            break;
        }
        if(MsgBase_698_ptr->service_type_==GET_REQUEST_CLIENT && MsgBase_698_ptr->service_sub_type_==uchar(GetRequestType::kGetRequestNormal))
        {
            OAD oad;
            oad.OI=ComuAddr;
            oad.attribute.feature=0;
            oad.attribute.seq=2;
            oad.element_index=0;
            shared_ptr<GetRequestNormal> p_IC_GetRequestNormal = std::dynamic_pointer_cast<GetRequestNormal>(MsgBase_698_ptr);

            if(p_IC_GetRequestNormal->oad_==oad)
            {
                sendMsg(dvcType,id,mtrlID,p_GetResponseNormal_ReadAddr);
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther698Msg(MsgBase_698_ptr,(*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[id]->mtrAddr,p_CtrInfoList);
                sendMsg(dvcType,id,tmpSendMsg);
            }
        }
        else
        {
            QByteArray tmpSendMsg=prcsOther698Msg(MsgBase_698_ptr,(*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[id]->mtrAddr,p_CtrInfoList);
            sendMsg(dvcType,id,tmpSendMsg);
        }
    }
}

void Script_Upgrade_STA_normal_2_MultiTask::sendMsg(DvcType dvcType,int id,int meterID, shared_ptr<void> frame)
{
    sendMsgOct.clear();
    if(frame==p_CcoRunStateInfo_10F4_Down)
    {
        p_CcoRunStateInfo_10F4_Down->ctrl_field_={kHplc,kActive,kDirDown};
        p_CcoRunStateInfo_10F4_Down->info_field_.info_field_down.comu_module_ident=0;
        p_CcoRunStateInfo_10F4_Down->info_field_.info_field_down.msg_seq=char(msgSeq++);
        sendMsgOct=p_CcoRunStateInfo_10F4_Down->EncodeFrame();
        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("》》查询路由工作状态10F4：%1\n").arg(QString(sendMsgOct.toHex())));
    }
    else if(frame==p_CcoRunModeInfo_03F10_Down)
    {
        p_CcoRunModeInfo_03F10_Down->ctrl_field_={kHplc,kActive,kDirDown};
        p_CcoRunModeInfo_03F10_Down->info_field_.info_field_down.comu_module_ident=0;
        p_CcoRunModeInfo_03F10_Down->info_field_.info_field_down.msg_seq=char(msgSeq++);
        sendMsgOct=p_CcoRunModeInfo_03F10_Down->EncodeFrame();
        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("》》查询路由运行模式03F10：%1\n").arg(QString(sendMsgOct.toHex())));
    }
    else if(frame==p_CcoCtrlPause_12F2)
    {
        p_CcoCtrlPause_12F2->ctrl_field_={kHplc,kActive,kDirDown};
        p_CcoCtrlPause_12F2->info_field_.info_field_down.comu_module_ident=0;
        p_CcoCtrlPause_12F2->info_field_.info_field_down.msg_seq=char(msgSeq++);
        sendMsgOct=p_CcoCtrlPause_12F2->EncodeFrame();
        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("》》路由暂停12F2：%1\n").arg(QString(sendMsgOct.toHex())));
    }
    else if(frame==p_StopSlaveNodeReg_11F6)
    {
        p_StopSlaveNodeReg_11F6->ctrl_field_={kHplc,kActive,kDirDown};
        p_StopSlaveNodeReg_11F6->info_field_.info_field_down.comu_module_ident=0;
        p_StopSlaveNodeReg_11F6->info_field_.info_field_down.msg_seq=char(msgSeq++);
        sendMsgOct=p_StopSlaveNodeReg_11F6->EncodeFrame();
        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("》》终止从节点主动注册11F6：%1\n").arg(QString(sendMsgOct.toHex())));
    }
    else if(frame==p_FileTransfer_15F1_Down)
    {
        if(emScriptRunState==Wait_FileTransferFinish)
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
            p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("》》清除下装文件15F1：%1\n").arg(QString(sendMsgOct.toHex())));
        }
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
        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("》》查询STA版本信息02F1：%1\n").arg(QString(sendMsgOct.toHex())));
    }
    else if(frame==p_ChkStaOutVrsnInfo_10F104_Down)
    {
        // TODO: Consider implementing batch querying to improve performance
        // Current implementation queries one device at a time, which may be slow for 256 devices
        // Batch querying could query multiple devices (e.g., 5-10) at once to reduce total time
        
        p_ChkStaOutVrsnInfo_10F104_Down->start_no_=meterIndex_10F104;
        p_ChkStaOutVrsnInfo_10F104_Down->this_query_num_=1;

        p_ChkStaOutVrsnInfo_10F104_Down->ctrl_field_={kHplc,kActive,kDirDown};
        p_ChkStaOutVrsnInfo_10F104_Down->info_field_.info_field_down.comu_module_ident=0;
        p_ChkStaOutVrsnInfo_10F104_Down->info_field_.info_field_down.msg_seq=char(msgSeq++);

        sendMsgOct=p_ChkStaOutVrsnInfo_10F104_Down->EncodeFrame();
        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("》》查询STA外部版本信息10F104：%1\n").arg(QString(sendMsgOct.toHex())));
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
        p_ActiveRegister_11F5->last_time_=activeTime;
        p_ActiveRegister_11F5->retransmit_times_=0;
        p_ActiveRegister_11F5->wait_time_slice_num_=0;

        sendMsgOct=p_ActiveRegister_11F5->EncodeFrame();
        sendMsgLog=QString("》》激活主动注册11F5：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, sendMsgLog);
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
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, sendMsgLog);
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
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, sendMsgLog);
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
        }
        else
            return;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, sendMsgLog);
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
            sendMsgLog=QString("》》监控从节点13F1,抄读645电表%0：%1\n").arg(QString((QByteArray(reinterpret_cast<char*>(tmpAddr),6).toHex()))).arg(QString(sendMsgOct.toHex()));
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
            sendMsgLog=QString("》》监控从节点13F1,抄读OOP电表%0：%1\n").arg(QString((QByteArray(reinterpret_cast<char*>(tmpAddr),6).toHex()))).arg(QString(sendMsgOct.toHex()));
        }
        else
            return;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, sendMsgLog);
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
            sendMsgLog=QString("》》并发抄表F1F1,抄读645电表%0：%1\n").arg(QString((QByteArray(reinterpret_cast<char*>(tmpAddr),6).toHex()))).arg(QString(sendMsgOct.toHex()));
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
            sendMsgLog=QString("》》并发抄表F1F1,抄读OOP电表%0：%1\n").arg(QString((QByteArray(reinterpret_cast<char*>(tmpAddr),6).toHex()))).arg(QString(sendMsgOct.toHex()));
        }
        else
            return;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, sendMsgLog);
    }
    else if(frame==p_MeterAddrResp_93)
    {
        memcpy(p_MeterAddrResp_93->addr,(*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[meterID]->mtrAddr,6);
        memcpy(p_MeterAddrResp_93->addr_,(*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[meterID]->mtrAddr,6);
        sendMsgOct=p_MeterAddrResp_93->EncodeFrame();
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
    }
    else
    {
        return;
    }

    uchar *sendMsg=new uchar[sendMsgOct.size()];
    memcpy(sendMsg,(uchar*)sendMsgOct.data(),sendMsgOct.size());
    p_AbstractScriptHost->sendMsg2Dvc(dvcType,id,sendMsg,sendMsgOct.size());
}

void Script_Upgrade_STA_normal_2_MultiTask::sendMsg(DvcType dvcType, int id, QByteArray msg)
{
    uchar *sendMsg=new uchar[msg.size()];
    memcpy(sendMsg,(uchar*)msg.data(),msg.size());
    p_AbstractScriptHost->sendMsg2Dvc(dvcType,id,sendMsg,msg.size());
}

void Script_Upgrade_STA_normal_2_MultiTask::LoadUpdateFile()
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
}

void Script_Upgrade_STA_normal_2_MultiTask::readInfoInit()
{
    readInfoList.clear();
    for(int i=0;i<p_CtrInfoList->size();i++)
    {
        QList<MeterInfoForSingleNet*> meterInfoList=p_CtrInfoList->at(i)->p_MeterInfoForSingleNetList->values();
        for(int j=0;j<meterInfoList.size();j++)
        {
            ReadInfo readInfo_ST;
            memcpy(readInfo_ST.meterNo.addr,meterInfoList.at(j)->mtrAddr,6);
            readInfo_ST.protocolType=meterInfoList.at(j)->prtcl;
            readInfo_ST.requestFlag_14F1=false;
          //  readInfo_ST.readFlag_13F1=WaitRead;
            readInfo_ST.readFlag_14F1=WaitRead;
          //  readInfo_ST.readFlag_F1F1=WaitRead;
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

    ushort meterCount = ushort(readInfoList.size());
    if(readNum_13F1 > meterCount)
        readNum_13F1 = meterCount;
    if(readNum_F1F1 > meterCount)
        readNum_F1F1 = meterCount;
    if(requestMeterNum_14F1 > meterCount)
        requestMeterNum_14F1 = meterCount;
}

int Script_Upgrade_STA_normal_2_MultiTask::getReadInfo(Address address)
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

ushort Script_Upgrade_STA_normal_2_MultiTask::getCurrentRequestMeterNum_14F1()
{
    ushort currentRequestMeterNum_14F1=0;
    for(int i=0;i<readInfoList.size();i++)
    {
        if(readInfoList[i].requestFlag_14F1==true)
        {
            currentRequestMeterNum_14F1++;
        }
    }
    return currentRequestMeterNum_14F1;
}

void Script_Upgrade_STA_normal_2_MultiTask::RetransmitFailedSegments()
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

Address Script_Upgrade_STA_normal_2_MultiTask::extractAddressFromAfn06F2(shared_ptr<Afn06F2> p_ReportReadData_Up)
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

bool Script_Upgrade_STA_normal_2_MultiTask::extractAndProcess3762Frame(QByteArray& buf, QByteArray& completeFrame)
{
    completeFrame.clear();

    p_AbstractScriptHost->updateProgress(ProcessState_Processing,
        QString("当前缓冲区数据: %1, 长度: %2").arg(QString(buf.toHex())).arg(buf.size()));

    // 首先跳过所有 0xFE 前导符（645帧的前导符）
    while (buf.size() > 0 && (unsigned char)buf[0] == 0xFE) {
        buf.remove(0, 1);
    }

    // 3762帧至少需要: 68 L1 L2 C A1 A2 ... CS 16
    if (buf.size() < 6) {
        return false;
    }

    // 搜索有效的3762帧起始位置
    int frameStart = -1;
    for (int i = 0; i < buf.size() - 5; i++) {
        if ((unsigned char)buf[i] == 0x68) {
            // 检查这个0x68是否是有效的3762帧起始
            uint16_t potentialLength = ((unsigned char)buf[i+2] << 8) | (unsigned char)buf[i+1];
            
            // 3762帧长度通常不会超过500字节，如果超过则很可能是645帧的地址字节
            if (potentialLength > 3 && potentialLength < 500) {
                // 进一步验证：如果有足够数据，检查帧尾是否为0x16
                if (buf.size() >= i + potentialLength) {
                    if ((unsigned char)buf[i + potentialLength - 1] == 0x16) {
                        frameStart = i;
                        break;
                    }
                } else {
                    // 数据不够，假设可能是有效帧，等待更多数据
                    frameStart = i;
                    break;
                }
            }
            // 如果长度不合理，继续搜索下一个0x68
        }
    }

    if (frameStart == -1) {
        // 没找到有效的3762帧起始，保留最后几个字节
        if (buf.size() > 5) {
            buf.remove(0, buf.size() - 5);
        }
        return false;
    }

    // 移除帧起始之前的无效数据
    if (frameStart > 0) {
        buf.remove(0, frameStart);
    }

    if (buf.size() < 6) {
        return false;
    }

    // 读取帧长度（小端序）
    uint16_t frameLength = ((unsigned char)buf[2] << 8) | (unsigned char)buf[1];

    // 再次验证长度合理性
    if (frameLength < 4 || frameLength > 500) {
        buf.remove(0, 1);
        return extractAndProcess3762Frame(buf, completeFrame);
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
            QString("帧结束标志错误，跳过此0x68"));
        buf.remove(0, 1);
        return extractAndProcess3762Frame(buf, completeFrame);
    }

    // 提取完整帧
    completeFrame = buf.left(frameLength);

    // 从缓冲区移除已处理的数据
    buf.remove(0, frameLength);

    p_AbstractScriptHost->updateProgress(ProcessState_Processing,
        QString("提取到完整3762帧: %1").arg(QString(completeFrame.toHex())));


    return true;
}
