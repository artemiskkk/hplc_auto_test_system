#include "BuildNetwork_GW.h"

BuildNetwork_GW::BuildNetwork_GW(QObject *parent) : QObject(parent)
{
    emScriptRunState=Init;
    buildNetworkResultFlag=false;

    emDvcType = ThreeSTA;//由 SingleSTA 改为 ThreeSTA   by chubo 20200605

    sendParams.clear();
    idList.clear();
    idList.append(1);

    sendMsgOct.clear();

    p_MsgBase_3762 = make_shared<qgdw_3762_protocol::Frame3762Helper>();
    p_SetCcoAddr_05F1 = make_shared<qgdw_3762_protocol::Afn05F1>();
    p_HardInit_01F1 = make_shared<qgdw_3762_protocol::Afn01F1>();
    p_ParamInit_01F2 = make_shared<qgdw_3762_protocol::Afn01F2>();
    p_AddSlaveNode_11F1 = make_shared<qgdw_3762_protocol::Afn11F1>();
    p_SetFreq_05F16 = make_shared<qgdw_3762_protocol::Afn05F16>();
    p_QueryNetTopoInfo_10F21 = make_shared<qgdw_3762_protocol::Afn10F21>();
    p_QueryNoise_03F2=make_shared<Afn03F2>();
    p_SetRfPoint_03F17=make_shared<Afn03F17>();
    p_SetChannel_F0F37=make_shared<AfnF0F37>();
    p_MsgBase_645 = make_shared<dlt_645_Protocol::Frame645Helper>();
    p_MeterAddrResp_93 = make_shared<dlt_645_Protocol::RspsNormal_ReadAddr_0x93>(addr,6);

    p_GetResponseNormal_ReadAddr=make_shared<GetResponseNormal>();


    p_timer = make_shared<QTimer>();
    p_maxAllowTimer = make_shared<QTimer>();
    p_delayTimer = make_shared<QTimer>();

    connect(p_timer.get(),SIGNAL(timeout()),this,SLOT(timer_timeoutProc()));
    connect(p_maxAllowTimer.get(),SIGNAL(timeout()),this,SLOT(maxAllowTimer_timeoutProc()));
    connect(p_delayTimer.get(),SIGNAL(timeout()),this,SLOT(delayTimer_timeoutProc()));

    p_BuildNetworkDetect=make_shared<BuildNetworkDetect>();
    connect(p_BuildNetworkDetect.get(),&BuildNetworkDetect::signalSendRebuildNetFlag,this,&BuildNetwork_GW::slotSendRebuildNetFlag);
}
BuildNetwork_GW::~BuildNetwork_GW()
{
    if(p_BuildNetworkDetect!=nullptr)
    {
        p_BuildNetworkDetect->stop();
        disconnect(p_BuildNetworkDetect.get(),nullptr,this,nullptr); // 断开所有信号连接，防止对象销毁后信号仍被发送
    }
    if(p_timer!=nullptr) {
        p_timer->stop();
        disconnect(p_timer.get(),nullptr,this,nullptr);
    }
    if(p_maxAllowTimer!=nullptr) {
        p_maxAllowTimer->stop();
        disconnect(p_maxAllowTimer.get(),nullptr,this,nullptr);
    }
    if(p_delayTimer!=nullptr) {
        p_delayTimer->stop();
        disconnect(p_delayTimer.get(),nullptr,this,nullptr);
    }
}
void BuildNetwork_GW::execute()
{
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("当前脚本版本为251027-164800"));
    QSettings property(QString("ScriptConfig.ini"),QSettings::IniFormat);
    bool powerState=property.value("TEST_DEVICE_PROPERTY/PowerState").toBool();
    p_AbstractScriptHost->updateProgress(ProcessState_Processing,
                                         QString("工装上电状态：%1").arg(powerState ? "true" : "false"));
    if(p_BuildNetworkDetect->getIsNeedDetect()==true&&needRebuildNetwork==false)//&&powerState==true)//上电状态
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "等待组网探测完成");
        emScriptRunState=Wait_BuildNetworkDetect_Finish;
        p_BuildNetworkDetect->execute();
        return;
    }
    p_AbstractScriptHost->updateProgress(ProcessState_Start, QString("开始组网流程!!!"));

    isFirstChkTopo=true;
    roundIndex=1;

    // checkTopologyIntervalTime =(p_CtrInfoList->at(0)->totalNodeCnt<500)?(checkTopologyIntervalTime + p_CtrInfoList->at(0)->totalNodeCnt/100):(checkTopologyIntervalTime + 2*p_CtrInfoList->at(0)->totalNodeCnt/100);
    checkTopologyIntervalTime=60;
    emScriptRunState=Wait_SetBaudRate_Finish;
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("单人工装板_设置波特率，等待完成"));

    sendParams.clear();
    idList.clear();
    if(flagStaHighComBaud==false)
        sendParams.append(2400);
    else
        sendParams.append(115200);

    idList = getDvcIdList(emDvcType);
    p_AbstractScriptHost->controlDvc(emDvcType,idList,CtrlCmd_SetBaudRate,sendParams);
    p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
    tryTimes=0;
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("设置STA三通波特率，等待--确认"));//由 单通 改为 三通   by chubo 20200605
}
void BuildNetwork_GW::stop()
{
    p_timer->stop();
    p_maxAllowTimer->stop();
    p_delayTimer->stop();
}
//void BuildNetwork_NW::addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq)
//{
//    p_CtrInfoList->clear();
//    freqBand=freq&0xFF;
//    comnAddAddrsInfo(p_CtrInfoList, p_ConcentratorInfoList, p_MeterInfoList, p_SchemeCfgInfoList);
//    concentratorCnt=p_CtrInfoList->size()&0xFFFF;
//}
void BuildNetwork_GW::setHost(AbstractScriptHost *host)
{
    p_AbstractScriptHost=host;
    p_BuildNetworkDetect->setHost(host);
}
bool BuildNetwork_GW::config(const QMap<QString,QString> *paraDic)
{
    bool result = false;
    if(paraDic!=nullptr)
    {
        p_BuildNetworkDetect->config(paraDic);
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
        if(paraDic->keys().contains("freqBand"))
        {
            //            this->freqBand = (*paraDic)["freqBand"].toUShort();
        }
        if(paraDic->keys().contains("addCntPerTime"))
        {
            this->addCntPerTime = (*paraDic)["addCntPerTime"].toUShort()&0xff;

            if(this->addCntPerTime<=0)
                this->addCntPerTime=20;
        }
        if(paraDic->keys().contains("topoCntPerTime"))
        {
            this->topoCntPerTime = (*paraDic)["topoCntPerTime"].toUShort()&0xff;

            if(this->topoCntPerTime<=0)
                this->topoCntPerTime=20;
        }
        if(paraDic->keys().contains("channelType"))
        {
            this->needRebuildNetwork=true;
            this->channelType = (*paraDic)["channelType"];
        }
        result = true;
    }
    return result;
}
void BuildNetwork_GW::processMsg(DvcType dvcType, int id, uchar *data, int datalen)
{
    if(p_BuildNetworkDetect->getIsNeedDetect()==true&&needRebuildNetwork==false)
    {
        emScriptRunState=Wait_BuildNetworkDetect_Finish;
        p_BuildNetworkDetect->processMsg(dvcType,id,data,datalen);
        return;
    }
    // 安全拷贝数据，不删除原始data指针
    QByteArray recvTempData = QByteArray(reinterpret_cast<char*>(data), datalen);

    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到报文：%1").arg(QString(recvTempData.toHex())));

    if(p_CtrInfoList->size()==0)
        return;

    if(dvcType==CCO_GW || dvcType==CCO_NW)
    {
        if(dvcType == p_CtrInfoList->at(0)->slotPosition && id == p_CtrInfoList->at(0)->dvcId)
        {
            p_CtrInfoList->at(0)->buf.append(recvTempData);
            processMsgFromCco(dvcType,id);
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
                    //                    if(emScriptRunState==Wait_AssignAddrsFinish)
                    //                    {
                    //                        bool haveCompleteMsg=true;
                    //                        while(haveCompleteMsg)
                    //                        {
                    //                            shared_ptr<Frame645Base> MsgBase_645_ptr = Frame645Helper::DecodeLocalMsg(&((*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[p_CtrInfoList->at(0)->keyList.at(i)]->buf698),haveCompleteMsg);

                    //                            if(MsgBase_645_ptr!=nullptr)
                    //                            {
                    //                                processMsgFromMeterOOP(dvcType,id,p_CtrInfoList->at(0)->keyList.at(i));
                    //                            }
                    //                            break;
                    //                        }
                    //                    }
                    //                    else
                    processMsgFromMeterOOP(dvcType,id,p_CtrInfoList->at(0)->keyList.at(i));
                    break;
                }
            }
        }
    }
    else if(dvcType==CJQ)
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
    else
    {
        return;
    }
}
void BuildNetwork_GW::processCtrlDvcRes(DvcType dvcType, QList<int> idList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params)
{
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("组网接口收到控制命令回复，设备类型=%1，命令类型=%2，参数个数=%3").arg(dvcType).arg(ctrlCmdType).arg(QString::number(params.size())));

    if(isSucs==false)
        return;

    p_timer->stop();

    Refresh_CtrInfo_Result_for_CtrlCmdRes(p_CtrInfoList->at(0), dvcType, idList.at(0), ctrlCmdType);    //该接口屏蔽

    switch(emScriptRunState)
    {
    case Init:
    {
        break;
    }
    case Wait_SetBaudRate_Finish:
    {
        tryTimes=0;
        sendParams.clear();
        idList.clear();

        if(dvcType == ThreeSTA && emDvcType == ThreeSTA)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("三通槽位_设置波特率成功"));
            emDvcType = SingleSTA;
            if(flagStaHighComBaud==false)
                sendParams.append(2400);
            else
                sendParams.append(115200);

            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("单通槽位_设置波特率，等待--确认"));
        }
        else if (dvcType == SingleSTA && emDvcType == SingleSTA)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("单通槽位_设置波特率成功"));
            emDvcType = CCO_GW;

            sendParams.append(9600);

            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("国网路由槽位_设置波特率%0，等待--确认").arg(sendParams.at(0)));
        }
        else if (dvcType == CCO_GW && emDvcType == CCO_GW)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("国网路由槽位_设置波特率成功"));
            emDvcType = CCO_NW;

            sendParams.append(9600);

            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("南网路由槽位_设置波特率，等待--确认"));
        }
        else if (dvcType == CCO_NW && emDvcType == CCO_NW)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("南网路由槽位_设置波特率成功"));
            emDvcType = CJQ;

            if(flagCJQHighComBaud==false)
                sendParams.append(2400);
            else
                sendParams.append(9600);

            idList = getDvcIdList(emDvcType);
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("采集器槽位_设置波特率，等待--确认"));
        }
        else if (dvcType == CJQ && emDvcType == CJQ)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("采集器槽位_设置波特率成功"));

            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("单人工装板_设置波特率完成"));

            //            emScriptRunState=Wait_PowerOn12V_CCO_Finish;
            //            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("单人工装板_12V上电，等待完成"));
            //            emDvcType = p_CtrInfoList->at(0)->slotPosition;
            //            idList = getDvcIdList(emDvcType);

            //            p_AbstractScriptHost->controlDvc(emDvcType,idList,CtrlCmd_PowerOn_12V,sendParams);
            //            p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
            //            tryTimes=0;
            //            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("CCO_12V上电，等待--确认"));

            QThread::msleep(100);
            tryTimes=0;
            emScriptRunState=Wait_PowerOff12V_Finish;
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "单人工装板_12V断电，等待完成");

            emDvcType = ThreeSTA;
            sendParams.clear();
            idList.clear();

            idList = getDvcIdList(emDvcType);

            p_AbstractScriptHost->controlDvc(emDvcType,idList,CtrlCmd_PowerOff_12V,sendParams);
            p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "三通槽位_12V断电，等待--确认");//由 单通 改为 三通   by chubo 20200605
            break;
        }
        else
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("单人工装控制命令槽位回复错误，命令下发槽位=%1，命令回复槽位=%2").arg(emDvcType).arg(dvcType));
            p_AbstractScriptHost->updateProgress(ProcessState_Error, QString("单人工装控制命令槽位回复错误"));
            stop();
            powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost,true);
            break;
        }

        idList = getDvcIdList(emDvcType);
        p_AbstractScriptHost->controlDvc(emDvcType,idList,CtrlCmd_SetBaudRate,sendParams);
        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);

        break;
    }
    case Wait_PowerOff12V_Finish:
    {
        tryTimes=0;
        sendParams.clear();
        idList.clear();
        if(dvcType == ThreeSTA && emDvcType == ThreeSTA)//由 SingleSTA 改为 ThreeSTA   by chubo 20200605
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("三通槽位_12V断电成功"));//由 单通 改为 三通   by chubo 20200605
            emDvcType = SingleSTA;//由 SingleSTA 改为 ThreeSTA   by chubo 20200605

            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("单通槽位_12V断电，等待--确认"));//由 三通 改为 单通   by chubo 20200605
        }
        else if (dvcType == SingleSTA && emDvcType == SingleSTA)//由 ThreeSTA 改为 SingleSTA   by chubo 20200605
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("单通槽位_12V断电成功"));//由 三通 改为 单通   by chubo 20200605
            emDvcType = CCO_GW;

            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("国网路由槽位_12V断电，等待--确认"));
        }
        else if (dvcType == CCO_GW && emDvcType == CCO_GW)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("国网路由槽位_12V断电成功"));
            emDvcType = CCO_NW;

            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("南网路由槽位_12V断电，等待--确认"));
        }
        else if (dvcType == CCO_NW && emDvcType == CCO_NW)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("南网路由槽位_12V断电成功"));
            emDvcType = CJQ;

            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("采集器槽位_220V断电，等待--确认"));
        }
        else if (dvcType == CJQ && emDvcType == CJQ)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("采集器槽位_220V断电成功"));
            emDvcType = ThreeSTA;//由 SingleSTA 改为 ThreeSTA   by chubo 20200605
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("单人工装板_12V断电完成"));

            //            emScriptRunState=Wait_SetBaudRate_Finish;
            //            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("单人工装板_设置波特率，等待完成"));

            //            sendParams.append(2400);

            //            idList = getDvcIdList(emDvcType);
            //            p_AbstractScriptHost->controlDvc(emDvcType,idList,CtrlCmd_SetBaudRate,sendParams);
            //            p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
            //            tryTimes=0;
            //            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("设置STA单通波特率，等待--确认"));

            QThread::msleep(1000);
            emScriptRunState=Wait_PowerOn12V_CCO_Finish;
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("单人工装板_12V上电，等待完成"));
            emDvcType = p_CtrInfoList->at(0)->slotPosition;
            sendParams.clear();
            idList.clear();
            idList = getDvcIdList(emDvcType);

            p_AbstractScriptHost->controlDvc(emDvcType,idList,CtrlCmd_PowerOn_12V,sendParams);
            p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
            tryTimes=0;
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("CCO_12V上电，等待--确认"));
            break;
        }
        else
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("单人工装控制命令槽位回复错误，命令下发槽位=%1，命令回复槽位=%2").arg(emDvcType).arg(dvcType));
            p_AbstractScriptHost->updateProgress(ProcessState_Error, QString("单人工装控制命令槽位回复错误"));
            stop();
            powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost,true);
            break;
        }

        idList = getDvcIdList(emDvcType);
        if(emDvcType==CJQ)
            p_AbstractScriptHost->controlDvc(emDvcType,idList,CtrlCmd_PowerOff_220V,sendParams);
        else
            p_AbstractScriptHost->controlDvc(emDvcType,idList,CtrlCmd_PowerOff_12V,sendParams);
        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);

        break;
    }
    case Wait_PowerOn12V_CCO_Finish:
    {
        tryTimes=0;
        sendParams.clear();
        idList.clear();

        if (dvcType == p_CtrInfoList->at(0)->slotPosition)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("CCO_12V上电成功"));

            emScriptRunState=Wait_PowerOn12V_STA_Finish;

            temp_DvcType = findDvcTypeToPowerOnOrRst();

            QString str = QString("12V上电");
            findDvcToPowerOnOrRst(str);
        }
        else
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("单人工装控制命令槽位回复错误，命令下发槽位=%1，命令回复槽位=%2").arg(p_CtrInfoList->at(0)->slotPosition).arg(dvcType));
            p_AbstractScriptHost->updateProgress(ProcessState_Error, QString("单人工装控制命令槽位回复错误"));
            stop();
            powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost,true);
            break;
        }
        break;
    }
    case Wait_PowerOn12V_STA_Finish:
    {
        tryTimes=0;
        sendParams.clear();
        idList.clear();

        if (dvcType == emDvcType)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("STA_12V上电成功"));

            QString str = QString("12V上电");
            findDvcToPowerOnOrRst(str);
        }
        else
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("单人工装控制命令槽位回复错误，命令下发槽位=%1，命令回复槽位=%2").arg(emDvcType).arg(dvcType));
            p_AbstractScriptHost->updateProgress(ProcessState_Error, QString("单人工装控制命令槽位回复错误"));
            stop();
            powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost,true);
            break;
        }

        break;
    }
    case Wait_RST_CCO_Finish:
    {
        tryTimes=0;

        sendParams.clear();
        idList.clear();

        if (dvcType == p_CtrInfoList->at(0)->slotPosition)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("CCO_复位成功"));

            emScriptRunState=Wait_RST_STA_Finish;

            temp_DvcType = findDvcTypeToPowerOnOrRst();

            QString str = QString("复位");
            findDvcToPowerOnOrRst(str);
        }
        else
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("单人工装控制命令槽位回复错误，命令下发槽位=%1，命令回复槽位=%2").arg(p_CtrInfoList->at(0)->slotPosition).arg(dvcType));
            p_AbstractScriptHost->updateProgress(ProcessState_Error, QString("单人工装控制命令槽位回复错误"));
            stop();
            powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost,true);
            break;
        }
        break;
    }
    case Wait_RST_STA_Finish:
    {
        tryTimes=0;

        sendParams.clear();
        idList.clear();

        if (dvcType == emDvcType)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("STA_复位成功"));

            QString str = QString("复位");
            findDvcToPowerOnOrRst(str);
        }
        else
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("单人工装控制命令槽位回复错误，命令下发槽位=%1，命令回复槽位=%2").arg(emDvcType).arg(dvcType));
            p_AbstractScriptHost->updateProgress(ProcessState_Error, QString("单人工装控制命令槽位回复错误"));
            stop();
            powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost,true);
            break;
        }

        break;
    }
    case Wait_AssignAddrsFinish:
    {
        break;
    }
    case Wait_00F1_for_05F1_SetCcoAddr:
    {
        emScriptRunState=Wait_00F1_for_01F2_ParamInit;
        break;
    }
    case Wait_00F1_for_01F2_ParamInit:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("CCO槽位切换115200bps成功"));
        currentCCOBaud=115200;
        // 使用 QPointer 防止对象销毁后 lambda 访问野指针导致崩溃
        QPointer<BuildNetwork_GW> guard(this);
        QTimer::singleShot(5000,this,[this, guard]
        {
            // 检查对象是否已被销毁
            if (!guard) {
                qWarning() << "BuildNetwork_GW已销毁，取消singleShot回调";
                return;
            }
            tryTimes=6;
            p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            emScriptRunState=Wait_00F1_for_01F2_ParamInit;
            sendMsg(p_CtrInfoList->at(0)->slotPosition,p_CtrInfoList->at(0)->dvcId,INSIGNIFICANCE,p_ParamInit_01F2);
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("1-发送--参数初始化（01F2），等待--确认"));
        });
        break;
    }
    case Wait_00F1_for_01F1_HardInit:
    {
        break;
    }
    case Wait_00F1_for_05F16_SetFreq:
    {
        break;
    }
    case Wait_AddMetersFinish:
    {
        break;
    }
    case Wait_BuildNetFinish:
    {
        break;
    }
    case BuildNetFinish:
    {
        break;
    }
    default:
    {
        //        p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==emScriptRunState");
        break;
    }
    }
}

void BuildNetwork_GW::setCtrInfoListAndFreq(shared_ptr<QList<shared_ptr<CtrInfo>>> p_CtrInfoList, uchar freq)
{
    this->concentratorCnt=p_CtrInfoList->size()&0xFFFF;
    this->p_CtrInfoList=p_CtrInfoList;
    this->freqBand=freq;

    if(!p_CtrInfoList->isEmpty())
        p_BuildNetworkDetect->addAddrsInfo(p_CtrInfoList->at(0));//当前单人工装板只考虑一个CCO
}

void BuildNetwork_GW::initBuildNetWork()
{
    if(p_BuildNetworkDetect!=nullptr)
        p_BuildNetworkDetect->stop();
    p_timer->stop();
    disconnect(p_timer.get(),nullptr,this,nullptr);
    p_maxAllowTimer->stop();
    disconnect(p_maxAllowTimer.get(),nullptr,this,nullptr);
    p_delayTimer->stop();
    disconnect(p_delayTimer.get(),nullptr,this,nullptr);
    
    // 清空CCO和STA的报文缓冲区，防止残留数据干扰帧解析
    if(p_CtrInfoList && !p_CtrInfoList->isEmpty() && p_CtrInfoList->at(0))
    {
        p_CtrInfoList->at(0)->buf.clear();
        p_CtrInfoList->at(0)->bufReadCtrlDvc.clear();
        // 清空所有电表的缓冲区
        if(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList)
        {
            for(auto& meterInfo : p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values())
            {
                if(meterInfo)
                {
                    meterInfo->buf645.clear();
                    meterInfo->buf698.clear();
                }
            }
        }
    }
}

void BuildNetwork_GW::timer_timeoutProc()
{
    p_timer->stop();
    switch(emScriptRunState)
    {
    case Wait_PowerOff12V_Finish:
    {
        if(++tryTimes>=3)
        {
            p_maxAllowTimer->stop();
            p_delayTimer->stop();
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_PowerOff12V_Finish timeout!!!");
        }
        else
        {
            p_AbstractScriptHost->controlDvc(emDvcType,idList,CtrlCmd_PowerOff_12V,sendParams);
            p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("重发12V断电，等待--确认"));
        }
        break;
    }
    case Wait_SetBaudRate_Finish:
    {
        if(++tryTimes>=3)
        {
            p_maxAllowTimer->stop();
            p_delayTimer->stop();
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_SetBaudRate_Finish timeout!!!");
        }
        else
        {
            p_AbstractScriptHost->controlDvc(emDvcType,idList,CtrlCmd_SetBaudRate,sendParams);
            p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("重发设置波特率，等待--确认"));
        }
        break;
    }
    case Wait_PowerOn12V_CCO_Finish:
    {
        if(++tryTimes>=3)
        {
            p_maxAllowTimer->stop();
            p_delayTimer->stop();
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_PowerOn12V_CCO_Finish timeout!!!");
        }
        else
        {
            p_AbstractScriptHost->controlDvc(emDvcType,idList,CtrlCmd_PowerOn_12V,sendParams);
            p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("重发CCO_12V上电，等待--确认"));
        }
        break;
    }
    case Wait_PowerOn12V_STA_Finish:
    {
        if(++tryTimes>=3)
        {
            p_maxAllowTimer->stop();
            p_delayTimer->stop();
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_PowerOn12V_STA_Finish timeout!!!");
        }
        else
        {
            p_AbstractScriptHost->controlDvc(emDvcType,idList,CtrlCmd_PowerOn_12V,sendParams);
            p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("重发STA_12V上电，等待--确认"));
        }
        break;
    }
    case Wait_RST_CCO_Finish:
    {
        if(++tryTimes>=3)
        {
            p_maxAllowTimer->stop();
            p_delayTimer->stop();
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_RST_CCO_Finish timeout!!!");
        }
        else
        {
            p_AbstractScriptHost->controlDvc(emDvcType,idList,CtrlCmd_ModuleRST,sendParams);
            p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("重发CCO_模块复位，等待--确认"));
        }
        break;
    }
    case Wait_RST_STA_Finish:
    {
        if(++tryTimes>=3)
        {
            p_maxAllowTimer->stop();
            p_delayTimer->stop();
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_RST_STA_Finish timeout!!!");
        }
        else
        {
            p_AbstractScriptHost->controlDvc(emDvcType,idList,CtrlCmd_ModuleRST,sendParams);
            p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("重发STA_模块复位，等待--确认"));
        }
        break;
    }
    case Wait_AssignAddrsFinish:
    {
        p_maxAllowTimer->stop();
        p_delayTimer->stop();
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_AssignAddrsFinish timeout!!!");
        break;
    }
    case Wait_00F1_for_05F1_SetCcoAddr:
    {
        if(++tryTimes>=3)
        {
            p_maxAllowTimer->stop();
            p_delayTimer->stop();
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("设置主节点地址;尝试3次;超时失败!!!"));
        }
        else
        {
            p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            sendMsg(p_CtrInfoList->at(0)->slotPosition,p_CtrInfoList->at(0)->dvcId,INSIGNIFICANCE,p_SetCcoAddr_05F1);
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("发送--设置主节点地址（05F1），等待--确认"));
        }
        break;
    }
    case Wait_00F1_for_01F2_ParamInit:
    {
        ++tryTimes;
        if(tryTimes>=3&&tryTimes<6)
        {
            idList.clear();
            sendParams.clear();
            sendParams.append(115200);
            idList = getDvcIdList(CCO_GW);
            p_AbstractScriptHost->controlDvc(CCO_GW,idList,CtrlCmd_SetBaudRate,sendParams);
            p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("设置CCO槽位波特率为115200bps，等待--确认"));
        }
        else if(tryTimes>=9)
        {
            p_maxAllowTimer->stop();
            p_delayTimer->stop();
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("参数初始化;切换到115200bps,尝试3次;超时失败!!!"));
        }
        else
        {
            p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            sendMsg(p_CtrInfoList->at(0)->slotPosition,p_CtrInfoList->at(0)->dvcId,INSIGNIFICANCE,p_ParamInit_01F2);
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("2-发送--参数初始化（01F2），等待--确认"));
        }
        break;
    }
    case Wait_00F1_for_01F1_HardInit:
    {
        if(++tryTimes>=3)
        {
            p_maxAllowTimer->stop();
            p_delayTimer->stop();
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("硬件初始化;尝试3次;超时失败!!!"));
        }
        else
        {
            p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            sendMsg(p_CtrInfoList->at(0)->slotPosition,p_CtrInfoList->at(0)->dvcId,INSIGNIFICANCE,p_HardInit_01F1);
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("发送--硬件初始化（01F1），等待--确认"));
        }
        break;
    }
    case Wait_00F1_for_05F16_SetFreq:
    {
        if(++tryTimes>=3)
        {
            p_maxAllowTimer->stop();
            p_delayTimer->stop();
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("设置频段;尝试3次;超时失败!!!"));
        }
        else
        {
            p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            sendMsg(p_CtrInfoList->at(0)->slotPosition,p_CtrInfoList->at(0)->dvcId,INSIGNIFICANCE,p_SetFreq_05F16);
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("发送--设置频段命令（05F16），等待--确认"));
        }
        break;
    }
    case Wait_00F1_for_F0F37_SetChannel:
    {
        if(++tryTimes>=3)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("设置路由信道;尝试3次;超时失败!!!"));
             p_delayTimer->start(6*1000);
        }
        else
        {
            p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            sendMsg(p_CtrInfoList->at(0)->slotPosition,p_CtrInfoList->at(0)->dvcId,INSIGNIFICANCE,p_SetChannel_F0F37);
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("发送--设置路由信道（F0F37），等待--确认"));
        }
        break;
    }

    case Wait_AddMetersFinish:
    {
        if(++tryTimes>=3)
        {
            p_maxAllowTimer->stop();
            p_delayTimer->stop();
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("添加从节点;尝试3次;超时失败!!!"));
        }
        else
        {
            index=0;
            p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            sendMsg(p_CtrInfoList->at(0)->slotPosition,p_CtrInfoList->at(0)->dvcId,INSIGNIFICANCE,p_AddSlaveNode_11F1);
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("发送--添加从节点命令（11F1），等待--确认"));
        }
        break;
    }
    case Wait_BuildNetFinish:
    {
        if(++index>=times)
        {
            index=0;
        }
        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
        sendMsg(p_CtrInfoList->at(0)->slotPosition,p_CtrInfoList->at(0)->dvcId,INSIGNIFICANCE,p_QueryNetTopoInfo_10F21);
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("报文接收超时，发送--查询网络拓扑信息（10F21），等待--确认"));
        break;
    }
    default:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==p_timer");
        break;
    }
    }
}

void BuildNetwork_GW::maxAllowTimer_timeoutProc()
{
    p_delayTimer->stop();
    p_timer->stop();
    p_maxAllowTimer->stop();
    switch(emScriptRunState)
    {
    case Wait_BuildNetFinish:
    {
        //        if(haveStartContinueTimer==false)
        //        {
        //            QString failedMeter = GenerateFailedMeterStr_Net(p_CtrInfoList->at(0));
        //            p_AbstractScriptHost->updateProgress(ProcessState_Processing, failedMeter);
        //            p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("  全网组网成功率：%1%；失败表列表：").arg(p_CtrInfoList->at(0)->inNetSuccessRate*100) + failedMeter);
        //        }
        //        else
        //        {
        QString failedMeter = GenerateFailedMeterStr_Net(p_CtrInfoList->at(0));
        if(p_CtrInfoList->at(0)->inNetSuccessRate>=netSucRateThresld)
        {
            buildNetworkResultFlag=true;
            //     p_CtrInfoList->at(0)->inNetConsume=static_cast<double>((havePassedTimeLen+timerAfterReachThresld)*1000)/1000.0;
            p_CtrInfoList->at(0)->inNetResult=true;
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("组网时间到，成功率%1%达到要求，").arg(p_CtrInfoList->at(0)->inNetSuccessRate*100) + failedMeter);
            emScriptRunState=BuildNetFinish;
            //sendMsg(p_CtrInfoList->at(0)->slotPosition,p_CtrInfoList->at(0)->dvcId,INSIGNIFICANCE,p_ChkNetTopoInfo_F0_06F003E8_Down);
            //   emScriptRunState=BuildNetFinish;
            //   resultFlag=true;
            sendMsg(CCO_GW,1,INSIGNIFICANCE,p_QueryNoise_03F2);
        }
        else
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("组网时间到，成功率%1%未达到要求，").arg(p_CtrInfoList->at(0)->inNetSuccessRate*100) + failedMeter);
        }
        break;
    }
    default:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==p_maxAllowTimer");
        break;
    }
    }
}
void BuildNetwork_GW::delayTimer_timeoutProc()
{
    p_delayTimer->stop();
    switch(emScriptRunState)
    {
    case Wait_00F1_for_01F2_ParamInit:
    {
        emScriptRunState=Wait_00F1_for_05F1_SetCcoAddr;
        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
        tryTimes=0;
        sendMsg(p_CtrInfoList->at(0)->slotPosition,p_CtrInfoList->at(0)->dvcId,INSIGNIFICANCE,p_SetCcoAddr_05F1);
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("发送--设置主节点地址（05F1），等待--确认"));
        break;
    }
    case Wait_00F1_for_01F1_HardInit:
    {
        sendMsg(p_CtrInfoList->at(0)->slotPosition,p_CtrInfoList->at(0)->dvcId,INSIGNIFICANCE,p_SetFreq_05F16);
        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
        tryTimes=0;
        emScriptRunState=Wait_00F1_for_05F16_SetFreq;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("发送--设置频段命令（05F16），等待--确认"));
        break;
    }
    case Wait_00F1_for_05F16_SetFreq:
    {
//        if(channelType.isEmpty())
//        {
//            if(p_CtrInfoList->at(0)->totalNodeCnt==0)
//            {
//                p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("档案为空,无需下发档案及等待组网完成"));

//                buildNetworkResultFlag=true;

//                p_CtrInfoList->at(0)->inNetConsume=0;

//                p_CtrInfoList->at(0)->inNetResult=true;
//                p_maxAllowTimer->stop();
//                //   p_delayTimer->stop();

//                tryTimes=0;
//                emScriptRunState=BuildNetFinish;
//                sendMsg(p_CtrInfoList->at(0)->slotPosition,p_CtrInfoList->at(0)->dvcId,INSIGNIFICANCE,p_QueryNetTopoInfo_10F21);

//                return;
//            }
//            times=(p_CtrInfoList->at(0)->totalNodeCnt%addCntPerTime)?p_CtrInfoList->at(0)->totalNodeCnt/addCntPerTime+1:p_CtrInfoList->at(0)->totalNodeCnt/addCntPerTime;
//            sendMsg(p_CtrInfoList->at(0)->slotPosition,p_CtrInfoList->at(0)->dvcId,INSIGNIFICANCE,p_AddSlaveNode_11F1);

//            tryTimes=0;
//            emScriptRunState=Wait_AddMetersFinish;
//            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("等待下发档案完成"));
//            p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
//            break;
//        }
//        else
//        {
            sendMsg(p_CtrInfoList->at(0)->slotPosition,p_CtrInfoList->at(0)->dvcId,INSIGNIFICANCE,p_SetChannel_F0F37);
            p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            tryTimes=0;
            emScriptRunState=Wait_00F1_for_F0F37_SetChannel;
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("发送--设置路由信道命令（F0F37），等待--确认"));
            break;
//        }
    }
    case Wait_00F1_for_F0F37_SetChannel:
    {
        if(p_CtrInfoList->at(0)->totalNodeCnt==0)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("档案为空,无需下发档案及等待组网完成"));

            buildNetworkResultFlag=true;

            p_CtrInfoList->at(0)->inNetConsume=0;

            p_CtrInfoList->at(0)->inNetResult=true;
            p_maxAllowTimer->stop();
            //   p_delayTimer->stop();

            tryTimes=0;
            emScriptRunState=BuildNetFinish;
            sendMsg(p_CtrInfoList->at(0)->slotPosition,p_CtrInfoList->at(0)->dvcId,INSIGNIFICANCE,p_QueryNetTopoInfo_10F21);

            return;
        }
        times=(p_CtrInfoList->at(0)->totalNodeCnt%addCntPerTime)?p_CtrInfoList->at(0)->totalNodeCnt/addCntPerTime+1:p_CtrInfoList->at(0)->totalNodeCnt/addCntPerTime;
        sendMsg(p_CtrInfoList->at(0)->slotPosition,p_CtrInfoList->at(0)->dvcId,INSIGNIFICANCE,p_AddSlaveNode_11F1);

        tryTimes=0;
        emScriptRunState=Wait_AddMetersFinish;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("等待下发档案完成"));
        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
        break;
    }

    case Wait_BuildNetFinish:
    {
        if(isFirstChkTopo)
        {
            isFirstChkTopo=false;
            startBuildNetFlag=true;
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("开始第(%1)轮查询网络拓扑").arg(roundIndex));
            sendMsg(p_CtrInfoList->at(0)->slotPosition,p_CtrInfoList->at(0)->dvcId,INSIGNIFICANCE,p_QueryNetTopoInfo_10F21);
            p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            tryTimes=0;
        }
        else
        {
            roundIndex+=1;
            //                if(roundIndex==6)
            //                {
            //                    startBuildNetFlag=true;
            //                }
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("开始第(%1)轮查询网络拓扑").arg(roundIndex));
            sendMsg(p_CtrInfoList->at(0)->slotPosition,p_CtrInfoList->at(0)->dvcId,INSIGNIFICANCE,p_QueryNetTopoInfo_10F21);
            p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
        }
        break;
    }
    default:
        break;
    }
}

void BuildNetwork_GW::slotSendRebuildNetFlag(bool isNeedRebuild)
{
    p_BuildNetworkDetect->stop();
    if(isNeedRebuild)
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Start, QString("开始组网流程!!!"));

        isFirstChkTopo=true;
        roundIndex=1;

        // checkTopologyIntervalTime =(p_CtrInfoList->at(0)->totalNodeCnt<500)?(checkTopologyIntervalTime + p_CtrInfoList->at(0)->totalNodeCnt/100):(checkTopologyIntervalTime + 2*p_CtrInfoList->at(0)->totalNodeCnt/100);
        checkTopologyIntervalTime=60;
        emScriptRunState=Wait_SetBaudRate_Finish;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("单人工装板_设置波特率，等待完成"));

        sendParams.clear();
        idList.clear();
        if(flagStaHighComBaud==false)
            sendParams.append(2400);
        else
            sendParams.append(115200);

        idList = getDvcIdList(emDvcType);
        p_AbstractScriptHost->controlDvc(emDvcType,idList,CtrlCmd_SetBaudRate,sendParams);
        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
        tryTimes=0;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("设置STA三通波特率，等待--确认"));//由 单通 改为 三通   by chubo 20200605
    }
    else
    {
        emScriptRunState=BuildNetFinish;
        startBuildNetFlag=true;
        buildNetworkResultFlag=true;
        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryNoise_03F2);//后者查询03F10
    }
}

void BuildNetwork_GW::processMsgFromCco(DvcType dvcType, int dvcId)
{
    if(p_AbstractScriptHost == nullptr || p_CtrInfoList == nullptr || p_CtrInfoList->size() == 0
            || p_CtrInfoList->at(0) == nullptr || p_MsgBase_3762 == nullptr)
    {
        if(p_AbstractScriptHost != nullptr)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Error, "BuildNetwork_GW invalid context in processMsgFromCco");
        }
        return;
    }

    QByteArray completeFrame;
    QByteArray& buf = p_CtrInfoList->at(0)->buf;
    bool haveCompleteMsg = extractAndProcess3762Frame(buf, completeFrame);

    while(haveCompleteMsg)
    {
        shared_ptr<Frame3762Base> p_Frame3762Base = nullptr;
        
        // 增加异常捕获，防止解析错误崩溃
        try {
            p_Frame3762Base = p_MsgBase_3762->DecodeLocalMsg(&completeFrame, haveCompleteMsg);
        }
        catch(...) {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, 
                QString("3762帧解析异常,跳过此帧: %1").arg(QString(completeFrame.toHex())));
            // 继续处理下一帧
            haveCompleteMsg = extractAndProcess3762Frame(buf, completeFrame);
            continue;
        }

        if(p_Frame3762Base==nullptr)
        {
             p_AbstractScriptHost->updateProgress(ProcessState_Processing, 
                QString("3762帧解析失败,跳过此帧: %1").arg(QString(completeFrame.toHex())));
             // 继续处理下一帧
             haveCompleteMsg = extractAndProcess3762Frame(buf, completeFrame);
             continue;
        }


        if(p_Frame3762Base==nullptr)
        {
            continue;
        }

        //        if(haveCompleteMsg==false)
        //        {
        //            break;
        //        }

        switch(emScriptRunState)
        {

        case Wait_00F1_for_01F2_ParamInit:
        {
            if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp )
            {
                p_timer->stop();

                p_delayTimer->start(6*1000);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到确认-参数初始化，等待6秒"));
                emScriptRunState=Wait_00F1_for_05F1_SetCcoAddr;
                if(flagPowerOnCJQ==true)
                    powerOn220V_CJQ(p_CtrInfoList,p_AbstractScriptHost);

                //                sendMsg(dvcType,dvcId,INSIGNIFICANCE,p_HardInit_01F1);
                //                p_timer->start(10*1000);
                //                tryTimes=0;
                //                emScriptRunState=Wait_00F1_for_01F1_HardInit;
                //                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--硬件初始化命令（01F1），等待--确认");
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }

            break;
        }
        case Wait_00F1_for_05F1_SetCcoAddr:
        {
            if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp )
            {
                p_timer->stop();
                sendMsg(dvcType,dvcId,INSIGNIFICANCE,p_HardInit_01F1);
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                tryTimes=0;
                emScriptRunState=Wait_00F1_for_01F1_HardInit;
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到确认-发送硬件初始化（01F1），等待--确认"));
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case Wait_00F1_for_01F1_HardInit:
        {
            if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();

                p_delayTimer->start(6*1000);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到确认-硬件初始化，等待6秒"));
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }

            break;
        }
        case Wait_00F1_for_05F16_SetFreq:
        {
            if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir==kDirUp)
            {
                p_timer->stop();
                p_delayTimer->start(6*1000);

                p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到确认-设置频段，等待6秒"));

            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }

            break;
        }
        case Wait_00F1_for_F0F37_SetChannel:
        {
            if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir==kDirUp)
            {
                p_timer->stop();

                p_delayTimer->start(6*1000);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到确认-设置频段，等待6秒"));

            }
           else if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x02&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir==kDirUp)
            {
                p_timer->stop();

                p_delayTimer->start(6*1000);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到确认-设置频段，等待6秒"));

            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }

            break;
        }

        case Wait_AddMetersFinish:
        {
            if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                if(p_timer == nullptr || p_delayTimer == nullptr || p_maxAllowTimer == nullptr || p_QueryNoise_03F2 == nullptr)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Error, "BuildNetwork_GW invalid timer/frame object in Wait_AddMetersFinish");
                    return;
                }
                p_timer->stop();

                if(++index==times)
                {
                    index=0;
                    if(topoCntPerTime == 0)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Error, "BuildNetwork_GW topoCntPerTime is zero");
                        return;
                    }
                    times=((p_CtrInfoList->at(0)->totalNodeCnt+1)%topoCntPerTime)?(p_CtrInfoList->at(0)->totalNodeCnt+1)/topoCntPerTime+1:(p_CtrInfoList->at(0)->totalNodeCnt+1)/topoCntPerTime;
                    emScriptRunState = Wait_BuildNetFinish;
                    startBuildNetFlag=true;
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryNoise_03F2);//后者查询03F10
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("下发档案完毕，等待组网完成，20s后开始查询拓扑"));//.arg(checkTopologyIntervalTime)
                    /// \brief powerOn220V_CJQ
                    p_delayTimer->start(20*1000);

                    if(haveStartContinueTimer==false)
                    {
                        int maxTimerMs = timerForReachThresld*1000-2000;
                        if(maxTimerMs < 1)
                            maxTimerMs = 1;
                        p_maxAllowTimer->start(maxTimerMs);
                    }
                    else
                    {
                        int maxTimerMs = (timerForReachThresld+timerAfterReachThresld)*1000-2000;
                        if(maxTimerMs < 1)
                            maxTimerMs = 1;
                        p_maxAllowTimer->start(maxTimerMs);
                    }
                }
                else
                {
                    sendMsg(dvcType,dvcId,INSIGNIFICANCE,p_AddSlaveNode_11F1);
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                }
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case Wait_BuildNetFinish:
        {
            if(p_Frame3762Base->afn_==0x10&&p_Frame3762Base->dt1_==0x10&&p_Frame3762Base->dt2_==0x02&&p_Frame3762Base->ctrl_field_.dir==kDirUp)
            {
                p_timer->stop();
                shared_ptr<Afn10F21> p_ResTopoInfo_Afn10F21=std::dynamic_pointer_cast<Afn10F21>(p_Frame3762Base);
                Refresh_CtrInfo_Result_for_BuildNet(index, p_CtrInfoList->at(0), p_ResTopoInfo_Afn10F21);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("当前组网成功率：%1%").arg(p_CtrInfoList->at(0)->inNetSuccessRate*100));
                //                if(p_CtrInfoList->at(0)->inNetSuccessRate>=netSucRateThresld)
                //                {
                if(p_CtrInfoList->at(0)->inNetSuccessRate==1.0)
                {
                    buildNetworkResultFlag=true;
                    if(haveStartContinueTimer==false)
                    {
                        p_CtrInfoList->at(0)->inNetConsume=double(timerForReachThresld*1000-p_maxAllowTimer->remainingTime())/1000.0;
                    }
                    else
                    {
                        p_CtrInfoList->at(0)->inNetConsume=double(havePassedTimeLen*1000+timerAfterReachThresld*1000-p_maxAllowTimer->remainingTime())/1000.0;
                    }

                    p_CtrInfoList->at(0)->inNetResult=true;
                    p_maxAllowTimer->stop();
                    p_delayTimer->stop();
                    //                    QString failedMeter = GenerateFailedMeterStr_Net(p_CtrInfoList->at(0));
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("组网完成，耗时%1s").arg(p_CtrInfoList->at(0)->inNetConsume));
                    tryTimes=0;
                    emScriptRunState=BuildNetFinish;
                    sendMsg(dvcType,dvcId,INSIGNIFICANCE,p_QueryNoise_03F2);//后者查询03F10
                    //emit signalBuildNetFinish();
                }
                else
                {
                    //                    if(haveStartContinueTimer==false)
                    //                    {
                    //                        havePassedTimeLen=double(timerForReachThresld*1000-p_maxAllowTimer->remainingTime())/1000.0;
                    //                        p_maxAllowTimer->start(timerAfterReachThresld*1000-2000);
                    //                        haveStartContinueTimer=true;
                    //                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("  组网成功率达到门限值：%1%; 继续等待时长：%2秒;").arg(p_CtrInfoList->at(0)->inNetSuccessRate*100).arg(timerAfterReachThresld));
                    //                    }
                    if(++index>=times)
                    {
                        index=0;
                    }
                    if(index==0)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("等待%1s后继续下一轮查询拓扑").arg(checkTopologyIntervalTime));
                        p_delayTimer->start(checkTopologyIntervalTime*1000);
                    }
                    else
                    {
                        sendMsg(dvcType,dvcId,INSIGNIFICANCE,p_QueryNetTopoInfo_10F21);
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
        case BuildNetFinish:
        {
            QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
            sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            break;
        }
        default:
        {
            QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
            sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            break;
        }
        }
         // 【修改结束】：循环尾部继续提取下一帧
        haveCompleteMsg = extractAndProcess3762Frame(buf, completeFrame);
    }
}
void BuildNetwork_GW::processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID)
{
    QString stringDvcType;
    if(dvcType == SingleSTA)
    {
        stringDvcType = QString("单通;");
    }
    else
    {
        stringDvcType = QString("三通;");
    }

    bool haveCompleteMsg=true;
    while(haveCompleteMsg)
    {
        //qInfo()<<stringDvcType + QString("解析前 buf645=%1").arg(QString((*p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList)[mtrlID]->buf645.toHex()));

        shared_ptr<dlt_645_Protocol::Frame645Base> MsgBase_645_ptr = dlt_645_Protocol::Frame645Helper::DecodeLocalMsg(&((*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[mtrlID]->buf645),haveCompleteMsg);
        //qInfo()<<stringDvcType + QString("解析后 buf645=%1").arg(QString((*p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList)[mtrlID]->buf645.toHex()));

        if(MsgBase_645_ptr==nullptr)
        {
            break;
        }
        //        if(haveCompleteMsg==false)
        //        {
        //            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("haveCompleteMsg==false"));

        //            break;
        //        }

        switch(emScriptRunState)
        {
        case Init:
        {
            break;
        }
        case Wait_PowerOff12V_Finish:
        {
            break;
        }
        case Wait_SetBaudRate_Finish:
        {
            break;
        }
        case Wait_PowerOn12V_CCO_Finish:
        {
            break;
        }
        case Wait_PowerOn12V_STA_Finish:
        {
            break;
        }
        case Wait_RST_CCO_Finish:
        {
            break;
        }
        case Wait_RST_STA_Finish:
        {
            break;
        }
        case Wait_AssignAddrsFinish:
        {
            if(MsgBase_645_ptr->ctrlCode_==dlt_645_Protocol::READ_ADDR)
            {
                sendMsg(dvcType,dvcId,mtrlID,p_MeterAddrResp_93);
                Refresh_CtrInfo_Result_for_AssignAddr(p_CtrInfoList->at(0),mtrlID);

                if(p_CtrInfoList->at(0)->sucsRate_CtrlCmd[0]>=1.0)
                {
                    sendMsg(p_CtrInfoList->at(0)->slotPosition,p_CtrInfoList->at(0)->dvcId,INSIGNIFICANCE,p_ParamInit_01F2);
                    tryTimes=0;
                    emScriptRunState=Wait_00F1_for_01F2_ParamInit;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "3-发送--参数初始化（01F2），等待--确认");
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                }
            }
            else
            {
                uchar di[4]={0x00};
                if(MsgBase_645_ptr->ctrlCode_==dlt_645_Protocol::READ_DATA)
                {
                    shared_ptr<dlt_645_Protocol::Rqst_ReadData_0x11> Rqst_ReadData_0x11_ptr = std::dynamic_pointer_cast<dlt_645_Protocol::Rqst_ReadData_0x11>(MsgBase_645_ptr);
                    memcpy(di,Rqst_ReadData_0x11_ptr->di,4);

                    QByteArray tmpSendMsg=prcsOther645Msg(MsgBase_645_ptr->ctrlCode_,MsgBase_645_ptr->dataLen_,di,MsgBase_645_ptr->addr_,(*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[mtrlID]->mtrAddr,p_CtrInfoList);
                    if(!tmpSendMsg.isEmpty())
                    {
                        uchar ComAddr[4]={0x01,0x04,0x00,0x04};
                        sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                        if(memcmp(di,ComAddr,4)==0)
                        {
                            Refresh_CtrInfo_Result_for_AssignAddr(p_CtrInfoList->at(0),mtrlID);
                            if(p_CtrInfoList->at(0)->sucsRate_CtrlCmd[0]>=1.0)
                            {
                                emScriptRunState=Wait_00F1_for_05F1_SetCcoAddr;
                                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                                sendMsg(p_CtrInfoList->at(0)->slotPosition,p_CtrInfoList->at(0)->dvcId,INSIGNIFICANCE,p_SetCcoAddr_05F1);
                                p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("发送--设置主节点地址（05F1），等待--确认"));
                            }
                        }
                    }
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther645Msg(MsgBase_645_ptr->ctrlCode_,MsgBase_645_ptr->dataLen_,di,MsgBase_645_ptr->addr_,(*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[mtrlID]->mtrAddr,p_CtrInfoList);
                    if(!tmpSendMsg.isEmpty())
                        sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
            }
            break;
        }
        case BuildNetFinish:
        {
            break;
        }
        default:
        {
            if(MsgBase_645_ptr->ctrlCode_==dlt_645_Protocol::READ_ADDR)
            {
                sendMsg(dvcType,dvcId,mtrlID,p_MeterAddrResp_93);
            }
            else
            {
                uchar di[4]={0x00};
                if(MsgBase_645_ptr->ctrlCode_==dlt_645_Protocol::READ_DATA)
                {
                    shared_ptr<dlt_645_Protocol::Rqst_ReadData_0x11> Rqst_ReadData_0x11_ptr = std::dynamic_pointer_cast<dlt_645_Protocol::Rqst_ReadData_0x11>(MsgBase_645_ptr);
                    memcpy(di,Rqst_ReadData_0x11_ptr->di,4);

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
void BuildNetwork_GW::processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID)
{
    QString stringDvcType;
    if(dvcType == SingleSTA)
    {
        stringDvcType = QString("单通;");
    }
    else
    {
        stringDvcType = QString("三通;");
    }

    bool haveCompleteMsg=true;
    while(haveCompleteMsg)
    {
        qInfo()<<stringDvcType + QString("解析前 buf698=%1").arg(QString((*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[mtrlID]->buf698.toHex()));
        shared_ptr<FrameOOPBase> MsgBase_OOP_ptr = FrameOOPHelper::DecodeMsg(&((*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[mtrlID]->buf698),haveCompleteMsg);
        //qInfo()<<stringDvcType + QString("解析后 buf645=%1").arg(QString((*p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList)[mtrlID]->buf645.toHex()));


        if(MsgBase_OOP_ptr==nullptr)
        {
            continue;
        }

        //        if(haveCompleteMsg==false)
        //        {
        //            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("haveCompleteMsg==false"));

        //            break;
        //        }

        switch(emScriptRunState)
        {
        case Init:
        {
            break;
        }
        case Wait_PowerOff12V_Finish:
        {
            break;
        }
        case Wait_SetBaudRate_Finish:
        {
            break;
        }
        case Wait_PowerOn12V_CCO_Finish:
        {
            break;
        }
        case Wait_PowerOn12V_STA_Finish:
        {
            break;
        }
        case Wait_RST_CCO_Finish:
        {
            break;
        }
        case Wait_RST_STA_Finish:
        {
            break;
        }
        case Wait_AssignAddrsFinish:
        {
            if(MsgBase_OOP_ptr->service_type_==GET_REQUEST_CLIENT&&MsgBase_OOP_ptr->service_sub_type_==uchar(GetRequestType::kGetRequestNormal))
            {
                shared_ptr<GetRequestNormal> p_GetRequestNormal=dynamic_pointer_cast<GetRequestNormal>(MsgBase_OOP_ptr);
                if(p_GetRequestNormal->oad_.OI==ComuAddr)
                {
                    sendMsg(dvcType,dvcId,mtrlID,p_GetResponseNormal_ReadAddr);
                    Refresh_CtrInfo_Result_for_AssignAddr(p_CtrInfoList->at(0),mtrlID);

                    if(p_CtrInfoList->at(0)->sucsRate_CtrlCmd[0]>=1.0)
                    {
                        sendMsg(p_CtrInfoList->at(0)->slotPosition,p_CtrInfoList->at(0)->dvcId,INSIGNIFICANCE,p_ParamInit_01F2);
                        tryTimes=0;
                        emScriptRunState=Wait_00F1_for_01F2_ParamInit;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "4-发送--参数初始化（01F2），等待--确认");
                        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                    }
                }

            }
            else
            {
                QByteArray tmpSendMsg=prcsOther698Msg(MsgBase_OOP_ptr,(*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[mtrlID]->mtrAddr,p_CtrInfoList);
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case BuildNetFinish:
        {
            break;
        }
        default:
        {
            if(MsgBase_OOP_ptr->service_type_==GET_REQUEST_CLIENT&&MsgBase_OOP_ptr->service_sub_type_==uchar(GetRequestType::kGetRequestNormal))
            {
                shared_ptr<GetRequestNormal> p_GetRequestNormal=dynamic_pointer_cast<GetRequestNormal>(MsgBase_OOP_ptr);
                if(p_GetRequestNormal->oad_.OI==ComuAddr)
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
void BuildNetwork_GW::processMsgFromCJQ(DvcType dvcType,int dvcId)
{
    Q_UNUSED(dvcType)
    Q_UNUSED(dvcId)
    return;


    bool haveCompleteMsg=true;
    while(haveCompleteMsg)
    {
        //        qInfo()<<QString("id=%1; 解析前 buf698=%2").arg(id).arg(QString((*p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList)[id]->buf698.toHex()));
        //        haveCompleteMsg=p_MsgBase_698_45->decode_698_45_MsgUp(&((*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[id]->buf698));
        //        qInfo()<<QString("id=%1; 解析后 buf698=%2").arg(id).arg(QString((*p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList)[id]->buf698.toHex()));

        if(haveCompleteMsg==false)
        {
            break;
        }

        switch(emScriptRunState)
        {
        case Init:
        {
            break;
        }
        case Wait_PowerOff12V_Finish:
        {
            break;
        }
        case Wait_SetBaudRate_Finish:
        {
            break;
        }
        case Wait_PowerOn12V_CCO_Finish:
        {
            break;
        }
        case Wait_PowerOn12V_STA_Finish:
        {
            break;
        }
        case Wait_RST_CCO_Finish:
        {
            break;
        }
        case Wait_RST_STA_Finish:
        {
            break;
        }
        case Wait_AssignAddrsFinish:
        {
            break;
        }
        case Wait_00F1_for_01F1_HardInit:
        {
            break;
        }
        case Wait_00F1_for_01F2_ParamInit:
        {
            break;
        }
        case Wait_00F1_for_05F16_SetFreq:
        {
            break;
        }
        case Wait_00F1_for_05F1_SetCcoAddr:
        {
            break;
        }
        case Wait_AddMetersFinish:
        {
            break;
        }
        case Wait_BuildNetFinish:
        {
            break;
        }
        case BuildNetFinish:
        {
            break;
        }
        default:
        {
            break;
        }
        }
    }
}
void BuildNetwork_GW::sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame)
{
    if(frame==p_ParamInit_01F2)
    {
        p_ParamInit_01F2->ctrl_field_.dir=kDirDown;
        p_ParamInit_01F2->ctrl_field_.prm=kActive;
        p_ParamInit_01F2->ctrl_field_.comn_type=kHplc;

        p_ParamInit_01F2->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_ParamInit_01F2->info_field_.info_field_down.comu_rate=0;
        p_ParamInit_01F2->info_field_.info_field_down.comu_module_ident=0;

        sendMsgOct=p_ParamInit_01F2->EncodeFrame();
        sendMsgLog=QString("》》参数初始化01F2：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_HardInit_01F1)
    {
        p_HardInit_01F1->ctrl_field_.dir=kDirDown;
        p_HardInit_01F1->ctrl_field_.prm=kActive;
        p_HardInit_01F1->ctrl_field_.comn_type=kHplc;

        p_HardInit_01F1->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_HardInit_01F1->info_field_.info_field_down.comu_rate=0;
        p_HardInit_01F1->info_field_.info_field_down.comu_module_ident=0;

        sendMsgOct=p_HardInit_01F1->EncodeFrame();
        sendMsgLog=QString("》》硬件初始化01F1：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_SetCcoAddr_05F1)
    {
        p_SetCcoAddr_05F1->ctrl_field_.dir=kDirDown;
        p_SetCcoAddr_05F1->ctrl_field_.prm=kActive;
        p_SetCcoAddr_05F1->ctrl_field_.comn_type=kHplc;

        p_SetCcoAddr_05F1->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_SetCcoAddr_05F1->info_field_.info_field_down.comu_rate=0;
        p_SetCcoAddr_05F1->info_field_.info_field_down.comu_module_ident=0;

        memcpy(&p_SetCcoAddr_05F1->primary_node_address_.addr,p_CtrInfoList->at(0)->ccoAddr,6);

        sendMsgOct=p_SetCcoAddr_05F1->EncodeFrame();
        sendMsgLog=QString("》》设置主节点地址05F1：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_SetFreq_05F16)
    {
        p_SetFreq_05F16->ctrl_field_.dir=kDirDown;
        p_SetFreq_05F16->ctrl_field_.prm=kActive;
        p_SetFreq_05F16->ctrl_field_.comn_type=kHplc;

        p_SetFreq_05F16->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_SetFreq_05F16->info_field_.info_field_down.comu_rate=0;
        p_SetFreq_05F16->info_field_.info_field_down.comu_module_ident=0;

        p_SetFreq_05F16->carrier_frequence_range_=char(freqBand);

        sendMsgOct=p_SetFreq_05F16->EncodeFrame();
        sendMsgLog=QString("》》设置频段05F16：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_SetChannel_F0F37)
    {
        p_SetChannel_F0F37->ctrl_field_.dir=kDirDown;
        p_SetChannel_F0F37->ctrl_field_.prm=kActive;
        p_SetChannel_F0F37->ctrl_field_.comn_type=kHplc;

        p_SetChannel_F0F37->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_SetChannel_F0F37->info_field_.info_field_down.comu_rate=0;
        p_SetChannel_F0F37->info_field_.info_field_down.comu_module_ident=0;

        if(channelType=="双信道"||channelType.isEmpty())
            p_SetChannel_F0F37->freq_=char(0x00);
        else if(channelType=="HPLC")
            p_SetChannel_F0F37->freq_=char(0x01);
        else if(channelType=="HRF")
            p_SetChannel_F0F37->freq_=char(0x02);

        sendMsgOct=p_SetChannel_F0F37->EncodeFrame();
        sendMsgLog=QString("》》设置路由信道F0F37：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_QueryNoise_03F2)
    {
        p_QueryNoise_03F2->ctrl_field_.dir=kDirDown;
        p_QueryNoise_03F2->ctrl_field_.prm=kActive;
        p_QueryNoise_03F2->ctrl_field_.comn_type=kHplc;

        p_QueryNoise_03F2->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_QueryNoise_03F2->info_field_.info_field_down.comu_rate=0;
        p_QueryNoise_03F2->info_field_.info_field_down.comu_module_ident=0;


        sendMsgOct=p_QueryNoise_03F2->EncodeFrame();
        sendMsgLog=QString("》》组网完成03F2：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_QueryNetTopoInfo_10F21)
    {
        p_QueryNetTopoInfo_10F21->ctrl_field_.dir=kDirDown;
        p_QueryNetTopoInfo_10F21->ctrl_field_.prm=kActive;
        p_QueryNetTopoInfo_10F21->ctrl_field_.comn_type=kHplc;

        p_QueryNetTopoInfo_10F21->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_QueryNetTopoInfo_10F21->info_field_.info_field_down.comu_rate=0;
        p_QueryNetTopoInfo_10F21->info_field_.info_field_down.comu_module_ident=0;

        if((index+1)*topoCntPerTime<=(p_CtrInfoList->at(0)->totalNodeCnt+1))
        {
            addCntThisTime=topoCntPerTime;
        }
        else
        {
            addCntThisTime=(p_CtrInfoList->at(0)->totalNodeCnt+1)%topoCntPerTime;
        }
        p_QueryNetTopoInfo_10F21->node_start_no_=index*topoCntPerTime+1;
        p_QueryNetTopoInfo_10F21->node_num_=addCntThisTime;

        sendMsgOct=p_QueryNetTopoInfo_10F21->EncodeFrame();
        sendMsgLog=QString("》》查询网络拓扑10F21：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_AddSlaveNode_11F1)
    {
        p_AddSlaveNode_11F1->ctrl_field_.dir=kDirDown;
        p_AddSlaveNode_11F1->ctrl_field_.prm=kActive;
        p_AddSlaveNode_11F1->ctrl_field_.comn_type=kHplc;

        p_AddSlaveNode_11F1->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_AddSlaveNode_11F1->info_field_.info_field_down.comu_rate=0;
        p_AddSlaveNode_11F1->info_field_.info_field_down.comu_module_ident=0;

        if((index+1)*addCntPerTime<=p_CtrInfoList->at(0)->totalNodeCnt)
        {
            addCntThisTime=addCntPerTime;
        }
        else
        {
            addCntThisTime=p_CtrInfoList->at(0)->totalNodeCnt%addCntPerTime;
        }

        shared_ptr<QList<NodeParameter>> p_NodeList = make_shared<QList<NodeParameter>>();
        Init_AttachedNodeInfo(index*addCntPerTime, addCntThisTime, p_CtrInfoList->at(0)->keyList, p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList, p_NodeList);

        p_AddSlaveNode_11F1->node_num_=addCntThisTime;
        p_AddSlaveNode_11F1->node_parameter_list_=*p_NodeList;

        sendMsgOct=p_AddSlaveNode_11F1->EncodeFrame();
        sendMsgLog=QString("》》添加从节点11F1：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
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
void BuildNetwork_GW::sendSrcMsg(DvcType dvcType, int dvcId, QByteArray msg)
{
    if(msg.size()<1)
    {
        return;
    }
    uchar *sendMsg=new uchar[static_cast<uint>(msg.size())];
    memcpy(sendMsg,reinterpret_cast<uchar*>(msg.data()),uint(msg.size()));

    p_AbstractScriptHost->sendMsg2Dvc(dvcType,dvcId,sendMsg,msg.size());

    QStringList dvcList;
    dvcList<<"单通"<<"三通"<<"国网路由"<<"南网路由"<<"采集器";

    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("测试脚本==>>>>上位机【设备类型:%1】的报文=%2").arg(dvcList.at(int(dvcType))).arg(QString(msg.toHex())));
    //qInfo()<<QString("测试脚本==>>>>上位机【设备类型:%1】的报文=%2").arg(dvcList.at((int)dvcType)).arg(QString(msg.toHex()));
}

QList<int> BuildNetwork_GW::getDvcIdList(DvcType dvcType)
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
uchar BuildNetwork_GW::findDvcTypeToPowerOnOrRst()
{
    uchar temp_type = 0x00;
    for(int i=0; i<p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size(); i++)
    {
        if(SingleSTA == p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(i)->slotPosition)
        {
            temp_type = temp_type | 0x02;//由0x01改为0x02   by chubo 20200605
            qDebug()<<QString("找到单通槽位档案");
        }
        else if (ThreeSTA == p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(i)->slotPosition)
        {
            temp_type = temp_type | 0x01;//由0x02改为0x01   by chubo 20200605
            qDebug()<<QString("找到三通槽位档案");
        }
        else if (CJQ == p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(i)->slotPosition)
        {
            temp_type = temp_type | 0x04;
            qDebug()<<QString("找到采集器槽位档案");
        }
    }
    return temp_type;
}
void BuildNetwork_GW::findDvcToPowerOnOrRst(QString str)
{

    if ((temp_DvcType&0x01) == 0x01)//由0x02改为0x01   by chubo 20200605
    {
        emDvcType = ThreeSTA;
        temp_DvcType = temp_DvcType & 0xFE;//由0xFD改为0xFE   by chubo 20200605
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("STA三通_")+str+QString("，等待--确认"));
    }
    else if((temp_DvcType&0x02) == 0x02)//由0x01改为0x02   by chubo 20200605
    {
        emDvcType = SingleSTA;
        temp_DvcType = temp_DvcType & 0xFD;//由0xFE改为0xFD   by chubo 20200605
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("STA单通_")+str+QString("，等待--确认"));
    }
    else if ((temp_DvcType&0x04) == 0x04)
    {
        emDvcType = CJQ;
        temp_DvcType = temp_DvcType & 0xFB;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("采集器_")+str+QString("，等待--确认"));
    }
    else
    {
        if(emScriptRunState == Wait_PowerOn12V_CCO_Finish || emScriptRunState == Wait_RST_CCO_Finish)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("电表档案配置信息中槽位信息有误"));
            p_AbstractScriptHost->updateProgress(ProcessState_Error, QString("电表档案配置信息中槽位信息有误"));
            stop();
            powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost,true);
        }
        else if (emScriptRunState == Wait_PowerOn12V_STA_Finish)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("单人工装板_12V上电完成"));

            QThread::msleep(100);
            emScriptRunState=Wait_RST_CCO_Finish;
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("单人工装板_模块复位，等待完成"));

            emDvcType = p_CtrInfoList->at(0)->slotPosition;
            idList = getDvcIdList(emDvcType);
            p_AbstractScriptHost->controlDvc(emDvcType,idList,CtrlCmd_ModuleRST,sendParams);
            p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("CCO_复位，等待--确认"));
        }
        else if (emScriptRunState == Wait_RST_STA_Finish)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("单人工装板_复位完成"));

            int totalNodeCnt = 0;
            for(int i=0; i<p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size(); i++)
            {
                if(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(i)->slotPosition == SingleSTA
                        || p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(i)->slotPosition == ThreeSTA
                        || p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(i)->slotPosition == CJQ)
                {
                    totalNodeCnt +=1;
                }
            }
            if(totalNodeCnt > 0)
            {
                emScriptRunState=Wait_AssignAddrsFinish;
                if(flagStaHighComBaud==true)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("电表12V上电完成，等待--分配通信地址(360秒)"));
                    p_timer->start(360*1000);
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("电表12V上电完成，等待--分配通信地址(60秒)"));
                    p_timer->start(60*1000);
                }
            }
            else
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("未找到需要分配地址的表档案"));

                sendMsg(p_CtrInfoList->at(0)->slotPosition,p_CtrInfoList->at(0)->dvcId,INSIGNIFICANCE,p_ParamInit_01F2);
                tryTimes=0;
                emScriptRunState=Wait_00F1_for_01F2_ParamInit;
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "5-发送--参数初始化（01F2），等待--确认");
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            }

        }

        return;
    }

    idList = getDvcIdList(emDvcType);
    if(emScriptRunState == Wait_PowerOn12V_CCO_Finish || emScriptRunState == Wait_PowerOn12V_STA_Finish)
    {
        if(emDvcType==CJQ)
            p_AbstractScriptHost->controlDvc(emDvcType,idList,CtrlCmd_PowerOn_220V,sendParams);
        else
            p_AbstractScriptHost->controlDvc(emDvcType,idList,CtrlCmd_PowerOn_12V,sendParams);
    }
    else if (emScriptRunState == Wait_RST_CCO_Finish || emScriptRunState == Wait_RST_STA_Finish)
    {
        p_AbstractScriptHost->controlDvc(emDvcType,idList,CtrlCmd_ModuleRST,sendParams);
    }
    p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
}



bool BuildNetwork_GW::extractAndProcess3762Frame(QByteArray& buf,QByteArray& completeFrame)
{
    completeFrame.clear();

    // 安全检查：防止空指针导致崩溃
    if(p_AbstractScriptHost == nullptr) {
        return false;
    }

    // 1. 去除前导的 0xFE/0x1E 字符
    while (buf.size() > 0 && ((unsigned char)buf[0] == 0xFE || (unsigned char)buf[0] == 0x1E)) {
        buf.remove(0, 1);
    }

    if (buf.size() < 6) {
        return false;
    }

    // 2. 寻找有效的帧起始符（同时验证帧长度合理性）
    int frameStart = -1;
    for (int i = 0; i < buf.size() - 5; i++) {
        if ((unsigned char)buf[i] == 0x68) {
            uint16_t potentialLength = 0;
            memcpy(&potentialLength, buf.constData() + i + 1, 2);
            // 帧长度合理性检查：4-500字节
            if (potentialLength > 3 && potentialLength < 500) {
                if (buf.size() >= i + potentialLength) {
                    // 验证帧结束标志
                    if ((unsigned char)buf[i + potentialLength - 1] == 0x16) {
                        frameStart = i;
                        break;
                    }
                } else {
                    // 数据还不完整，等待更多数据
                    frameStart = i;
                    break;
                }
            }
        }
    }

    if (frameStart == -1) {
        // 没找到有效帧，保留最后5个字节等待下次拼接
        if (buf.size() > 5) buf.remove(0, buf.size() - 5);
        return false;
    }

    // 3. 移除帧起始之前的无效数据
    if (frameStart > 0) buf.remove(0, frameStart);
    if (buf.size() < 6) return false;

    // 4. 解析帧长度
    uint16_t frameLength = 0;
    memcpy(&frameLength, buf.constData() + 1, 2);

    // 帧长度再次校验
    if (frameLength < 4 || frameLength > 500) {
        buf.remove(0, 1);
        return extractAndProcess3762Frame(buf, completeFrame);
    }

    // 5. 检查数据是否完整
    if (buf.size() < frameLength) {
        p_AbstractScriptHost->updateProgress(ProcessState_Processing,
            QString("等待完整帧，需要%1字节，当前%2字节").arg(frameLength).arg(buf.size()));
        return false;
    }

    // 6. 验证帧结束标志
    if ((unsigned char)buf[frameLength - 1] != 0x16) {
        buf.remove(0, 1);
        return extractAndProcess3762Frame(buf, completeFrame);
    }

    // 7. 提取完整帧
    completeFrame = buf.left(frameLength);
    buf.remove(0, frameLength);

    p_AbstractScriptHost->updateProgress(ProcessState_Processing,
        QString("提取到完整3762帧: %1").arg(QString(completeFrame.toHex())));


    return true;
}

