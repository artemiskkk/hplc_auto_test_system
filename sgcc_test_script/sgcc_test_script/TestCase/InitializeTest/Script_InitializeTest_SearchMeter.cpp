#include "Script_InitializeTest_SearchMeter.h"

Script_InitializeTest_SearchMeter::Script_InitializeTest_SearchMeter(QObject *parent) : QObject(parent)
{
    emScriptRunState=TestInit;
    emQueryState=Wait_QueryMasterAddr_03F4;
    resultFlag=false;
    sendMsgOct.clear();

    p_CtrInfoList=make_shared<QList<shared_ptr<CtrInfo>>>();
    p_BuildNetwork_GW=new BuildNetwork_GW();

    p_Frame3762Helper=make_shared<Frame3762Helper>();
    p_HardReset_01F1=make_shared<Afn01F1>();
    p_ParameterInit_01F2=make_shared<Afn01F2>();
    p_QueryMasterAddr_03F4=make_shared<Afn03F4>();
    p_QueryNodeNum_10F1=make_shared<Afn10F1>();
    p_QueryNodeInfo_10F2=make_shared<Afn10F2>();
    p_QueryRouterID_10F40=make_shared<Afn10F40>();
    p_QueryRouterChipID_10F40=make_shared<Afn10F40>();
    p_QueryRouterSN_F0F41=make_shared<AfnF0F41>();
    p_QueryFreq_03F16=make_shared<Afn03F16>();
    p_QueryWorkSwitch_10F4=make_shared<Afn10F4>();
    p_Confirm_00F1=make_shared<Afn00F1>();
    p_ActiveRegister_11F5=make_shared<Afn11F5>();

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
Script_InitializeTest_SearchMeter::~Script_InitializeTest_SearchMeter()
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
void Script_InitializeTest_SearchMeter::execute()
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
        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryWorkSwitch_10F4);
        emScriptRunState=Wait_QueryDefaultWorkSwitch_Finish;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询默认工作开关，等待--回复");
        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
    }
    p_maxAllowTimer->start((timerForReachThresld+timerAfterReachThresld)*1000);

}
void Script_InitializeTest_SearchMeter::stop()
{
    if(p_AbstractScriptHost==nullptr)
        return;
    p_AbstractScriptHost->updateProgress(ProcessState_Error, "Test stop!");
    if(needPowerOff==true)
        powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost);
}
void Script_InitializeTest_SearchMeter::addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq)
{
    p_CtrInfoList->clear();
    comnAddAddrsInfo(p_CtrInfoList, p_ConcentratorInfoList, p_MeterInfoList, p_SchemeCfgInfoList);
    concentratorCnt=ushort(p_CtrInfoList->size());
    uchar dstFreq=freq&0x0f;

    uchar dstPrtcl=uchar(freq)>>4;
    if(dstPrtcl==0x03)
        setMeterAddrsPrtcl(p_CtrInfoList,dstPrtcl);

    p_BuildNetwork_GW->setCtrInfoListAndFreq(p_CtrInfoList, dstFreq);
    //改动
    runFreq=dstFreq;
}
void Script_InitializeTest_SearchMeter::setHost(AbstractScriptHost *host)
{
    p_AbstractScriptHost=host;
    p_BuildNetwork_GW->setHost(host);
}
bool Script_InitializeTest_SearchMeter::config(const QMap<QString,QString> *paraDic)
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
void Script_InitializeTest_SearchMeter::processMsg(DvcType dvcType, int id, uchar *data, int datalen)
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
                tryTimes=0;
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryWorkSwitch_10F4);
                emScriptRunState=Wait_QueryDefaultWorkSwitch_Finish;
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询默认工作开关，等待--回复");
                p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                p_maxAllowTimer->start(timerForReachThresld*1000);
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
void Script_InitializeTest_SearchMeter::processCtrlDvcRes(DvcType dvcType, QList<int> idList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params)
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
        case Wait_ParaInitTest_Finish:
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

void Script_InitializeTest_SearchMeter::processMsgFromCCO(DvcType dvcType, int dvcId)
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
        msgSeq=uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq);
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
            case Wait_QueryDefaultWorkSwitch_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x10&&p_Frame3762Base->dt1_==0x08&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    shared_ptr<Afn10F4> p_QueryWorkSwitch_10F4_Up=dynamic_pointer_cast<Afn10F4>(p_Frame3762Base);
                    current_state_ =  uchar(p_QueryWorkSwitch_10F4_Up->router_operate_state_unit_.work_switch_.current_state_);
                    event_report_flag_10F4_ = uchar(p_QueryWorkSwitch_10F4_Up->router_operate_state_unit_.work_switch_.event_report_flag_);
                    area_difference_flag_10F4_ = uchar(p_QueryWorkSwitch_10F4_Up->router_operate_state_unit_.work_switch_.area_difference_flag_);

                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_ActiveRegister_11F5);
                    emScriptRunState=Wait_ActiveRegister_Finish;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--激活主动注册（11F5），等待--确认");
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_ActiveRegister_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "激活主动注册回复确认");
                }
                else if(p_Frame3762Base->afn_ == 0x06&&p_Frame3762Base->dt1_==0x08&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    shared_ptr<Afn06F4> p_ReportRegisterNode_Up=dynamic_pointer_cast<Afn06F4>(p_Frame3762Base);
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("上报注册从节点，地址为%1；").arg(QString(QByteArray(p_ReportRegisterNode_Up->report_node_info_unit_.report_node_address_.addr,6).toHex())));

                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送-回复确认");
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_Confirm_00F1);

                    if(hardResetFlag==true)
                    {
                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_HardReset_01F1);
                        emScriptRunState=Wait_HardResetTest_Finish;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--路由硬件复位（01F1），等待--确认");
                        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                    {
                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_ParameterInit_01F2);
                        emScriptRunState=Wait_ParaInitTest_Finish;
                        emQueryState=Wait_QueryMasterAddr_03F4;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--路由参数初始化（01F2），等待--确认");
                        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                    }

                }
                break;
            }
            case Wait_HardResetTest_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到确认");
                }
                else if(p_Frame3762Base->afn_ == 0x03&&p_Frame3762Base->dt1_==0x02&&p_Frame3762Base->dt2_==0x01&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到03F10上报，发送--查询主节点地址（03F4），等待--回复");
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryMasterAddr_03F4);
                    emQueryState=Wait_QueryMasterAddr_03F4;
                    p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                }
                else if(p_Frame3762Base->afn_ == 0x03&&p_Frame3762Base->dt1_==0x08&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    shared_ptr<Afn03F4> p_QueryMasterAddr_03F4_Up=dynamic_pointer_cast<Afn03F4>(p_Frame3762Base);
                    //判断是否正确
                    if(memcmp(p_QueryMasterAddr_03F4_Up->master_node_address_.addr,p_CtrInfoList->at(0)->ccoAddr,6)==0)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,  QString("查询主节点地址为%1，符合要求").arg(QString(QByteArray(p_QueryMasterAddr_03F4_Up->master_node_address_.addr,6).toHex())));
                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryNodeNum_10F1);
                        emQueryState=Wait_QueryNodeNum_10F1;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询从节点数量（10F1），等待--回复");
                        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("查询主节点地址为%1，不符合要求").arg(QString(QByteArray(p_QueryMasterAddr_03F4_Up->master_node_address_.addr,6).toHex())));
                    }
                }
                else if(p_Frame3762Base->afn_ == 0x10&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    shared_ptr<Afn10F1> p_QueryNodeNum_10F1_Up=dynamic_pointer_cast<Afn10F1>(p_Frame3762Base);
                    if(p_QueryNodeNum_10F1_Up->node_total_num_==p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size())
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("查询从节点数量为%1，符合要求").arg(QString::number(p_QueryNodeNum_10F1_Up->node_total_num_)));
                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryNodeInfo_10F2);
                        emQueryState=Wait_QueryNodeInfo_10F2;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询从节点信息（10F2），等待--回复");
                        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("查询从节点数量为%1，不符合要求").arg(QString::number(p_QueryNodeNum_10F1_Up->node_total_num_)));
                    }
                }
                else if(p_Frame3762Base->afn_ == 0x10&&p_Frame3762Base->dt1_==0x02&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    shared_ptr<Afn10F2> p_QueryNodeInfo_10F2_Up=dynamic_pointer_cast<Afn10F2>(p_Frame3762Base);
                    //可以考虑多加判断，只查一次
                    if(meterIsExist(p_QueryNodeInfo_10F2_Up->node_info_data_unit_.node_info_group_list_.at(0).node_address_)==true)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("查询从节点信息，符合要求"));

                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryFreq_03F16);
                        emQueryState=Wait_QueryFreq_03F16;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询运行频段（03F16），等待--回复");
                        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("查询从节点信息，不符合要求"));
                    }
                }
                else if(p_Frame3762Base->afn_ == 0x03&&p_Frame3762Base->dt1_==char(0x80)&&p_Frame3762Base->dt2_==0x01&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    shared_ptr<Afn03F16> p_QueryFreq_03F16_Up=dynamic_pointer_cast<Afn03F16>(p_Frame3762Base);
                    if(uchar(p_QueryFreq_03F16_Up->carrier_frequence_range_)==runFreq)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("运行频段为%1，符合要求").arg(runFreq));

                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryRouterID_10F40);
                        emQueryState=Wait_QueryRouterID_10F40;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询路由ID和芯片ID（10F40），等待--回复");
                        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("运行频段为%1，不符合要求").arg(p_QueryFreq_03F16_Up->carrier_frequence_range_));
                    }
                }
                else if(p_Frame3762Base->afn_ == char(0x10)&&p_Frame3762Base->dt1_==char(0x80)&&p_Frame3762Base->dt2_==0x04&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    shared_ptr<Afn10F40> p_QueryRouterID_10F40_Up=dynamic_pointer_cast<Afn10F40>(p_Frame3762Base);
                    if(emQueryState==Wait_QueryRouterID_10F40)
                    {
                        if(memcmp(p_QueryRouterID_10F40_Up->read_chip_id_unit_up_.mac_address_.addr,p_CtrInfoList->at(0)->ccoAddr,6)==0
                                &&p_QueryRouterID_10F40_Up->read_chip_id_unit_up_.id_type_==0x02
                                &&p_QueryRouterID_10F40_Up->read_chip_id_unit_up_.device_type_==0x02)
                        {
                            routerID_10F40_ = QString(p_QueryRouterID_10F40_Up->read_chip_id_unit_up_.id_content_.toHex());
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString(">>>收到路由模块ID为：%1").arg(routerID_10F40_));
//                            p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("路由ID为%1，符合要求").arg(QString(p_QueryRouterID_10F40_Up->read_chip_id_unit_up_.id_content_.toHex())));

                            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryRouterChipID_10F40);
                            emQueryState=Wait_QueryChipID_10F40;
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询芯片ID（10F40），等待--回复");
                            p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                        }
                        else
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("路由ID为%1，不符合要求").arg(QString(p_QueryRouterID_10F40_Up->read_chip_id_unit_up_.id_content_.toHex())));
                        }
                    }
                    else if(emQueryState==Wait_QueryChipID_10F40)
                    {
                        if(memcmp(p_QueryRouterID_10F40_Up->read_chip_id_unit_up_.mac_address_.addr,p_CtrInfoList->at(0)->ccoAddr,6)==0
                                &&p_QueryRouterID_10F40_Up->read_chip_id_unit_up_.id_type_==0x01
                                &&p_QueryRouterID_10F40_Up->read_chip_id_unit_up_.device_type_==0x02)
                        {
                            chipID_10F40_ = QString(p_QueryRouterID_10F40_Up->read_chip_id_unit_up_.id_content_.toHex());
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing,  QString(">>>收到路由芯片ID为：%1").arg(chipID_10F40_));
                            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryWorkSwitch_10F4);
                            emQueryState=Wait_QueryWorkSwitch_10F4;
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询路由工作开关（10F4），等待--回复");
                            p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                        }
                        else
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("芯片ID为%1，不符合要求").arg(QString(p_QueryRouterID_10F40_Up->read_chip_id_unit_up_.id_content_.toHex())));
                        }
                    }
                    else
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed,  QString(">>>Wait_QueryInitialState_Finish，查询模块ID/芯片ID时收到非期望报文"));
                }
                else if(p_Frame3762Base->afn_ == char(0xF0)&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x05&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    shared_ptr<AfnF0F41> p_QueryRouterSN_F0F41_Up=dynamic_pointer_cast<AfnF0F41>(p_Frame3762Base);
                    if(memcmp(p_QueryRouterSN_F0F41_Up->mac_address_.addr,p_CtrInfoList->at(0)->ccoAddr,6)==0
                            &&p_QueryRouterSN_F0F41_Up->device_type_==0x02)
                    {
                        //待比对
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("路由SN为%1，符合要求").arg(QString(p_QueryRouterSN_F0F41_Up->sn_content_.toHex())));

                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryWorkSwitch_10F4);
                        emQueryState=Wait_QueryWorkSwitch_10F4;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询路由工作开关（10F4），等待--回复");
                        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("路由SN为%1，不符合要求").arg(QString(p_QueryRouterSN_F0F41_Up->sn_content_.toHex())));
                    }
                }
                else if(p_Frame3762Base->afn_ == 0x10&&p_Frame3762Base->dt1_==0x08&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    shared_ptr<Afn10F4> p_QueryWorkSwitch_10F4_Up=dynamic_pointer_cast<Afn10F4>(p_Frame3762Base);
                    uchar event_report_flag_10F4 = p_QueryWorkSwitch_10F4_Up->router_operate_state_unit_.work_switch_.event_report_flag_;
                    uchar area_difference_flag_10F4 = p_QueryWorkSwitch_10F4_Up->router_operate_state_unit_.work_switch_.area_difference_flag_;
                    uchar current_state = p_QueryWorkSwitch_10F4_Up->router_operate_state_unit_.work_switch_.current_state_;

                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString(">>>>收到10F40查询路由工作开关，事件上报开关为：%1，台区区分开关为：%2，当前状态为：%3").arg(event_report_flag_10F4).arg(area_difference_flag_10F4).arg(current_state));
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString(">>>>期望10F40查询路由工作开关，事件上报开关为：%1，台区区分开关为：%2，当前状态为：3(路由当前状态为：11-其他)").arg(event_report_flag_10F4_).arg(area_difference_flag_10F4_));

                    if (event_report_flag_10F4 ==event_report_flag_10F4_ && area_difference_flag_10F4==area_difference_flag_10F4_ && current_state == current_state_)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("路由工作开关，符合要求"));
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("路由硬件复位测试成功;\n\n\n开始测试路由参数初始化"));

                        hardResetFlag=false;
                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_ActiveRegister_11F5);
                        emScriptRunState=Wait_ActiveRegister_Finish;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--激活主动注册（11F5），等待--确认");
                        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("路由工作开关，不符合要求"));
                    }
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_ParaInitTest_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到确认，发送--查询主节点地址（03F4），等待--回复");
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryMasterAddr_03F4);
                    emQueryState=Wait_QueryMasterAddr_03F4;
                    p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                }
                else if(p_Frame3762Base->afn_ == 0x03&&p_Frame3762Base->dt1_==0x02&&p_Frame3762Base->dt2_==0x01&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, "参数初始化后收到03F10上报");
                }
                else if(p_Frame3762Base->afn_ == 0x03&&p_Frame3762Base->dt1_==0x08&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    shared_ptr<Afn03F4> p_QueryMasterAddr_03F4_Up=dynamic_pointer_cast<Afn03F4>(p_Frame3762Base);
                    //判断是否正确
                    if(memcmp(p_QueryMasterAddr_03F4_Up->master_node_address_.addr,p_CtrInfoList->at(0)->ccoAddr,6)==0)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,  QString("查询主节点地址为%1，符合要求").arg(QString(QByteArray(p_QueryMasterAddr_03F4_Up->master_node_address_.addr,6).toHex())));
                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryNodeNum_10F1);
                        emQueryState=Wait_QueryNodeNum_10F1;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询从节点数量（10F1），等待--回复");
                        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("查询主节点地址为%1，不符合要求").arg(QString(QByteArray(p_QueryMasterAddr_03F4_Up->master_node_address_.addr,6).toHex())));
                    }
                }
                else if(p_Frame3762Base->afn_ == 0x10&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    shared_ptr<Afn10F1> p_QueryNodeNum_10F1_Up=dynamic_pointer_cast<Afn10F1>(p_Frame3762Base);
                    if(p_QueryNodeNum_10F1_Up->node_total_num_==0)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("查询从节点数量为%1，符合要求").arg(QString::number(p_QueryNodeNum_10F1_Up->node_total_num_)));
                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryNodeInfo_10F2);
                        emQueryState=Wait_QueryNodeInfo_10F2;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询从节点信息（10F2），等待--回复");
                        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("查询从节点数量为%1，不符合要求").arg(QString::number(p_QueryNodeNum_10F1_Up->node_total_num_)));
                    }
                }
                else if(p_Frame3762Base->afn_ == 0x10&&p_Frame3762Base->dt1_==0x02&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    shared_ptr<Afn10F2> p_QueryNodeInfo_10F2_Up=dynamic_pointer_cast<Afn10F2>(p_Frame3762Base);
                    if(p_QueryNodeInfo_10F2_Up->node_info_data_unit_.node_total_num_==0 )
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("查询从节点信息，符合要求"));

                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryFreq_03F16);
                        emQueryState=Wait_QueryFreq_03F16;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询运行频段（03F16），等待--回复");
                        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("查询从节点信息，不符合要求"));
                    }
                }
                else if(p_Frame3762Base->afn_ == 0x03&&p_Frame3762Base->dt1_==char(0x80)&&p_Frame3762Base->dt2_==0x01&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    shared_ptr<Afn03F16> p_QueryFreq_03F16_Up=dynamic_pointer_cast<Afn03F16>(p_Frame3762Base);
                    if(uchar(p_QueryFreq_03F16_Up->carrier_frequence_range_)==runFreq)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("运行频段为%1，符合要求").arg(runFreq));

                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryRouterID_10F40);
                        emQueryState=Wait_QueryRouterID_10F40;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询路由ID和芯片ID（10F40），等待--回复");
                        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("运行频段为%1，不符合要求").arg(runFreq));
                    }
                }
                else if(p_Frame3762Base->afn_ == char(0x10)&&p_Frame3762Base->dt1_==char(0x80)&&p_Frame3762Base->dt2_==0x04&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    shared_ptr<Afn10F40> p_QueryRouterID_10F40_Up=dynamic_pointer_cast<Afn10F40>(p_Frame3762Base);
                    if(emQueryState==Wait_QueryRouterID_10F40)
                    {
                        if(memcmp(p_QueryRouterID_10F40_Up->read_chip_id_unit_up_.mac_address_.addr,p_CtrInfoList->at(0)->ccoAddr,6)==0
                                &&p_QueryRouterID_10F40_Up->read_chip_id_unit_up_.id_type_==0x02
                                &&p_QueryRouterID_10F40_Up->read_chip_id_unit_up_.device_type_==0x02)
                        {
                            QString target_data  = QString(p_QueryRouterID_10F40_Up->read_chip_id_unit_up_.id_content_.toHex());
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing,  QString(">>>收到路由模块ID为：%1").arg(target_data));
                            if (routerID_10F40_ == target_data)
                            {
                                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryRouterChipID_10F40);
                                emQueryState=Wait_QueryChipID_10F40;
                                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询芯片ID（10F40），等待--回复");
                                p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                            }
                            else
                            {
                                p_AbstractScriptHost->updateProgress(ProcessState_Failed,  QString(">>>Wait_QueryCheckState_Finish，Wait_QueryRouterID_10F40，期望路由模块ID和接收路由模块ID不一致测试失败，期望路由模块ID：%1，接收路由模块ID：%2").arg(routerID_10F40_).arg(target_data));
                            }
                        }
                        else
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("路由ID为%1，不符合要求").arg(QString(p_QueryRouterID_10F40_Up->read_chip_id_unit_up_.id_content_.toHex())));
                        }
                    }
                    else if(emQueryState==Wait_QueryChipID_10F40)
                    {
                        if(memcmp(p_QueryRouterID_10F40_Up->read_chip_id_unit_up_.mac_address_.addr,p_CtrInfoList->at(0)->ccoAddr,6)==0
                                &&p_QueryRouterID_10F40_Up->read_chip_id_unit_up_.id_type_==0x01
                                &&p_QueryRouterID_10F40_Up->read_chip_id_unit_up_.device_type_==0x02)
                        {
                            QString target_data = QString(p_QueryRouterID_10F40_Up->read_chip_id_unit_up_.id_content_.toHex());
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing,  QString(">>>收到路由芯片ID为：%1").arg(target_data));
                            if (chipID_10F40_==target_data)
                            {
                                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryWorkSwitch_10F4);
                                emQueryState=Wait_QueryWorkSwitch_10F4;
                                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询路由工作开关（10F4），等待--回复");
                                p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                            }
                            else
                            {
                                p_AbstractScriptHost->updateProgress(ProcessState_Failed,  QString(">>>Wait_QueryCheckState_Finish，Wait_QueryChipID_10F40，期望路由芯片ID和接收路由芯片ID不一致测试失败，期望路由芯片ID：%1，接收路由芯片ID：%2,").arg(chipID_10F40_).arg(target_data));
                            }
                        }
                        else
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("芯片ID为%1，不符合要求").arg(QString(p_QueryRouterID_10F40_Up->read_chip_id_unit_up_.id_content_.toHex())));
                        }
                    }
                    else
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed,  QString(">>>Wait_QueryInitialState_Finish，查询模块ID/芯片ID时收到非期望报文"));
                }
                else if(p_Frame3762Base->afn_ == char(0xF0)&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x05&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    shared_ptr<AfnF0F41> p_QueryRouterSN_F0F41_Up=dynamic_pointer_cast<AfnF0F41>(p_Frame3762Base);
                    if(memcmp(p_QueryRouterSN_F0F41_Up->mac_address_.addr,p_CtrInfoList->at(0)->ccoAddr,6)==0
                            &&p_QueryRouterSN_F0F41_Up->device_type_==0x02)
                    {
                        //待比对
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("路由SN为%1，符合要求").arg(QString(p_QueryRouterSN_F0F41_Up->sn_content_.toHex())));

                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryWorkSwitch_10F4);
                        emQueryState=Wait_QueryWorkSwitch_10F4;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询路由工作开关（10F4），等待--回复");
                        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("路由SN为%1，不符合要求").arg(QString(p_QueryRouterSN_F0F41_Up->sn_content_.toHex())));
                    }
                }
                else if(p_Frame3762Base->afn_ == 0x10&&p_Frame3762Base->dt1_==0x08&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    shared_ptr<Afn10F4> p_QueryWorkSwitch_10F4_Up=dynamic_pointer_cast<Afn10F4>(p_Frame3762Base);

                    uchar event_report_flag_10F4 = p_QueryWorkSwitch_10F4_Up->router_operate_state_unit_.work_switch_.event_report_flag_;
                    uchar area_difference_flag_10F4 = p_QueryWorkSwitch_10F4_Up->router_operate_state_unit_.work_switch_.area_difference_flag_;
                    uchar current_state = p_QueryWorkSwitch_10F4_Up->router_operate_state_unit_.work_switch_.current_state_;

                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString(">>>>收到10F40查询路由工作开关，事件上报开关为：%1，台区区分开关为：%2，当前状态为：%3").arg(event_report_flag_10F4).arg(area_difference_flag_10F4).arg(current_state));
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString(">>>>期望10F40查询路由工作开关，事件上报开关为：%1，台区区分开关为：%2").arg(event_report_flag_10F4_).arg(area_difference_flag_10F4_));

                    if (event_report_flag_10F4 ==event_report_flag_10F4_ && area_difference_flag_10F4==area_difference_flag_10F4_) // 需要再次讨论，路由参数初始化后，需不需要置路由当前状态   current_state == current_state_
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("路由工作开关，符合要求"));

                        if(p_maxAllowTimer != nullptr) p_maxAllowTimer->stop();
                        emScriptRunState=ScriptSuccess;
                        resultFlag=true;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("路由参数初始化测试成功"));
                        p_AbstractScriptHost->updateProgress(ProcessState_Success, QString("路由初始化测试成功;"));
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("路由工作开关，不符合要求"));
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
void Script_InitializeTest_SearchMeter::processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID)
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
void Script_InitializeTest_SearchMeter::processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID)
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
void Script_InitializeTest_SearchMeter::sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame)
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
    else if(frame==p_HardReset_01F1)
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
    else if(frame==p_ParameterInit_01F2)
    {
        p_ParameterInit_01F2->ctrl_field_.dir=kDirDown;
        p_ParameterInit_01F2->ctrl_field_.prm=kActive;
        p_ParameterInit_01F2->ctrl_field_.comn_type=kHplc;

        p_ParameterInit_01F2->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_ParameterInit_01F2->info_field_.info_field_down.comu_rate=0;
        p_ParameterInit_01F2->info_field_.info_field_down.comu_module_ident=0;

        sendMsgOct=p_ParameterInit_01F2->EncodeFrame();
        sendMsgLog=QString("》》路由参数初始化01F2：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
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
        p_ActiveRegister_11F5->last_time_=2;
        p_ActiveRegister_11F5->retransmit_times_=0;
        p_ActiveRegister_11F5->wait_time_slice_num_=0;

        sendMsgOct=p_ActiveRegister_11F5->EncodeFrame();
        sendMsgLog=QString("》》激活主动注册11F5：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_QueryMasterAddr_03F4)
    {
        p_QueryMasterAddr_03F4->ctrl_field_.dir=kDirDown;
        p_QueryMasterAddr_03F4->ctrl_field_.prm=kActive;
        p_QueryMasterAddr_03F4->ctrl_field_.comn_type=kHplc;

        p_QueryMasterAddr_03F4->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_QueryMasterAddr_03F4->info_field_.info_field_down.comu_rate=0;
        p_QueryMasterAddr_03F4->info_field_.info_field_down.comu_module_ident=0;

        sendMsgOct=p_QueryMasterAddr_03F4->EncodeFrame();
        sendMsgLog=QString("》》查询主节点地址03F4：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_QueryNodeNum_10F1)
    {
        p_QueryNodeNum_10F1->ctrl_field_.dir=kDirDown;
        p_QueryNodeNum_10F1->ctrl_field_.prm=kActive;
        p_QueryNodeNum_10F1->ctrl_field_.comn_type=kHplc;

        p_QueryNodeNum_10F1->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_QueryNodeNum_10F1->info_field_.info_field_down.comu_rate=0;
        p_QueryNodeNum_10F1->info_field_.info_field_down.comu_module_ident=0;

        sendMsgOct=p_QueryNodeNum_10F1->EncodeFrame();
        sendMsgLog=QString("》》查询从节点数量10F1：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_QueryNodeInfo_10F2)
    {
        p_QueryNodeInfo_10F2->ctrl_field_.dir=kDirDown;
        p_QueryNodeInfo_10F2->ctrl_field_.prm=kActive;
        p_QueryNodeInfo_10F2->ctrl_field_.comn_type=kHplc;

        p_QueryNodeInfo_10F2->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_QueryNodeInfo_10F2->info_field_.info_field_down.comu_rate=0;
        p_QueryNodeInfo_10F2->info_field_.info_field_down.comu_module_ident=0;

        p_QueryNodeInfo_10F2->node_start_no_=0x00;
        p_QueryNodeInfo_10F2->node_num_=0x05;

        sendMsgOct=p_QueryNodeInfo_10F2->EncodeFrame();
        sendMsgLog=QString("》》查询从节点信息10F2：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
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
    else if(frame==p_QueryRouterID_10F40)
        {
            p_QueryRouterID_10F40->ctrl_field_.dir=kDirDown;
            p_QueryRouterID_10F40->ctrl_field_.prm=kActive;
            p_QueryRouterID_10F40->ctrl_field_.comn_type=kHplc;

            p_QueryRouterID_10F40->info_field_.info_field_down.msg_seq=char(msgSeq++);
            p_QueryRouterID_10F40->info_field_.info_field_down.comu_rate=0;
            p_QueryRouterID_10F40->info_field_.info_field_down.comu_module_ident=0;

            p_QueryRouterID_10F40->read_chip_id_unit_down_.device_type_=0x02;
            memcpy(p_QueryRouterID_10F40->read_chip_id_unit_down_.mac_address_.addr,p_CtrInfoList->at(0)->ccoAddr,6);
            p_QueryRouterID_10F40->read_chip_id_unit_down_.id_type_=0x02;

            sendMsgOct=p_QueryRouterID_10F40->EncodeFrame();
            sendMsgLog=QString("》》查询CCO模块ID 10F40：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
        }
        else if(frame==p_QueryRouterChipID_10F40)
        {
            p_QueryRouterChipID_10F40->ctrl_field_.dir=kDirDown;
            p_QueryRouterChipID_10F40->ctrl_field_.prm=kActive;
            p_QueryRouterChipID_10F40->ctrl_field_.comn_type=kHplc;

            p_QueryRouterChipID_10F40->info_field_.info_field_down.msg_seq=char(msgSeq++);
            p_QueryRouterChipID_10F40->info_field_.info_field_down.comu_rate=0;
            p_QueryRouterChipID_10F40->info_field_.info_field_down.comu_module_ident=0;

            p_QueryRouterChipID_10F40->read_chip_id_unit_down_.device_type_=0x02;
            memcpy(p_QueryRouterChipID_10F40->read_chip_id_unit_down_.mac_address_.addr,p_CtrInfoList->at(0)->ccoAddr,6);
            p_QueryRouterChipID_10F40->read_chip_id_unit_down_.id_type_=0x01;

            sendMsgOct=p_QueryRouterChipID_10F40->EncodeFrame();
            sendMsgLog=QString("》》查询CCO芯片ID 10F40：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
        }
    else if(frame==p_QueryRouterSN_F0F41)
    {
        p_QueryRouterSN_F0F41->ctrl_field_.dir=kDirDown;
        p_QueryRouterSN_F0F41->ctrl_field_.prm=kActive;
        p_QueryRouterSN_F0F41->ctrl_field_.comn_type=kHplc;

        p_QueryRouterSN_F0F41->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_QueryRouterSN_F0F41->info_field_.info_field_down.comu_rate=0;
        p_QueryRouterSN_F0F41->info_field_.info_field_down.comu_module_ident=0;

        memcpy(p_QueryRouterSN_F0F41->mac_address_.addr,p_CtrInfoList->at(0)->ccoAddr,6);
        p_QueryRouterSN_F0F41->device_type_=0x02;

        sendMsgOct=p_QueryRouterSN_F0F41->EncodeFrame();
        sendMsgLog=QString("》》查询路由SN F0F41：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_QueryWorkSwitch_10F4)
    {
        p_QueryWorkSwitch_10F4->ctrl_field_.dir=kDirDown;
        p_QueryWorkSwitch_10F4->ctrl_field_.prm=kActive;
        p_QueryWorkSwitch_10F4->ctrl_field_.comn_type=kHplc;

        p_QueryWorkSwitch_10F4->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_QueryWorkSwitch_10F4->info_field_.info_field_down.comu_rate=0;
        p_QueryWorkSwitch_10F4->info_field_.info_field_down.comu_module_ident=0;

        sendMsgOct=p_QueryWorkSwitch_10F4->EncodeFrame();
        sendMsgLog=QString("》》查询路由工作开关10F4：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
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
void Script_InitializeTest_SearchMeter::sendSrcMsg(DvcType dvcType, int dvcId, QByteArray msg)
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

void Script_InitializeTest_SearchMeter::timer_timeoutProc()
{
    switch(emScriptRunState)
    {
        case Wait_BuildNetFinish_Whole:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_BuildNetFinish_Whole timeout!!!");
            break;
        }
        case Wait_QueryDefaultWorkSwitch_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_QueryDefaultWorkSwitch_Finish timeout!!!");
            break;
        }
        case Wait_HardResetTest_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_HardResetTest_Finish timeout!!!");
            break;
        }
        case Wait_ParaInitTest_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_ParaInitTest_Finish timeout!!!");
            break;
        }
        default:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==p_timer");
            break;
        }
    }
}
void Script_InitializeTest_SearchMeter::maxAllowTimer_timeoutProc()
{
    switch(emScriptRunState)
    {
        case Wait_HardResetTest_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_HardResetTest_Finish timeout!!!");
            break;
        }
        case Wait_ParaInitTest_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_ParaInitTest_Finish timeout!!!");
            break;
        }
        default:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==p_maxAllowTimer");
            break;
        }
    }
}
void Script_InitializeTest_SearchMeter::delayTimer_timeoutProc()
{
    p_delayTimer->stop();
    index=0;
    p_maxAllowTimer->stop();
}

bool Script_InitializeTest_SearchMeter::meterIsExist(Address meterAddr)
{
    for(int i=0;i<p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().size();i++)
    {
        if(memcmp(meterAddr.addr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(i)->mtrAddr,6)==0)
        {
            return true;
        }
    }
    return false;
}
