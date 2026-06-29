#include "Script_ReadMeter_14F1_13F1_F1F1.h"

Script_ReadMeter_14F1_13F1_F1F1::Script_ReadMeter_14F1_13F1_F1F1(QObject *parent) : QObject(parent)
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
    p_13F1Timer=new QTimer();
    p_F1F1Timer=new QTimer();
    p_14F1Timer=new QTimer();

    connect(p_timer,SIGNAL(timeout()),this,SLOT(timer_timeoutProc()));
    connect(p_maxAllowTimer,SIGNAL(timeout()),this,SLOT(maxAllowTimer_timeoutProc()));
    connect(p_13F1Timer,SIGNAL(timeout()),this,SLOT(timeoutProc_13F1()));
    connect(p_F1F1Timer,SIGNAL(timeout()),this,SLOT(timeoutProc_F1F1()));
    connect(p_14F1Timer,SIGNAL(timeout()),this,SLOT(timeoutProc_14F1()));
}
Script_ReadMeter_14F1_13F1_F1F1::~Script_ReadMeter_14F1_13F1_F1F1()
{
    cleanupMeterMutex();
    p_BuildNetwork_GW->initBuildNetWork();
    if(needPowerOff==true)
        powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost);
    p_BuildNetwork_GW->initBuildNetWork();
    delete p_BuildNetwork_GW;
    p_BuildNetwork_GW=nullptr;

    m_msgSeqAtomic = 0;  // 初始化原子变量
    if(p_timer) {
        p_timer->stop();
        delete p_timer;
        p_timer=nullptr;
    }

    if(p_maxAllowTimer) {
        p_maxAllowTimer->stop();
        delete p_maxAllowTimer;
        p_maxAllowTimer=nullptr;
    }

    if(p_13F1Timer) {
        p_13F1Timer->stop();
        delete p_13F1Timer;
        p_13F1Timer=nullptr;
    }

    if(p_F1F1Timer) {
        p_F1F1Timer->stop();
        delete p_F1F1Timer;
        p_F1F1Timer=nullptr;
    }

    if(p_14F1Timer) {
        p_14F1Timer->stop();
        delete p_14F1Timer;
        p_14F1Timer=nullptr;
    }
}

void Script_ReadMeter_14F1_13F1_F1F1::execute()
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
void Script_ReadMeter_14F1_13F1_F1F1::stop()
{
    if(p_AbstractScriptHost==nullptr)
        return;
    p_AbstractScriptHost->updateProgress(ProcessState_Error, "Test stop!");
    if(needPowerOff==true)
        powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost);
}
void Script_ReadMeter_14F1_13F1_F1F1::addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq)
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
void Script_ReadMeter_14F1_13F1_F1F1::setHost(AbstractScriptHost *host)
{
    p_AbstractScriptHost=host;
    p_BuildNetwork_GW->setHost(host);
}
bool Script_ReadMeter_14F1_13F1_F1F1::config(const QMap<QString,QString> *paraDic)
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
void Script_ReadMeter_14F1_13F1_F1F1::processMsg(DvcType dvcType, int id, uchar *data, int datalen)
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
                emScriptRunState=Wait_RouterRestart_Finish;
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
                QMutexLocker locker(&m_ccoProcessMutex);

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
                    int mtrlID = p_CtrInfoList->at(0)->keyList.at(i);

                    // 在append之前就加锁
                    if(!meterMutexMap.contains(mtrlID))
                    {
                        initMeterMutex(mtrlID);
                    }

                    // 使用阻塞锁，因为这里必须等待
                    QMutexLocker locker(meterMutexMap[mtrlID]);

                    if((*p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList)[p_CtrInfoList->at(0)->keyList.at(i)]->prtcl==0x02)
                    {
                        (*p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList)[p_CtrInfoList->at(0)->keyList.at(i)]->buf645.append(recvTempData);
                        processMsgFromMeter645_NoLock(dvcType, id, mtrlID);
                        break;
                    }
                    else if((*p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList)[p_CtrInfoList->at(0)->keyList.at(i)]->prtcl==0x03)
                    {
                        (*p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList)[p_CtrInfoList->at(0)->keyList.at(i)]->buf698.append(recvTempData);
                        processMsgFromMeterOOP_NoLock(dvcType, id, mtrlID);
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
void Script_ReadMeter_14F1_13F1_F1F1::processCtrlDvcRes(DvcType dvcType, QList<int> idList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params)
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

void Script_ReadMeter_14F1_13F1_F1F1::resultCheck()
{
    if(endFlag_13F1&&endFlag_14F1&&endFlag_F1F1)
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
        if(suc_13F1==meterSum && suc_14F1==meterSum && suc_F1F1==meterSum)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Success, QString("13F1、14F1、F1F1全部抄读成功！"));
            emScriptRunState=ScriptSuccess;
        }
        else
        {
            QString result = QString("13F1成功数%1  失败数%2  未抄数%3  失败表%4  未抄表%5\n14F1成功数%6  失败数%7  未抄数%8  失败表%9  未抄表%10\nF1F1成功数%11  失败数%12  未抄数%13  失败表%14  未抄表%15\n")
                    .arg(suc_13F1).arg(fail_13F1).arg(not_13F1).arg(failAddr_13F1).arg(notReadAddr_13F1).arg(suc_14F1).arg(fail_14F1).arg(not_14F1).arg(failAddr_14F1).arg(notReadAddr_14F1).arg(suc_F1F1).arg(fail_F1F1).arg(not_F1F1).arg(failAddr_F1F1).arg(notReadAddr_F1F1);
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("13F1、14F1、F1F1未全部抄读成功，统计（失败数包含回复数据空和超时未回复）：\n")+result);
        }
    }
}

void Script_ReadMeter_14F1_13F1_F1F1::processMsgFromCCO(DvcType dvcType, int dvcId)
{
    if(p_AbstractScriptHost == nullptr || p_CtrInfoList == nullptr || p_CtrInfoList->size() == 0
            || p_CtrInfoList->at(0) == nullptr || p_Frame3762Helper == nullptr)
    {
        if(p_AbstractScriptHost != nullptr)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Error, "MultiTask 14F1/13F1/F1F1 invalid CCO context");
        }
        return;
    }
    if(dvcId!=p_CtrInfoList->at(0)->ctrlID)
        return;
    QByteArray completeFrame;
    QByteArray& buf = p_CtrInfoList->at(0)->buf;
    bool haveCompleteMsg=extractAndProcess3762Frame(buf,completeFrame);
    while(haveCompleteMsg)
    {
        qInfo()<<QString("CCO-解析前 buf3762=%1").arg(QString((completeFrame.toHex())));
        shared_ptr<Frame3762Base> p_Frame3762Base = nullptr;
        try
        {
            p_Frame3762Base = p_Frame3762Helper->DecodeLocalMsg(&(completeFrame),haveCompleteMsg);
        }
        catch(...)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Error,
                QString("MultiTask 3762 decode exception, skip frame: %1").arg(QString(completeFrame.toHex())));
            haveCompleteMsg=extractAndProcess3762Frame(buf,completeFrame);
            continue;
        }
        qInfo()<<QString("CCO-解析后 buf3762=%1").arg(QString((completeFrame.toHex())));
        if(p_Frame3762Base==nullptr)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Error,
                QString("MultiTask 3762 decode failed, skip frame: %1").arg(QString(completeFrame.toHex())));
            haveCompleteMsg=extractAndProcess3762Frame(buf,completeFrame);
            continue;
        }
        m_msgSeqAtomic = uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq);
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
        case Wait_RouterRestart_Finish:
        {
            if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到确认，等待14F1请求");
                emScriptRunState=Wait_ReadMeter_Finish;
                p_timer->start(10*1000);
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
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("路由14F1请求表不全，%0个表未请求").arg(noReq_14F1));
                    }

                    endFlag_14F1=true;
                    resultCheck();
                }
                else if(p_Frame3762Base->afn_==0x14&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    p_14F1Timer->stop();
                    shared_ptr<Afn14F1> p_RouterRequestRead_14F1_Up=dynamic_pointer_cast<Afn14F1>(p_Frame3762Base);
                    if(p_RouterRequestRead_14F1_Up==nullptr)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Error, "MultiTask 14F1 cast failed");
                        break;
                    }
                    int currentMeterIndex=getReadInfo(p_RouterRequestRead_14F1_Up->node_address_);
                    if(currentMeterIndex<0 || currentMeterIndex>=readInfoList.size())
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Error,
                            QString("MultiTask 14F1 meter not in archive: %1").arg(QString(QByteArray(p_RouterRequestRead_14F1_Up->node_address_.addr,6).toHex())));
                        break;
                    }
                    readInfoList[currentMeterIndex].requestFlag_14F1=true;
                    requestNum_14F1++;
                    if(requestNum_14F1==1)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("路由重启后收到第%1个14F1请求，回复抄读数据项，表号%2，同时开始下发13F1、F1F1").arg(requestNum_14F1).arg(QString(QByteArray(p_RouterRequestRead_14F1_Up->node_address_.addr,6).toHex())));
                        sendMsg(dvcType,dvcId,currentMeterIndex,p_RouterRequestRead_14F1);
                        p_14F1Timer->start(65*1000);

                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("发送第%1条13F1抄表").arg(++requestNum_13F1));
                        sendMsg(dvcType,dvcId,readNo_13F1,p_MonitorSlaveNode_13F1);
                        p_13F1Timer->start(100*1000);

                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("发送第%1条F1F1抄表").arg(++requestNum_F1F1));
                        sendMsg(dvcType,dvcId,readNo_F1F1,p_ParallelReadMeter_F1F1);
                        p_F1F1Timer->start(100*1000);
                    }
                    else if(requestNum_14F1>readInfoList.size()*2)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到第%0个14F1请求，超过表数的2倍，14F1一轮抄表结束，06F2上报抄读成功报文数%1，上报数据为空报文数%2").arg(requestNum_14F1).arg(reportSucNum_14F1).arg(reportFailNum_14F1));
                        sendMsg(p_CtrInfoList->at(0)->slotPosition,p_CtrInfoList->at(0)->dvcId,INSIGNIFICANCE,p_RouterPause_12F2);
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--路由暂停（12F2），等待--确认");
                        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到第%0个14F1请求，回复抄读数据项，表号").arg(requestNum_14F1)+QByteArray(p_RouterRequestRead_14F1_Up->node_address_.addr,6).toHex());
                        sendMsg(dvcType,dvcId,currentMeterIndex,p_RouterRequestRead_14F1);
                        p_14F1Timer->start(65*1000);
                    }
                }
                else if(p_Frame3762Base->afn_==0x06&&p_Frame3762Base->dt1_==0x02&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    shared_ptr<Afn06F2> p_ReportReadData_Up=dynamic_pointer_cast<Afn06F2>(p_Frame3762Base);
                    if(p_ReportReadData_Up==nullptr)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Error, "MultiTask 06F2 cast failed");
                        break;
                    }
                    Address srcAddr = extractAddressFromAfn06F2(p_ReportReadData_Up);
                    int currentMeterIndex=getReadInfo(srcAddr);

                    if(currentMeterIndex<0)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "不在档案中");
                        break;
                    }

                    if(p_ReportReadData_Up->report_data_unit_.frame_length_>0)
                    {
                        reportSucNum_14F1++;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到第%0条06F2成功上报抄读数据，表号").arg(reportSucNum_14F1)+QByteArray(srcAddr.addr,6).toHex());
                        readInfoList[currentMeterIndex].readFlag_14F1=ReadSuccess;
                    }
                    else
                    {
                        reportFailNum_14F1++;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到第%0条06F2上报抄读数据内容为空，表号").arg(reportFailNum_14F1)+QByteArray(srcAddr.addr,6).toHex());
                        readInfoList[currentMeterIndex].readFlag_14F1=ReadFail;
                    }
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "回复确认");
                    sendMsg(dvcType,dvcId,INSIGNIFICANCE,p_Confirm_00F1);
                }
                else if(p_Frame3762Base->afn_==0x13&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_13F1Timer->stop();
                    shared_ptr<Afn13F1> p_MonitorSlaveNode_13F1_Up=dynamic_pointer_cast<Afn13F1>(p_Frame3762Base);
                    if(p_MonitorSlaveNode_13F1_Up==nullptr)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Error, "MultiTask 13F1 cast failed");
                        break;
                    }
                    Address srcAddr;
                    memcpy(srcAddr.addr,p_MonitorSlaveNode_13F1_Up->address_field_.src_addr,6);
                    int currentMeterIndex=getReadInfo(srcAddr);
                    if(currentMeterIndex<0 || currentMeterIndex>=readInfoList.size())
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Error,
                            QString("MultiTask 13F1 meter not in archive: %1").arg(QString(QByteArray(srcAddr.addr,6).toHex())));
                        break;
                    }
                    if(p_MonitorSlaveNode_13F1_Up->data_field_up_.frame_length_>0)
                    {
                        responseSucNum_13F1++;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到第%0条13F1抄读成功回复数据，表号").arg(responseSucNum_13F1)+QByteArray(srcAddr.addr,6).toHex());
                        readInfoList[currentMeterIndex].readFlag_13F1=ReadSuccess;
                    }
                    else
                    {
                        responseFailNum_13F1++;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到第%0条13F1抄读回复数据内容为空，表号").arg(responseFailNum_13F1)+QByteArray(srcAddr.addr,6).toHex());
                        readInfoList[currentMeterIndex].readFlag_13F1=ReadFail;
                    }
                    if(requestNum_13F1<readInfoList.size())
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("发送第%1条13F1继续抄下一只表").arg(++requestNum_13F1));
                        sendMsg(dvcType,dvcId,++readNo_13F1,p_MonitorSlaveNode_13F1);
                        p_13F1Timer->start(100*1000);
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
                    p_F1F1Timer->stop();
                    shared_ptr<AfnF1F1> p_ParallelReadMeter_F1F1_Up=dynamic_pointer_cast<AfnF1F1>(p_Frame3762Base);
                    if(p_ParallelReadMeter_F1F1_Up==nullptr)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Error, "MultiTask F1F1 cast failed");
                        break;
                    }
                    Address srcAddr;
                    memcpy(srcAddr.addr,p_ParallelReadMeter_F1F1_Up->address_field_.src_addr,6);
                    int currentMeterIndex=getReadInfo(srcAddr);
                    if(currentMeterIndex<0 || currentMeterIndex>=readInfoList.size())
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Error,
                            QString("MultiTask F1F1 meter not in archive: %1").arg(QString(QByteArray(srcAddr.addr,6).toHex())));
                        break;
                    }
                    if(p_ParallelReadMeter_F1F1_Up->unit_up_.frame_length_>0)
                    {
                        responseSucNum_F1F1++;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到第%0条F1F1抄读成功回复数据，表号").arg(responseSucNum_F1F1)+QByteArray(srcAddr.addr,6).toHex());
                        readInfoList[currentMeterIndex].readFlag_F1F1=ReadSuccess;
                    }
                    else
                    {
                        responseFailNum_F1F1++;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到第%0条F1F1抄读回复数据内容为空，表号").arg(responseFailNum_F1F1)+QByteArray(srcAddr.addr,6).toHex());
                        readInfoList[currentMeterIndex].readFlag_F1F1=ReadFail;
                    }
                    if(requestNum_F1F1<readInfoList.size())
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("发送第%1条F1F1继续抄下一只表").arg(++requestNum_F1F1));
                        sendMsg(dvcType,dvcId,++readNo_F1F1,p_ParallelReadMeter_F1F1);
                        p_F1F1Timer->start(100*1000);
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("F1F1一轮抄表结束，发送报文数%1，抄读成功报文数%2，回复数据为空报文数%3，回复超时数%4").arg(requestNum_F1F1).arg(responseSucNum_F1F1).arg(responseFailNum_F1F1).arg(timeoutNum_F1F1));
                        endFlag_F1F1=true;
                        resultCheck();
                    }
                }
                else if(p_Frame3762Base->afn_==0x06&&p_Frame3762Base->dt1_==0x04&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    shared_ptr<Afn06F3> p_ReportMissionState_Up=dynamic_pointer_cast<Afn06F3>(p_Frame3762Base);
                    if(p_ReportMissionState_Up==nullptr)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Error, "MultiTask 06F3 cast failed");
                        break;
                    }
                    if(p_ReportMissionState_Up->router_work_task_change_==0x01)
                    {
                        p_14F1Timer->stop();
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到06F3上报路由工况变动：抄表任务结束，回复确认"));
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("共收到%0个14F1请求，06F2上报抄读成功报文数%1，上报数据为空报文数%2").arg(requestNum_14F1).arg(reportSucNum_14F1).arg(reportFailNum_14F1));
                        endFlag_14F1=true;
                        resultCheck();
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "收到06F3上报路由工况变动，非抄表任务结束："+QString::number(p_ReportMissionState_Up->router_work_task_change_));
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
            default:
            {
                break;
            }
//            default:
//            {
//                p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==emScriptRunState");
//                break;
//            }
        }
        haveCompleteMsg=extractAndProcess3762Frame(buf,completeFrame);
    }
}
void Script_ReadMeter_14F1_13F1_F1F1::processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID)
{
    if(!meterMutexMap.contains(mtrlID))
    {
        initMeterMutex(mtrlID);
    }

    // 尝试获取锁（非阻塞）增加重试机制
    int retryCount = 0;
    const int MAX_RETRY = 3;

    while(retryCount < MAX_RETRY)
    {
        if(meterMutexMap[mtrlID]->tryLock())
        {
            // 成功获取锁
            break;
        }

        // 未能获取锁，短暂等待后重试
        retryCount++;
        if(retryCount < MAX_RETRY)
        {
            QThread::msleep(10);  // 等待10ms后重试
        }
    }

    if(retryCount >= MAX_RETRY)
    {
        // 重试3次后仍无法获取锁，跳过本次处理
        uchar* mtrAddr = (*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[mtrlID]->mtrAddr;
        QString meterAddr = QString(QByteArray(reinterpret_cast<char*>(mtrAddr), 6).toHex());

        p_AbstractScriptHost->updateProgress(ProcessState_Processing,
                                             QString("表 %1 (ID:%2) 正在处理消息中，重试%3次后仍无法获取锁，跳过本次处理")
                                             .arg(meterAddr).arg(mtrlID).arg(MAX_RETRY));

        return;
    }
    // 成功获取锁，设置处理标志
    meterProcessingMap[mtrlID] = true;
    try {
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
                    waitForResponseInterval(mtrlID);// 处理前等待响应间隔
                    if(MsgBase_645_ptr->ctrlCode_==READ_ADDR)
                    {
                        sendMsg(dvcType,dvcId,mtrlID,p_MeterAddrResp_93);
                        // 更新最后响应时间
                        lastResponseTimeMap[mtrlID] = QDateTime::currentMSecsSinceEpoch();
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
                        // 更新最后响应时间
                        lastResponseTimeMap[mtrlID] = QDateTime::currentMSecsSinceEpoch();
                    }
                    break;
                }
            }
        }
    }
    catch (const std::exception& e) {
           p_AbstractScriptHost->updateProgress(ProcessState_Error,
               QString("处理645消息异常: %1").arg(e.what()));
       }
       catch (...) {
           p_AbstractScriptHost->updateProgress(ProcessState_Error,
               "处理645消息未知异常");
       }
    // 清除处理标志并释放锁
    meterProcessingMap[mtrlID] = false;
    meterMutexMap[mtrlID]->unlock();
}
void Script_ReadMeter_14F1_13F1_F1F1::processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID)
{
    if(!meterMutexMap.contains(mtrlID))
    {
        initMeterMutex(mtrlID);
    }

    // 尝试获取锁（非阻塞）增加重试机制
    int retryCount = 0;
    const int MAX_RETRY = 3;

    while(retryCount < MAX_RETRY)
    {
        if(meterMutexMap[mtrlID]->tryLock())
        {
            // 成功获取锁
            break;
        }

        // 未能获取锁，短暂等待后重试
        retryCount++;
        if(retryCount < MAX_RETRY)
        {
            QThread::msleep(10);  // 等待10ms后重试
        }
    }

    if(retryCount >= MAX_RETRY)
    {
        // 重试3次后仍无法获取锁，跳过本次处理
        uchar* mtrAddr = (*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[mtrlID]->mtrAddr;
        QString meterAddr = QString(QByteArray(reinterpret_cast<char*>(mtrAddr), 6).toHex());

        p_AbstractScriptHost->updateProgress(ProcessState_Processing,
                                             QString("表 %1 (ID:%2) 正在处理消息中，重试%3次后仍无法获取锁，跳过本次处理")
                                             .arg(meterAddr).arg(mtrlID).arg(MAX_RETRY));

        return;
    }
    meterProcessingMap[mtrlID] = true;

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
                waitForResponseInterval(mtrlID);

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
                lastResponseTimeMap[mtrlID] = QDateTime::currentMSecsSinceEpoch();
                break;
            }
        }
    }
    meterProcessingMap[mtrlID] = false;
    meterMutexMap[mtrlID]->unlock();
}

void Script_ReadMeter_14F1_13F1_F1F1::sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame)
{
    // 使用原子msgSeq
    int currentMsgSeq = m_msgSeqAtomic.load();
    if(frame==p_Confirm_00F1)
    {
        p_Confirm_00F1->ctrl_field_.dir=kDirDown;
        p_Confirm_00F1->ctrl_field_.prm=kPassive;
        p_Confirm_00F1->ctrl_field_.comn_type=kHplc;

        p_Confirm_00F1->info_field_.info_field_down.msg_seq=char(currentMsgSeq);
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
        int newSeq = m_msgSeqAtomic.fetchAndAddAcquire(1);
        p_RouterRestart_12F1->ctrl_field_.dir=kDirDown;
        p_RouterRestart_12F1->ctrl_field_.prm=kActive;
        p_RouterRestart_12F1->ctrl_field_.comn_type=kHplc;

        p_RouterRestart_12F1->info_field_.info_field_down.msg_seq=char(newSeq);
        p_RouterRestart_12F1->info_field_.info_field_down.comu_rate=0;
        p_RouterRestart_12F1->info_field_.info_field_down.comu_module_ident=0;

        sendMsgOct=p_RouterRestart_12F1->EncodeFrame();
        sendMsgLog=QString("》》路由重启12F1：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_RouterPause_12F2)
    {
        int newSeq = m_msgSeqAtomic.fetchAndAddAcquire(1);
        p_RouterPause_12F2->ctrl_field_.dir=kDirDown;
        p_RouterPause_12F2->ctrl_field_.prm=kActive;
        p_RouterPause_12F2->ctrl_field_.comn_type=kHplc;

        p_RouterPause_12F2->info_field_.info_field_down.msg_seq=char(newSeq);
        p_RouterPause_12F2->info_field_.info_field_down.comu_rate=0;
        p_RouterPause_12F2->info_field_.info_field_down.comu_module_ident=0;

        sendMsgOct=p_RouterPause_12F2->EncodeFrame();
        sendMsgLog=QString("》》路由暂停12F2：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_RouterRecover_12F3)
    {
        int newSeq = m_msgSeqAtomic.fetchAndAddAcquire(1);
        p_RouterRecover_12F3->ctrl_field_.dir=kDirDown;
        p_RouterRecover_12F3->ctrl_field_.prm=kActive;
        p_RouterRecover_12F3->ctrl_field_.comn_type=kHplc;

        p_RouterRecover_12F3->info_field_.info_field_down.msg_seq=char(newSeq);
        p_RouterRecover_12F3->info_field_.info_field_down.comu_rate=0;
        p_RouterRecover_12F3->info_field_.info_field_down.comu_module_ident=0;

        sendMsgOct=p_RouterRecover_12F3->EncodeFrame();
        sendMsgLog=QString("》》路由恢复12F3：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_RouterRequestRead_14F1)
    {
        int newSeq = m_msgSeqAtomic.loadAcquire();
        p_RouterRequestRead_14F1->ctrl_field_.dir=kDirDown;
        p_RouterRequestRead_14F1->ctrl_field_.prm=kPassive;
        p_RouterRequestRead_14F1->ctrl_field_.comn_type=kHplc;

        p_RouterRequestRead_14F1->info_field_.info_field_down.msg_seq=char(newSeq);
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
            int newSeq = m_msgSeqAtomic.fetchAndAddAcquire(1);
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

            p_MonitorSlaveNode_13F1->info_field_.info_field_down.msg_seq=char(newSeq);
            p_MonitorSlaveNode_13F1->info_field_.info_field_down.comu_rate=0;
            p_MonitorSlaveNode_13F1->info_field_.info_field_down.comu_module_ident=1;

            uchar tmpCcoAddr[6];
            memcpy(tmpCcoAddr,p_CtrInfoList->at(0)->ccoAddr,6);
            memcpy(p_MonitorSlaveNode_13F1->address_field_.src_addr,tmpCcoAddr,6);
            memcpy(p_MonitorSlaveNode_13F1->address_field_.dst_addr,tmpAddr,6);

            sendMsgOct=p_MonitorSlaveNode_13F1->EncodeFrame();
            sendMsgLog=QString("》》监控从节点13F1,抄读645电表%0：%1\n").arg(QString((QByteArray(reinterpret_cast<char*>(tmpAddr),6).toHex()))).arg(QString(sendMsgOct.toHex()));

        //    startTime=QDateTime::currentDateTime().toString(CURRENT_TIME_FORMAT);
        }
        else if(comPrtclType==OOP)
        {
            int newSeq = m_msgSeqAtomic.fetchAndAddAcquire(1);
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

            p_MonitorSlaveNode_13F1->info_field_.info_field_down.msg_seq=char(newSeq);
            p_MonitorSlaveNode_13F1->info_field_.info_field_down.comu_rate=0;
            p_MonitorSlaveNode_13F1->info_field_.info_field_down.comu_module_ident=1;

            uchar tmpCcoAddr[6];
            memcpy(tmpCcoAddr,p_CtrInfoList->at(0)->ccoAddr,6);
            memcpy(p_MonitorSlaveNode_13F1->address_field_.src_addr,tmpCcoAddr,6);
            memcpy(p_MonitorSlaveNode_13F1->address_field_.dst_addr,tmpAddr,6);

            sendMsgOct=p_MonitorSlaveNode_13F1->EncodeFrame();
            sendMsgLog=QString("》》监控从节点13F1,抄读OOP电表%0：%1\n").arg(QString((QByteArray(reinterpret_cast<char*>(tmpAddr),6).toHex()))).arg(QString(sendMsgOct.toHex()));

         //   startTime=QDateTime::currentDateTime().toString(CURRENT_TIME_FORMAT);
        }
        else
            return;
    }
    else if(frame==p_ParallelReadMeter_F1F1)
    {
        int newSeq = m_msgSeqAtomic.fetchAndAddAcquire(1);
        p_ParallelReadMeter_F1F1->ctrl_field_.dir=kDirDown;
        p_ParallelReadMeter_F1F1->ctrl_field_.prm=kActive;
        p_ParallelReadMeter_F1F1->ctrl_field_.comn_type=kHplc;

        p_ParallelReadMeter_F1F1->info_field_.info_field_down.msg_seq=char(newSeq);
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
            sendMsgLog=QString("》》并发抄表F1F1,抄读OOP电表%0：%1\n").arg(QString((QByteArray(reinterpret_cast<char*>(tmpAddr),6).toHex()))).arg(QString(sendMsgOct.toHex()));

      //      startTime=QDateTime::currentDateTime().toString(CURRENT_TIME_FORMAT);
        }
        else
            return;
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
void Script_ReadMeter_14F1_13F1_F1F1::sendSrcMsg(DvcType dvcType, int dvcId, QByteArray msg)
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

void Script_ReadMeter_14F1_13F1_F1F1::timeoutProc_13F1()
{
    p_13F1Timer->stop();
    switch(emScriptRunState)
    {
        case Wait_ReadMeter_Finish:
        {
            timeoutNum_13F1++;
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("发送13F1后第%0次100s内未回复").arg(timeoutNum_13F1));
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
void Script_ReadMeter_14F1_13F1_F1F1::maxAllowTimer_timeoutProc()
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
void Script_ReadMeter_14F1_13F1_F1F1::timer_timeoutProc()
{
    p_timer->stop();
    switch(emScriptRunState)
    {
        case Wait_BuildNetFinish_Whole:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait build whole net finish timeout!!!");
            break;
        }
        case Wait_RouterRestart_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "路由重启超时未确认");
            break;
        }
        case Wait_ReadMeter_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "路由重启后10s内未收到14F1请求");
            break;
        }
        default:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==p_timer");
            break;
        }
    }
}
void Script_ReadMeter_14F1_13F1_F1F1::timeoutProc_F1F1()
{
    p_F1F1Timer->stop();
    if(emScriptRunState==Wait_ReadMeter_Finish)
    {
        timeoutNum_F1F1++;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("发送F1F1后第%0次100s内未回复").arg(timeoutNum_F1F1));
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

void Script_ReadMeter_14F1_13F1_F1F1::timeoutProc_14F1()
{
    p_14F1Timer->stop();
    if(emScriptRunState==Wait_ReadMeter_Finish)
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("第%0次14F1请求后65s内未收到下一次请求").arg(requestNum_14F1));
    }
    else
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Error, "14F1Timer状态机错误");
    }
}

void Script_ReadMeter_14F1_13F1_F1F1::readInfoInit()
{
    readInfoList.clear();
    cleanupMeterMutex();//清理旧的互斥锁
    for(int i=0;i<p_CtrInfoList->size();i++)
    {
        //为所有表预先初始化互斥锁
        QList<int> keyList = p_CtrInfoList->at(i)->keyList;
        for(int k=0; k<keyList.size(); k++)
        {
            int mtrlID = keyList.at(k);
            initMeterMutex(mtrlID);
            p_AbstractScriptHost->updateProgress(ProcessState_Processing,
                QString("为表ID %1 初始化互斥锁").arg(mtrlID));
        }

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
bool Script_ReadMeter_14F1_13F1_F1F1::isMeterExist(Address address)
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
int Script_ReadMeter_14F1_13F1_F1F1::getReadInfo(Address address)
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
double Script_ReadMeter_14F1_13F1_F1F1::calSuccessRate()
{
    double successCount=0.0;
    for(int i=0;i<readInfoList.size();i++)
    {
     //   if(readInfoList.at(i).readFlag==ReadSuccess)
            successCount++;
    }
    return successCount/double(readInfoList.size());
}

QString Script_ReadMeter_14F1_13F1_F1F1::calCostTime()
{
    double totalConsume=0.0;
    double successCount=0.0;
    for(int i=0;i<readInfoList.size();i++)
    {
   //     if(readInfoList.at(i).readFlag==ReadSuccess)
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
QString Script_ReadMeter_14F1_13F1_F1F1::getFailMeterNo()
{
    QString failMeterNo;
    int count=0;
    for(int i=0;i<readInfoList.size();i++)
    {
   //     if(readInfoList.at(i).readFlag!=ReadSuccess)
        {
            count++;
            failMeterNo+=QString(QByteArray(readInfoList.at(i).meterNo.addr,6).toHex())+";";
            if(count%8==0)
                failMeterNo+="\n";
        }

    }
    return failMeterNo;
}
void Script_ReadMeter_14F1_13F1_F1F1::CalcAvrgConsumeTimeLen(uchar rdFlag)
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

// 1. initMeterMutex - 初始化单个表的互斥锁
void Script_ReadMeter_14F1_13F1_F1F1::initMeterMutex(int mtrlID)
{
    if(!meterMutexMap.contains(mtrlID))
    {
        meterMutexMap[mtrlID] = new QMutex();
        meterProcessingMap[mtrlID] = false;
        lastResponseTimeMap[mtrlID] = 0;
    }
}

// 2. cleanupMeterMutex - 清理所有互斥锁
void Script_ReadMeter_14F1_13F1_F1F1::cleanupMeterMutex()
{
    // 删除所有互斥锁对象
    for(auto it = meterMutexMap.begin(); it != meterMutexMap.end(); ++it)
    {
        if(it.value() != nullptr)
        {
            delete it.value();
        }
    }

    // 清空所有容器
    meterMutexMap.clear();
    meterProcessingMap.clear();
    lastResponseTimeMap.clear();

    p_AbstractScriptHost->updateProgress(ProcessState_Processing,
        "清理所有表的互斥锁完成");
}

// 3. waitForResponseInterval - 等待响应间隔
void Script_ReadMeter_14F1_13F1_F1F1::waitForResponseInterval(int mtrlID)
{
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    qint64 lastTime = lastResponseTimeMap.value(mtrlID, 0);
    qint64 elapsed = currentTime - lastTime;

    if(elapsed < MIN_RESPONSE_INTERVAL_MS)
    {
        int waitTime = MIN_RESPONSE_INTERVAL_MS - elapsed;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing,
            QString("表 %1 响应间隔保护，等待 %2 ms").arg(mtrlID).arg(waitTime));
        QThread::msleep(waitTime);
    }
}

bool Script_ReadMeter_14F1_13F1_F1F1::extractAndProcess3762Frame(QByteArray& buf,QByteArray& completeFrame)
{
    completeFrame.clear();

    p_AbstractScriptHost->updateProgress(ProcessState_Processing,
        QString("当前缓冲区数据: %1, 长度: %2").arg(QString(buf.toHex())).arg(buf.size()));


    if (buf.size() < 5) { // 至少需要起始字符+长度字段
        return false;
    }

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

    // 移除帧起始之前的不完整数据
    if (frameStart > 0) {
        buf.remove(0, frameStart);
        frameStart = 0;
    }

    if (buf.size() < 3) {
        return false;
    }

    uint16_t frameLength = 0;
    memcpy(&frameLength, buf.constData() + 1, 2);

    // 长度字段包括6个固定字节，所以总帧长度应该是frameLength
    if (buf.size() < frameLength) {
        p_AbstractScriptHost->updateProgress(ProcessState_Processing,
            QString("等待完整帧，需要%1字节，当前%2字节").arg(frameLength).arg(buf.size()));
        return false;
    }

    // 检查帧结束标志0x16
    if ((unsigned char)buf[frameLength - 1] != 0x16) {
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "帧结束标志错误，跳过此帧");
        buf.remove(0, 1); // 移除起始字符，继续查找下一帧
        return true; // 返回true表示有数据处理（虽然是错误情况）
    }

    // 提取完整帧
    completeFrame = buf.left(frameLength);

    // 从缓冲区移除已处理的数据
    buf.remove(0, frameLength);

    p_AbstractScriptHost->updateProgress(ProcessState_Processing,
        QString("提取到完整3762帧: %1").arg(QString(completeFrame.toHex())));



    return true;
}

void Script_ReadMeter_14F1_13F1_F1F1::processMsgFromMeter645_NoLock(DvcType dvcType, int dvcId, int mtrlID)
{
    // 注意：这个函数被调用时，外层 processMsg() 已经持有锁了，所以不需要再加锁

    bool haveCompleteMsg = true;
    while(haveCompleteMsg)
    {
        shared_ptr<dlt_645_Protocol::Frame645Base> MsgBase_645_ptr =
            dlt_645_Protocol::Frame645Helper::DecodeLocalMsg(
                &((*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[mtrlID]->buf645),
                haveCompleteMsg);

        if(MsgBase_645_ptr == nullptr)
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
                // 响应限流保护
                waitForResponseInterval(mtrlID);

                if(MsgBase_645_ptr->ctrlCode_==READ_ADDR)
                {
                    sendMsg(dvcType, dvcId, mtrlID, p_MeterAddrResp_93);

                    // 更新最后响应时间
                    lastResponseTimeMap[mtrlID] = QDateTime::currentMSecsSinceEpoch();
                }
                else
                {
                    uchar di[4]={0x00};
                    if(MsgBase_645_ptr->ctrlCode_==READ_DATA)
                    {
                        shared_ptr<dlt_645_Protocol::Rqst_ReadData_0x11> p_ReadData_0x11 =
                            std::dynamic_pointer_cast<dlt_645_Protocol::Rqst_ReadData_0x11>(MsgBase_645_ptr);
                        memcpy(di, p_ReadData_0x11->di, 4);

                        QByteArray tmpSendMsg = prcsOther645Msg(
                            MsgBase_645_ptr->ctrlCode_,
                            MsgBase_645_ptr->dataLen_,
                            di,
                            MsgBase_645_ptr->addr_,
                            (*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[mtrlID]->mtrAddr,
                            p_CtrInfoList);

                        sendSrcMsg(dvcType, dvcId, tmpSendMsg);
                    }
                    else
                    {
                        QByteArray tmpSendMsg = prcsOther645Msg(
                            MsgBase_645_ptr->ctrlCode_,
                            MsgBase_645_ptr->dataLen_,
                            di,
                            MsgBase_645_ptr->addr_,
                            (*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[mtrlID]->mtrAddr,
                            p_CtrInfoList);

                        sendSrcMsg(dvcType, dvcId, tmpSendMsg);
                    }

                    // 更新最后响应时间
                    lastResponseTimeMap[mtrlID] = QDateTime::currentMSecsSinceEpoch();
                }
                break;
            }
        }
    }
}

// 2. processMsgFromMeterOOP_NoLock 的完整实现
void Script_ReadMeter_14F1_13F1_F1F1::processMsgFromMeterOOP_NoLock(DvcType dvcType, int dvcId, int mtrlID)
{
    // 这个函数被调用时，外层 processMsg() 已经持有锁了，所以不需要再加锁

    bool haveCompleteMsg = true;
    while(haveCompleteMsg)
    {
        shared_ptr<FrameOOPBase> MsgBase_OOP_ptr =
            FrameOOPHelper::DecodeMsg(
                &((*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[mtrlID]->buf698),
                haveCompleteMsg);

        if(MsgBase_OOP_ptr == nullptr)
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
                // 响应限流保护
                waitForResponseInterval(mtrlID);

                if(MsgBase_OOP_ptr->service_type_==GET_REQUEST_CLIENT &&
                   MsgBase_OOP_ptr->service_sub_type_==uchar(GetRequestType::kGetRequestNormal))
                {
                    OAD oad;
                    oad.OI = ComuAddr;
                    oad.attribute.feature = 0;
                    oad.attribute.seq = 2;
                    oad.element_index = 0;

                    shared_ptr<GetRequestNormal> p_GetRequestNormal =
                        dynamic_pointer_cast<GetRequestNormal>(MsgBase_OOP_ptr);

                    if(p_GetRequestNormal->oad_.OI == oad.OI)
                    {
                        sendMsg(dvcType, dvcId, mtrlID, p_GetResponseNormal_ReadAddr);
                    }
                    else
                    {
                        QByteArray tmpSendMsg = prcsOther698Msg(
                            MsgBase_OOP_ptr,
                            (*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[mtrlID]->mtrAddr,
                            p_CtrInfoList);
                        sendSrcMsg(dvcType, dvcId, tmpSendMsg);
                    }
                }
                else
                {
                    QByteArray tmpSendMsg = prcsOther698Msg(
                        MsgBase_OOP_ptr,
                        (*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[mtrlID]->mtrAddr,
                        p_CtrInfoList);
                    sendSrcMsg(dvcType, dvcId, tmpSendMsg);
                }

                // 更新最后响应时间
                lastResponseTimeMap[mtrlID] = QDateTime::currentMSecsSinceEpoch();
                break;
            }
        }
    }
}


Address Script_ReadMeter_14F1_13F1_F1F1::extractAddressFromAfn06F2(shared_ptr<Afn06F2> p_ReportReadData_Up)
{
    Address srcAddr;
    memset(srcAddr.addr, 0, sizeof(srcAddr.addr));

    if (!p_ReportReadData_Up) {
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "错误: Afn06F2对象为空");
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

