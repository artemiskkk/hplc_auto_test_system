#include "Script_VersionHead_ControllerSta.h"

Script_VersionHead_ControllerSta::Script_VersionHead_ControllerSta(QObject *parent) : QObject(parent)
{
    emScriptRunState=TestInit;
    resultFlag=false;
    sendMsgOct.clear();

    p_CtrInfoList=make_shared<QList<shared_ptr<CtrInfo>>>();
    p_BuildNetwork_GW=new BuildNetwork_GW();

    p_Frame3762Helper=make_shared<Frame3762Helper>();
    p_Confirm_00F1=make_shared<Afn00F1>();
    p_HardwareInit_01F1 = make_shared<qgdw_3762_protocol::Afn01F1>();
    p_QueryNetScale_10F9=make_shared<Afn10F9>();
    p_ChkStaVrsnHead_02F1_Down=make_shared<Afn02F1>();
    p_SetStaVrsnHead_02F1_Down=make_shared<Afn02F1>();
    p_ChkStaOutVrsnInfo_10F104_Down=make_shared<Afn10F104>();
    p_ChkStaInVrsnInfo_02F1_Down=make_shared<Afn02F1>();

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
Script_VersionHead_ControllerSta::~Script_VersionHead_ControllerSta()
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
void Script_VersionHead_ControllerSta::execute()
{
    p_AbstractScriptHost->updateProgress(ProcessState_Start, "Test start!");

    resultFlag=false;

    if(needBuildNet==true)
    {
        p_BuildNetwork_GW->execute();
        emScriptRunState=Wait_BuildNetFinish_Whole;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "等待全网组网完成");
        p_timer->start((timerForReachThresld+timerAfterReachThresld)*1000);
    }
    else
    {
        //发送电表断电
        //sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryNodeInfo_10F2);
    //    emScriptRunState=Wait_EventReport_Finish;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "电表断电--等待事件上报");
        //时间再定
        //p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
    }
    p_maxAllowTimer->start((timerForReachThresld+timerAfterReachThresld)*1000);
    for(int i=0;i<p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().size();i++)
    {
        if(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(i)->slotPosition == SingleSTA)
        {
            singleStaAddr = QByteArray(reinterpret_cast<char*>(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(i)->mtrAddr),6).toHex();
        }
        if(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(i)->slotPosition == ThreeSTA)
        {
            threeStaAddr = QByteArray(reinterpret_cast<char*>(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(i)->mtrAddr),6).toHex();
        }
    }
}
void Script_VersionHead_ControllerSta::stop()
{
    if(p_AbstractScriptHost==nullptr)
        return;
    p_AbstractScriptHost->updateProgress(ProcessState_Error, "Test stop!");
    if(needPowerOff==true)
        powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost);
}
void Script_VersionHead_ControllerSta::addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq)
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
void Script_VersionHead_ControllerSta::setHost(AbstractScriptHost *host)
{
    p_AbstractScriptHost=host;
    p_BuildNetwork_GW->setHost(host);
}
bool Script_VersionHead_ControllerSta::config(const QMap<QString,QString> *paraDic)
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
        if(paraDic->keys().contains("SetHead"))
        {
            this->SetHead = (*paraDic)["SetHead"];
        }
        result = true;
    }
    return result;
}
void Script_VersionHead_ControllerSta::processMsg(DvcType dvcType, int id, uchar *data, int datalen)
{
    if(p_CtrInfoList->size()==0)
        return;
    if(emScriptRunState==Wait_BuildNetFinish_Whole)
    {
        if(!p_BuildNetwork_GW->startBuildNetFlag)
            p_BuildNetwork_GW->processMsg(dvcType,id,data,datalen);
        else
        {
            p_BuildNetwork_GW->initBuildNetWork();
            p_timer->stop();

            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "路由单三通全部断电，1min后单三通上电...");
            QList<double> sendParams;
            QList<int> idList;
            sendParams.clear();
            idList.clear();
            idList = findDvcIdList(p_CtrInfoList,CCO_GW);
            p_AbstractScriptHost->controlDvc(CCO_GW,idList,CtrlCmd_PowerOff_12V,sendParams);
            QThread::msleep(50);
            powerOff12V_STA(p_CtrInfoList,p_AbstractScriptHost);
            emScriptRunState = PowerOffAll;
            p_delayTimer->start(60*1000);
        }
    }
    else
    {
        QByteArray tmpRecvTempData=QByteArray::fromRawData(reinterpret_cast<char*>(data),datalen);
        QByteArray recvTempData;
        recvTempData.append(tmpRecvTempData);
        delete[] data;

        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到报文：%1").arg(QString(recvTempData.toHex())));

        if(dvcType == CCO_GW)
        {
            if(dvcType == p_CtrInfoList->at(0)->slotPosition && id == p_CtrInfoList->at(0)->dvcId)
            {
                p_CtrInfoList->at(0)->buf.append(recvTempData);
                processMsgFromCCO(dvcType,id);
            }
        }
        else if(dvcType == ReadCtrlDvc)
        {
            p_CtrInfoList->at(0)->buf.append(recvTempData);
            processMsgFromCCO(dvcType,id);
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
void Script_VersionHead_ControllerSta::processCtrlDvcRes(DvcType dvcType, QList<int> idList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params)
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
//        case Wait_EventReport_Finish:
//        {
//            break;
//        }
        case ScriptSuccess:
        {
            break;
        }
    default:
        break;
    }
}

void Script_VersionHead_ControllerSta::processMsgFromCCO(DvcType dvcType, int dvcId)
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
        case ReadHeadBefore_single:
        {
            dtValue3762=get3762Dt(p_Frame3762Base->dt1_,p_Frame3762Base->dt2_);
            if(p_Frame3762Base->afn_==0x02 && dtValue3762==1 && p_Frame3762Base->ctrl_field_.dir==kDirUp)
            {
                p_timer->stop();
                shared_ptr<Afn02F1> p_ChkStaVrsnHead_02F1_Up=dynamic_pointer_cast<Afn02F1>(p_Frame3762Base);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "单通初始版本头："+p_ChkStaVrsnHead_02F1_Up->frame_content_.mid(28).toHex());

                sendMsg(ReadCtrlDvc,p_CtrInfoList->at(0)->ctrlID,1,p_ChkStaInVrsnInfo_02F1_Down);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--抄控器读单通内部版本");
                p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
                emScriptRunState = ReadInVrsnBefore_single;
                resendflag = false;
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case ReadInVrsnBefore_single:
        {
            dtValue3762=get3762Dt(p_Frame3762Base->dt1_,p_Frame3762Base->dt2_);
            if(p_Frame3762Base->afn_==0x02 && dtValue3762==1 && p_Frame3762Base->ctrl_field_.dir==kDirUp)
            {
                p_timer->stop();
                shared_ptr<Afn02F1> p_ChkStaInVrsnInfo_02F1_Up=dynamic_pointer_cast<Afn02F1>(p_Frame3762Base);
                QByteArray rptVrsnMnfcCode;
                rptVrsnMnfcCode.append(p_ChkStaInVrsnInfo_02F1_Up->frame_content_[24])
                        .append(p_ChkStaInVrsnInfo_02F1_Up->frame_content_[25]);

                QByteArray rptVrsnChipCode;
                rptVrsnChipCode.append(p_ChkStaInVrsnInfo_02F1_Up->frame_content_[26])
                        .append(p_ChkStaInVrsnInfo_02F1_Up->frame_content_[27]);

                QByteArray rptVrsn;
                rptVrsn.append(p_ChkStaInVrsnInfo_02F1_Up->frame_content_[29])
                        .append(p_ChkStaInVrsnInfo_02F1_Up->frame_content_[30]);

                QByteArray rptVrsnDate;
                rptVrsnDate.append(p_ChkStaInVrsnInfo_02F1_Up->frame_content_[34])
                           .append(p_ChkStaInVrsnInfo_02F1_Up->frame_content_[35])
                           .append(p_ChkStaInVrsnInfo_02F1_Up->frame_content_[36]);

                QByteArray rptVrsnTime;
                rptVrsnTime.append(p_ChkStaInVrsnInfo_02F1_Up->frame_content_[37])
                           .append(p_ChkStaInVrsnInfo_02F1_Up->frame_content_[38])
                           .append(p_ChkStaInVrsnInfo_02F1_Up->frame_content_[39]);
                singleStaInVrsn_init = "单通初始厂商芯片代码"+rptVrsnMnfcCode+rptVrsnChipCode+"  内部版本"+(rptVrsnDate+rptVrsnTime+rptVrsn).toHex();
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, singleStaInVrsn_init);

                sendMsg(ReadCtrlDvc,p_CtrInfoList->at(0)->ctrlID,1,p_SetStaVrsnHead_02F1_Down);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--抄控器设置单通版本头");
                p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
                emScriptRunState = SetHead_single;
                resendflag = false;
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case SetHead_single:
        {
            dtValue3762=get3762Dt(p_Frame3762Base->dt1_,p_Frame3762Base->dt2_);
            if(p_Frame3762Base->afn_==0x02 && dtValue3762==1 && p_Frame3762Base->ctrl_field_.dir==kDirUp)
            {
                p_timer->stop();
                shared_ptr<Afn02F1> p_ChkStaInVrsnInfo_02F1_Up=dynamic_pointer_cast<Afn02F1>(p_Frame3762Base);
                if(p_ChkStaInVrsnInfo_02F1_Up->frame_content_.at(26) == 0)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "抄控器设置单通版本头成功，查询新版本头");
                    sendMsg(ReadCtrlDvc,p_CtrInfoList->at(0)->ctrlID,1,p_ChkStaVrsnHead_02F1_Down);
                    p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
                    emScriptRunState = ReadHeadAfter_single;
                    resendflag = false;
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, "抄控器设置单通版本头失败，原因"+QString::number(p_ChkStaInVrsnInfo_02F1_Up->frame_content_.at(26)));
                }
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case ReadHeadAfter_single:
        {
            dtValue3762=get3762Dt(p_Frame3762Base->dt1_,p_Frame3762Base->dt2_);
            if(p_Frame3762Base->afn_==0x02 && dtValue3762==1 && p_Frame3762Base->ctrl_field_.dir==kDirUp)
            {
                p_timer->stop();
                shared_ptr<Afn02F1> p_ChkStaVrsnHead_02F1_Up=dynamic_pointer_cast<Afn02F1>(p_Frame3762Base);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "单通新版本头："+p_ChkStaVrsnHead_02F1_Up->frame_content_.mid(28).toHex());
                if(p_ChkStaVrsnHead_02F1_Up->frame_content_.mid(28).toHex()==SetHead)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "单通版本头和设置一致，开始读单通内部版本");
                    sendMsg(ReadCtrlDvc,p_CtrInfoList->at(0)->ctrlID,1,p_ChkStaInVrsnInfo_02F1_Down);
                    p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
                    emScriptRunState =  ReadInVrsnAfter_single;
                    resendflag = false;
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, "单通版本头和设置不一致");
                }
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case ReadInVrsnAfter_single:
        {
            dtValue3762=get3762Dt(p_Frame3762Base->dt1_,p_Frame3762Base->dt2_);
            if(p_Frame3762Base->afn_==0x02 && dtValue3762==1 && p_Frame3762Base->ctrl_field_.dir==kDirUp)
            {
                p_timer->stop();
                shared_ptr<Afn02F1> p_ChkStaInVrsnInfo_02F1_Up=dynamic_pointer_cast<Afn02F1>(p_Frame3762Base);
                QByteArray rptVrsnMnfcCode;
                rptVrsnMnfcCode.append(p_ChkStaInVrsnInfo_02F1_Up->frame_content_[24])
                        .append(p_ChkStaInVrsnInfo_02F1_Up->frame_content_[25]);

                QByteArray rptVrsnChipCode;
                rptVrsnChipCode.append(p_ChkStaInVrsnInfo_02F1_Up->frame_content_[26])
                        .append(p_ChkStaInVrsnInfo_02F1_Up->frame_content_[27]);

                QByteArray rptVrsn;
                rptVrsn.append(p_ChkStaInVrsnInfo_02F1_Up->frame_content_[29])
                        .append(p_ChkStaInVrsnInfo_02F1_Up->frame_content_[30]);

                QByteArray rptVrsnDate;
                rptVrsnDate.append(p_ChkStaInVrsnInfo_02F1_Up->frame_content_[34])
                           .append(p_ChkStaInVrsnInfo_02F1_Up->frame_content_[35])
                           .append(p_ChkStaInVrsnInfo_02F1_Up->frame_content_[36]);

                QByteArray rptVrsnTime;
                rptVrsnTime.append(p_ChkStaInVrsnInfo_02F1_Up->frame_content_[37])
                           .append(p_ChkStaInVrsnInfo_02F1_Up->frame_content_[38])
                           .append(p_ChkStaInVrsnInfo_02F1_Up->frame_content_[39]);
                QString singleStaInVrsn = "单通厂商芯片代码"+rptVrsnMnfcCode+rptVrsnChipCode+"  内部版本"+(rptVrsnDate+rptVrsnTime+rptVrsn).toHex();
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, singleStaInVrsn);
                if(singleStaInVrsn_init.right(16) == singleStaInVrsn.right(16))
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "单通内部版本未改变，开始读三通初始版本头");
                    sendMsg(ReadCtrlDvc,p_CtrInfoList->at(0)->ctrlID,2,p_ChkStaVrsnHead_02F1_Down);
                    p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
                    emScriptRunState = ReadHeadBefore_three;
                    resendflag = false;
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, "单通内部版本发生改变");
                }
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case ReadHeadBefore_three:
        {
            dtValue3762=get3762Dt(p_Frame3762Base->dt1_,p_Frame3762Base->dt2_);
            if(p_Frame3762Base->afn_==0x02 && dtValue3762==1 && p_Frame3762Base->ctrl_field_.dir==kDirUp)
            {
                p_timer->stop();
                shared_ptr<Afn02F1> p_ChkStaVrsnHead_02F1_Up=dynamic_pointer_cast<Afn02F1>(p_Frame3762Base);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "三通初始版本头："+p_ChkStaVrsnHead_02F1_Up->frame_content_.mid(28).toHex());

                sendMsg(ReadCtrlDvc,p_CtrInfoList->at(0)->ctrlID,2,p_ChkStaInVrsnInfo_02F1_Down);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--抄控器读三通内部版本");
                p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
                emScriptRunState = ReadInVrsnBefore_three;
                resendflag = false;
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case ReadInVrsnBefore_three:
        {
            dtValue3762=get3762Dt(p_Frame3762Base->dt1_,p_Frame3762Base->dt2_);
            if(p_Frame3762Base->afn_==0x02 && dtValue3762==1 && p_Frame3762Base->ctrl_field_.dir==kDirUp)
            {
                p_timer->stop();
                shared_ptr<Afn02F1> p_ChkStaInVrsnInfo_02F1_Up=dynamic_pointer_cast<Afn02F1>(p_Frame3762Base);
                QByteArray rptVrsnMnfcCode;
                rptVrsnMnfcCode.append(p_ChkStaInVrsnInfo_02F1_Up->frame_content_[24])
                        .append(p_ChkStaInVrsnInfo_02F1_Up->frame_content_[25]);

                QByteArray rptVrsnChipCode;
                rptVrsnChipCode.append(p_ChkStaInVrsnInfo_02F1_Up->frame_content_[26])
                        .append(p_ChkStaInVrsnInfo_02F1_Up->frame_content_[27]);

                QByteArray rptVrsn;
                rptVrsn.append(p_ChkStaInVrsnInfo_02F1_Up->frame_content_[29])
                        .append(p_ChkStaInVrsnInfo_02F1_Up->frame_content_[30]);

                QByteArray rptVrsnDate;
                rptVrsnDate.append(p_ChkStaInVrsnInfo_02F1_Up->frame_content_[34])
                           .append(p_ChkStaInVrsnInfo_02F1_Up->frame_content_[35])
                           .append(p_ChkStaInVrsnInfo_02F1_Up->frame_content_[36]);

                QByteArray rptVrsnTime;
                rptVrsnTime.append(p_ChkStaInVrsnInfo_02F1_Up->frame_content_[37])
                           .append(p_ChkStaInVrsnInfo_02F1_Up->frame_content_[38])
                           .append(p_ChkStaInVrsnInfo_02F1_Up->frame_content_[39]);
                threeStaInVrsn_init = "三通初始厂商芯片代码"+rptVrsnMnfcCode+rptVrsnChipCode+"  内部版本"+(rptVrsnDate+rptVrsnTime+rptVrsn).toHex();
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, threeStaInVrsn_init);

                sendMsg(ReadCtrlDvc,p_CtrInfoList->at(0)->ctrlID,2,p_SetStaVrsnHead_02F1_Down);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--抄控器设置三通版本头");
                p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
                emScriptRunState = SetHead_three;
                resendflag = false;
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case SetHead_three:
        {
            dtValue3762=get3762Dt(p_Frame3762Base->dt1_,p_Frame3762Base->dt2_);
            if(p_Frame3762Base->afn_==0x02 && dtValue3762==1 && p_Frame3762Base->ctrl_field_.dir==kDirUp)
            {
                p_timer->stop();
                shared_ptr<Afn02F1> p_ChkStaInVrsnInfo_02F1_Up=dynamic_pointer_cast<Afn02F1>(p_Frame3762Base);
                if(p_ChkStaInVrsnInfo_02F1_Up->frame_content_.at(26) == 0)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "抄控器设置三通版本头成功，查询新版本头");
                    sendMsg(ReadCtrlDvc,p_CtrInfoList->at(0)->ctrlID,2,p_ChkStaVrsnHead_02F1_Down);
                    p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
                    emScriptRunState = ReadHeadAfter_three;
                    resendflag = false;
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, "抄控器设置三通版本头失败，原因"+QString::number(p_ChkStaInVrsnInfo_02F1_Up->frame_content_.at(26)));
                }
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case ReadHeadAfter_three:
        {
            dtValue3762=get3762Dt(p_Frame3762Base->dt1_,p_Frame3762Base->dt2_);
            if(p_Frame3762Base->afn_==0x02 && dtValue3762==1 && p_Frame3762Base->ctrl_field_.dir==kDirUp)
            {
                p_timer->stop();
                shared_ptr<Afn02F1> p_ChkStaVrsnHead_02F1_Up=dynamic_pointer_cast<Afn02F1>(p_Frame3762Base);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "三通新版本头："+p_ChkStaVrsnHead_02F1_Up->frame_content_.mid(28).toHex());
                if(p_ChkStaVrsnHead_02F1_Up->frame_content_.mid(28).toHex()==SetHead)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "三通版本头和设置一致，开始读三通内部版本");
                    sendMsg(ReadCtrlDvc,p_CtrInfoList->at(0)->ctrlID,2,p_ChkStaInVrsnInfo_02F1_Down);
                    p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
                    emScriptRunState =  ReadInVrsnAfter_three;
                    resendflag = false;
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, "三通版本头和设置不一致");
                }
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case ReadInVrsnAfter_three:
        {
            dtValue3762=get3762Dt(p_Frame3762Base->dt1_,p_Frame3762Base->dt2_);
            if(p_Frame3762Base->afn_==0x02 && dtValue3762==1 && p_Frame3762Base->ctrl_field_.dir==kDirUp)
            {
                p_timer->stop();
                shared_ptr<Afn02F1> p_ChkStaInVrsnInfo_02F1_Up=dynamic_pointer_cast<Afn02F1>(p_Frame3762Base);
                QByteArray rptVrsnMnfcCode;
                rptVrsnMnfcCode.append(p_ChkStaInVrsnInfo_02F1_Up->frame_content_[24])
                        .append(p_ChkStaInVrsnInfo_02F1_Up->frame_content_[25]);

                QByteArray rptVrsnChipCode;
                rptVrsnChipCode.append(p_ChkStaInVrsnInfo_02F1_Up->frame_content_[26])
                        .append(p_ChkStaInVrsnInfo_02F1_Up->frame_content_[27]);

                QByteArray rptVrsn;
                rptVrsn.append(p_ChkStaInVrsnInfo_02F1_Up->frame_content_[29])
                        .append(p_ChkStaInVrsnInfo_02F1_Up->frame_content_[30]);

                QByteArray rptVrsnDate;
                rptVrsnDate.append(p_ChkStaInVrsnInfo_02F1_Up->frame_content_[34])
                           .append(p_ChkStaInVrsnInfo_02F1_Up->frame_content_[35])
                           .append(p_ChkStaInVrsnInfo_02F1_Up->frame_content_[36]);

                QByteArray rptVrsnTime;
                rptVrsnTime.append(p_ChkStaInVrsnInfo_02F1_Up->frame_content_[37])
                           .append(p_ChkStaInVrsnInfo_02F1_Up->frame_content_[38])
                           .append(p_ChkStaInVrsnInfo_02F1_Up->frame_content_[39]);
                QString threeStaInVrsn = "三通厂商芯片代码"+rptVrsnMnfcCode+rptVrsnChipCode+"  内部版本"+(rptVrsnDate+rptVrsnTime+rptVrsn).toHex();
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, threeStaInVrsn);
                if(threeStaInVrsn_init.right(16) == threeStaInVrsn.right(16))
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "三通内部版本未改变，路由上电开始组网");
                    QList<double> sendParams;
                    QList<int> idList;
                    sendParams.clear();
                    idList.clear();
                    idList = findDvcIdList(p_CtrInfoList,CCO_GW);
                    p_AbstractScriptHost->controlDvc(CCO_GW,idList,CtrlCmd_PowerOn_12V,sendParams);

                    emScriptRunState = QueryOutVrsn_10F104;
                    p_delayTimer->start(30*1000);
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, "三通内部版本发生改变");
                }
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case QueryOutVrsn_10F104:
        {
            dtValue3762=get3762Dt(p_Frame3762Base->dt1_,p_Frame3762Base->dt2_);
            if(p_Frame3762Base->afn_==0x10 && dtValue3762==9 && p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                shared_ptr<Afn10F9> p_QueryNetScale_10F9_Up=std::dynamic_pointer_cast<Afn10F9>(p_Frame3762Base);
                if(p_QueryNetScale_10F9_Up->network_scale_==p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size()+1)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "网络规模与档案一致，组网完成，开始10F104查询单三通外部版本");
                    p_delayTimer->stop();

                    meterIndex_10F104 = 1;
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_ChkStaOutVrsnInfo_10F104_Down);
                    p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                    single_10F104=false;
                    three_10F104=false;
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("当前网络规模%0，组网未完成，30s后再查").arg(p_QueryNetScale_10F9_Up->network_scale_));
                }
            }
            else if(p_Frame3762Base->afn_==0x10 && dtValue3762==104 && p_Frame3762Base->ctrl_field_.dir==kDirUp)
            {
                p_timer->stop();
                shared_ptr<Afn10F104> p_ChkStaOutVrsnInfo_10F104_Up=std::dynamic_pointer_cast<Afn10F104>(p_Frame3762Base);

                QString nodeAddr = QByteArray(p_ChkStaOutVrsnInfo_10F104_Up->network_node_info_unit_.node_info_group_list_[0].node_address_.addr,6).toHex();
                bool addrExist = false;
                for(int i=0;i<p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().size();i++)
                {
                    if(nodeAddr==QByteArray(reinterpret_cast<char*>(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(i)->mtrAddr),6).toHex())
                    {
                        addrExist = true;
                        break;
                    }
                }
                if(addrExist == false)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, "10F104回复节点地址在档案中不存在："+nodeAddr);
                }

                if(nodeAddr==singleStaAddr)
                {
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
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "10F104单通厂商芯片代码"+(rptVrsnMnfcCode+rptVrsnChipCode).toHex()
                                                         +"  外部版本"+(rptVrsnDate+rptVrsn).toHex());
                    QString code = SetHead.mid(8,8);
                    QString year = QString::number((SetHead.mid(26,2)+SetHead.mid(24,2)).toInt(nullptr,16)).right(2);
                    QString month = QString("%1").arg(SetHead.mid(28,2).toInt(nullptr,16),2,10,QChar('0'));
                    QString day = QString("%1").arg(SetHead.mid(30,2).toInt(nullptr,16),2,10,QChar('0'));
                    QString vrsn1 = QString("%1").arg(SetHead.mid(16,2).toInt(nullptr,16),2,10,QChar('0'));
                    QString vrsn2 = QString("%1").arg(SetHead.mid(18,2).toInt(nullptr,16),2,10,QChar('0'));
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,"设置的版本头中单通厂商芯片"+code+"  版本"+year+month+day+vrsn1+vrsn2);
                    if((rptVrsnMnfcCode+rptVrsnChipCode).toHex()==code && (rptVrsnDate+rptVrsn).toHex()==year+month+day+vrsn1+vrsn2)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,"版本头与单通外部版本一致");
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed,"版本头与单通外部版本不一致");
                    }
                    single_10F104=true;
                }
                else if(nodeAddr==threeStaAddr)
                {
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
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "10F104三通厂商芯片代码"+(rptVrsnMnfcCode+rptVrsnChipCode).toHex()
                                                         +"  外部版本"+(rptVrsnDate+rptVrsn).toHex());
                    QString code = SetHead.mid(8,8);
                    QString year = QString::number((SetHead.mid(50,2)+SetHead.mid(48,2)).toInt(nullptr,16)).right(2);
                    QString month = QString("%1").arg(SetHead.mid(52,2).toInt(nullptr,16),2,10,QChar('0'));
                    QString day = QString("%1").arg(SetHead.mid(54,2).toInt(nullptr,16),2,10,QChar('0'));
                    QString vrsn1 = QString("%1").arg(SetHead.mid(40,2).toInt(nullptr,16),2,10,QChar('0'));
                    QString vrsn2 = QString("%1").arg(SetHead.mid(42,2).toInt(nullptr,16),2,10,QChar('0'));
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,"设置的版本头中三通厂商芯片"+code+"  版本"+year+month+day+vrsn1+vrsn2);
                    if((rptVrsnMnfcCode+rptVrsnChipCode).toHex()==code && (rptVrsnDate+rptVrsn).toHex()==year+month+day+vrsn1+vrsn2)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,"版本头与三通外部版本一致");
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed,"版本头与三通外部版本不一致");
                    }
                    three_10F104=true;
                }
//                else if(nodeAddr=="ffffffffffff")
//                {
//                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("10F104回复节点地址全F"));
//                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("10F104回复节点非单三通，继续查询下一个"));
                }
                if(single_10F104==true && three_10F104==true)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Success, "10F104外部版本查询完毕，符合要求");
                }
                else
                {
                    ++meterIndex_10F104;
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_ChkStaOutVrsnInfo_10F104_Down);
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
        default:
            break;
        }
    }
}
void Script_VersionHead_ControllerSta::processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID)
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
void Script_VersionHead_ControllerSta::processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID)
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
void Script_VersionHead_ControllerSta::sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame)
{
    if(frame==p_Confirm_00F1)
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
    else if(frame==p_ChkStaVrsnHead_02F1_Down)
    {
        p_ChkStaVrsnHead_02F1_Down->protocol_type_ = 0x04;
        p_ChkStaVrsnHead_02F1_Down->frame_content_ = QByteArray::fromHex("FEF80F000105010001000000000000000000");
        p_ChkStaVrsnHead_02F1_Down->frame_content_[8] = char(msgSeq++);
        p_ChkStaVrsnHead_02F1_Down->frame_content_.append(QByteArray(reinterpret_cast<char*>((*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[meterID]->mtrAddr),6));
        p_ChkStaVrsnHead_02F1_Down->frame_content_.append(0x5d).append(0x02);
        p_ChkStaVrsnHead_02F1_Down->frame_length_ = uchar(p_ChkStaVrsnHead_02F1_Down->frame_content_.length());

        p_ChkStaVrsnHead_02F1_Down->ctrl_field_={kHplc,kActive,kDirDown};
        p_ChkStaVrsnHead_02F1_Down->info_field_.info_field_down.comu_module_ident=1;
        p_ChkStaVrsnHead_02F1_Down->info_field_.info_field_down.msg_seq=char(msgSeq);
        memset(p_ChkStaVrsnHead_02F1_Down->address_field_.src_addr,0x00,6);
        memcpy(p_ChkStaVrsnHead_02F1_Down->address_field_.dst_addr,(*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[meterID]->mtrAddr,6);

        sendMsgOct=p_ChkStaVrsnHead_02F1_Down->EncodeFrame();
        sendMsgLog=QString("》》查询STA版本头02F1：%1\n").arg(QString(sendMsgOct.toHex()));
    }
    else if(frame==p_SetStaVrsnHead_02F1_Down)
    {
        p_SetStaVrsnHead_02F1_Down->protocol_type_ = 0x04;
        p_SetStaVrsnHead_02F1_Down->frame_content_ = QByteArray::fromHex("FEF90F000105010001000000000000000000");
        p_SetStaVrsnHead_02F1_Down->frame_content_[8]=char(msgSeq++);
        p_SetStaVrsnHead_02F1_Down->frame_content_.append(QByteArray(reinterpret_cast<char*>((*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[meterID]->mtrAddr),6));
        p_SetStaVrsnHead_02F1_Down->frame_content_.append(0x5d).append(0x02);
        ushort versionHead_len = ushort(SetHead.length());
        p_SetStaVrsnHead_02F1_Down->frame_content_.append(*(reinterpret_cast<char*>(&versionHead_len)));
        p_SetStaVrsnHead_02F1_Down->frame_content_.append(*(reinterpret_cast<char*>(&versionHead_len)+1));
        p_SetStaVrsnHead_02F1_Down->frame_content_.append(QByteArray::fromHex(SetHead.toLatin1()));
        p_SetStaVrsnHead_02F1_Down->frame_length_ = uchar(p_SetStaVrsnHead_02F1_Down->frame_content_.length());

        p_SetStaVrsnHead_02F1_Down->ctrl_field_={kHplc,kActive,kDirDown};
        p_SetStaVrsnHead_02F1_Down->info_field_.info_field_down.comu_module_ident=1;
        p_SetStaVrsnHead_02F1_Down->info_field_.info_field_down.msg_seq=char(msgSeq);
        memset(p_SetStaVrsnHead_02F1_Down->address_field_.src_addr,0x00,6);
        memcpy(p_SetStaVrsnHead_02F1_Down->address_field_.dst_addr,(*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[meterID]->mtrAddr,6);
        sendMsgOct=p_SetStaVrsnHead_02F1_Down->EncodeFrame();
        sendMsgLog=QString("》》设置STA版本头02F1：%1\n").arg(QString(sendMsgOct.toHex()));
    }
    else if(frame==p_ChkStaOutVrsnInfo_10F104_Down)
    {
        p_ChkStaOutVrsnInfo_10F104_Down->start_no_=meterIndex_10F104;
        p_ChkStaOutVrsnInfo_10F104_Down->this_query_num_=1;

        p_ChkStaOutVrsnInfo_10F104_Down->ctrl_field_={kHplc,kActive,kDirDown};
        p_ChkStaOutVrsnInfo_10F104_Down->info_field_.info_field_down.comu_module_ident=0;
        p_ChkStaOutVrsnInfo_10F104_Down->info_field_.info_field_down.msg_seq=char(msgSeq++);

        sendMsgOct=p_ChkStaOutVrsnInfo_10F104_Down->EncodeFrame();
        sendMsgLog=QString("》》查询STA外部版本信息10F104：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_ChkStaInVrsnInfo_02F1_Down)
    {
        p_ChkStaInVrsnInfo_02F1_Down->protocol_type_ = 0x04;
        p_ChkStaInVrsnInfo_02F1_Down->frame_content_ = QByteArray::fromHex("fefe0f000105000301000000000000000000");
        p_ChkStaInVrsnInfo_02F1_Down->frame_content_[8]=char(msgSeq++);
        p_ChkStaInVrsnInfo_02F1_Down->frame_content_.append(QByteArray(reinterpret_cast<char*>((*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[meterID]->mtrAddr),6));
        p_ChkStaInVrsnInfo_02F1_Down->frame_length_ = uchar(p_ChkStaInVrsnInfo_02F1_Down->frame_content_.length());

        p_ChkStaInVrsnInfo_02F1_Down->ctrl_field_={kHplc,kActive,kDirDown};
        p_ChkStaInVrsnInfo_02F1_Down->info_field_.info_field_down.comu_module_ident=1;
        p_ChkStaInVrsnInfo_02F1_Down->info_field_.info_field_down.msg_seq=char(msgSeq);
        memset(p_ChkStaInVrsnInfo_02F1_Down->address_field_.src_addr,0x00,6);
        memcpy(p_ChkStaInVrsnInfo_02F1_Down->address_field_.dst_addr,(*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[meterID]->mtrAddr,6);
        sendMsgOct=p_ChkStaInVrsnInfo_02F1_Down->EncodeFrame();
        sendMsgLog=QString("》》查询STA版本信息02F1：%1\n").arg(QString(sendMsgOct.toHex()));
    }
    else if(frame==p_HardwareInit_01F1)
    {
        p_HardwareInit_01F1->ctrl_field_.dir=kDirDown;
        p_HardwareInit_01F1->ctrl_field_.prm=kActive;
        p_HardwareInit_01F1->ctrl_field_.comn_type=kHplc;

        p_HardwareInit_01F1->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_HardwareInit_01F1->info_field_.info_field_down.comu_rate=0;
        p_HardwareInit_01F1->info_field_.info_field_down.comu_module_ident=0;

        sendMsgOct=p_HardwareInit_01F1->EncodeFrame();
        sendMsgLog=QString("》》硬件初始化01F1：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
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
void Script_VersionHead_ControllerSta::sendSrcMsg(DvcType dvcType, int dvcId, QByteArray msg)
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

void Script_VersionHead_ControllerSta::timer_timeoutProc()
{
    p_timer->stop();
    switch(emScriptRunState)
    {
    case ReadHeadBefore_single:
    case ReadHeadAfter_single:
    {
        if(resendflag == false)
        {
            sendMsg(ReadCtrlDvc,p_CtrInfoList->at(0)->ctrlID,1,p_ChkStaVrsnHead_02F1_Down);
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "超时重发--抄控器读单通版本头");
            p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
            resendflag = true;
        }
        else
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "抄控器读单通版本头未回复");
        }
        break;
    }
    case ReadInVrsnAfter_single:
    case ReadInVrsnBefore_single:
    {
        if(resendflag == false)
        {
            sendMsg(ReadCtrlDvc,p_CtrInfoList->at(0)->ctrlID,1,p_ChkStaInVrsnInfo_02F1_Down);
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "超时重发--抄控器读单通内部版本");
            p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
            resendflag = true;
        }
        else
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "抄控器读单通内部版本未回复");
        }
        break;
    }
    case SetHead_single:
    {
        if(resendflag == false)
        {
            sendMsg(ReadCtrlDvc,p_CtrInfoList->at(0)->ctrlID,1,p_SetStaVrsnHead_02F1_Down);
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "超时重发--抄控器设置单通版本头");
            p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
            resendflag = true;
        }
        else
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "抄控器设置单通版本头未回复");
        }
        break;
    }
    case ReadHeadBefore_three:
    case ReadHeadAfter_three:
    {
        if(resendflag == false)
        {
            sendMsg(ReadCtrlDvc,p_CtrInfoList->at(0)->ctrlID,2,p_ChkStaVrsnHead_02F1_Down);
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "超时重发--抄控器读三通版本头");
            p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
            resendflag = true;
        }
        else
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "抄控器读三通版本头未回复");
        }
        break;
    }
    case ReadInVrsnAfter_three:
    case ReadInVrsnBefore_three:
    {
        if(resendflag == false)
        {
            sendMsg(ReadCtrlDvc,p_CtrInfoList->at(0)->ctrlID,2,p_ChkStaInVrsnInfo_02F1_Down);
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "超时重发--抄控器读三通内部版本");
            p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
            resendflag = true;
        }
        else
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "抄控器读三通内部版本未回复");
        }
        break;
    }
    case SetHead_three:
    {
        if(resendflag == false)
        {
            sendMsg(ReadCtrlDvc,p_CtrInfoList->at(0)->ctrlID,2,p_SetStaVrsnHead_02F1_Down);
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "超时重发--抄控器设置三通版本头");
            p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
            resendflag = true;
        }
        else
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "抄控器设置三通版本头未回复");
        }
        break;
    }
    default:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Error, "超时未回复");
        break;
    }
    }
}
void Script_VersionHead_ControllerSta::maxAllowTimer_timeoutProc()
{
    p_maxAllowTimer->stop();
    switch(emScriptRunState)
    {
//    case Wait_EventReport_Finish:
//    {
//        if(receiveEventFlag==true)
//        {
//            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "组网到达最大等待时间，普通电表事件汇总：\n"+eventMsgList);
//            p_AbstractScriptHost->updateProgress(ProcessState_Success, "收到电表事件上报");
//        }
//        else
//        {
//            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "组网到达最大等待时间，未收到电表事件上报");
//        }
//        break;
//    }
    default:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==p_maxAllowTimer");
        break;
    }
    }
}
void Script_VersionHead_ControllerSta::delayTimer_timeoutProc()
{
    if(emScriptRunState == PowerOffAll)
    {
        p_delayTimer->stop();
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "单三通上电，20s后抄控器读初始版本头");
        powerOn12V_STA(p_CtrInfoList,p_AbstractScriptHost);
        emScriptRunState = ReadHeadBefore_single;
        p_delayTimer->start(20*1000);
    }
    else if(emScriptRunState == ReadHeadBefore_single)
    {
        p_delayTimer->stop();
        sendMsg(ReadCtrlDvc,p_CtrInfoList->at(0)->ctrlID,1,p_ChkStaVrsnHead_02F1_Down);
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--抄控器读单通初始版本头");
        p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
        resendflag = false;
    }
    else
    {
        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryNetScale_10F9);
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询网络规模（10F9），等待--回复");
        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
    }
}

void Script_VersionHead_ControllerSta::calPowerOnReportRate()
{
    QString powerOnFailMeter;
    QString powerOffFailMeter;
    QString otherEventReportMeter;
    struct MeterStruct
    {
        Address meterNo;
        bool powerOnReportFlag=false;
        bool powerOffReportFlag=false;
    };
    QList<MeterStruct> allMeterList;
    for(int i=0;i<p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().size();i++)
    {
        if(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(i)->slotPosition==SingleSTA||p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(i)->slotPosition==ThreeSTA)
            continue;
        MeterStruct meter;
        memcpy(meter.meterNo.addr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(i)->mtrAddr,6);
        allMeterList.append(meter);
    }
    for(int i=0;i<allMeterList.size();i++)
    {
        for(int j=0;j<powerOnReportList.size();j++)
        {
            if(allMeterList.at(i).meterNo==powerOnReportList.at(j).reportNodeAddress)
            {
                allMeterList[i].powerOnReportFlag=true;
            }
        }
        for(int j=0;j<powerOffReportList.size();j++)
        {
            if(allMeterList.at(i).meterNo==powerOffReportList.at(j).reportNodeAddress)
            {
                allMeterList[i].powerOffReportFlag=true;
            }
        }
    }
    for(int i=0;i<otherEventReportList.size();i++)
    {
        otherEventReportMeter.append(QString(QByteArray(otherEventReportList.at(i).reportNodeAddress.addr,6).toHex())+";");
    }
    for(int i=0;i<allMeterList.size();i++)
    {
        if(allMeterList.at(i).powerOnReportFlag==false)
            powerOnFailMeter.append(QString(QByteArray(allMeterList.at(i).meterNo.addr,6).toHex())+";");
        if(allMeterList.at(i).powerOffReportFlag==false)
            powerOffFailMeter.append(QString(QByteArray(allMeterList.at(i).meterNo.addr,6).toHex())+";");
    }
    if(!otherEventReportMeter.isEmpty())
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("上报其它事件的电表如下：\n%1").arg(powerOffFailMeter));
    }
    if(powerOnFailMeter.isEmpty()==true&&powerOffFailMeter.isEmpty()==true)
    {
        emScriptRunState=ScriptSuccess;
        resultFlag=true;
        p_AbstractScriptHost->updateProgress(ProcessState_Success, QString("停复电事件上报测试成功！"));
    }
    else if(powerOnFailMeter.isEmpty()!=true&&powerOffFailMeter.isEmpty()!=true)
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("停电事件上报不全！\n失败电表如下：\n%1").arg(powerOffFailMeter));
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("上电事件上报不全！\n失败电表如下：\n%1").arg(powerOnFailMeter));
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("停复电事件均上报不全！"));
    }
    else if(powerOnFailMeter.isEmpty()!=true&&powerOffFailMeter.isEmpty()==true)
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("上电事件上报不全！\n失败电表如下：\n%1").arg(powerOnFailMeter));
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("复电事件上报不全！"));
    }
    else if(powerOnFailMeter.isEmpty()==true&&powerOffFailMeter.isEmpty()!=true)
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("停电事件上报不全！\n失败电表如下：\n%1").arg(powerOffFailMeter));
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("停电事件上报不全！"));
    }
}
