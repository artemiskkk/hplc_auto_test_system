#include "Script_FreqManage_FreqAndFreqDivideSwitch.h"

Script_FreqManage_FreqAndFreqDivideSwitch::Script_FreqManage_FreqAndFreqDivideSwitch(QObject *parent) : QObject(parent)
{
    emScriptRunState=TestInit;
  //  resultFlag=false;
    sendMsgOct.clear();

    p_CtrInfoList=make_shared<QList<shared_ptr<CtrInfo>>>();
    p_BuildNetwork_GW=new BuildNetwork_GW();

    p_Frame3762Helper=make_shared<Frame3762Helper>();
    //p_HardReset_01F1=make_shared<Afn01F1>();
    //p_EraseFlash_F0F12=make_shared<AfnF0F12>();
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

    //**F2D1
    FreqAndDivideStruct dst;
    dst.dstFreq=0x02;//**目的运行频段2
    dst.dstFreqDivide=0x01;//**目的分频系数1
    //**F0D1
    FreqAndDivideStruct dst1;
    dst1.dstFreq=0x00;//**目的运行频段0
    dst1.dstFreqDivide=0x01;//**目的分频系数1
    //**F0D2
    FreqAndDivideStruct dst2;
    dst2.dstFreq=0x00;//**目的运行频段0
    dst2.dstFreqDivide=0x02;//**目的分频系数2
    //**F1D2
    FreqAndDivideStruct dst3;
    dst3.dstFreq=0x01;//**目的运行频段1
    dst3.dstFreqDivide=0x02;//**目的分频系数2

    freqAndDivideList<<dst<<dst1<<dst2<<dst3;
}

Script_FreqManage_FreqAndFreqDivideSwitch::~Script_FreqManage_FreqAndFreqDivideSwitch()
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

void Script_FreqManage_FreqAndFreqDivideSwitch::execute()
{
    p_AbstractScriptHost->updateProgress(ProcessState_Start, "Test start!");

    emScriptRunState=TestInit;
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
//        emScriptRunState=Wait_SetInitFreq_Finish;
//        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_SetFreq_05F16);
//        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--设置初始频段（05F16），等待--确认");
//        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);

        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "开始查询初始频段！");
        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryFreq_03F16);
        emScriptRunState=Wait_QueryInitFreq_Finish;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询初始频段（03F16），等待--回复");
        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
    }
    p_maxAllowTimer->start((timerForReachThresld+timerAfterReachThresld)*1000);

}

void Script_FreqManage_FreqAndFreqDivideSwitch::stop()
{
    if(p_AbstractScriptHost==nullptr)
        return;
    p_AbstractScriptHost->updateProgress(ProcessState_Error, "Test stop!");
    if(needPowerOff==true)
        powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost);
}

void Script_FreqManage_FreqAndFreqDivideSwitch::addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq)
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

void Script_FreqManage_FreqAndFreqDivideSwitch::setHost(AbstractScriptHost *host)
{
    p_AbstractScriptHost=host;
    p_BuildNetwork_GW->setHost(host);
}

bool Script_FreqManage_FreqAndFreqDivideSwitch::config(const QMap<QString, QString> *paraDic)
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

void Script_FreqManage_FreqAndFreqDivideSwitch::processMsg(DvcType dvcType, int id, uchar *data, int datalen)
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
//            emScriptRunState=Wait_SetInitFreq_Finish;
//            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_SetFreq_05F16);
//            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--设置初始频段（05F16），等待--确认");
//            p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);

            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "开始查询初始频段！");
            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryFreq_03F16);
            emScriptRunState=Wait_QueryInitFreq_Finish;
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询初始频段（03F16），等待--回复");
            p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
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

void Script_FreqManage_FreqAndFreqDivideSwitch::processCtrlDvcRes(DvcType dvcType, QList<int> idList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params)
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

void Script_FreqManage_FreqAndFreqDivideSwitch::processMsgFromCCO(DvcType dvcType, int dvcId)
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
//            case Wait_SetInitFreq_Finish:
//            {
//                //**收到回复00F1
//                if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
//                {
//                    p_timer->stop();
//                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("初始运行频段设置，回复确认！"));
//                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("等待%1s后，设置初始分频系数！").arg(waitTime));
//                    p_delayTimer->start(waitTime*1000);
//                }
//                else
//                {
//                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
//                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
//                }
//                break;
//            }
//            case Wait_SetInitFreqDivide_Finish:
//            {
//                //**收到回复00F1
//                if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
//                {
//                    p_timer->stop();
//                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("初始分频系数设置，回复确认！"));
//                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("等待%1s后，查询初始频段！").arg(waitTime));
//                    p_delayTimer->start(waitTime*1000);
//                }
//                else
//                {
//                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
//                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
//                }
//                break;
//            }
            case Wait_QueryInitFreq_Finish:
            {
                //**收到03F16查询频段上报
                if(p_Frame3762Base->afn_ == 0x03&&p_Frame3762Base->dt1_==char(0x80)&&p_Frame3762Base->dt2_==0x01&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    shared_ptr<Afn03F16> p_QueryFreq_03F16_Up=dynamic_pointer_cast<Afn03F16>(p_Frame3762Base);
                    if(initFreq==p_QueryFreq_03F16_Up->carrier_frequence_range_)
                    {
                        //**打印log
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("初始运行频段为【%1】，符合要求").arg(QString::number(p_QueryFreq_03F16_Up->carrier_frequence_range_)));
                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryFreqDivide_F0F51);
                        emScriptRunState=Wait_QueryInitFreqDivide_Finish;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询初始分频系数（F0F51），等待--回复");
                        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("初始运行频段为【%1】，不符合要求").arg(QString::number(p_QueryFreq_03F16_Up->carrier_frequence_range_)));
                    }
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_QueryInitFreqDivide_Finish:
            {
                //收到F0F51查询分频系数上报
                if(p_Frame3762Base->afn_ ==char(0xF0)&&p_Frame3762Base->dt1_==04&&p_Frame3762Base->dt2_==0x06&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    shared_ptr<AfnF0F51> p_QueryFreqDivide_F0F51_Up=dynamic_pointer_cast<AfnF0F51>(p_Frame3762Base);
                    if(initFreqDivide==p_QueryFreqDivide_F0F51_Up->frequence_division_)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("初始分频系数为【%1】，符合要求").arg(QString::number(p_QueryFreqDivide_F0F51_Up->frequence_division_)));
                        emScriptRunState=Wait_SetDstFreqDivide2_Finish;
                        index=0;
                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_SetFreqDivide_F0F50);
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--设置目的分频系数【2】（F0F50），等待--否认");
                        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("初始分频系数为【%1】，不符合要求").arg(QString::number(p_QueryFreqDivide_F0F51_Up->frequence_division_)));
                    }
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_SetDstFreqDivide2_Finish:
            {
                if(adFlag==true)
                {
                    //**收到00F2否认上报
                    if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x02&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                    {
                        p_timer->stop();
                        //**（1）设置分频系数为2，回复否认
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("目的分频系数【2】设置，回复否认，正常！"));
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("等待%1s后，查询目的分频系数！").arg(waitTime));
                        p_delayTimer->start(waitTime*1000);
                    }
                    //**收到回复00F1
                    else if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                    {
                        //**处理设置分频系数为2，回复确认为异常
                        p_timer->stop();
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("目的分频系数【2】设置，回复确认，状态异常！"));
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Script_FreqManage_FreqAndFreqDivideSwitch Failed!!!");
                        break;
                    }
                    else
                    {
                        QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                        sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                    }
                    break;
                }
                else
                {
                    if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                    {
                        p_timer->stop();
                        //**（2）设置分频系数为2，回复确认
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("目的分频系数【2】设置，回复确认，正常！"));
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("等待%1s后，查询目的分频系数！").arg(waitTime));
                        p_delayTimer->start(waitTime*1000);
                    }
                    else if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x02&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                    {
                        //**处理设置分频系数为2，回复否认为异常
                        p_timer->stop();
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("目的分频系数【2】设置，回复否认，状态异常！"));
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Script_FreqManage_FreqAndFreqDivideSwitch Failed!!!");
                        break;
                    }
                    else
                    {
                        QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                        sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                    }
                    break;
                }
            }
            case Wait_QueryDstFreqDivide2_Finish:
            {
                //**收到F0F51查询分频系数上报
                if(p_Frame3762Base->afn_ ==char(0xF0)&&p_Frame3762Base->dt1_==04&&p_Frame3762Base->dt2_==0x06&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
               {
                    p_timer->stop();
                    shared_ptr<AfnF0F51> p_QueryFreqDivide_F0F51_Up=dynamic_pointer_cast<AfnF0F51>(p_Frame3762Base);
                    if(0x01==p_QueryFreqDivide_F0F51_Up->frequence_division_)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("目的分频系数为【%1】，符合要求").arg(QString::number(p_QueryFreqDivide_F0F51_Up->frequence_division_)));
                        emScriptRunState=Wait_QueryDstFreq2_Finish;
                        index=0;
                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryFreq_03F16);
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询目的运行频段（03F16），等待--回复");
                        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                    }
                    else if(0x02==p_QueryFreqDivide_F0F51_Up->frequence_division_)
                    {
                        if(resultFlag==true&&transFlag==true)
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("目的分频系数为【%1】，符合要求").arg(QString::number(p_QueryFreqDivide_F0F51_Up->frequence_division_)));
                            emScriptRunState=Wait_QueryDstFreq0_Finish;
                            resultFlag=false;
                            index=0;
                            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryFreq_03F16);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询目的运行频段（03F16），等待--回复");
                            p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                        }
                        else if(resultFlag==false&&transFlag==true)
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("目的分频系数为【%1】，符合要求").arg(QString::number(p_QueryFreqDivide_F0F51_Up->frequence_division_)));
                            emScriptRunState=Wait_SetDstFreq1_Finish;
                            index=0;
                            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_SetFreq_05F16);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--切换目的运行频段【1】（05F16），等待--确认");
                            p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                        }
                        else if(resultFlag==false&&transFlag==false)
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("目的分频系数为【%1】，符合要求").arg(QString::number(p_QueryFreqDivide_F0F51_Up->frequence_division_)));
                            emScriptRunState=Wait_SetDstFreq2_Finish;
                            index=0;
                            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_SetFreq_05F16);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--切换目的运行频段【2】（05F16），等待--确认");
                            p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                        }
                        else if(resultFlag==true&&transFlag==false)
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("目的分频系数为【%1】，符合要求").arg(QString::number(p_QueryFreqDivide_F0F51_Up->frequence_division_)));
                            emScriptRunState=Wait_SetDstFreqDivide1_Finish;
                            index=0;
                            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_SetFreqDivide_F0F50);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--设置目的分频系数【1】（F0F50），等待--确认");
                            p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                        }
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("目的分频系数为【%1】，不符合要求").arg(QString::number(p_QueryFreqDivide_F0F51_Up->frequence_division_)));
                    }
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;

            }
            case Wait_QueryDstFreq2_Finish:
            {
                //**收到03F16查询频段上报
                if(p_Frame3762Base->afn_ == 0x03&&p_Frame3762Base->dt1_==char(0x80)&&p_Frame3762Base->dt2_==0x01&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    shared_ptr<Afn03F16> p_QueryFreq_03F16_Up=dynamic_pointer_cast<Afn03F16>(p_Frame3762Base);

                    if(resultFlag==true)
                    {
                        if(0x02==p_QueryFreq_03F16_Up->carrier_frequence_range_)
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("目的运行频段为【%1】，符合要求").arg(QString::number(p_QueryFreq_03F16_Up->carrier_frequence_range_)));
                            emScriptRunState=Wait_SetDstFreq0_Finish;
                            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_SetFreq_05F16);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--切换目的运行频段【0】（05F16），等待--确认");
                            p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                        }
                        else if(0x00==p_QueryFreq_03F16_Up->carrier_frequence_range_)
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("目的运行频段为【%1】，符合要求").arg(QString::number(p_QueryFreq_03F16_Up->carrier_frequence_range_)));
                            emScriptRunState=Wait_SetDstFreq2_Finish;
                            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_SetFreq_05F16);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--切换目的运行频段【2】（05F16），等待--确认");

                        }
                    }
                    else
                    {
                        if(0x00==p_QueryFreq_03F16_Up->carrier_frequence_range_)
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("目的运行频段为【%1】，符合要求").arg(QString::number(p_QueryFreq_03F16_Up->carrier_frequence_range_)));
                            emScriptRunState=Wait_QueryDstFreqDivide2_Finish;
                            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryFreqDivide_F0F51);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询目的分频系数（F0F51），等待--回复");

                        }
                        else if(0x01==p_QueryFreq_03F16_Up->carrier_frequence_range_)
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("目的运行频段为【%1】，符合要求").arg(QString::number(p_QueryFreq_03F16_Up->carrier_frequence_range_)));
                            emScriptRunState=Wait_QueryDstFreqDivide2_Finish;
                            resultFlag=true;
                            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryFreqDivide_F0F51);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询目的分频系数（F0F51），等待--回复");

                        }
                        else if(0x02==p_QueryFreq_03F16_Up->carrier_frequence_range_)
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("目的运行频段为【%1】，符合要求").arg(QString::number(p_QueryFreq_03F16_Up->carrier_frequence_range_)));
                            emScriptRunState=ScriptSuccess;
                            resultFlag=true;
                            transFlag=true;
                            p_AbstractScriptHost->updateProgress(ProcessState_Success, QString("分频互切测试成功;"));
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
            case Wait_SetDstFreq0_Finish:
            {
                //**收到回复00F1
                if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    //**目的运行频段0设置，回复确认
                    adFlag=false;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("目的运行频段【0】设置，回复确认！"));
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("等待%1s后，查询目的运行频段！").arg(waitTime));
                    p_delayTimer->start(waitTime*1000);
                }
                //**收到回复00F2否认，异常处理
                else if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x02&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("目的运行频段【0】设置，回复否认！状态异常！"));
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("Script_FreqManage_FreqAndFreqDivideSwitch Failed!!"));
                    break;
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_QueryDstFreq0_Finish:
            {
                //**收到03F16查询频段上报
                if(p_Frame3762Base->afn_ == 0x03&&p_Frame3762Base->dt1_==char(0x80)&&p_Frame3762Base->dt2_==0x01&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    shared_ptr<Afn03F16> p_QueryFreq_03F16_Up=dynamic_pointer_cast<Afn03F16>(p_Frame3762Base);
                    if(0x00==p_QueryFreq_03F16_Up->carrier_frequence_range_)
                    {
                        if(resultFlag==true)
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("目的运行频段为【%1】，符合要求").arg(QString::number(p_QueryFreq_03F16_Up->carrier_frequence_range_)));
                            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryFreqDivide_F0F51);
                            emScriptRunState=Wait_QueryDstFreqDivide1_Finish;
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询目的分频系数（F0F51），等待--回复");
                            p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                        }
                        else
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("目的运行频段为【%1】，符合要求").arg(QString::number(p_QueryFreq_03F16_Up->carrier_frequence_range_)));
                            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_SetFreq_05F16);
                            emScriptRunState=Wait_SetDstFreq2_Finish;
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--切换目的运行频段【2】（05F16），等待--确认");

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
            case Wait_QueryDstFreqDivide1_Finish:
            {
                //**收到F0F51查询分频系数上报
                if(p_Frame3762Base->afn_ ==char(0xF0)&&p_Frame3762Base->dt1_==04&&p_Frame3762Base->dt2_==0x06&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    shared_ptr<AfnF0F51> p_QueryFreqDivide_F0F51_Up=dynamic_pointer_cast<AfnF0F51>(p_Frame3762Base);
                    if(0x01==p_QueryFreqDivide_F0F51_Up->frequence_division_)
                    {
                        if(resultFlag==true&&transFlag==true)
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("目的分频系数为【%1】，符合要求").arg(QString::number(p_QueryFreqDivide_F0F51_Up->frequence_division_)));
                            emScriptRunState=Wait_SetDstFreqDivide2_Finish;
                            index=0;
                            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_SetFreqDivide_F0F50);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--设置目的分频系数【2】（F0F50），等待--确认");
                            p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                        }
                        else if(resultFlag==true&&transFlag==false)
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("目的分频系数为【%1】，符合要求").arg(QString::number(p_QueryFreqDivide_F0F51_Up->frequence_division_)));
                            emScriptRunState=Wait_QueryDstFreq1_Finish;
                            index=0;
                            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryFreq_03F16);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询目的运行频段（03F16），等待--确认");
                            p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
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
            case Wait_SetDstFreq2_Finish:
            {
            //**收到00F2否认上报
            if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x02&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                //**目的运行频段2设置，回复否认
                p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("目的运行频段【2】设置，回复否认！"));
                p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("等待%1s后，查询目的运行频段！").arg(waitTime));
                p_delayTimer->start(waitTime*1000);
            }
            //**收到00F1确认上报
            else if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("目的运行频段【2】设置，回复确认！"));
                p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("等待%1s后，查询目的运行频段！").arg(waitTime));
                p_delayTimer->start(waitTime*1000);
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
                break;
            }
            case Wait_SetDstFreq2_ab_Finish:
            {
                //**收到回复00F1
                if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    //**目的运行频段1设置，回复确认
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("目的运行频段【2】设置，回复确认！"));
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("等待%1s后，查询目的运行频段！").arg(waitTime));
                    p_delayTimer->start(waitTime*1000);
                }
                //**收到回复00F2否认，异常处理
                else if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x02&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("目的运行频段【2】设置，回复否认！状态异常！"));
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("Script_FreqManage_FreqAndFreqDivideSwitch Failed!!"));
                    break;
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_SetDstFreq1_Finish:
            {
                //**收到回复00F1
                if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    //**目的运行频段1设置，回复确认
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("目的运行频段【1】设置，回复确认！"));
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("等待%1s后，查询目的运行频段！").arg(waitTime));
                    p_delayTimer->start(waitTime*1000);
                }
                //**收到回复00F2否认，异常处理
                else if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x02&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("目的运行频段【1】设置，回复否认！状态异常！"));
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("Script_FreqManage_FreqAndFreqDivideSwitch Failed!!"));
                    break;
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_QueryDstFreq1_Finish:
            {
                //**收到03F16查询频段上报
                if(p_Frame3762Base->afn_ == 0x03&&p_Frame3762Base->dt1_==char(0x80)&&p_Frame3762Base->dt2_==0x01&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    shared_ptr<Afn03F16> p_QueryFreq_03F16_Up=dynamic_pointer_cast<Afn03F16>(p_Frame3762Base);
                    if(0x01==p_QueryFreq_03F16_Up->carrier_frequence_range_)
                    {
                        //**这里两个flag的值应该是改变了 再顺一遍时记得改成满足F0F51的条件
                        //**原resultFlag==true改为resultFlag==false,新加transFlag=false;
                        if(resultFlag==false&&transFlag==true)
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("目的运行频段为【%1】，符合要求").arg(QString::number(p_QueryFreq_03F16_Up->carrier_frequence_range_)));
                            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryFreqDivide_F0F51);
                            transFlag=false;
                            emScriptRunState=Wait_QueryDstFreqDivide2_Finish;
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询目的分频系数（F0F51），等待--回复");
                            p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                        }
                        else if(resultFlag==true&&transFlag==false)
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("目的运行频段为【%1】，符合要求").arg(QString::number(p_QueryFreq_03F16_Up->carrier_frequence_range_)));
                            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_SetFreq_05F16);
                            emScriptRunState=Wait_SetDstFreq2_ab_Finish;
                            resultFlag=false;
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--切换目的运行频段【2】（05F16），等待--确认");
                            p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                        }
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("目的运行频段为【%1】，不符合要求").arg(QString::number(p_QueryFreq_03F16_Up->carrier_frequence_range_)));
                    }
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_SetDstFreqDivide1_Finish:
            {
                //**收到回复00F1
                if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("目的分频系数【1】设置，回复确认！"));
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("等待%1s后，查询目的分频系数！").arg(waitTime));
                    p_delayTimer->start(waitTime*1000);
                }
                //**收到回复00F2否认，异常处理
                else if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x02&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("目的分频系数【1】设置，回复否认！状态异常！"));
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("Script_FreqManage_FreqAndFreqDivideSwitch Failed!!"));
                    break;
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
        }
    }
}

void Script_FreqManage_FreqAndFreqDivideSwitch::sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame)
{
    if(frame==p_QueryFreq_03F16)
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
        else if(emScriptRunState==Wait_SetDstFreq0_Finish)
            p_SetFreq_05F16->carrier_frequence_range_=freqAndDivideList.at(1).dstFreq;
        else if(emScriptRunState==Wait_SetDstFreq2_Finish)
            p_SetFreq_05F16->carrier_frequence_range_=freqAndDivideList.at(0).dstFreq;
        else if(emScriptRunState==Wait_SetDstFreq1_Finish)
            p_SetFreq_05F16->carrier_frequence_range_=freqAndDivideList.at(3).dstFreq;

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
        else if(emScriptRunState==Wait_SetDstFreqDivide2_Finish)
            p_SetFreqDivide_F0F50->frequence_division_=freqAndDivideList.at(2).dstFreqDivide;
        else if(emScriptRunState==Wait_SetDstFreqDivide1_Finish)
            p_SetFreqDivide_F0F50->frequence_division_=freqAndDivideList.at(0).dstFreqDivide;

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

void Script_FreqManage_FreqAndFreqDivideSwitch::sendSrcMsg(DvcType dvcType, int dvcId, QByteArray msg)
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
void Script_FreqManage_FreqAndFreqDivideSwitch::processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID)
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

void Script_FreqManage_FreqAndFreqDivideSwitch::processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID)
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

void Script_FreqManage_FreqAndFreqDivideSwitch::timer_timeoutProc()
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
        case Wait_QueryDstFreq0_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_QueryDstFreq0_Finish timeout!!!");
            break;
        }
        case Wait_QueryDstFreq1_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_QueryDstFreq1_Finish timeout!!!");
            break;
        }
        case Wait_QueryDstFreq2_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_QueryDstFreq2_Finish timeout!!!");
            break;
        }
        case Wait_QueryDstFreqDivide1_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_QueryDstFreqDivide1_Finish timeout!!!");
            break;
        }
        case Wait_QueryDstFreqDivide2_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_QueryDstFreqDivide2_Finish timeout!!!");
            break;
        }
//        case Wait_HardResetTest_Finish:
//        {
//            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_HardResetTest_Finish timeout!!!");
//            break;
//        }
//        case Wait_EraseFlash_Finish:
//        {
//            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_EraseFlash_Finish timeout!!!");
//            break;
//        }
        default:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==p_timer");
            break;
        }
    }
}

void Script_FreqManage_FreqAndFreqDivideSwitch::maxAllowTimer_timeoutProc()
{
      p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==p_maxAllowTimer");
//    switch(emScriptRunState)
//    {
//        case Wait_HardResetTest_Finish:
//        {
//            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_HardResetTest_Finish maxAllow timeout!!!");
//            break;
//        }
//        case Wait_EraseFlash_Finish:
//        {
//            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_EraseFlash_Finish maxAllow timeout!!!");
//            break;
//        }
//        default:
//        {
//            p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==p_maxAllowTimer");
//            break;
//        }
//    }
}

void Script_FreqManage_FreqAndFreqDivideSwitch::delayTimer_timeoutProc()
{
    p_delayTimer->stop();
//    if(emScriptRunState==Wait_SetInitFreq_Finish)
//    {
//        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "开始设置初始分频系数！");
//        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_SetFreqDivide_F0F50);
//        emScriptRunState=Wait_SetInitFreqDivide_Finish;
//        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--设置初始分频系数（F0F50），等待--确认");
//        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
//    }
//    else if(emScriptRunState==Wait_SetInitFreqDivide_Finish)
//    {
//        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "开始查询初始频段！");
//        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryFreq_03F16);
//        emScriptRunState=Wait_QueryInitFreq_Finish;
//        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询初始频段（03F16），等待--回复");
//        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
//    }
    if(emScriptRunState==Wait_SetDstFreqDivide2_Finish)
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("开始查询目的分频系数！"));
        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryFreqDivide_F0F51);
        emScriptRunState=Wait_QueryDstFreqDivide2_Finish;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("发送--查询目的分频系数（F0F51），等待--回复"));
        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);

    }
    else if(emScriptRunState==Wait_SetDstFreq0_Finish)
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "开始查询目的运行频段！");
        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryFreq_03F16);
        emScriptRunState=Wait_QueryDstFreq0_Finish;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询目的运行频段（03F16），等待--回复");
        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
    }
    else if(emScriptRunState==Wait_SetDstFreq2_Finish)
    {
        if(resultFlag==true)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "开始查询目的运行频段！");
            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryFreq_03F16);
            emScriptRunState=Wait_QueryDstFreq0_Finish;
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询目的运行频段（03F16），等待--回复");
            p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
        }
        else
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "开始查询目的运行频段！");
            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryFreq_03F16);
            emScriptRunState=Wait_QueryDstFreq2_Finish;
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询目的运行频段（03F16），等待--回复");
            p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
        }
    }
    else if(emScriptRunState==Wait_SetDstFreq1_Finish)
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "开始查询目的运行频段！");
        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryFreq_03F16);
        emScriptRunState=Wait_QueryDstFreq1_Finish;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询目的运行频段（03F16），等待--回复");
        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
    }
    else if(emScriptRunState==Wait_SetDstFreqDivide1_Finish)
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("开始查询目的分频系数！"));
        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryFreqDivide_F0F51);
        emScriptRunState=Wait_QueryDstFreqDivide1_Finish;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("发送--查询目的分频系数（F0F51），等待--回复"));
        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
    }
    else if(emScriptRunState==Wait_SetDstFreq2_ab_Finish)
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "开始查询目的运行频段！");
        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryFreq_03F16);
        emScriptRunState=Wait_QueryDstFreq2_Finish;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询目的运行频段（03F16），等待--回复");
        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
    }
}




