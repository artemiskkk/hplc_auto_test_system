#include "Script_RouteRunningStatus_Gansu.h"

Script_RouteRunningStatus_Gansu::Script_RouteRunningStatus_Gansu(QObject *parent) : QObject(parent)
{
    emScriptRunState=TestInit;
    resultFlag=false;
    sendMsgOct.clear();
    transResList.clear();

    p_CtrInfoList=make_shared<QList<shared_ptr<CtrInfo>>>();
    p_BuildNetwork_GW=new BuildNetwork_GW();

    p_Frame3762Helper=make_shared<Frame3762Helper>();
//    p_HardReset_01F1=make_shared<Afn01F1>();
//    p_ParaInit_01F2 = make_shared<Afn01F2>();
//    p_QueryNetScale_10F9=make_shared<Afn10F9>();
//    p_QueryNodeNum_10F1=make_shared<Afn10F1>();
//    p_AddSlaveNode_11F1=make_shared<Afn11F1>();
//    p_QueryBasicNetworkInfo_F0F100=make_shared<AfnF0F100>();
//    p_QueryNetTopoInfo_10F21=make_shared<Afn10F21>();
    p_QueryRouteRunStatus_10F4=make_shared<Afn10F4_Beijing>();
    p_StationIdentiSwitch_05F6=make_shared<Afn05F6>();
    p_StartMeterSearch_11F5=make_shared<Afn11F5>();
    p_StopMeterSearch_11F6=make_shared<Afn11F6>();
    p_FileTransfer_15F1_Down=make_shared<Afn15F1>();

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
Script_RouteRunningStatus_Gansu::~Script_RouteRunningStatus_Gansu()
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

void Script_RouteRunningStatus_Gansu::execute()
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
        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryRouteRunStatus_10F4);
        emScriptRunState=Wait_QueryRouteRunStatus_10F4_Finish;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询路由运行状态（10F4），等待--回复");
        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);

        netScale=ushort(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size())+1;


//        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_HardReset_01F1);
//        emScriptRunState=Wait_HardReset_Finish;
//        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--硬件初始化（01F1），等待--确认");
//        p_timer->start(10*1000);

    }
    p_maxAllowTimer->start((timerForReachThresld+timerAfterReachThresld)*1000);
}

void Script_RouteRunningStatus_Gansu::stop()
{
    if(p_AbstractScriptHost==nullptr)
        return;
    p_AbstractScriptHost->updateProgress(ProcessState_Error, "Test stop!");
    if(needPowerOff==true)
        powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost);
}

void Script_RouteRunningStatus_Gansu::addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq)
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

void Script_RouteRunningStatus_Gansu::setHost(AbstractScriptHost *host)
{
    p_AbstractScriptHost=host;
    p_BuildNetwork_GW->setHost(host);
}
bool Script_RouteRunningStatus_Gansu::config(const QMap<QString,QString> *paraDic)
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

void Script_RouteRunningStatus_Gansu::processMsg(DvcType dvcType,int id,uchar* data,int datalen)
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
//            if(p_BuildNetwork_GW->emScriptRunState==BuildNetFinish&&p_BuildNetwork_GW->resultFlag==true)
//            {
                tryTimes=0;
                netScale=ushort(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size())+1;

                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryRouteRunStatus_10F4);
                emScriptRunState=Wait_QueryRouteRunStatus_10F4_Finish;
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询路由运行状态（10F4），等待--回复");
                p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);

//                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_HardReset_01F1);
//                emScriptRunState=Wait_HardReset_Finish;
//                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "组网完成，发送--硬件初始化（01F1），等待--确认");
//                p_timer->start(10*1000);
   //         }
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
                if(dvcType==SingleSTA)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到单通报文：%1").arg(QString(recvTempData.toHex())));
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到三通报文：%1").arg(QString(recvTempData.toHex())));
                }
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

void Script_RouteRunningStatus_Gansu::processCtrlDvcRes(DvcType dvcType,QList<int> idList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params)
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

//        case Wait_HardReset_Finish:
//        {
//            break;
//        }
//        case Wait_ParaInit_Finish:
//        {
//            break;
//        }
//        case Wait_AddSlaveNode_11F1_Finish:
//        {
//           break;
//        }
//        case Wait_QueryNetTopoInfo_10F21_Finish:
//        {
//            break;
//        }
        case Wait_QueryRouteRunStatus_10F4_Finish:
        {
            break;
        }
        case Wait_StationIdentiSwitch_05F6_Open_Finish:
        {
            break;
        }
        case Wait_QueryRouteRunStatus_10F4_2_Finish:
        {
            break;
        }
        case Wait_StationIdentiSwitch_05F6_Close_Finish:
        {
            break;
        }
        case Wait_QueryRouteRunStatus_10F4_3_Finish:
        {
            break;
        }
        case Wait_StartMeterSearch_11F5_Finish:
        {
            break;
        }
        case Wait_QueryRouteRunStatus_10F4_4_Finish:
        {
            break;
        }
        case Wait_StopMeterSearch_11F6_Finish:
        {
            break;
        }
        case Wait_QueryRouteRunStatus_10F4_5_Finish:
        {
            break;
        }
        case Wait__15F1_BeforeUpgrdCco_Finish:
        {
            break;
        }
        case Wait_FileTransfer_15F1_Down_Finish:
        {
            break;
        }
        case Wait_QueryRouteRunStatus_10F4_6_Finish:
        {
            break;
        }
        case Wait_QueryRouteRunStatus_10F4_7_Finish:
        {
            break;
        }
        case ScriptSuccess:
        {
            break;
        }
    }
}

void Script_RouteRunningStatus_Gansu::sendSrcMsg(DvcType dvcType, int dvcId, QByteArray msg)
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

void Script_RouteRunningStatus_Gansu::processMsgFromCCO(DvcType dvcType, int dvcId)
{
    if(dvcId!=p_CtrInfoList->at(0)->ctrlID)
        return;
    bool haveCompleteMsg=true;
    while(haveCompleteMsg)
    {
        qInfo()<<QString("CCO-解析前 buf3762=%1").arg(QString((p_CtrInfoList->at(0)->buf.toHex())));
        shared_ptr<Frame3762Base> p_Frame3762Base = p_Frame3762Helper->DecodeLocalMsg(&(p_CtrInfoList->at(0)->buf),haveCompleteMsg,Beijing);
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
//            case Wait_HardReset_Finish:
//            {
//                if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
//                {
//                    p_timer->stop();//接收到一条完整报文
//                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "硬件初始化回复确认！");
//                }
//                else if(p_Frame3762Base->afn_ == 0x03&&p_Frame3762Base->dt1_==0x02&&p_Frame3762Base->dt2_==0x01&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
//                {
//                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_ParaInit_01F2);
//                    emScriptRunState=Wait_ParaInit_Finish;
//                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--参数初始化（01F2），等待--确认");
//                    p_timer->start(10*1000);
//                }
//                else
//                {
//                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
//                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
//                }
//                break;
//            }
//            case Wait_ParaInit_Finish:
//            {
//                if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
//                {
//                    p_timer->stop();//接收到一条完整报文
//                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "参数初始化回复确认！");
//                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_AddSlaveNode_11F1);
//                    emScriptRunState=Wait_AddSlaveNode_11F1_Finish;
//                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--添加从节点（11F1），等待--确认");
//                    p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
//                }
//                else
//                {
//                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
//                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
//                }
//                break;
//            }

//            case Wait_AddSlaveNode_11F1_Finish:
//            {
//                if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
//                {
//                    p_timer->stop();//接收到一条完整报文
//                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("添加从节点收到确认！"));
//                    index+=num;
//                    if(index<p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size())
//                    {
//                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_AddSlaveNode_11F1);
//                        emScriptRunState=Wait_AddSlaveNode_11F1_Finish;
//                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--添加从节点（11F1），等待--确认");
//                        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
//                    }
//                    else
//                    {
//                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询网络拓扑信息(10F21)，等待--确认");
//                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryNetTopoInfo_10F21);
//                        emScriptRunState=Wait_QueryNetTopoInfo_10F21_Finish;
//                        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
//                    }
//                }
//                else
//                {
//                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
//                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
//                }
//                break;
//            }








//            case Wait_QueryNetTopoInfo_10F21_Finish:
//            {
//                if(p_Frame3762Base->afn_ == 0x10&&p_Frame3762Base->dt1_==0x10&&p_Frame3762Base->dt2_==0x02&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
//                {
//                    p_timer->stop();//接收到一条完整报文
//                    shared_ptr<Afn10F21> p_QueryNetTopoInfo_10F21_Up=dynamic_pointer_cast<Afn10F21>(p_Frame3762Base);
//                    if(p_QueryNetTopoInfo_10F21_Up->network_typelogy_info_unit_.this_node_num_==netScale-1&&
//                            p_QueryNetTopoInfo_10F21_Up->network_typelogy_info_unit_.node_start_no_==0x01)
//                    {
//                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryRouteRunStatus_10F4);
//                        emScriptRunState=Wait_QueryRouteRunStatus_10F4_Finish;
//                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询路由运行状态（10F4），等待--回复");
//                        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
//                    }
//                    else
//                    {
//                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("当前节点总数量为%1，不符合要求").arg(p_QueryNetTopoInfo_10F21_Up->network_typelogy_info_unit_.node_total_num_));
//                    }
//                }
//                else
//                {
//                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
//                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
//                }
//                break;
//            }
            case Wait_QueryRouteRunStatus_10F4_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x10&&p_Frame3762Base->dt1_==0x08&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    qDebug()<<"到11";
                    shared_ptr<Afn10F4_Beijing> p_QueryRouteRunStatus_10F4_Up=dynamic_pointer_cast<Afn10F4_Beijing>(p_Frame3762Base);
//                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString::number((p_QueryRouteRunStatus_10F4_Up->router_operate_state_unit_.work_switch_.work_state_)));
//                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString::number(p_QueryRouteRunStatus_10F4_Up->router_operate_state_unit_.work_switch_.work_state_1&0xff));
//                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString(QByteArray(reinterpret_cast<char*>(p_QueryRouteRunStatus_10F4_Up->router_operate_state_unit_.work_switch_.current_state_),1).toHex()));
qDebug()<<"到12";
                    if(uchar(p_QueryRouteRunStatus_10F4_Up->router_operate_state_unit_.work_switch_.work_state_1)==3)
                    {
//                    if(GetBit(p_QueryRouteRunStatus_10F4_Up->router_operate_state_unit_.work_switch_.current_state_,3)=="1"&&
//                            GetBit(p_QueryRouteRunStatus_10F4_Up->router_operate_state_unit_.work_switch_.current_state_,4)=="1")
//                    {

                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_StationIdentiSwitch_05F6);
                        emScriptRunState=Wait_StationIdentiSwitch_05F6_Open_Finish;
                        emAreadifference_State = OpenIdenti;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--开启台区识别开关（05F6），等待--确认");
                        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("台区开关不是关闭状态"));
                    }
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }

            case Wait_StationIdentiSwitch_05F6_Open_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "开启台区识别开关回复确认！");
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryRouteRunStatus_10F4);
                    emScriptRunState=Wait_QueryRouteRunStatus_10F4_2_Finish;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询路由运行状态（10F4）_2，等待--回复");
                    p_timer->start(10*1000);
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;

            }
            case Wait_QueryRouteRunStatus_10F4_2_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x10&&p_Frame3762Base->dt1_==0x08&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    shared_ptr<Afn10F4_Beijing> p_QueryRouteRunStatus_10F4_Up=dynamic_pointer_cast<Afn10F4_Beijing>(p_Frame3762Base);
                    if(uchar(p_QueryRouteRunStatus_10F4_Up->router_operate_state_unit_.work_switch_.area_difference_flag_)==0x01)
                    {
                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_StationIdentiSwitch_05F6);
                        emScriptRunState=Wait_StationIdentiSwitch_05F6_Close_Finish;
                        emAreadifference_State = CloseIdenti;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--关闭台区识别开关（05F6），等待--确认");
                        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("台区开关不是开启状态"));
                    }
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_StationIdentiSwitch_05F6_Close_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "关闭台区识别开关回复确认！");
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryRouteRunStatus_10F4);
                    emScriptRunState=Wait_QueryRouteRunStatus_10F4_3_Finish;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询路由运行状态（10F4）_3，等待--回复");
                    p_timer->start(10*1000);
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;

            }
        case Wait_QueryRouteRunStatus_10F4_3_Finish:
        {
            if(p_Frame3762Base->afn_ == 0x10&&p_Frame3762Base->dt1_==0x08&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                shared_ptr<Afn10F4_Beijing> p_QueryRouteRunStatus_10F4_Up=dynamic_pointer_cast<Afn10F4_Beijing>(p_Frame3762Base);
                if(uchar(p_QueryRouteRunStatus_10F4_Up->router_operate_state_unit_.work_switch_.area_difference_flag_)==0x00&&
                        uchar(p_QueryRouteRunStatus_10F4_Up->router_operate_state_unit_.work_switch_.work_state_1)==0x03)
                {
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_StartMeterSearch_11F5);
                    emScriptRunState=Wait_StartMeterSearch_11F5_Finish;
//                    emAreadifference_State = CloseIdenti;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--启动搜表（11F5），等待--确认");
                    p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("台区开关不是关闭状态"));
                }
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case Wait_StartMeterSearch_11F5_Finish:
        {
            if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "启动搜表回复确认！");
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryRouteRunStatus_10F4);
                emScriptRunState=Wait_QueryRouteRunStatus_10F4_4_Finish;
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询路由运行状态（10F4）_4，等待--回复");
                p_timer->start(10*1000);
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case Wait_QueryRouteRunStatus_10F4_4_Finish:
        {
            if(p_Frame3762Base->afn_ == 0x10&&p_Frame3762Base->dt1_==0x08&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                shared_ptr<Afn10F4_Beijing> p_QueryRouteRunStatus_10F4_Up=dynamic_pointer_cast<Afn10F4_Beijing>(p_Frame3762Base);
                if(uchar(p_QueryRouteRunStatus_10F4_Up->router_operate_state_unit_.work_switch_.work_state_1)==0x01)
                {
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_StopMeterSearch_11F6);
                    emScriptRunState=Wait_StopMeterSearch_11F6_Finish;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--停止搜表（11F6），等待--确认");
                    p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("启动搜表失败"));
                }
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case Wait_StopMeterSearch_11F6_Finish:
        {
            if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "停止搜表回复确认！");
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryRouteRunStatus_10F4);
                emScriptRunState=Wait_QueryRouteRunStatus_10F4_5_Finish;
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询路由运行状态（10F4）_5，等待--回复");
                p_timer->start(10*1000);
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case Wait_QueryRouteRunStatus_10F4_5_Finish:
        {
            if(p_Frame3762Base->afn_ == 0x10&&p_Frame3762Base->dt1_==0x08&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                shared_ptr<Afn10F4_Beijing> p_QueryRouteRunStatus_10F4_Up=dynamic_pointer_cast<Afn10F4_Beijing>(p_Frame3762Base);
                if(uchar(p_QueryRouteRunStatus_10F4_Up->router_operate_state_unit_.work_switch_.work_state_1)==0x03)
                {
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_FileTransfer_15F1_Down);
                    emScriptRunState=Wait__15F1_BeforeUpgrdCco_Finish;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "文件传输（15F1），等待--确认");
                    p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("路由状态有误"));
                }
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case Wait__15F1_BeforeUpgrdCco_Finish:
        {
          //  ushort dtValue3762=p_MsgBase_1376_2->p_DT_3762->fnGetDTvalue(p_MsgBase_1376_2->p_DT_3762->DT[0],p_MsgBase_1376_2->p_DT_3762->DT[1]);
           // if(p_MsgBase_1376_2->ucAFN==0x15 && dtValue3762==1  && p_MsgBase_1376_2->stCtrlField.DIR==UP_3762)
//            dtValue3762=get3762Dt(p_Frame3762Base->dt1_,p_Frame3762Base->dt2_);
//            if(p_Frame3762Base->afn_ == 0x15&&dtValue3762==1&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
//            {
            if(p_Frame3762Base->afn_ == 0x15&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                qDebug()<<"收到15F1_Up清除下装文件回复******"<<"状态机的值："<<emScriptRunState;
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到15F1_Up清除下装文件应答");
                p_timer->stop();
//                if(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.operate_state_word_.report_work_flag_==1 || p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.work_switch_.current_state_!=3)
//                {
//                    //emScriptRunState=ScriptComplete;
//                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("路由工作状态错误：工作标志%1，当前状态%2").arg(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.operate_state_word_.report_work_flag_).arg(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.work_switch_.current_state_));
//                   // break;
//                }

                /****调入升级包并计算每段长度和总段数*****/
                LoadUpdateFile();

                emScriptRunState=Wait_FileTransfer_15F1_Down_Finish;
//                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,p_FileTransfer_15F1_Down);
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_FileTransfer_15F1_Down);
                p_timer->start(30*1000);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "等待升级文件传输完成");
             //   p_maxAllowTimer->start(timerForReachThresld_Upgrade*1000);
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case Wait_FileTransfer_15F1_Down_Finish:
        {
            if(p_Frame3762Base->afn_ == 0x15&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                qDebug()<<"收到15F1_Up报文******"<<"状态机的值："<<emScriptRunState;
                p_timer->stop();

              //  FileTransfer_15F1_Up *p_FileTransfer_15F1_Up=(FileTransfer_15F1_Up*)p_MsgBase_1376_2->p_DT_3762;
                shared_ptr<Afn15F1> p_FileTransfer_15F1_Up=dynamic_pointer_cast<Afn15F1>(p_Frame3762Base);
                Refresh_TestResult_15F1(p_FileTransfer_15F1_Up);

                ushort sucSegs=0;
                if(++fileIndex==p_FileTransfer_15F1_Down->file_transfer_unit_.total_num_)
                {
                    sucSegs=Refresh_SuccessCnt_15F1();
                }

                if(sucSegs==p_FileTransfer_15F1_Down->file_transfer_unit_.total_num_)
                {
//                    p_CtrInfoList->at(0)->successConsume[0]=(double)(timerForReachThresld_Upgrade*1000-p_maxAllowTimer->remainingTime())/1000.0;

//                    if(isStdPrcs)
//                    {
//                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,p_CcoRunModeInfo_03F10_Down);
//                        emScriptRunState=Wait_Res_for_03F10_WaitTimeLen;
//                        p_timer->start(10*1000);
//                    }
//                    else
//                    {
//                        emScriptRunState=ScriptComplete;
//                    //    p_maxAllowTimer->start(timerAfterTransferFinished*1000);
//                    }
//                    p_timer->stop();
//                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "停止搜表回复确认！");
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryRouteRunStatus_10F4);
                    emScriptRunState=Wait_QueryRouteRunStatus_10F4_6_Finish;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询路由运行状态（10F4）_6，等待--回复");
                    p_timer->start(10*1000);
                }
                else
                {
                    if(fileIndex<totalSegs)
                    {
//                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,p_FileTransfer_15F1_Down);
                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_FileTransfer_15F1_Down);
                        p_timer->start(30*1000);
                    }
                    else
                    {
                        p_maxAllowTimer->stop();
                        emScriptRunState=ScriptSuccess;
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("  升级文件传输成功率：%1%; 升级文件传输耗时：%2秒;").arg((double)(sucSegs)/(double)(totalSegs)*100).arg(p_CtrInfoList->at(0)->successConsume[0]));

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
        case Wait_QueryRouteRunStatus_10F4_6_Finish:
        {
            if(p_Frame3762Base->afn_ == 0x10&&p_Frame3762Base->dt1_==0x08&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                shared_ptr<Afn10F4_Beijing> p_QueryRouteRunStatus_10F4_Up=dynamic_pointer_cast<Afn10F4_Beijing>(p_Frame3762Base);
                if(uchar(p_QueryRouteRunStatus_10F4_Up->router_operate_state_unit_.work_switch_.work_state_1)==0x02)
                {
//                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryRouteRunStatus_10F4);
                    emScriptRunState=Wait_QueryRouteRunStatus_10F4_7_Finish;
                    p_delayTimer->start(10*1000);
//                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--路由运行状态（10F4），等待--确认");
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("启动搜表失败"));
                }
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case Wait_QueryRouteRunStatus_10F4_7_Finish:
        {
            if(p_Frame3762Base->afn_ == 0x10&&p_Frame3762Base->dt1_==0x08&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_delayTimer->stop();
                shared_ptr<Afn10F4_Beijing> p_QueryRouteRunStatus_10F4_Up=dynamic_pointer_cast<Afn10F4_Beijing>(p_Frame3762Base);

                if(uchar(p_QueryRouteRunStatus_10F4_Up->router_operate_state_unit_.work_switch_.work_state_1)==0x03)
                {
                    emScriptRunState=ScriptSuccess;
                    resultFlag=true;
                    p_AbstractScriptHost->updateProgress(ProcessState_Success, QString("查询网络基本信息测试成功！"));

//                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "延迟30s发送--查询网络规模（10F9），等待--回复");
//                    //延时查询时间，根据网络规模，可以进行配置
//                    p_delayTimer->start(30*1000);
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "延迟30s发送--查询路由运行状态（10F4），等待--回复");
                    //延时查询时间，根据网络规模，可以进行配置
                    p_delayTimer->start(30*1000);

//                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryChipID_10F112);
//                    emScriptRunState=Wait_QueryChipID_10F112_Finish;
//                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询模块芯片ID（10F112），等待--确认");
//                    p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                }
//                else
//                {
//                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("查询网络规模（10F9）异常"));
//                }
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }

//            case QueryBasicNetworkInfo_F0F100_2_Finish:
//            {
//                if(p_Frame3762Base->afn_ == char(0xf0)&&p_Frame3762Base->dt1_==0x08&&p_Frame3762Base->dt2_==0x0C&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
//                {
//                    p_timer->stop();
//                    shared_ptr<AfnF0F100> p_QueryBasicNetworkInfo_F0F100_Up=dynamic_pointer_cast<AfnF0F100>(p_Frame3762Base);

//                    if(p_QueryBasicNetworkInfo_F0F100_Up->network_basic_unit_.node_total_num_ == 1)
//                    {
//                        emScriptRunState=ScriptSuccess;
//                        resultFlag=true;
//                        p_AbstractScriptHost->updateProgress(ProcessState_Success, QString("查询网络基本信息测试成功！"));
//                    }
//                    else
//                    {
//                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("测试失败！"));
//                    }
//                }
//                else
//                {
//                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
//                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
//                }
//                break;
//            }
            case ScriptSuccess:
            {
                break;
            }
        }
    }
}

//QString Script_RouteRunningStatus_Gansu::GetBit( uchar x, int y)
//{
//    return  QString::number((x) >> (y-1)&1);
//}

void Script_RouteRunningStatus_Gansu::processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID)
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

void Script_RouteRunningStatus_Gansu::processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID)
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

void Script_RouteRunningStatus_Gansu::sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame)
{
    sendMsgOct.clear();
//    if(frame==p_QueryNetScale_10F9)
//    {
//        p_QueryNetScale_10F9->ctrl_field_.dir=kDirDown;
//        p_QueryNetScale_10F9->ctrl_field_.prm=kActive;
//        p_QueryNetScale_10F9->ctrl_field_.comn_type=kHplc;

//        p_QueryNetScale_10F9->info_field_.info_field_down.msg_seq=char(msgSeq++);
//        p_QueryNetScale_10F9->info_field_.info_field_down.comu_rate=0;
//        p_QueryNetScale_10F9->info_field_.info_field_down.comu_module_ident=0;

//        sendMsgOct=p_QueryNetScale_10F9->EncodeFrame();
//        sendMsgLog=QString("》》查询网络规模10F9：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
//    }
//    else
    if(frame==p_FileTransfer_15F1_Down)
    {
        if(emScriptRunState==Wait_FileTransfer_15F1_Down_Finish)
        {
            p_FileTransfer_15F1_Down->file_transfer_unit_.file_identify_=0x03;
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
            p_FileTransfer_15F1_Down->info_field_.info_field_down.msg_seq=char((msgSeq>=255)?0:++msgSeq);

         //   p_MsgBase_1376_2->ucAFN=0x15;
         //   sendMsgOct=p_MsgBase_1376_2->encode_3762_MsgDown(p_MsgBase_1376_2,p_FileTransfer_15F1_Down);

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
            p_FileTransfer_15F1_Down->info_field_.info_field_down.msg_seq=char((msgSeq>=255)?0:++msgSeq);

           // p_MsgBase_1376_2->ucAFN=0x15;
           // sendMsgOct=p_MsgBase_1376_2->encode_3762_MsgDown(p_MsgBase_1376_2,p_FileTransfer_15F1_Down);
            sendMsgOct=p_FileTransfer_15F1_Down->EncodeFrame();
            p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("》》清除下装文件15F1：%1\n").arg(QString(sendMsgOct.toHex())));
        }
    }
    else if(frame==p_QueryRouteRunStatus_10F4)
    {
        p_QueryRouteRunStatus_10F4->ctrl_field_.dir=kDirDown;
        p_QueryRouteRunStatus_10F4->ctrl_field_.prm=kActive;
        p_QueryRouteRunStatus_10F4->ctrl_field_.comn_type=kHplc;

        p_QueryRouteRunStatus_10F4->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_QueryRouteRunStatus_10F4->info_field_.info_field_down.comu_rate=0;
        p_QueryRouteRunStatus_10F4->info_field_.info_field_down.comu_module_ident=0;

        sendMsgOct=p_QueryRouteRunStatus_10F4->EncodeFrame();
        sendMsgLog=QString("》》查询路由运行状态10F4：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_StationIdentiSwitch_05F6)
    {
        p_StationIdentiSwitch_05F6->ctrl_field_.dir=kDirDown;
        p_StationIdentiSwitch_05F6->ctrl_field_.prm=kActive;
        p_StationIdentiSwitch_05F6->ctrl_field_.comn_type=kHplc;

        p_StationIdentiSwitch_05F6->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_StationIdentiSwitch_05F6->info_field_.info_field_down.comu_rate=0;
        p_StationIdentiSwitch_05F6->info_field_.info_field_down.comu_module_ident=0;

        if(emAreadifference_State==OpenIdenti)
            p_StationIdentiSwitch_05F6->area_identify_enable_flag_=0x01;
        else if(emAreadifference_State==CloseIdenti)
            p_StationIdentiSwitch_05F6->area_identify_enable_flag_=0x00;

        sendMsgOct=p_StationIdentiSwitch_05F6->EncodeFrame();
        sendMsgLog=QString("》》设置台区识别开关05F6：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_StartMeterSearch_11F5)
    {
        p_StartMeterSearch_11F5->ctrl_field_.dir=kDirDown;
        p_StartMeterSearch_11F5->ctrl_field_.prm=kActive;
        p_StartMeterSearch_11F5->ctrl_field_.comn_type=kHplc;

        p_StartMeterSearch_11F5->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_StartMeterSearch_11F5->info_field_.info_field_down.comu_rate=0;
        p_StartMeterSearch_11F5->info_field_.info_field_down.comu_module_ident=0;

        QByteArray startTime=QByteArray::fromHex(QDateTime::currentDateTime().toString("ssmmhhddMMyy").toLatin1());
        memcpy(&p_StartMeterSearch_11F5->start_time_,startTime,6);
        p_StartMeterSearch_11F5->last_time_=activeTime;
        p_StartMeterSearch_11F5->retransmit_times_=0;
        p_StartMeterSearch_11F5->wait_time_slice_num_=0;

        sendMsgOct=p_StartMeterSearch_11F5->EncodeFrame();
        sendMsgLog=QString("》》启动搜表11F5：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_StopMeterSearch_11F6)
    {
        p_StopMeterSearch_11F6->ctrl_field_.dir=kDirDown;
        p_StopMeterSearch_11F6->ctrl_field_.prm=kActive;
        p_StopMeterSearch_11F6->ctrl_field_.comn_type=kHplc;

        p_StopMeterSearch_11F6->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_StopMeterSearch_11F6->info_field_.info_field_down.comu_rate=0;
        p_StopMeterSearch_11F6->info_field_.info_field_down.comu_module_ident=0;

        sendMsgOct=p_StopMeterSearch_11F6->EncodeFrame();
        sendMsgLog=QString("》》停止搜表：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
//    else if(frame==p_HardReset_01F1)
//    {
//        p_HardReset_01F1->ctrl_field_.dir=kDirDown;
//        p_HardReset_01F1->ctrl_field_.prm=kActive;
//        p_HardReset_01F1->ctrl_field_.comn_type=kHplc;

//        p_HardReset_01F1->info_field_.info_field_down.msg_seq=char(msgSeq++);
//        p_HardReset_01F1->info_field_.info_field_down.comu_rate=0;
//        p_HardReset_01F1->info_field_.info_field_down.comu_module_ident=0;

//        sendMsgOct=p_HardReset_01F1->EncodeFrame();
//        sendMsgLog=QString("》》硬件初始化01F1：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
//    }

//    else if(frame==p_ParaInit_01F2)
//    {
//        p_ParaInit_01F2->ctrl_field_.dir=kDirDown;
//        p_ParaInit_01F2->ctrl_field_.prm=kActive;
//        p_ParaInit_01F2->ctrl_field_.comn_type=kHplc;

//        p_ParaInit_01F2->info_field_.info_field_down.msg_seq=char(msgSeq++);
//        p_ParaInit_01F2->info_field_.info_field_down.comu_rate=0;
//        p_ParaInit_01F2->info_field_.info_field_down.comu_module_ident=0;

//        sendMsgOct=p_ParaInit_01F2->EncodeFrame();
//        sendMsgLog=QString("》》参数初始化01F1：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
//    }
//    else if(frame==p_QueryNetTopoInfo_10F21)
//    {
//        p_QueryNetTopoInfo_10F21->ctrl_field_.dir=kDirDown;
//        p_QueryNetTopoInfo_10F21->ctrl_field_.prm=kActive;
//        p_QueryNetTopoInfo_10F21->ctrl_field_.comn_type=kHplc;

//        p_QueryNetTopoInfo_10F21->info_field_.info_field_down.msg_seq=char(msgSeq++);
//        p_QueryNetTopoInfo_10F21->info_field_.info_field_down.comu_rate=0;
//        p_QueryNetTopoInfo_10F21->info_field_.info_field_down.comu_module_ident=0;

//        p_QueryNetTopoInfo_10F21->node_start_no_ = uchar(0x01);
//        p_QueryNetTopoInfo_10F21->node_num_ = uchar(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().size());

//        sendMsgOct=p_QueryNetTopoInfo_10F21->EncodeFrame();
//        sendMsgLog=QString("》》查询网络拓扑信息（10F21）：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
//    }
//    else if(frame==p_QueryNodeNum_10F1)
//    {
//        p_QueryNodeNum_10F1->ctrl_field_.dir=kDirDown;
//        p_QueryNodeNum_10F1->ctrl_field_.prm=kActive;
//        p_QueryNodeNum_10F1->ctrl_field_.comn_type=kHplc;

//        p_QueryNodeNum_10F1->info_field_.info_field_down.msg_seq=char(msgSeq++);
//        p_QueryNodeNum_10F1->info_field_.info_field_down.comu_rate=0;
//        p_QueryNodeNum_10F1->info_field_.info_field_down.comu_module_ident=0;

//        sendMsgOct=p_QueryNodeNum_10F1->EncodeFrame();
//        sendMsgLog=QString("》》查询从节点数量10F1：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
//    }
//    else if(frame==p_AddSlaveNode_11F1)
//    {
//        p_AddSlaveNode_11F1->ctrl_field_.dir=kDirDown;
//        p_AddSlaveNode_11F1->ctrl_field_.prm=kActive;
//        p_AddSlaveNode_11F1->ctrl_field_.comn_type=kHplc;

//        p_AddSlaveNode_11F1->info_field_.info_field_down.msg_seq=char(msgSeq++);
//        p_AddSlaveNode_11F1->info_field_.info_field_down.comu_rate=0;
//        p_AddSlaveNode_11F1->info_field_.info_field_down.comu_module_ident=0;

//        p_AddSlaveNode_11F1->node_parameter_list_.clear();
//        if(index+num<p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().size())//每次添加表数num，
//        {
//            for(int i=0;i<num;i++)
//            {
//                NodeParameter nodePara;
//                memcpy(nodePara.node_address_.addr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(index+i)->mtrAddr,6);
//                nodePara.protocol_type_=char(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(index+i)->prtcl);
//                p_AddSlaveNode_11F1->node_parameter_list_.append(nodePara);
//            }
//        }
//        else
//        {
//            for(int i=0;i<p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().size()-index;i++)
//            {
//                NodeParameter nodePara;
//                memcpy(nodePara.node_address_.addr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(index+i)->mtrAddr,6);
//                nodePara.protocol_type_=char(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(index+i)->prtcl);
//                p_AddSlaveNode_11F1->node_parameter_list_.append(nodePara);
//            }
//        }
//        p_AddSlaveNode_11F1->node_num_=uchar(p_AddSlaveNode_11F1->node_parameter_list_.size());
//        sendMsgOct=p_AddSlaveNode_11F1->EncodeFrame();
//        sendMsgLog=QString("》》添加从节点11F1：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
//    }
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

    else
    {
        return;
    }


    p_AbstractScriptHost->updateProgress(ProcessState_Processing, sendMsgLog);

    uchar *sendMsg=new uchar[uint(sendMsgOct.size())];
    memcpy(sendMsg,reinterpret_cast<uchar*>(sendMsgOct.data()),uint(sendMsgOct.size()));
    p_AbstractScriptHost->sendMsg2Dvc(dvcType,dvcId,sendMsg,sendMsgOct.size());
}
void Script_RouteRunningStatus_Gansu::timer_timeoutProc()
{
    p_timer->stop();
    switch(emScriptRunState)
    {
        case Wait_BuildNetFinish_Whole:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_BuildNetFinish_Whole timeout!!!");
            break;
        }
//        case Wait_HardReset_Finish:
//        {
//            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_HardReset_Finish timeout!!!");
//            break;
//        }
//        case Wait_ParaInit_Finish:
//        {
//            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_ParaInit_Finish timeout!!!");
//            break;
//        }
//        case Wait_AddSlaveNode_11F1_Finish:
//        {
//            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_AddSlaveNode_11F1_Finish timeout!!!");
//            break;
//        }
//        case Wait_QueryNetTopoInfo_10F21_Finish:
//        {
//            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_QueryNetTopoInfo_10F21_Finish timeout!!!");
//            break;
//        }
        case Wait_QueryRouteRunStatus_10F4_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_QueryRouteRunStatus_10F4_Finish timeout!!!");
            break;
        }
        case Wait_StationIdentiSwitch_05F6_Open_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_StationIdentiSwitch_05F6_Open_Finish timeout!!!");
            break;
        }
        case Wait_QueryRouteRunStatus_10F4_2_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_QueryRouteRunStatus_10F4_2_Finish timeout!!!");
            break;
        }
        case Wait_StationIdentiSwitch_05F6_Close_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_StationIdentiSwitch_05F6_Close_Finish timeout!!!");
            break;
        }
        case Wait_QueryRouteRunStatus_10F4_3_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_QueryRouteRunStatus_10F4_3_Finish timeout!!!");
            break;
        }
        case Wait_StartMeterSearch_11F5_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_StartMeterSearch_11F5_Finish timeout!!!");
            break;
        }
        case Wait_QueryRouteRunStatus_10F4_4_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_QueryRouteRunStatus_10F4_4_Finish timeout!!!");
            break;
        }
        case Wait_StopMeterSearch_11F6_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_StopMeterSearch_11F6_Finish timeout!!!");
            break;
        }
        case Wait_QueryRouteRunStatus_10F4_5_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_QueryRouteRunStatus_10F4_5_Finish timeout!!!");
            break;
        }
        case Wait__15F1_BeforeUpgrdCco_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait__15F1_BeforeUpgrdCco_Finish timeout!!!");
            break;
        }
        case Wait_FileTransfer_15F1_Down_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_FileTransfer_15F1_Down_Finish timeout!!!");
            break;
        }
        case Wait_QueryRouteRunStatus_10F4_6_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_QueryRouteRunStatus_10F4_6_Finish timeout!!!");
            break;
        }
        case Wait_QueryRouteRunStatus_10F4_7_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_QueryRouteRunStatus_10F4_7_Finish timeout!!!");
            break;
        }
        default:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==p_timer");
            break;
        }
    }
}

void Script_RouteRunningStatus_Gansu::delayTimer_timeoutProc()
{
    p_delayTimer->stop();

    switch(emScriptRunState)
    {
        case Wait_QueryRouteRunStatus_10F4_7_Finish:
        {
//            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryNetScale_10F9);
//            emScriptRunState=Wait_QueryNetScale_2_10F9_Finish;
//            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询网络规模（10F9），等待--回复");
//            p_delayTimer->start(10*1000);

            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryRouteRunStatus_10F4);
            emScriptRunState=Wait_QueryRouteRunStatus_10F4_7_Finish;
            p_delayTimer->start(10*1000);
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--路由运行状态（10F4），等待--确认");
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

void Script_RouteRunningStatus_Gansu::maxAllowTimer_timeoutProc()
{
    p_maxAllowTimer->stop();
    switch(emScriptRunState)
    {
        case Wait_BuildNetFinish_Whole:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_BuildNetFinish_Whole maxAllow timeout!!!");
            break;
        }
//        case Wait_HardReset_Finish:
//        {
//            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_HardReset_Finish maxAllow timeout!!!");
//            break;
//        }
//        case Wait_ParaInit_Finish:
//        {
//            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_ParaInit_Finish maxAllow timeout!!!");
//            break;
//        }
//        case Wait_AddSlaveNode_11F1_Finish:
//        {
//            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_AddSlaveNode_11F1_Finish maxAllow timeout!!!");
//            break;
//        }
//        case Wait_QueryNetTopoInfo_10F21_Finish:
//        {
//            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_QueryNetTopoInfo_10F21_Finish maxAllow timeout!!!");
//            break;
//        }
        case Wait_QueryRouteRunStatus_10F4_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_QueryRouteRunStatus_10F4_Finish maxAllow timeout!!!");
            break;
        }
        case Wait_StationIdentiSwitch_05F6_Open_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_StationIdentiSwitch_05F6_Open_Finish maxAllow timeout!!!");
            break;
        }
        case Wait_QueryRouteRunStatus_10F4_2_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_QueryRouteRunStatus_10F4_2_Finish maxAllow timeout!!!");
            break;
        }
        case Wait_StationIdentiSwitch_05F6_Close_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_StationIdentiSwitch_05F6_Close_Finish maxAllow timeout!!!");
            break;
        }
        case Wait_QueryRouteRunStatus_10F4_3_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_QueryRouteRunStatus_10F4_3_Finish maxAllow timeout!!!");
            break;
        }
        case Wait_StartMeterSearch_11F5_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_StartMeterSearch_11F5_Finish maxAllow timeout!!!");
            break;
        }
        case Wait_QueryRouteRunStatus_10F4_4_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_QueryRouteRunStatus_10F4_4_Finish maxAllow timeout!!!");
            break;
        }
        case Wait_StopMeterSearch_11F6_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_StopMeterSearch_11F6_Finish maxAllow timeout!!!");
            break;
        }
        case Wait_QueryRouteRunStatus_10F4_5_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_QueryRouteRunStatus_10F4_5_Finish maxAllow timeout!!!");
            break;
        }
        case Wait__15F1_BeforeUpgrdCco_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait__15F1_BeforeUpgrdCco_Finish maxAllow timeout!!!");
            break;
        }
        case Wait_FileTransfer_15F1_Down_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_FileTransfer_15F1_Down_Finish maxAllow timeout!!!");
            break;
        }
        case Wait_QueryRouteRunStatus_10F4_6_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_QueryRouteRunStatus_10F4_6_Finish maxAllow timeout!!!");
            break;
        }
        case Wait_QueryRouteRunStatus_10F4_7_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_QueryRouteRunStatus_10F4_7_Finish maxAllow timeout!!!");
            break;
        }
        default:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==p_timer");
            break;
        }
    }
}
void Script_RouteRunningStatus_Gansu::LoadUpdateFile()
{
    QStringList fileNameFilters;
    QStringList fileList;
    QString path;


    if(1==dstUpgradeDvc)
    {
        path=tr("DataBase\\Upgrade\\路由程序(旧-新)");
    }
    else if(2==dstUpgradeDvc)
    {
        path=tr("DataBase\\Upgrade\\路由程序(新-新)");
    }
    else if(3==dstUpgradeDvc)
    {
        path=tr("DataBase\\Upgrade\\路由程序(文件类型错误)");
    }
    else if(4==dstUpgradeDvc)
    {
        path=tr("DataBase\\Upgrade\\路由程序(大于512K)");
    }
    else if(5==dstUpgradeDvc)
    {
        path=tr("DataBase\\Upgrade\\路由程序(重发100包和末包)");
    }
    else if(6==dstUpgradeDvc)
    {
        path=tr("DataBase\\Upgrade\\路由程序(100包长度256)");
    }
    else if(7==dstUpgradeDvc)
    {
        path=tr("DataBase\\Upgrade\\路由程序(先发101包后发100包)");
    }
    else
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Error, "dstUpgradeDvc配置有误");
        return;
    }

    QDir *updateFileDir=new QDir(path);


//    QDir updateFileDir(path);

//    fileNameFilters<<"*.bin";
//    updateFileDir.setNameFilters(fileNameFilters);
//    fileList=updateFileDir.entryList();


//    qDebug()<<"fileList: "<<fileList;

//    if(fileList.size()!=1)
//    {
//        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "没有STA升级文件或STA升级文件多于1个!!!");
//        return;
//    }
//    QString filePath=path+"\\"+fileList.at(0);

    fileNameFilters << "*" ;

    QList<QFileInfo> *fileInfo=new QList<QFileInfo>(updateFileDir->entryInfoList(fileNameFilters,QDir::Files,QDir::NoSort));

    qDebug()<<QString("获取到的文件数：%1").arg(fileInfo->count());

    QString str_file_name;
    for (int i=0;i<fileInfo->count();i++)
    {
        str_file_name += fileInfo->at(i).fileName() + "---";
    }
    qDebug()<<str_file_name;

    if(fileInfo->count() != 1)
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "没有CCO升级文件或CCO升级文件多于1个!!!");
        return;
    }

    QString filePath=path+"\\"+fileInfo->at(0).fileName();

    delete fileInfo;
    delete updateFileDir;


    char upgradeFileBuf[1024*1024];
    QFile file;
    file.setFileName(filePath);
    if(!file.open(QIODevice::ReadOnly))
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "打开CCO升级文件失败!!!");
        return;
    }


    QDataStream in(&file);
    dataLen=in.readRawData(upgradeFileBuf,1024*1024);
    file.close();

    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("升级包大小=(%1);").arg(dataLen));
    rawUpdateFile=QByteArray::fromRawData(upgradeFileBuf,dataLen);

    totalSegs = ushort((dataLen%SEG_LEN_STA==0)?dataLen/SEG_LEN_STA:(dataLen/SEG_LEN_STA+1));

    p_FileTransfer_15F1_Down->file_transfer_unit_.total_num_=totalSegs;
    for(int i=0; i<p_FileTransfer_15F1_Down->file_transfer_unit_.total_num_; i++)
    {
        transResList.append(false);
    }
}
void Script_RouteRunningStatus_Gansu::Refresh_TestResult_15F1(shared_ptr<Afn15F1> p_FileTransfer_15F1_Up)
{
  //  p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到确认：段标志%1，fileIndex%2").arg(p_FileTransfer_15F1_Up->current_identify_).arg(fileIndex));
    if(p_FileTransfer_15F1_Up->current_identify_==fileIndex)
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("第(%1)段传输完成;").arg(fileIndex));
        transResList[fileIndex]=true;
    }
}
ushort Script_RouteRunningStatus_Gansu::Refresh_SuccessCnt_15F1()
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
