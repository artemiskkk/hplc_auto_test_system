#include "Script_FreqManage_SetFreq0To0.h"

Script_FreqManage_SetFreq0To0::Script_FreqManage_SetFreq0To0(QObject *parent) : QObject(parent)
{
    emScriptRunState=TestInit;
    emQueryState=Wait_QueryFreq_03F16;
    resultFlag=false;
    sendMsgOct.clear();

    p_CtrInfoList=make_shared<QList<shared_ptr<CtrInfo>>>();
    p_BuildNetwork_GW=new BuildNetwork_GW();

    p_Frame3762Helper=make_shared<Frame3762Helper>();
    p_HardReset_01F1=make_shared<Afn01F1>();
    p_EraseFlash_F0F12=make_shared<AfnF0F12>();
    p_QueryFreq_03F16=make_shared<Afn03F16>();
    p_QueryFreqDivide_F0F51=make_shared<AfnF0F51>();
    p_SetFreq_05F16=make_shared<Afn05F16>();
    p_SetFreqDivide_F0F50=make_shared<AfnF0F50>();

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
Script_FreqManage_SetFreq0To0::~Script_FreqManage_SetFreq0To0()
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
void Script_FreqManage_SetFreq0To0::execute()
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
        p_timer->start((timerForReachThresld+timerAfterReachThresld)*1000);
    }
    else
    {
        emScriptRunState=Wait_SetInitFreq_Finish;
        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_SetFreq_05F16);
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--设置初始频段（05F16），等待--确认");
        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
    }
    p_maxAllowTimer->start((timerForReachThresld+timerAfterReachThresld)*1000);

}
void Script_FreqManage_SetFreq0To0::stop()
{
    if(p_AbstractScriptHost==nullptr)
        return;
    p_AbstractScriptHost->updateProgress(ProcessState_Error, "Test stop!");
    if(needPowerOff==true)
        powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost);
}
void Script_FreqManage_SetFreq0To0::addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq)
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
void Script_FreqManage_SetFreq0To0::setHost(AbstractScriptHost *host)
{
    p_AbstractScriptHost=host;
    p_BuildNetwork_GW->setHost(host);
}
bool Script_FreqManage_SetFreq0To0::config(const QMap<QString,QString> *paraDic)
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
void Script_FreqManage_SetFreq0To0::processMsg(DvcType dvcType, int id, uchar *data, int datalen)
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
            emScriptRunState=Wait_SetInitFreq_Finish;
            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_SetFreq_05F16);
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--设置初始频段（05F16），等待--确认");
            p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
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
void Script_FreqManage_SetFreq0To0::processCtrlDvcRes(DvcType dvcType, QList<int> idList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params)
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
        case Wait_HardResetTest_Finish:
        {
            break;
        }
        case Wait_EraseFlash_Finish:
        {
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

void Script_FreqManage_SetFreq0To0::processMsgFromCCO(DvcType dvcType, int dvcId)
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
            case Wait_SetInitFreq_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("初始运行频段设置，回复确认！"));
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("等待%1s后，查询初始频段！").arg(waitTime));
                    p_delayTimer->start(waitTime*1000);
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_QueryInitFreq_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x03&&p_Frame3762Base->dt1_==char(0x80)&&p_Frame3762Base->dt2_==0x01&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    shared_ptr<Afn03F16> p_QueryFreq_03F16_Up=dynamic_pointer_cast<Afn03F16>(p_Frame3762Base);
                    if(initFreq==p_QueryFreq_03F16_Up->carrier_frequence_range_)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("初始运行频段为%1，符合要求").arg(QString::number(p_QueryFreq_03F16_Up->carrier_frequence_range_)));
                        //                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryFreqDivide_F0F51);
                        //                        emQueryState=Wait_QueryFreqDivide_F0F51;
                        //                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询初始分频系数（F0F51），等待--回复");
                        //                        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                        emScriptRunState=Wait_SetDstFreq_Finish;
                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_SetFreq_05F16);
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--设置目的频段（05F16），等待--确认");
                        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("初始运行频段为%1，不符合要求").arg(QString::number(p_QueryFreq_03F16_Up->carrier_frequence_range_)));
                    }
                }
                else if(p_Frame3762Base->afn_ ==char(0xF0)&&p_Frame3762Base->dt1_==04&&p_Frame3762Base->dt2_==0x06&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    shared_ptr<AfnF0F51> p_QueryFreqDivide_F0F51_Up=dynamic_pointer_cast<AfnF0F51>(p_Frame3762Base);
                    if(p_QueryFreqDivide_F0F51_Up->frequence_division_>=0)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("初始分频系数为%1，符合要求").arg(QString::number(p_QueryFreqDivide_F0F51_Up->frequence_division_)));
                        emScriptRunState=Wait_SetDstFreq_Finish;
                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_SetFreq_05F16);
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--设置目的频段（05F16），等待--确认");
                        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("初始分频系数为%1，不符合要求").arg(QString::number(p_QueryFreqDivide_F0F51_Up->frequence_division_)));
                    }
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_SetDstFreq_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("目的运行频段设置，回复确认！"));
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("等待%1s后，查询目的频段！").arg(waitTime));
                    p_delayTimer->start(waitTime*1000);
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_QueryDstFreq_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x03&&p_Frame3762Base->dt1_==char(0x80)&&p_Frame3762Base->dt2_==0x01&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    shared_ptr<Afn03F16> p_QueryFreq_03F16_Up=dynamic_pointer_cast<Afn03F16>(p_Frame3762Base);
                    if(dstFreq==p_QueryFreq_03F16_Up->carrier_frequence_range_)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("当前运行频段为%1，符合要求").arg(QString::number(p_QueryFreq_03F16_Up->carrier_frequence_range_)));
                        //                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryFreqDivide_F0F51);
                        //                        emQueryState=Wait_QueryFreqDivide_F0F51;
                        //                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询当前分频系数（F0F51），等待--回复");
                        //                        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_HardReset_01F1);
                        emScriptRunState=Wait_HardResetTest_Finish;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--路由硬件复位（01F1），等待--确认");
                        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("当前运行频段为%1，不符合要求").arg(QString::number(p_QueryFreq_03F16_Up->carrier_frequence_range_)));
                    }
                }
                else if(p_Frame3762Base->afn_ ==char(0xF0)&&p_Frame3762Base->dt1_==04&&p_Frame3762Base->dt2_==0x06&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    shared_ptr<AfnF0F51> p_QueryFreqDivide_F0F51_Up=dynamic_pointer_cast<AfnF0F51>(p_Frame3762Base);
                    if(p_QueryFreqDivide_F0F51_Up->frequence_division_>=0)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("当前分频系数为%1，符合要求").arg(QString::number(p_QueryFreqDivide_F0F51_Up->frequence_division_)));
                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_HardReset_01F1);
                        emScriptRunState=Wait_HardResetTest_Finish;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--路由硬件复位（01F1），等待--确认");
                        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("当前分频系数为%1，不符合要求").arg(QString::number(p_QueryFreqDivide_F0F51_Up->frequence_division_)));
                    }
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_HardResetTest_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    break;
                }
                else if(p_Frame3762Base->afn_ == 0x03&&p_Frame3762Base->dt1_==0x02&&p_Frame3762Base->dt2_==0x01&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "路由硬件复位完成！");
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryFreq_03F16);
                    emScriptRunState=Wait_HardResetTest_Finish;
                    emQueryState=Wait_QueryFreq_03F16;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询当前频段（03F16），等待--回复");
                    p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                }
                else if(p_Frame3762Base->afn_ == 0x03&&p_Frame3762Base->dt1_==char(0x80)&&p_Frame3762Base->dt2_==0x01&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    shared_ptr<Afn03F16> p_QueryFreq_03F16_Up=dynamic_pointer_cast<Afn03F16>(p_Frame3762Base);
                    //判断是否正确
                    if(dstFreq==p_QueryFreq_03F16_Up->carrier_frequence_range_)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("路由硬件复位后，当前运行频段为%1，符合要求").arg(QString::number(p_QueryFreq_03F16_Up->carrier_frequence_range_)));

                        emScriptRunState=ScriptSuccess;
                        resultFlag=true;
                        p_AbstractScriptHost->updateProgress(ProcessState_Success, QString("路由频段管理(0→0)，测试成功;"));
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("路由硬件复位后，当前运行频段为%1，不符合要求").arg(QString::number(p_QueryFreq_03F16_Up->carrier_frequence_range_)));
                    }
                }
                else if(p_Frame3762Base->afn_ ==char(0xF0)&&p_Frame3762Base->dt1_==04&&p_Frame3762Base->dt2_==0x06&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    shared_ptr<AfnF0F51> p_QueryFreqDivide_F0F51_Up=dynamic_pointer_cast<AfnF0F51>(p_Frame3762Base);
                    if(p_QueryFreqDivide_F0F51_Up->frequence_division_>=0)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("路由硬件复位后，当前分频系数为%1，符合要求").arg(QString::number(p_QueryFreqDivide_F0F51_Up->frequence_division_)));

                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_EraseFlash_F0F12);
                        emScriptRunState=Wait_EraseFlash_Finish;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--擦除路由Flash（F0F12），等待--确认");
                        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("路由硬件复位后，当前分频系数为%1，不符合要求").arg(QString::number(p_QueryFreqDivide_F0F51_Up->frequence_division_)));
                    }
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_EraseFlash_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    break;
                }
                else if(p_Frame3762Base->afn_ == 0x03&&p_Frame3762Base->dt1_==0x02&&p_Frame3762Base->dt2_==0x01&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "擦除路由Flash完成！");
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryFreq_03F16);
                    emScriptRunState=Wait_EraseFlash_Finish;
                    emQueryState=Wait_QueryFreq_03F16;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询当前频段（03F16），等待--回复");
                    p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                }
                else if(p_Frame3762Base->afn_ == 0x03&&p_Frame3762Base->dt1_==char(0x80)&&p_Frame3762Base->dt2_==0x01&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    shared_ptr<Afn03F16> p_QueryFreq_03F16_Up=dynamic_pointer_cast<Afn03F16>(p_Frame3762Base);
                    //判断是否正确
                    if(0x02==p_QueryFreq_03F16_Up->carrier_frequence_range_)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("擦除路由Flash后，当前运行频段为%1，符合要求").arg(QString::number(p_QueryFreq_03F16_Up->carrier_frequence_range_)));
                        emScriptRunState=ScriptSuccess;
                        resultFlag=true;
                        p_AbstractScriptHost->updateProgress(ProcessState_Success, QString("路由频段管理(0→0)，测试成功;"));
//                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryFreqDivide_F0F51);
//                        emQueryState=Wait_QueryFreqDivide_F0F51;
//                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询当前分频系数（F0F51），等待--回复");
//                        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("擦除路由Flash后，当前运行频段为%1，不符合要求").arg(QString::number(p_QueryFreq_03F16_Up->carrier_frequence_range_)));
                    }
                }
                else if(p_Frame3762Base->afn_ ==char(0xF0)&&p_Frame3762Base->dt1_==04&&p_Frame3762Base->dt2_==0x06&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    shared_ptr<AfnF0F51> p_QueryFreqDivide_F0F51_Up=dynamic_pointer_cast<AfnF0F51>(p_Frame3762Base);
                    if(p_QueryFreqDivide_F0F51_Up->frequence_division_>=0)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("擦除路由Flash后，当前分频系数为%1，符合要求").arg(QString::number(p_QueryFreqDivide_F0F51_Up->frequence_division_)));

                        emScriptRunState=ScriptSuccess;
                        resultFlag=true;
                        p_AbstractScriptHost->updateProgress(ProcessState_Success, QString("路由频段管理(0→0)，测试成功;"));
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("当前分频系数为%1，不符合要求").arg(QString::number(p_QueryFreqDivide_F0F51_Up->frequence_division_)));
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
void Script_FreqManage_SetFreq0To0::processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID)
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
void Script_FreqManage_SetFreq0To0::processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID)
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
void Script_FreqManage_SetFreq0To0::sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame)
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
    else if(frame==p_EraseFlash_F0F12)
    {
        p_EraseFlash_F0F12->ctrl_field_.dir=kDirDown;
        p_EraseFlash_F0F12->ctrl_field_.prm=kActive;
        p_EraseFlash_F0F12->ctrl_field_.comn_type=kHplc;

        p_EraseFlash_F0F12->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_EraseFlash_F0F12->info_field_.info_field_down.comu_rate=0;
        p_EraseFlash_F0F12->info_field_.info_field_down.comu_module_ident=0;

        sendMsgOct=p_EraseFlash_F0F12->EncodeFrame();
        sendMsgLog=QString("》》擦除路由Flash F0F12：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_QueryFreq_03F16)
    {
        p_QueryFreq_03F16->ctrl_field_.dir=kDirDown;
        p_QueryFreq_03F16->ctrl_field_.prm=kActive;
        p_QueryFreq_03F16->ctrl_field_.comn_type=kHplc;

        p_QueryFreq_03F16->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_QueryFreq_03F16->info_field_.info_field_down.comu_rate=0;
        p_QueryFreq_03F16->info_field_.info_field_down.comu_module_ident=0;

        sendMsgOct=p_QueryFreq_03F16->EncodeFrame();
        sendMsgLog=QString("》》查询频段03F16：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_SetFreq_05F16)
    {
        p_SetFreq_05F16->ctrl_field_.dir=kDirDown;
        p_SetFreq_05F16->ctrl_field_.prm=kActive;
        p_SetFreq_05F16->ctrl_field_.comn_type=kHplc;

        p_SetFreq_05F16->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_SetFreq_05F16->info_field_.info_field_down.comu_rate=0;
        p_SetFreq_05F16->info_field_.info_field_down.comu_module_ident=0;
        if(emScriptRunState==Wait_SetInitFreq_Finish)
            p_SetFreq_05F16->carrier_frequence_range_=initFreq;
        else if(emScriptRunState==Wait_SetDstFreq_Finish)
            p_SetFreq_05F16->carrier_frequence_range_=dstFreq;

        sendMsgOct=p_SetFreq_05F16->EncodeFrame();
        sendMsgLog=QString("》》设置频段05F16：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_SetFreqDivide_F0F50)
    {
        p_SetFreqDivide_F0F50->ctrl_field_.dir=kDirDown;
        p_SetFreqDivide_F0F50->ctrl_field_.prm=kActive;
        p_SetFreqDivide_F0F50->ctrl_field_.comn_type=kHplc;

        p_SetFreqDivide_F0F50->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_SetFreqDivide_F0F50->info_field_.info_field_down.comu_rate=0;
        p_SetFreqDivide_F0F50->info_field_.info_field_down.comu_module_ident=0;

        if(emScriptRunState==Wait_SetInitFreq_Finish)
            p_SetFreqDivide_F0F50->frequence_division_=initFreqDivide;
        else if(emScriptRunState==Wait_SetDstFreq_Finish)
            p_SetFreqDivide_F0F50->frequence_division_=dstFreqDivide;

        sendMsgOct=p_SetFreqDivide_F0F50->EncodeFrame();
        sendMsgLog=QString("》》设置分频系数F0F50：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_QueryFreqDivide_F0F51)
    {
        p_QueryFreqDivide_F0F51->ctrl_field_.dir=kDirDown;
        p_QueryFreqDivide_F0F51->ctrl_field_.prm=kActive;
        p_QueryFreqDivide_F0F51->ctrl_field_.comn_type=kHplc;

        p_QueryFreqDivide_F0F51->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_QueryFreqDivide_F0F51->info_field_.info_field_down.comu_rate=0;
        p_QueryFreqDivide_F0F51->info_field_.info_field_down.comu_module_ident=0;

        sendMsgOct=p_QueryFreqDivide_F0F51->EncodeFrame();
        sendMsgLog=QString("》》查询路由分频系数F0F51：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
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
void Script_FreqManage_SetFreq0To0::sendSrcMsg(DvcType dvcType, int dvcId, QByteArray msg)
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

void Script_FreqManage_SetFreq0To0::timer_timeoutProc()
{
    switch(emScriptRunState)
    {
        case Wait_BuildNetFinish_Whole:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_BuildNetFinish_Whole timeout!!!");
            break;
        }
        case Wait_QueryInitFreq_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_QueryInitFreq_Finish timeout!!!");
            break;
        }
        case Wait_HardResetTest_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_HardResetTest_Finish timeout!!!");
            break;
        }
        case Wait_EraseFlash_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_EraseFlash_Finish timeout!!!");
            break;
        }
        default:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==p_timer");
            break;
        }
    }
}
void Script_FreqManage_SetFreq0To0::maxAllowTimer_timeoutProc()
{
    switch(emScriptRunState)
    {
        case Wait_HardResetTest_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_HardResetTest_Finish maxAllow timeout!!!");
            break;
        }
        case Wait_EraseFlash_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_EraseFlash_Finish maxAllow timeout!!!");
            break;
        }
        default:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==p_maxAllowTimer");
            break;
        }
    }
}
void Script_FreqManage_SetFreq0To0::delayTimer_timeoutProc()
{
    p_delayTimer->stop();
    if(emScriptRunState==Wait_SetInitFreq_Finish)
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "开始查询初始频段！");
        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryFreq_03F16);
        emScriptRunState=Wait_QueryInitFreq_Finish;
        emQueryState=Wait_QueryFreq_03F16;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询初始频段（03F16），等待--回复");
        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
    }
    else if(emScriptRunState==Wait_SetDstFreq_Finish)
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "开始查询目的频段！");
        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryFreq_03F16);
        emScriptRunState=Wait_QueryDstFreq_Finish;
        emQueryState=Wait_QueryFreq_03F16;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询目的频段（03F16），等待--回复");
        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
    }
}
