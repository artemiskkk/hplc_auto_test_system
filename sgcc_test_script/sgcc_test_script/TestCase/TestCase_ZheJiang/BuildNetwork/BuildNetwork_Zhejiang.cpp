#include "BuildNetwork_Zhejiang.h"

BuildNetwork_Zhejiang::BuildNetwork_Zhejiang(QObject *parent) : QObject(parent)
{
    emScriptRunState=Init;
    resultFlag=false;

    emDvcType = ThreeSTA;

    sendParams.clear();
    idList.clear();
    idList.append(1);

    sendMsgOct.clear();

    p_MsgBase_3762 = make_shared<qgdw_3762_protocol::Frame3762Helper>();
    p_Confirm_00F1=make_shared<Afn00F1>();
    p_SetCcoAddr_05F1 = make_shared<qgdw_3762_protocol::Afn05F1>();
    p_HardInit_01F1 = make_shared<qgdw_3762_protocol::Afn01F1>();
    p_ParamInit_01F2 = make_shared<qgdw_3762_protocol::Afn01F2>();
    p_AddSlaveNode_11F1 = make_shared<qgdw_3762_protocol::Afn11F1>();
    p_SetFreq_05F16 = make_shared<qgdw_3762_protocol::Afn05F16>();
    p_QueryNetTopoInfo_10F21 = make_shared<qgdw_3762_protocol::Afn10F21>();
    p_QueryNoise_03F2=make_shared<Afn03F2>();

    p_MsgBase_645 = make_shared<dlt_645_Protocol::Frame645Helper>();
    p_MeterAddrResp_93 = make_shared<dlt_645_Protocol::RspsNormal_ReadAddr_0x93>(addr,6);

    p_GetResponseNormal_ReadAddr=make_shared<GetResponseNormal>();


    p_timer = make_shared<QTimer>(this);
    p_maxAllowTimer = make_shared<QTimer>(this);
    p_delayTimer = make_shared<QTimer>(this);

    connect(p_timer.get(),SIGNAL(timeout()),this,SLOT(timer_timeoutProc()));
    connect(p_maxAllowTimer.get(),SIGNAL(timeout()),this,SLOT(maxAllowTimer_timeoutProc()));
    connect(p_delayTimer.get(),SIGNAL(timeout()),this,SLOT(delayTimer_timeoutProc()));
}
BuildNetwork_Zhejiang::~BuildNetwork_Zhejiang()
{
    p_timer->stop();
    p_maxAllowTimer->stop();
    p_delayTimer->stop();
}
void BuildNetwork_Zhejiang::execute()
{
    p_AbstractScriptHost->updateProgress(ProcessState_Start, QString("开始组网流程!!!"));

    isFirstChkTopo=true;
    roundIndex=1;

    checkTopologyIntervalTime =(p_CtrInfoList->at(0)->totalNodeCnt<500)?(checkTopologyIntervalTime + p_CtrInfoList->at(0)->totalNodeCnt/100):(checkTopologyIntervalTime + 2*p_CtrInfoList->at(0)->totalNodeCnt/100);

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
void BuildNetwork_Zhejiang::stop()
{
    p_timer->stop();
    p_maxAllowTimer->stop();
    p_delayTimer->stop();
}
void BuildNetwork_Zhejiang::setHost(AbstractScriptHost *host)
{
    p_AbstractScriptHost=host;
}
bool BuildNetwork_Zhejiang::config(const QMap<QString,QString> *paraDic)
{
    bool result = false;
    if(paraDic!=nullptr)
    {
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
        result = true;
    }

    return result;
}
void BuildNetwork_Zhejiang::processMsg(DvcType dvcType, int id, uchar *data, int datalen)
{
    QByteArray tmpRecvTempData=QByteArray::fromRawData(reinterpret_cast<char*>(data),datalen);
    QByteArray recvTempData;
    recvTempData.append(tmpRecvTempData);
    delete[]data;

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
void BuildNetwork_Zhejiang::processCtrlDvcRes(DvcType dvcType, QList<int> idList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params)
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

            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("国网路由槽位_设置波特率，等待--确认"));
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
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "三通槽位_12V断电，等待--确认");
            break;
        }
        else
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("单人工装控制命令槽位回复错误，命令下发槽位=%1，命令回复槽位=%2").arg(emDvcType).arg(dvcType));
            p_AbstractScriptHost->updateProgress(ProcessState_Error, QString("单人工装控制命令槽位回复错误"));
            stop();
            powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost);
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
        if(dvcType == ThreeSTA && emDvcType == ThreeSTA)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("三通槽位_12V断电成功"));
            emDvcType = SingleSTA;

            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("单通槽位_12V断电，等待--确认"));
        }
        else if (dvcType == SingleSTA && emDvcType == SingleSTA)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("单通槽位_12V断电成功"));
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
            emDvcType = ThreeSTA;
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("单人工装板_12V断电完成"));

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
            powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost);
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
            powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost);
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
            powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost);
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
            powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost);
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
            powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost);
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
        break;
    }
    case Wait_00F1_for_01F2_ParamInit:
    {
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
    }
}

void BuildNetwork_Zhejiang::setCtrInfoListAndFreq(shared_ptr<QList<shared_ptr<CtrInfo>>> p_CtrInfoList, uchar freq)
{
    this->concentratorCnt=p_CtrInfoList->size()&0xFFFF;
    this->p_CtrInfoList=p_CtrInfoList;
    this->freqBand=freq;
}
void BuildNetwork_Zhejiang::initBuildNetWork()
{
    p_timer->stop();
    p_maxAllowTimer->stop();
    p_delayTimer->stop();
}

void BuildNetwork_Zhejiang::timer_timeoutProc()
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
            if(++tryTimes>=3)
            {
                p_maxAllowTimer->stop();
                p_delayTimer->stop();
                p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("参数初始化;尝试3次;超时失败!!!"));
            }
            else
            {
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                sendMsg(p_CtrInfoList->at(0)->slotPosition,p_CtrInfoList->at(0)->dvcId,INSIGNIFICANCE,p_ParamInit_01F2);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("发送--参数初始化（01F2），等待--确认"));
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
void BuildNetwork_Zhejiang::maxAllowTimer_timeoutProc()
{
    p_delayTimer->stop();
    p_timer->stop();
    p_maxAllowTimer->stop();

    switch(emScriptRunState)
    {
    case Wait_BuildNetFinish:
    {
        if(haveStartContinueTimer==false)
        {
            QString failedMeter = GenerateFailedMeterStr_Net(p_CtrInfoList->at(0));
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, failedMeter);
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("  全网组网成功率：%1%；失败表列表：").arg(p_CtrInfoList->at(0)->inNetSuccessRate*100) + failedMeter);
        }
        else
        {
            if(p_CtrInfoList->at(0)->inNetSuccessRate>=netSucRateThresld)
            {
                p_timer->stop();

                resultFlag=true;
                p_CtrInfoList->at(0)->inNetConsume=static_cast<double>((havePassedTimeLen+timerAfterReachThresld)*1000)/1000.0;
                p_CtrInfoList->at(0)->inNetResult=true;
                p_maxAllowTimer->stop();

                QString failedMeter = GenerateFailedMeterStr_Net(p_CtrInfoList->at(0));
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, failedMeter);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("  全网组网成功率：%1%; 全网组网耗时：%2秒;失败表列表：").arg(p_CtrInfoList->at(0)->inNetSuccessRate*100).arg(p_CtrInfoList->at(0)->inNetConsume+10) + failedMeter);
                emScriptRunState=BuildNetFinish;
                resultFlag=true;
                sendMsg(CCO_GW,1,INSIGNIFICANCE,p_QueryNoise_03F2);
            }
            else
            {
                QString failedMeter = GenerateFailedMeterStr_Net(p_CtrInfoList->at(0));
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, failedMeter);
                p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("  全网组网成功率：%1%；失败表列表：").arg(p_CtrInfoList->at(0)->inNetSuccessRate*100) + failedMeter);
            }
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
void BuildNetwork_Zhejiang::delayTimer_timeoutProc()
{
    p_delayTimer->stop();    
    switch(emScriptRunState)
    {
        case Wait_00F1_for_01F2_ParamInit:
        {
            sendMsg(p_CtrInfoList->at(0)->slotPosition,p_CtrInfoList->at(0)->dvcId,INSIGNIFICANCE,p_HardInit_01F1);
            p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            tryTimes=0;
            emScriptRunState=Wait_00F1_for_01F1_HardInit;
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("发送--硬件初始化（01F1），等待--确认"));
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
            if(flagNeedAddMeter==true)
            {
                if(p_CtrInfoList->at(0)->totalNodeCnt==0)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("档案为空,无需下发档案及等待组网完成"));
    
                    resultFlag=true;
    
                    p_CtrInfoList->at(0)->inNetConsume=0;
    
                    p_CtrInfoList->at(0)->inNetResult=true;
                    p_maxAllowTimer->stop();
                    p_delayTimer->stop();
    
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
            }
            else
            {
                ////////
                resultFlag=true;
                sendMsg(p_CtrInfoList->at(0)->slotPosition,p_CtrInfoList->at(0)->dvcId,INSIGNIFICANCE,p_QueryNoise_03F2);
            }            
            break;
        }
        case Wait_BuildNetFinish:
        {
            if(isFirstChkTopo)
            {
                isFirstChkTopo=false;
                startBuildNetFlag=true;
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("等待组网完成"));
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("第(%1)轮查询网络拓扑").arg(roundIndex));
                sendMsg(p_CtrInfoList->at(0)->slotPosition,p_CtrInfoList->at(0)->dvcId,INSIGNIFICANCE,p_QueryNetTopoInfo_10F21);
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                tryTimes=0;
                emScriptRunState=Wait_BuildNetFinish;
            }
            else
            {
                roundIndex+=1;
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("第(%1)轮查询网络拓扑").arg(roundIndex));
                sendMsg(p_CtrInfoList->at(0)->slotPosition,p_CtrInfoList->at(0)->dvcId,INSIGNIFICANCE,p_QueryNetTopoInfo_10F21);
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            }
            break;
        }
        default:
            break;
    }
}

void BuildNetwork_Zhejiang::processMsgFromCco(DvcType dvcType, int dvcId)
{
    bool haveCompleteMsg=true;
    while(haveCompleteMsg)
    {
        qInfo()<<QString("CCO-解析前 buf3762=%1").arg(QString((p_CtrInfoList->at(0)->buf.toHex())));
        shared_ptr<Frame3762Base> p_Frame3762Base = p_MsgBase_3762->DecodeLocalMsg(&(p_CtrInfoList->at(0)->buf),haveCompleteMsg);
        qInfo()<<QString("CCO-解析后 buf3762=%1").arg(QString((p_CtrInfoList->at(0)->buf.toHex())));

        if(p_Frame3762Base==nullptr)
        {
            continue;
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
            if(p_Frame3762Base->afn_ == 0x06&&p_Frame3762Base->dt1_==0x08&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp )
            {
                p_timer->stop();
                shared_ptr<Afn06F4> p_ReportNode_06F4_Up=dynamic_pointer_cast<Afn06F4>(p_Frame3762Base);
                if(p_ReportNode_06F4_Up==nullptr)
                    continue;
                if(p_ReportNode_06F4_Up->report_node_info_unit_.report_node_device_type_==0x01)
                {
                    if(p_ReportNode_06F4_Up->report_node_info_unit_.node_info_list_.size()==0)
                    {
                        NodeInfo node;
                        node.node_address_=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_address_;
                        node.node_protocol_=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_protocol_;
                        node.deviceType=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_device_type_;
                        reportNodeList_06F4.append(node);
                    }
                    for(int i=0;i<p_ReportNode_06F4_Up->report_node_info_unit_.node_info_list_.size();i++)
                    {
                        NodeInfo node;
                        node.node_address_=p_ReportNode_06F4_Up->report_node_info_unit_.node_info_list_.at(i).node_address_;
                        node.node_protocol_=p_ReportNode_06F4_Up->report_node_info_unit_.node_info_list_.at(i).node_protocol_;
                        node.deviceType=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_device_type_;
                        reportNodeList_06F4.append(node);
                    }
                }
                else if(p_ReportNode_06F4_Up->report_node_info_unit_.report_node_device_type_==0x00)
                {
                    NodeInfo node;
                    node.node_address_=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_address_;
                    node.node_protocol_=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_protocol_;
                    node.deviceType=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_device_type_;
                    reportNodeList_06F4.append(node);
                }
                sendMsg(dvcType,dvcId,INSIGNIFICANCE,p_Confirm_00F1);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "回复确认");
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case Wait_PowerOn12V_CCO_Finish:
        {
            if(p_Frame3762Base->afn_ == 0x06&&p_Frame3762Base->dt1_==0x08&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp )
            {
                p_timer->stop();
                shared_ptr<Afn06F4> p_ReportNode_06F4_Up=dynamic_pointer_cast<Afn06F4>(p_Frame3762Base);
                if(p_ReportNode_06F4_Up==nullptr)
                    continue;
                if(p_ReportNode_06F4_Up->report_node_info_unit_.report_node_device_type_==0x01)
                {
                    if(p_ReportNode_06F4_Up->report_node_info_unit_.node_info_list_.size()==0)
                    {
                        NodeInfo node;
                        node.node_address_=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_address_;
                        node.node_protocol_=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_protocol_;
                        node.deviceType=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_device_type_;
                        reportNodeList_06F4.append(node);
                    }
                    for(int i=0;i<p_ReportNode_06F4_Up->report_node_info_unit_.node_info_list_.size();i++)
                    {
                        NodeInfo node;
                        node.node_address_=p_ReportNode_06F4_Up->report_node_info_unit_.node_info_list_.at(i).node_address_;
                        node.node_protocol_=p_ReportNode_06F4_Up->report_node_info_unit_.node_info_list_.at(i).node_protocol_;
                        node.deviceType=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_device_type_;
                        reportNodeList_06F4.append(node);
                    }
                }
                else if(p_ReportNode_06F4_Up->report_node_info_unit_.report_node_device_type_==0x00)
                {
                    NodeInfo node;
                    node.node_address_=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_address_;
                    node.node_protocol_=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_protocol_;
                    node.deviceType=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_device_type_;
                    reportNodeList_06F4.append(node);
                }
                sendMsg(dvcType,dvcId,INSIGNIFICANCE,p_Confirm_00F1);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "回复确认");
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case Wait_PowerOn12V_STA_Finish:
        {
            if(p_Frame3762Base->afn_ == 0x06&&p_Frame3762Base->dt1_==0x08&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp )
            {
                p_timer->stop();
                shared_ptr<Afn06F4> p_ReportNode_06F4_Up=dynamic_pointer_cast<Afn06F4>(p_Frame3762Base);
                if(p_ReportNode_06F4_Up==nullptr)
                    continue;
                if(p_ReportNode_06F4_Up->report_node_info_unit_.report_node_device_type_==0x01)
                {
                    if(p_ReportNode_06F4_Up->report_node_info_unit_.node_info_list_.size()==0)
                    {
                        NodeInfo node;
                        node.node_address_=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_address_;
                        node.node_protocol_=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_protocol_;
                        node.deviceType=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_device_type_;
                        reportNodeList_06F4.append(node);
                    }
                    for(int i=0;i<p_ReportNode_06F4_Up->report_node_info_unit_.node_info_list_.size();i++)
                    {
                        NodeInfo node;
                        node.node_address_=p_ReportNode_06F4_Up->report_node_info_unit_.node_info_list_.at(i).node_address_;
                        node.node_protocol_=p_ReportNode_06F4_Up->report_node_info_unit_.node_info_list_.at(i).node_protocol_;
                        node.deviceType=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_device_type_;
                        reportNodeList_06F4.append(node);
                    }
                }
                else if(p_ReportNode_06F4_Up->report_node_info_unit_.report_node_device_type_==0x00)
                {
                    NodeInfo node;
                    node.node_address_=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_address_;
                    node.node_protocol_=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_protocol_;
                    node.deviceType=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_device_type_;
                    reportNodeList_06F4.append(node);
                }
                sendMsg(dvcType,dvcId,INSIGNIFICANCE,p_Confirm_00F1);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "回复确认");
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case Wait_RST_CCO_Finish:
        {
            if(p_Frame3762Base->afn_ == 0x06&&p_Frame3762Base->dt1_==0x08&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp )
            {
                p_timer->stop();
                shared_ptr<Afn06F4> p_ReportNode_06F4_Up=dynamic_pointer_cast<Afn06F4>(p_Frame3762Base);
                if(p_ReportNode_06F4_Up==nullptr)
                    continue;
                if(p_ReportNode_06F4_Up->report_node_info_unit_.report_node_device_type_==0x01)
                {
                    if(p_ReportNode_06F4_Up->report_node_info_unit_.node_info_list_.size()==0)
                    {
                        NodeInfo node;
                        node.node_address_=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_address_;
                        node.node_protocol_=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_protocol_;
                        node.deviceType=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_device_type_;
                        reportNodeList_06F4.append(node);
                    }
                    for(int i=0;i<p_ReportNode_06F4_Up->report_node_info_unit_.node_info_list_.size();i++)
                    {
                        NodeInfo node;
                        node.node_address_=p_ReportNode_06F4_Up->report_node_info_unit_.node_info_list_.at(i).node_address_;
                        node.node_protocol_=p_ReportNode_06F4_Up->report_node_info_unit_.node_info_list_.at(i).node_protocol_;
                        node.deviceType=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_device_type_;
                        reportNodeList_06F4.append(node);
                    }
                }
                else if(p_ReportNode_06F4_Up->report_node_info_unit_.report_node_device_type_==0x00)
                {
                    NodeInfo node;
                    node.node_address_=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_address_;
                    node.node_protocol_=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_protocol_;
                    node.deviceType=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_device_type_;
                    reportNodeList_06F4.append(node);
                }
                sendMsg(dvcType,dvcId,INSIGNIFICANCE,p_Confirm_00F1);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "回复确认");
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case Wait_RST_STA_Finish:
        {
            if(p_Frame3762Base->afn_ == 0x06&&p_Frame3762Base->dt1_==0x08&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp )
            {
                p_timer->stop();
                shared_ptr<Afn06F4> p_ReportNode_06F4_Up=dynamic_pointer_cast<Afn06F4>(p_Frame3762Base);
                if(p_ReportNode_06F4_Up==nullptr)
                    continue;
                if(p_ReportNode_06F4_Up->report_node_info_unit_.report_node_device_type_==0x01)
                {
                    if(p_ReportNode_06F4_Up->report_node_info_unit_.node_info_list_.size()==0)
                    {
                        NodeInfo node;
                        node.node_address_=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_address_;
                        node.node_protocol_=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_protocol_;
                        node.deviceType=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_device_type_;
                        reportNodeList_06F4.append(node);
                    }
                    for(int i=0;i<p_ReportNode_06F4_Up->report_node_info_unit_.node_info_list_.size();i++)
                    {
                        NodeInfo node;
                        node.node_address_=p_ReportNode_06F4_Up->report_node_info_unit_.node_info_list_.at(i).node_address_;
                        node.node_protocol_=p_ReportNode_06F4_Up->report_node_info_unit_.node_info_list_.at(i).node_protocol_;
                        node.deviceType=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_device_type_;
                        reportNodeList_06F4.append(node);
                    }
                }
                else if(p_ReportNode_06F4_Up->report_node_info_unit_.report_node_device_type_==0x00)
                {
                    NodeInfo node;
                    node.node_address_=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_address_;
                    node.node_protocol_=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_protocol_;
                    node.deviceType=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_device_type_;
                    reportNodeList_06F4.append(node);
                }
                sendMsg(dvcType,dvcId,INSIGNIFICANCE,p_Confirm_00F1);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "回复确认");
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case Wait_AssignAddrsFinish:
        {
            if(p_Frame3762Base->afn_ == 0x06&&p_Frame3762Base->dt1_==0x08&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp )
            {
                p_timer->stop();
                shared_ptr<Afn06F4> p_ReportNode_06F4_Up=dynamic_pointer_cast<Afn06F4>(p_Frame3762Base);
                if(p_ReportNode_06F4_Up==nullptr)
                    continue;
                if(p_ReportNode_06F4_Up->report_node_info_unit_.report_node_device_type_==0x01)
                {
                    if(p_ReportNode_06F4_Up->report_node_info_unit_.node_info_list_.size()==0)
                    {
                        NodeInfo node;
                        node.node_address_=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_address_;
                        node.node_protocol_=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_protocol_;
                        node.deviceType=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_device_type_;
                        reportNodeList_06F4.append(node);
                    }
                    for(int i=0;i<p_ReportNode_06F4_Up->report_node_info_unit_.node_info_list_.size();i++)
                    {
                        NodeInfo node;
                        node.node_address_=p_ReportNode_06F4_Up->report_node_info_unit_.node_info_list_.at(i).node_address_;
                        node.node_protocol_=p_ReportNode_06F4_Up->report_node_info_unit_.node_info_list_.at(i).node_protocol_;
                        node.deviceType=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_device_type_;
                        reportNodeList_06F4.append(node);
                    }
                }
                else if(p_ReportNode_06F4_Up->report_node_info_unit_.report_node_device_type_==0x00)
                {
                    NodeInfo node;
                    node.node_address_=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_address_;
                    node.node_protocol_=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_protocol_;
                    node.deviceType=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_device_type_;
                    reportNodeList_06F4.append(node);
                }
                sendMsg(dvcType,dvcId,INSIGNIFICANCE,p_Confirm_00F1);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "回复确认");
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
                sendMsg(dvcType,dvcId,INSIGNIFICANCE,p_ParamInit_01F2);
                tryTimes=0;
                emScriptRunState=Wait_00F1_for_01F2_ParamInit;
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--参数初始化（01F2），等待--确认");
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            }
            else if(p_Frame3762Base->afn_ == 0x06&&p_Frame3762Base->dt1_==0x08&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp )
            {
                p_timer->stop();
                shared_ptr<Afn06F4> p_ReportNode_06F4_Up=dynamic_pointer_cast<Afn06F4>(p_Frame3762Base);
                if(p_ReportNode_06F4_Up==nullptr)
                    continue;
                if(p_ReportNode_06F4_Up->report_node_info_unit_.report_node_device_type_==0x01)
                {
                    if(p_ReportNode_06F4_Up->report_node_info_unit_.node_info_list_.size()==0)
                    {
                        NodeInfo node;
                        node.node_address_=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_address_;
                        node.node_protocol_=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_protocol_;
                        node.deviceType=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_device_type_;
                        reportNodeList_06F4.append(node);
                    }
                    for(int i=0;i<p_ReportNode_06F4_Up->report_node_info_unit_.node_info_list_.size();i++)
                    {
                        NodeInfo node;
                        node.node_address_=p_ReportNode_06F4_Up->report_node_info_unit_.node_info_list_.at(i).node_address_;
                        node.node_protocol_=p_ReportNode_06F4_Up->report_node_info_unit_.node_info_list_.at(i).node_protocol_;
                        node.deviceType=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_device_type_;
                        reportNodeList_06F4.append(node);
                    }
                }
                else if(p_ReportNode_06F4_Up->report_node_info_unit_.report_node_device_type_==0x00)
                {
                    NodeInfo node;
                    node.node_address_=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_address_;
                    node.node_protocol_=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_protocol_;
                    node.deviceType=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_device_type_;
                    reportNodeList_06F4.append(node);
                }
                sendMsg(dvcType,dvcId,INSIGNIFICANCE,p_Confirm_00F1);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "回复确认");
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case Wait_00F1_for_01F2_ParamInit:
        {
            if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp )
            {
                p_timer->stop();

                p_delayTimer->start(6*1000);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到确认-参数初始化，等待6秒"));
                if(flagPowerOnCJQ==true)
                    powerOn220V_CJQ(p_CtrInfoList,p_AbstractScriptHost);
            }
            else if(p_Frame3762Base->afn_ == 0x06&&p_Frame3762Base->dt1_==0x08&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp )
            {
                p_timer->stop();
                shared_ptr<Afn06F4> p_ReportNode_06F4_Up=dynamic_pointer_cast<Afn06F4>(p_Frame3762Base);
                if(p_ReportNode_06F4_Up==nullptr)
                    continue;
                if(p_ReportNode_06F4_Up->report_node_info_unit_.report_node_device_type_==0x01)
                {
                    if(p_ReportNode_06F4_Up->report_node_info_unit_.node_info_list_.size()==0)
                    {
                        NodeInfo node;
                        node.node_address_=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_address_;
                        node.node_protocol_=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_protocol_;
                        node.deviceType=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_device_type_;
                        reportNodeList_06F4.append(node);
                    }
                    for(int i=0;i<p_ReportNode_06F4_Up->report_node_info_unit_.node_info_list_.size();i++)
                    {
                        NodeInfo node;
                        node.node_address_=p_ReportNode_06F4_Up->report_node_info_unit_.node_info_list_.at(i).node_address_;
                        node.node_protocol_=p_ReportNode_06F4_Up->report_node_info_unit_.node_info_list_.at(i).node_protocol_;
                        node.deviceType=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_device_type_;
                        reportNodeList_06F4.append(node);
                    }
                }
                else if(p_ReportNode_06F4_Up->report_node_info_unit_.report_node_device_type_==0x00)
                {
                    NodeInfo node;
                    node.node_address_=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_address_;
                    node.node_protocol_=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_protocol_;
                    node.deviceType=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_device_type_;
                    reportNodeList_06F4.append(node);
                }
                sendMsg(dvcType,dvcId,INSIGNIFICANCE,p_Confirm_00F1);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "回复确认");
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
            else if(p_Frame3762Base->afn_ == 0x06&&p_Frame3762Base->dt1_==0x08&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp )
            {
                p_timer->stop();
                shared_ptr<Afn06F4> p_ReportNode_06F4_Up=dynamic_pointer_cast<Afn06F4>(p_Frame3762Base);
                if(p_ReportNode_06F4_Up==nullptr)
                    continue;
                if(p_ReportNode_06F4_Up->report_node_info_unit_.report_node_device_type_==0x01)
                {
                    if(p_ReportNode_06F4_Up->report_node_info_unit_.node_info_list_.size()==0)
                    {
                        NodeInfo node;
                        node.node_address_=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_address_;
                        node.node_protocol_=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_protocol_;
                        node.deviceType=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_device_type_;
                        reportNodeList_06F4.append(node);
                    }
                    for(int i=0;i<p_ReportNode_06F4_Up->report_node_info_unit_.node_info_list_.size();i++)
                    {
                        NodeInfo node;
                        node.node_address_=p_ReportNode_06F4_Up->report_node_info_unit_.node_info_list_.at(i).node_address_;
                        node.node_protocol_=p_ReportNode_06F4_Up->report_node_info_unit_.node_info_list_.at(i).node_protocol_;
                        node.deviceType=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_device_type_;
                        reportNodeList_06F4.append(node);
                    }
                }
                else if(p_ReportNode_06F4_Up->report_node_info_unit_.report_node_device_type_==0x00)
                {
                    NodeInfo node;
                    node.node_address_=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_address_;
                    node.node_protocol_=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_protocol_;
                    node.deviceType=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_device_type_;
                    reportNodeList_06F4.append(node);
                }
                sendMsg(dvcType,dvcId,INSIGNIFICANCE,p_Confirm_00F1);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "回复确认");
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
            else if(p_Frame3762Base->afn_ == 0x06&&p_Frame3762Base->dt1_==0x08&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp )
            {
                p_timer->stop();
                shared_ptr<Afn06F4> p_ReportNode_06F4_Up=dynamic_pointer_cast<Afn06F4>(p_Frame3762Base);
                if(p_ReportNode_06F4_Up==nullptr)
                    continue;
                if(p_ReportNode_06F4_Up->report_node_info_unit_.report_node_device_type_==0x01)
                {
                    if(p_ReportNode_06F4_Up->report_node_info_unit_.node_info_list_.size()==0)
                    {
                        NodeInfo node;
                        node.node_address_=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_address_;
                        node.node_protocol_=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_protocol_;
                        node.deviceType=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_device_type_;
                        reportNodeList_06F4.append(node);
                    }
                    for(int i=0;i<p_ReportNode_06F4_Up->report_node_info_unit_.node_info_list_.size();i++)
                    {
                        NodeInfo node;
                        node.node_address_=p_ReportNode_06F4_Up->report_node_info_unit_.node_info_list_.at(i).node_address_;
                        node.node_protocol_=p_ReportNode_06F4_Up->report_node_info_unit_.node_info_list_.at(i).node_protocol_;
                        node.deviceType=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_device_type_;
                        reportNodeList_06F4.append(node);
                    }
                }
                else if(p_ReportNode_06F4_Up->report_node_info_unit_.report_node_device_type_==0x00)
                {
                    NodeInfo node;
                    node.node_address_=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_address_;
                    node.node_protocol_=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_protocol_;
                    node.deviceType=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_device_type_;
                    reportNodeList_06F4.append(node);
                }
                sendMsg(dvcType,dvcId,INSIGNIFICANCE,p_Confirm_00F1);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "回复确认");
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
                p_timer->stop();

                if(++index==times)
                {
                    index=0;
                    times=((p_CtrInfoList->at(0)->totalNodeCnt+1)%topoCntPerTime)?(p_CtrInfoList->at(0)->totalNodeCnt+1)/topoCntPerTime+1:(p_CtrInfoList->at(0)->totalNodeCnt+1)/topoCntPerTime;
                    emScriptRunState = Wait_BuildNetFinish;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("下发档案完成，等待%1s后查询拓扑").arg(checkTopologyIntervalTime));
                    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                    /// \brief powerOn220V_CJQ

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                    p_delayTimer->start(checkTopologyIntervalTime*1000);

                    if(haveStartContinueTimer==false)
                    {
                        p_maxAllowTimer->start(timerForReachThresld*1000-2000);
                    }
                    else
                    {
                        p_maxAllowTimer->start((timerForReachThresld+timerAfterReachThresld)*1000-2000);
                    }
                }
                else
                {
                    sendMsg(dvcType,dvcId,INSIGNIFICANCE,p_AddSlaveNode_11F1);
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                }
            }
            else if(p_Frame3762Base->afn_ == 0x06&&p_Frame3762Base->dt1_==0x08&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp )
            {
                p_timer->stop();
                shared_ptr<Afn06F4> p_ReportNode_06F4_Up=dynamic_pointer_cast<Afn06F4>(p_Frame3762Base);
                if(p_ReportNode_06F4_Up==nullptr)
                    continue;
                if(p_ReportNode_06F4_Up->report_node_info_unit_.report_node_device_type_==0x01)
                {
                    if(p_ReportNode_06F4_Up->report_node_info_unit_.node_info_list_.size()==0)
                    {
                        NodeInfo node;
                        node.node_address_=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_address_;
                        node.node_protocol_=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_protocol_;
                        node.deviceType=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_device_type_;
                        reportNodeList_06F4.append(node);
                    }
                    for(int i=0;i<p_ReportNode_06F4_Up->report_node_info_unit_.node_info_list_.size();i++)
                    {
                        NodeInfo node;
                        node.node_address_=p_ReportNode_06F4_Up->report_node_info_unit_.node_info_list_.at(i).node_address_;
                        node.node_protocol_=p_ReportNode_06F4_Up->report_node_info_unit_.node_info_list_.at(i).node_protocol_;
                        node.deviceType=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_device_type_;
                        reportNodeList_06F4.append(node);
                    }
                }
                else if(p_ReportNode_06F4_Up->report_node_info_unit_.report_node_device_type_==0x00)
                {
                    NodeInfo node;
                    node.node_address_=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_address_;
                    node.node_protocol_=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_protocol_;
                    node.deviceType=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_device_type_;
                    reportNodeList_06F4.append(node);
                }
                sendMsg(dvcType,dvcId,INSIGNIFICANCE,p_Confirm_00F1);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "回复确认");
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
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("当前组网成功率：%1%;").arg(p_CtrInfoList->at(0)->inNetSuccessRate*100));

                if(++index>=times)
                {
                    index=0;
                }

                if(p_CtrInfoList->at(0)->inNetSuccessRate>=netSucRateThresld)
                {
                    if(p_CtrInfoList->at(0)->inNetSuccessRate==1.0)
                    {
                        resultFlag=true;
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

                        QString failedMeter = GenerateFailedMeterStr_Net(p_CtrInfoList->at(0));
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, failedMeter);
                        tryTimes=0;
                        emScriptRunState=BuildNetFinish;
                        resultFlag=true;
                        sendMsg(dvcType,dvcId,INSIGNIFICANCE,p_QueryNoise_03F2);//后者查询03F10
                        //emit signalBuildNetFinish();
                    }
                    else
                    {
                        if(haveStartContinueTimer==false)
                        {
                            havePassedTimeLen=double(timerForReachThresld*1000-p_maxAllowTimer->remainingTime())/1000.0;
                            p_maxAllowTimer->start(timerAfterReachThresld*1000-2000);
                            haveStartContinueTimer=true;
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("  组网成功率达到门限值：%1%; 继续等待时长：%2秒;").arg(p_CtrInfoList->at(0)->inNetSuccessRate*100).arg(timerAfterReachThresld));
                        }
                        if(index==0)
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("等待%1s后查询拓扑").arg(checkTopologyIntervalTime));
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
                    if(index==0)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("等待%1s后查询拓扑").arg(checkTopologyIntervalTime));
                        p_delayTimer->start(checkTopologyIntervalTime*1000);
                    }
                    else
                    {
                        sendMsg(dvcType,dvcId,INSIGNIFICANCE,p_QueryNetTopoInfo_10F21);
                        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                    }
                }
            }
            else if(p_Frame3762Base->afn_ == 0x06&&p_Frame3762Base->dt1_==0x08&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp )
            {
                p_timer->stop();
                shared_ptr<Afn06F4> p_ReportNode_06F4_Up=dynamic_pointer_cast<Afn06F4>(p_Frame3762Base);
                if(p_ReportNode_06F4_Up==nullptr)
                    continue;
                if(p_ReportNode_06F4_Up->report_node_info_unit_.report_node_device_type_==0x01)
                {
                    if(p_ReportNode_06F4_Up->report_node_info_unit_.node_info_list_.size()==0)
                    {
                        NodeInfo node;
                        node.node_address_=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_address_;
                        node.node_protocol_=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_protocol_;
                        node.deviceType=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_device_type_;
                        reportNodeList_06F4.append(node);
                    }
                    for(int i=0;i<p_ReportNode_06F4_Up->report_node_info_unit_.node_info_list_.size();i++)
                    {
                        NodeInfo node;
                        node.node_address_=p_ReportNode_06F4_Up->report_node_info_unit_.node_info_list_.at(i).node_address_;
                        node.node_protocol_=p_ReportNode_06F4_Up->report_node_info_unit_.node_info_list_.at(i).node_protocol_;
                        node.deviceType=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_device_type_;
                        reportNodeList_06F4.append(node);
                    }
                }
                else if(p_ReportNode_06F4_Up->report_node_info_unit_.report_node_device_type_==0x00)
                {
                    NodeInfo node;
                    node.node_address_=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_address_;
                    node.node_protocol_=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_protocol_;
                    node.deviceType=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_device_type_;
                    reportNodeList_06F4.append(node);
                }
                sendMsg(dvcType,dvcId,INSIGNIFICANCE,p_Confirm_00F1);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "回复确认");
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
        }
    }
}
void BuildNetwork_Zhejiang::processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID)
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
        shared_ptr<dlt_645_Protocol::Frame645Base> MsgBase_645_ptr = dlt_645_Protocol::Frame645Helper::DecodeLocalMsg(&((*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[mtrlID]->buf645),haveCompleteMsg);

        if(MsgBase_645_ptr==nullptr)
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
                if(MsgBase_645_ptr->ctrlCode_==dlt_645_Protocol::READ_ADDR)
                {
                    sendMsg(dvcType,dvcId,mtrlID,p_MeterAddrResp_93);
                    Refresh_CtrInfo_Result_for_AssignAddr(p_CtrInfoList->at(0),mtrlID);

                    if(p_CtrInfoList->at(0)->sucsRate_CtrlCmd[0]>=1.0)
                    {
                        emScriptRunState=Wait_00F1_for_05F1_SetCcoAddr;
                        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                        sendMsg(p_CtrInfoList->at(0)->slotPosition,p_CtrInfoList->at(0)->dvcId,INSIGNIFICANCE,p_SetCcoAddr_05F1);
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("发送--设置主节点地址（05F1），等待--确认"));
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
void BuildNetwork_Zhejiang::processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID)
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
        if(MsgBase_OOP_ptr==nullptr)
        {
            continue;
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
            if(MsgBase_OOP_ptr->service_type_==GET_REQUEST_CLIENT&&MsgBase_OOP_ptr->service_sub_type_==uchar(GetRequestType::kGetRequestNormal))
            {
                shared_ptr<GetRequestNormal> p_GetRequestNormal=dynamic_pointer_cast<GetRequestNormal>(MsgBase_OOP_ptr);
                if(p_GetRequestNormal->oad_.OI==ComuAddr)
                {
                    sendMsg(dvcType,dvcId,mtrlID,p_GetResponseNormal_ReadAddr);
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
void BuildNetwork_Zhejiang::processMsgFromCJQ(DvcType dvcType,int dvcId)
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
        }
    }
}
void BuildNetwork_Zhejiang::sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame)
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
        sendMsgLog=QString("》》设置频段05F6：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
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
void BuildNetwork_Zhejiang::sendSrcMsg(DvcType dvcType, int dvcId, QByteArray msg)
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
}

QList<int> BuildNetwork_Zhejiang::getDvcIdList(DvcType dvcType)
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
uchar BuildNetwork_Zhejiang::findDvcTypeToPowerOnOrRst()
{
    uchar temp_type = 0x00;
    for(int i=0; i<p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size(); i++)
    {
        if(SingleSTA == p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(i)->slotPosition)
        {
            temp_type = temp_type | 0x02;
            qDebug()<<QString("找到单通槽位档案");
        }
        else if (ThreeSTA == p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(i)->slotPosition)
        {
            temp_type = temp_type | 0x01;
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
void BuildNetwork_Zhejiang::findDvcToPowerOnOrRst(QString str)
{

    if ((temp_DvcType&0x01) == 0x01)
    {
        emDvcType = ThreeSTA;
        temp_DvcType = temp_DvcType & 0xFE;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("STA三通_")+str+QString("，等待--确认"));
    }
    else if((temp_DvcType&0x02) == 0x02)
    {
        emDvcType = SingleSTA;
        temp_DvcType = temp_DvcType & 0xFD;
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
            powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost);
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

                emScriptRunState=Wait_00F1_for_05F1_SetCcoAddr;
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                sendMsg(p_CtrInfoList->at(0)->slotPosition,p_CtrInfoList->at(0)->dvcId,INSIGNIFICANCE,p_SetCcoAddr_05F1);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("发送--设置主节点地址（05F1），等待--确认"));
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

