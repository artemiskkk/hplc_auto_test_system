#include "Script_CCO_AddrManage_normal.h"

Script_CCO_AddrManage_normal::Script_CCO_AddrManage_normal(QObject *parent) : QObject(parent)
{
    emScriptRunState=TestInit;
    resultFlag=false;
    sendMsgOct.clear();

    p_CtrInfoList=make_shared<QList<shared_ptr<CtrInfo>>>();
    p_BuildNetwork_GW=new BuildNetwork_GW();

    p_Frame3762Helper=make_shared<Frame3762Helper>();
    p_QueryCcoAddr_03F4_Down=make_shared<Afn03F4>();
    p_SetCcoAddr_05F1_Down=make_shared<Afn05F1>();
    p_HardInit_01F1_Down=make_shared<Afn01F1>();
    p_ParaInit_01F2_Down=make_shared<Afn01F2>();

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
Script_CCO_AddrManage_normal::~Script_CCO_AddrManage_normal()
{
   // p_BuildNetwork_GW->initBuildNetWork();
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

void Script_CCO_AddrManage_normal::execute()
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
        emScriptRunState=Init_SetCcoBaudRate;
        sendParams.clear();
        idList.clear();
        sendParams.append(9600);
        idList = findDvcIdList(p_CtrInfoList,CCO_GW);
        p_AbstractScriptHost->controlDvc(CCO_GW,idList,CtrlCmd_SetBaudRate,sendParams);
        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
        tryTimes=0;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("设置CCO波特率，等待--确认"));
    }
}

void Script_CCO_AddrManage_normal::stop()
{
    if(p_AbstractScriptHost==nullptr)
        return;
    p_AbstractScriptHost->updateProgress(ProcessState_Error, "Test stop!");
    powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost);
}
void Script_CCO_AddrManage_normal::addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq)
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
void Script_CCO_AddrManage_normal::setHost(AbstractScriptHost *host)
{
    p_AbstractScriptHost=host;
    p_BuildNetwork_GW->setHost(host);
}
bool Script_CCO_AddrManage_normal::config(const QMap<QString,QString> *paraDic)
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
        if(paraDic->keys().contains("ccoNewAddr"))
        {
            ccoNewAddr.clear();
            this->ccoNewAddr=QByteArray::fromHex((*paraDic)["ccoNewAddr"].toStdString().data());
        }
        result = true;
    }
    return result;
}
void Script_CCO_AddrManage_normal::processMsg(DvcType dvcType, int id, uchar *data, int datalen)
{
    if(p_CtrInfoList->size()==0)
        return;
    if(emScriptRunState==Wait_BuildNetFinish_Whole)
    {
        if(!p_BuildNetwork_GW->buildNetworkResultFlag)
            p_BuildNetwork_GW->processMsg(dvcType,id,data,datalen);
        else
        {
            p_BuildNetwork_GW->initBuildNetWork();
            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryCcoAddr_03F4_Down);
            emScriptRunState=Wait_QueryCcoInitAddr_Finish;
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询初始主节点地址，等待--回复");
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
void Script_CCO_AddrManage_normal::processCtrlDvcRes(DvcType dvcType, QList<int> idList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params)
{
    if(isSucs==false)
        return;
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到控制命令回复，参数个数=%1").arg(QString::number(params.size())));
    switch(emScriptRunState)
    {
    case TestInit:
    {
        break;
    }
    case Init_SetCcoBaudRate:
    {
        if (dvcType == CCO_GW)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("CCO槽位_设置波特率成功"));
            emScriptRunState=Init_PowerOnCco;

            sendParams.clear();
            idList.clear();
            idList = findDvcIdList(p_CtrInfoList,CCO_GW);
            p_AbstractScriptHost->controlDvc(CCO_GW,idList,CtrlCmd_PowerOn_12V,sendParams);
            p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
            tryTimes=0;
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("CCO上电，等待--确认"));
        }
        break;
    }
    case Init_PowerOnCco:
    {
        if (dvcType == CCO_GW)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("CCO上电成功,等待10s..."));
            emScriptRunState=CcoPowerOnInit;
            p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
        }
        break;
    }
    case Wait_BuildNetFinish_Whole:
    {
        p_BuildNetwork_GW->processCtrlDvcRes(dvcType,idList,ctrlCmdType,isSucs,params);
        break;
    }
    case Wait_HardInit_Ack:
    {
        break;
    }
    case Wait_ParaInit_Ack:
    {
        break;
    }
    case ScriptComplete:
    {
        break;
    }
    default:
        break;
    }
}

void Script_CCO_AddrManage_normal::processMsgFromCCO(DvcType dvcType, int dvcId)
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
        case TestInit:
        {
            break;
        }
        case Wait_BuildNetFinish_Whole:
        {
            break;
        }
        case Wait_QueryCcoInitAddr_Finish:
        {
            dtValue3762=get3762Dt(p_Frame3762Base->dt1_,p_Frame3762Base->dt2_);
            if(p_Frame3762Base->afn_ == 0x03&&dtValue3762==4&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                shared_ptr<Afn03F4> p_QueryCcoInitAddr_03F4_Up=dynamic_pointer_cast<Afn03F4>(p_Frame3762Base);
                if(isArrayEqual(p_CtrInfoList->at(0)->ccoAddr,reinterpret_cast<uchar*>(p_QueryCcoInitAddr_03F4_Up->master_node_address_.addr),6))
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "初始主节点地址正确");
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_SetCcoAddr_05F1_Down);
                    emScriptRunState=Wait_SetCcoAddr_Finish;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--设置主节点地址，等待--确认");
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                }
                else
                {
                    ccoAddrNow.clear();
                    for(int i=0;i<6;i++)
                    {
                        ccoAddrNow.append(p_QueryCcoInitAddr_03F4_Up->master_node_address_.addr[i]);
                    }
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, "初始主节点地址有误："+QString(ccoAddrNow.toHex()));
                    emScriptRunState=ScriptComplete;
                }
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case Wait_SetCcoAddr_Finish:
        {
            dtValue3762=get3762Dt(p_Frame3762Base->dt1_,p_Frame3762Base->dt2_);
            if(p_Frame3762Base->afn_ == 0x00&& dtValue3762==1 &&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到设置主节点地址确认，等待10s...");
                emScriptRunState=Wait_CcoReset;
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            }
            else if(p_Frame3762Base->afn_ == 0x00&& dtValue3762==2 &&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                p_AbstractScriptHost->updateProgress(ProcessState_Failed, "设置主节点地址否认");
                emScriptRunState=ScriptComplete;
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case Wait_QueryCcoNewAddr_Finish:
        {
            dtValue3762=get3762Dt(p_Frame3762Base->dt1_,p_Frame3762Base->dt2_);
            if(p_Frame3762Base->afn_ == 0x03&&dtValue3762==4&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                shared_ptr<Afn03F4> p_QueryCcoNewAddr_03F4_Up=dynamic_pointer_cast<Afn03F4>(p_Frame3762Base);
                if(memcmp(ccoNewAddr,p_QueryCcoNewAddr_03F4_Up->master_node_address_.addr,6)==0)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "查询主节点新地址正确，路由执行断电");
                    //路由断电
                    idList.clear();
                    sendParams.clear();
                    idList = findDvcIdList(p_CtrInfoList,CCO_GW);
                    p_AbstractScriptHost->controlDvc(CCO_GW,idList,CtrlCmd_PowerOff_12V,sendParams);
                    //  QThread::msleep(50);

                    emScriptRunState=Wait_CcoPoweroff;
                    p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                }
                else
                {
                    ccoAddrNow.clear();
                    for(int i=0;i<6;i++)
                    {
                        ccoAddrNow.append(p_QueryCcoNewAddr_03F4_Up->master_node_address_.addr[i]);
                    }
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, "主节点地址设置失败："+QString(ccoAddrNow.toHex()));
                    emScriptRunState=ScriptComplete;
                }
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case Wait_QueryCcoAddrAfterPoweroff_Finish:
        {
            dtValue3762=get3762Dt(p_Frame3762Base->dt1_,p_Frame3762Base->dt2_);
            if(p_Frame3762Base->afn_ == 0x03&&dtValue3762==4&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                shared_ptr<Afn03F4> p_QueryCcoAddrAfterPoweroff_03F4_Up=dynamic_pointer_cast<Afn03F4>(p_Frame3762Base);
                if(!memcmp(p_QueryCcoAddrAfterPoweroff_03F4_Up->master_node_address_.addr,ccoNewAddr,6))
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "断电后查询主节点地址正确");
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_HardInit_01F1_Down);
                    emScriptRunState=Wait_HardInit_Ack;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--硬件初始化，等待--确认");
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                }
                else
                {
                    ccoAddrNow.clear();
                    for(int i=0;i<6;i++)
                    {
                        ccoAddrNow.append(p_QueryCcoAddrAfterPoweroff_03F4_Up->master_node_address_.addr[i]);
                    }
                    //   memcpy(ccoAddrNow.data(),p_QueryCcoAddrAfterPoweroff_03F4_Up->master_node_address_.addr,6);
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, "断电后查询主节点地址有误："+QString(ccoAddrNow.toHex()));
                }
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case Wait_HardInit_Ack:
        {
            dtValue3762=get3762Dt(p_Frame3762Base->dt1_,p_Frame3762Base->dt2_);
            if(p_Frame3762Base->afn_ == 0x00&& dtValue3762==1 &&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到硬件初始化确认，等待10s...");
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                emScriptRunState=Wait_CcoHardInit;
            }
            else if(p_Frame3762Base->afn_ == 0x00&& dtValue3762==2 &&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                p_AbstractScriptHost->updateProgress(ProcessState_Failed, "硬件初始化否认");
                emScriptRunState=ScriptComplete;
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case Wait_QueryCcoAddrAfterHardInit_Finish:
        {
            dtValue3762=get3762Dt(p_Frame3762Base->dt1_,p_Frame3762Base->dt2_);
            if(p_Frame3762Base->afn_ == 0x03&&dtValue3762==4&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                shared_ptr<Afn03F4> p_QueryCcoAddrAfterHardInit_03F4_Up=dynamic_pointer_cast<Afn03F4>(p_Frame3762Base);
                if(!memcmp(ccoNewAddr,p_QueryCcoAddrAfterHardInit_03F4_Up->master_node_address_.addr,6))
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "硬件初始化后查询主节点地址正确");
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_ParaInit_01F2_Down);
                    emScriptRunState=Wait_ParaInit_Ack;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--参数初始化，等待--确认");
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                }
                else
                {
                    ccoAddrNow.clear();
                    for(int i=0;i<6;i++)
                    {
                        ccoAddrNow.append(p_QueryCcoAddrAfterHardInit_03F4_Up->master_node_address_.addr[i]);
                    }
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, "硬件初始化后查询主节点地址有误："+QString(ccoAddrNow.toHex()));
                }
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case Wait_ParaInit_Ack:
        {
            dtValue3762=get3762Dt(p_Frame3762Base->dt1_,p_Frame3762Base->dt2_);
            if(p_Frame3762Base->afn_ == 0x00&& dtValue3762==1 &&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到参数初始化确认，等待10s...");
                emScriptRunState=Wait_CcoParaInit;
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            }
            else if(p_Frame3762Base->afn_ == 0x00&& dtValue3762==2 &&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                p_AbstractScriptHost->updateProgress(ProcessState_Failed, "参数初始化否认");
                emScriptRunState=ScriptComplete;
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case Wait_QueryCcoAddrAfterParaInit_Finish:
        {
            dtValue3762=get3762Dt(p_Frame3762Base->dt1_,p_Frame3762Base->dt2_);
            if(p_Frame3762Base->afn_ == 0x03&&dtValue3762==4&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                shared_ptr<Afn03F4> p_QueryCcoAddrAfterParaInit_03F4_Up=dynamic_pointer_cast<Afn03F4>(p_Frame3762Base);
                if(0==memcmp(ccoNewAddr,p_QueryCcoAddrAfterParaInit_03F4_Up->master_node_address_.addr,6))
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Success, "参数初始化后查询主节点地址正确");
                    emScriptRunState=ScriptComplete;
                }
                else
                {
                    ccoAddrNow.clear();
                    for(int i=0;i<6;i++)
                    {
                        ccoAddrNow.append(p_QueryCcoAddrAfterParaInit_03F4_Up->master_node_address_.addr[i]);
                    }
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, "参数初始化后查询主节点地址有误："+QString(ccoAddrNow.toHex()));
                }
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case ScriptComplete:
        {
            break;
        }
        default:
        {
            QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
            sendSrcMsg(dvcType,dvcId,tmpSendMsg);
        }
        }
    }
}
void Script_CCO_AddrManage_normal::processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID)
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
            case ScriptComplete:
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
void Script_CCO_AddrManage_normal::processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID)
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
            case ScriptComplete:
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
void Script_CCO_AddrManage_normal::sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame)
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
    else if(frame==p_ParaInit_01F2_Down)
    {
        p_ParaInit_01F2_Down->ctrl_field_.dir=kDirDown;
        p_ParaInit_01F2_Down->ctrl_field_.prm=kActive;
        p_ParaInit_01F2_Down->ctrl_field_.comn_type=kHplc;

        p_ParaInit_01F2_Down->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_ParaInit_01F2_Down->info_field_.info_field_down.comu_rate=0;
        p_ParaInit_01F2_Down->info_field_.info_field_down.comu_module_ident=0;

        sendMsgOct=p_ParaInit_01F2_Down->EncodeFrame();
        sendMsgLog=QString("》》路由参数初始化01F2：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
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

        memcpy(p_SetCcoAddr_05F1_Down->primary_node_address_.addr,ccoNewAddr,6);
        sendMsgOct=p_SetCcoAddr_05F1_Down->EncodeFrame();
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
void Script_CCO_AddrManage_normal::sendSrcMsg(DvcType dvcType, int dvcId, QByteArray msg)
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

void Script_CCO_AddrManage_normal::timer_timeoutProc()
{
    p_timer->stop();
    switch(emScriptRunState)
    {
    case Wait_BuildNetFinish_Whole:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_BuildNetFinish_Whole timeout!!!");
        break;
    }
    case Init_SetCcoBaudRate:
    {
        if(++tryTimes>=3)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_SetCcoBaudRate_Finish timeout!!!");
        }
        else
        {
            p_AbstractScriptHost->controlDvc(CCO_GW,idList,CtrlCmd_SetBaudRate,sendParams);
            p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("重发设置CCO波特率，等待--确认"));
        }
        break;
    }
    case Init_PowerOnCco:
    {
        if(++tryTimes>=3)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_PowerOnCco_Finish timeout!!!");
        }
        else
        {
            p_AbstractScriptHost->controlDvc(CCO_GW,idList,CtrlCmd_PowerOn_12V,sendParams);
            p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("重发CCO上电，等待--确认"));
        }
        break;
    }
    case CcoPowerOnInit:
    {
        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryCcoAddr_03F4_Down);
        emScriptRunState=Wait_QueryCcoInitAddr_Finish;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询初始主节点地址，等待--回复");
        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
        break;
    }
    case Wait_QueryCcoInitAddr_Finish:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_QueryCcoInitAddr_Finish timeout!!!");
        break;
    }
    case Wait_SetCcoAddr_Finish:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_SetCcoAddr_Finish timeout!!!");
        break;
    }
    case Wait_CcoReset:
    {
        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryCcoAddr_03F4_Down);
        emScriptRunState=Wait_QueryCcoNewAddr_Finish;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--设置后查询主节点地址，等待--");
        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
        break;
    }
    case Wait_QueryCcoNewAddr_Finish:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_QueryCcoNewAddr_Finish timeout!!!");
        break;
    }
    case Wait_CcoPoweroff:
    {
        p_timer->stop();
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "路由断电5s后上电");
        //路由上电
        idList.clear();
        sendParams.clear();
        idList = findDvcIdList(p_CtrInfoList,CCO_GW);
        p_AbstractScriptHost->controlDvc(CCO_GW,idList,CtrlCmd_PowerOn_12V,sendParams);

        emScriptRunState=Wait_CcoPoweron;
        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
        break;
    }
    case Wait_CcoPoweron:
    {
        p_timer->stop();
        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryCcoAddr_03F4_Down);
        emScriptRunState=Wait_QueryCcoAddrAfterPoweroff_Finish;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--路由重新上电后查询主节点地址，等待--");
        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
        break;
    }
    case Wait_QueryCcoAddrAfterPoweroff_Finish:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_QueryCcoAddrAfterPoweroff_Finish timeout!!!");
        break;
    }
    case Wait_HardInit_Ack:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_HardwareInit_Finish timeout!!!");
        break;
    }
    case Wait_CcoHardInit:
    {
        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryCcoAddr_03F4_Down);
        emScriptRunState=Wait_QueryCcoAddrAfterHardInit_Finish;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--硬件初始化后查询主节点地址，等待--");
        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
        break;
    }
    case Wait_QueryCcoAddrAfterHardInit_Finish:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_QueryCcoAddrAfterHardInit_Finish timeout!!!");
        break;
    }
    case Wait_ParaInit_Ack:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_ParaInit_Ack timeout!!!");
        break;
    }
    case Wait_CcoParaInit:
    {
        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryCcoAddr_03F4_Down);
        emScriptRunState=Wait_QueryCcoAddrAfterParaInit_Finish;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--参数初始化后查询主节点地址，等待--");
        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
        break;
    }
    case Wait_QueryCcoAddrAfterParaInit_Finish:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_QueryCcoAddrAfterParaInit_Finish timeout!!!");
        break;
    }
    default:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==p_timer");
        break;
    }
    }
}

void Script_CCO_AddrManage_normal::maxAllowTimer_timeoutProc()
{
/*    switch(emScriptRunState)
    {
        case Wait_QueryInitWorkMode_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_QueryInitWorkMode_Finish maxAllow timeout!!!");
            break;
        }
        case Wait_HardResetTest_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_HardResetTest_Finish maxAllow timeout!!!");
            break;
        }
        case Wait_ParaInitTest_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_ParaInitTest_Finish maxAllow timeout!!!");
            break;
        }
        default:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==p_maxAllowTimer");
            break;
        }
    }*/
}

void Script_CCO_AddrManage_normal::delayTimer_timeoutProc()
{
    p_delayTimer->stop();
    index=0;
    p_maxAllowTimer->stop();
}
