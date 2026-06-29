#include "Script_FlashTest_V3.h"

Script_FlashTest_V3::Script_FlashTest_V3(QObject *parent) : QObject(parent)
{
    emScriptRunState=TestInit;
    emQueryState=Wait_QueryMasterAddr_03F4;
    resultFlag=false;
    sendMsgOct.clear();

    p_CtrInfoList=make_shared<QList<shared_ptr<CtrInfo>>>();
    p_BuildNetwork_GW=new BuildNetwork_GW();

    p_Frame3762Helper=make_shared<Frame3762Helper>();
    p_EraseFlash_F0F12=make_shared<AfnF0F12>();
    p_QueryMasterAddr_03F4=make_shared<Afn03F4>();
    p_QueryNodeNum_10F1=make_shared<Afn10F1>();
    p_QueryNodeInfo_10F2=make_shared<Afn10F2>();
    p_QueryRouterID_F0F40=make_shared<AfnF0F40>();
    p_QueryChipID_F0F40=make_shared<AfnF0F40>();
    p_QueryRouterSN_F0F41=make_shared<AfnF0F41>();
    p_QueryFreq_03F16=make_shared<Afn03F16>();
    p_QueryWorkSwitch_10F4=make_shared<Afn10F4>();

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
Script_FlashTest_V3::~Script_FlashTest_V3()
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
void Script_FlashTest_V3::execute()
{
    p_AbstractScriptHost->updateProgress(ProcessState_Start, "Test start!");

    emScriptRunState=TestInit;
    resultFlag=false;
    addrList.clear();
    memcpy(ccoAddr,p_CtrInfoList->at(0)->ccoAddr,6);
    if(needBuildNet==true)
    {
        p_BuildNetwork_GW->execute();
        emScriptRunState=Wait_BuildNetFinish_Whole;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "等待全网组网完成");
        p_timer->start((timerForReachThresld+timerAfterReachThresld)*1000);
    }
    else
    {
        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryFreq_03F16);
        emScriptRunState=Wait_QueryInitFreq_Finish;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询初始运行频段（03F16），等待--回复");
        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
    }
    p_maxAllowTimer->start((timerForReachThresld+timerAfterReachThresld)*1000);
}
void Script_FlashTest_V3::stop()
{
    if(p_AbstractScriptHost==nullptr)
        return;
    p_AbstractScriptHost->updateProgress(ProcessState_Error, "Test stop!");
    if(needPowerOff==true)
        powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost);
}
void Script_FlashTest_V3::addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq)
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
  //  runFreq=dstFreq;
}
void Script_FlashTest_V3::setHost(AbstractScriptHost *host)
{
    p_AbstractScriptHost=host;
    p_BuildNetwork_GW->setHost(host);
}
bool Script_FlashTest_V3::config(const QMap<QString,QString> *paraDic)
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
void Script_FlashTest_V3::processMsg(DvcType dvcType, int id, uchar *data, int datalen)
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
            tryTimes=0;
            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryFreq_03F16);
            emScriptRunState=Wait_QueryInitFreq_Finish;
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询初始运行频段（03F16），等待--回复");
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
void Script_FlashTest_V3::processCtrlDvcRes(DvcType dvcType, QList<int> idList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params)
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

void Script_FlashTest_V3::processMsgFromCCO(DvcType dvcType, int dvcId)
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
        case Wait_QueryInitFreq_Finish:
        {
            if(p_Frame3762Base->afn_ == 0x03&&p_Frame3762Base->dt1_==char(0x80)&&p_Frame3762Base->dt2_==0x01&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                shared_ptr<Afn03F16> p_QueryFreq_03F16_Up=dynamic_pointer_cast<Afn03F16>(p_Frame3762Base);
                initFreq=uchar(p_QueryFreq_03F16_Up->carrier_frequence_range_);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("初始运行频段为%1").arg(initFreq));

                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_EraseFlash_F0F12);
                emScriptRunState=Wait_EraseFlash_Finish;
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--路由擦除Flash，等待--确认");
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
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
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到确认");
                break;
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
                if(memcmp(p_QueryMasterAddr_03F4_Up->master_node_address_.addr,ccoAddr,6)==0)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,  QString("主节点地址不变，符合要求"));
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryNodeNum_10F1);
                    emQueryState=Wait_QueryNodeNum_10F1;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询从节点数量（10F1），等待--回复");
                    p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("主节点地址变为%1，不符合要求").arg(QString(QByteArray(p_QueryMasterAddr_03F4_Up->master_node_address_.addr,6).toHex())));
                }
            }
            else if(p_Frame3762Base->afn_ == 0x10&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                shared_ptr<Afn10F1> p_QueryNodeNum_10F1_Up=dynamic_pointer_cast<Afn10F1>(p_Frame3762Base);
                if(p_QueryNodeNum_10F1_Up->node_total_num_==0)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("查询从节点数量为0，符合要求"));
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryNodeInfo_10F2);
                    emQueryState=Wait_QueryNodeInfo_10F2;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询从节点信息（10F2），等待--回复");
                    p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("查询从节点数量为%1，不符合要求").arg(p_QueryNodeNum_10F1_Up->node_total_num_));
                }
            }
            else if(p_Frame3762Base->afn_ == 0x10&&p_Frame3762Base->dt1_==0x02&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                shared_ptr<Afn10F2> p_QueryNodeInfo_10F2_Up=dynamic_pointer_cast<Afn10F2>(p_Frame3762Base);
                //可以考虑多加判断，只查一次
                if(p_QueryNodeInfo_10F2_Up->node_info_data_unit_.node_total_num_==0)
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
                if(uchar(p_QueryFreq_03F16_Up->carrier_frequence_range_)==initFreq)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("频段保持不变，符合要求"));

                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryRouterID_F0F40);
                    emQueryState=Wait_QueryRouterID_F0F40;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询路由ID和芯片ID（F0F40），等待--回复");
                    p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("运行频段变为%1，不符合要求").arg(uchar(p_QueryFreq_03F16_Up->carrier_frequence_range_)));
                }
            }
            else if(p_Frame3762Base->afn_ == char(0xF0)&&p_Frame3762Base->dt1_==char(0x80)&&p_Frame3762Base->dt2_==0x04&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                shared_ptr<AfnF0F40> p_QueryRouterID_F0F40_Up=dynamic_pointer_cast<AfnF0F40>(p_Frame3762Base);
                if(emQueryState==Wait_QueryRouterID_F0F40)
                {
                    if(memcmp(p_QueryRouterID_F0F40_Up->read_chip_id_unit_up_.mac_address_.addr,ccoAddr,6)==0
                            &&p_QueryRouterID_F0F40_Up->read_chip_id_unit_up_.id_type_==0x02)
                    {
                        //待比对
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("路由ID为%1，符合要求").arg(QString(p_QueryRouterID_F0F40_Up->read_chip_id_unit_up_.id_content_.toHex())));

                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryChipID_F0F40);
                        emQueryState=Wait_QueryChipID_F0F40;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询芯片ID（F0F40），等待--回复");
                        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("路由ID为%1，不符合要求").arg(QString(p_QueryRouterID_F0F40_Up->read_chip_id_unit_up_.id_content_.toHex())));
                    }
                }
                else if(emQueryState==Wait_QueryChipID_F0F40)
                {
                    if(memcmp(p_QueryRouterID_F0F40_Up->read_chip_id_unit_up_.mac_address_.addr,ccoAddr,6)==0
                            &&p_QueryRouterID_F0F40_Up->read_chip_id_unit_up_.id_type_==0x01)
                    {
                        //待比对
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("芯片ID为%1，符合要求").arg(QString(p_QueryRouterID_F0F40_Up->read_chip_id_unit_up_.id_content_.toHex())));

                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryRouterSN_F0F41);
                        emQueryState=Wait_QueryRouterSN_F0F41;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询路由SN（F0F41），等待--回复");
                        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("芯片ID为%1，不符合要求").arg(QString(p_QueryRouterID_F0F40_Up->read_chip_id_unit_up_.id_content_.toHex())));
                    }
                }
            }
            else if(p_Frame3762Base->afn_ == char(0xF0)&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x05&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                shared_ptr<AfnF0F41> p_QueryRouterSN_F0F41_Up=dynamic_pointer_cast<AfnF0F41>(p_Frame3762Base);
                if(memcmp(p_QueryRouterSN_F0F41_Up->mac_address_.addr,ccoAddr,6)==0
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
                uchar workSwitch;
                memcpy(&workSwitch,&p_QueryWorkSwitch_10F4_Up->router_operate_state_unit_.work_switch_,1);
                if(workSwitch==defaultWorkSwitch)//待修改
                {
                    //待比对
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("路由工作开关为C5，符合要求"));

                    emScriptRunState=ScriptSuccess;
                    resultFlag=true;
                    p_AbstractScriptHost->updateProgress(ProcessState_Success, QString("路由擦除Flash测试成功;"));
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("路由工作开关非C5，不符合要求：%1").arg(workSwitch,2,16,QChar('0')));
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

void Script_FlashTest_V3::processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID)
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
void Script_FlashTest_V3::processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID)
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
void Script_FlashTest_V3::sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame)
{
    if(frame==p_EraseFlash_F0F12)
    {
        p_EraseFlash_F0F12->ctrl_field_.dir=kDirDown;
        p_EraseFlash_F0F12->ctrl_field_.prm=kActive;
        p_EraseFlash_F0F12->ctrl_field_.comn_type=kHplc;

        p_EraseFlash_F0F12->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_EraseFlash_F0F12->info_field_.info_field_down.comu_rate=0;
        p_EraseFlash_F0F12->info_field_.info_field_down.comu_module_ident=0;

        sendMsgOct=p_EraseFlash_F0F12->EncodeFrame();
        sendMsgLog=QString("》》路由擦除Flash F0F12：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
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
    else if(frame==p_QueryRouterID_F0F40)
    {
        p_QueryRouterID_F0F40->ctrl_field_.dir=kDirDown;
        p_QueryRouterID_F0F40->ctrl_field_.prm=kActive;
        p_QueryRouterID_F0F40->ctrl_field_.comn_type=kHplc;

        p_QueryRouterID_F0F40->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_QueryRouterID_F0F40->info_field_.info_field_down.comu_rate=0;
        p_QueryRouterID_F0F40->info_field_.info_field_down.comu_module_ident=0;

        memcpy(p_QueryRouterID_F0F40->read_chip_id_unit_down_.mac_address_.addr,ccoAddr,6);
        p_QueryRouterID_F0F40->read_chip_id_unit_down_.id_type_=0x02;
        p_QueryRouterID_F0F40->read_chip_id_unit_down_.id_format_=0x00;
        p_QueryRouterID_F0F40->read_chip_id_unit_down_.id_length_=0x32;

        sendMsgOct=p_QueryRouterID_F0F40->EncodeFrame();
        sendMsgLog=QString("》》查询路由ID F0F40：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_QueryChipID_F0F40)
    {
        p_QueryChipID_F0F40->ctrl_field_.dir=kDirDown;
        p_QueryChipID_F0F40->ctrl_field_.prm=kActive;
        p_QueryChipID_F0F40->ctrl_field_.comn_type=kHplc;

        p_QueryChipID_F0F40->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_QueryChipID_F0F40->info_field_.info_field_down.comu_rate=0;
        p_QueryChipID_F0F40->info_field_.info_field_down.comu_module_ident=0;

        memcpy(p_QueryChipID_F0F40->read_chip_id_unit_down_.mac_address_.addr,ccoAddr,6);
        p_QueryChipID_F0F40->read_chip_id_unit_down_.id_type_=0x01;
        p_QueryChipID_F0F40->read_chip_id_unit_down_.id_format_=0x00;
        p_QueryChipID_F0F40->read_chip_id_unit_down_.id_length_=0x18;

        sendMsgOct=p_QueryChipID_F0F40->EncodeFrame();
        sendMsgLog=QString("》》查询路由芯片ID F0F40：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_QueryRouterSN_F0F41)
    {
        p_QueryRouterSN_F0F41->ctrl_field_.dir=kDirDown;
        p_QueryRouterSN_F0F41->ctrl_field_.prm=kActive;
        p_QueryRouterSN_F0F41->ctrl_field_.comn_type=kHplc;

        p_QueryRouterSN_F0F41->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_QueryRouterSN_F0F41->info_field_.info_field_down.comu_rate=0;
        p_QueryRouterSN_F0F41->info_field_.info_field_down.comu_module_ident=0;

        memcpy(p_QueryRouterSN_F0F41->mac_address_.addr,ccoAddr,6);
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
void Script_FlashTest_V3::sendSrcMsg(DvcType dvcType, int dvcId, QByteArray msg)
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

void Script_FlashTest_V3::timer_timeoutProc()
{
    switch(emScriptRunState)
    {
        case Wait_BuildNetFinish_Whole:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_BuildNetFinish_Whole timeout!!!");
            break;
        }
        case Wait_EraseFlash_Finish:
        {
//            if(emQueryState==Wait_QueryNodeInfo_10F2)
//            {
//                p_timer->stop();
//                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryFreq_03F16);
//                emQueryState=Wait_QueryFreq_05F16;
//                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询运行频段（03F16），等待--回复");
//                p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
//            }
//            else
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
void Script_FlashTest_V3::maxAllowTimer_timeoutProc()
{
    switch(emScriptRunState)
    {
        case Wait_EraseFlash_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_EraseFlash_Finish timeout!!!");
            break;
        }
        default:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==p_maxAllowTimer");
            break;
        }
    }
}
void Script_FlashTest_V3::delayTimer_timeoutProc()
{
    p_delayTimer->stop();
    index=0;
    p_maxAllowTimer->stop();
}

bool Script_FlashTest_V3::meterIsExist(Address meterAddr)
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

