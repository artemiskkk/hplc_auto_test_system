#include "Script_CcoHardWareInit_NotBuildFinish_1.h"

Script_CcoHardWareInit_NotBuildFinish_1::Script_CcoHardWareInit_NotBuildFinish_1(QObject *parent) : QObject(parent)
{
    emScriptRunState = ScriptInit;
    emQueryState = Wait_QueryMasterAddr_03F4;
    resultFlag = false;
    sendMsgOct.clear();

    p_CtrInfoList=make_shared<QList<shared_ptr<CtrInfo>>>();
    p_BuildNetwork_GW=new BuildNetwork_GW();

    p_CtrInfoList=make_shared<QList<shared_ptr<CtrInfo>>>();
    p_Frame3762Helper=make_shared<Frame3762Helper>();
    p_QueryMasterAddr_03F4=make_shared<Afn03F4>();
    p_QueryNetNum_10F21 = make_shared<Afn10F21>();
    p_QueryModuleInfor_10F7= make_shared<Afn10F7>();
    p_QueryNodeNum_10F1=make_shared<Afn10F1>();
    p_QueryNodeInfo_10F2=make_shared<Afn10F2>();
    p_QueryNetSize_10F9=make_shared<Afn10F9>();
    p_QueryRouterID_F0F40=make_shared<AfnF0F40>();
    p_QueryChipID_F0F40=make_shared<AfnF0F40>();
    p_QueryRouterSN_F0F41=make_shared<AfnF0F41>();
    p_QueryFreq_03F16=make_shared<Afn03F16>();
    p_QueryWorkSwitch_10F4=make_shared<Afn10F4>();
    p_ResetRouter_12F1 = make_shared<Afn12F1>();
    p_RouterRequestRead_14F1=make_shared<Afn14F1>();
    p_RouterRequestTime_14F2 = make_shared<Afn14F2>();
    p_QueryBulidChanle =make_shared<AfnF0F38>();

    p_HardReset_01F1=make_shared<Afn01F1>();
    p_ParameterInit_01F2=make_shared<Afn01F2>();
    p_MonitorSlaveNode_13F1=make_shared<Afn13F1>();
    p_confirm_00F1 = make_shared<Afn00F1>();


    p_Frame645Helper=make_shared<Frame645Helper>();
    p_MeterAddrResp_93=make_shared<RspsNormal_ReadAddr_0x93>(addr,6);
    p_FrameOOPHelper=make_shared<FrameOOPHelper>();
    p_GetResponseNormal_ReadAddr=make_shared<GetResponseNormal>();

    p_timer=new QTimer(this);
    p_maxAllowTimer=new QTimer(this);
    p_delayTimer=new QTimer(this);
    connect(p_timer,SIGNAL(timeout()),this,SLOT(timer_timeoutProc()));
    connect(p_maxAllowTimer,SIGNAL(timeout()),this,SLOT(maxAllowTimer_timeoutProc()));
    connect(p_delayTimer,SIGNAL(timeout()),this,SLOT(delayTimer_timeoutProc()));
}

Script_CcoHardWareInit_NotBuildFinish_1::~Script_CcoHardWareInit_NotBuildFinish_1()
{
    p_BuildNetwork_GW->initBuildNetWork();
    if(needPowerOff==true)//断电处理
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
void Script_CcoHardWareInit_NotBuildFinish_1::setHost(AbstractScriptHost *host)
{
    p_AbstractScriptHost=host;
    p_BuildNetwork_GW->setHost(host);
}
void Script_CcoHardWareInit_NotBuildFinish_1::addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq)
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
bool Script_CcoHardWareInit_NotBuildFinish_1::config(const QMap<QString,QString> *paraDic)
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
            (*paraDic)["needBuildNet"].toLower()=="true"?this->needBuildNet=true:this->needBuildNet=false;
        }
        if(paraDic->keys().contains("needPowerOff"))
        {
            (*paraDic)["needPowerOff"].toLower()=="true"?this->needPowerOff=true:this->needPowerOff=false;
        }
        result = true;
    }
    return result;
}


void Script_CcoHardWareInit_NotBuildFinish_1::execute()
{
    p_AbstractScriptHost->updateProgress(ProcessState_Start,"脚本执行流程描述打印：-----whc(2024年04月13)\r\n"
                                                             "1.通用组网流程，组网场景2(不需要组网完成)，Wait_BuildNetFinish_Whole；\r\n"
                                                             "2.03F4查询主节点地址，F0F38查询组网方式，10F1查询从节点数量，10F2查询节点信息，10F21查询网络拓扑信息，F0F40查询路由模块ID，F0F40查询路由芯片ID，F0F41查询SN，03F16查询频段，10F4查询路由状态，10F7查询模块ID，Wait_QueryInitialState_Finish；\r\n"
                                                             "3.01F1硬件初始化，03F10上报路由工况，Wait_QueryHardWareInit_Finish；\r\n"
                                                             "4.10F21查询网络拓扑信息，10F7查询模块ID，03F4查询主节点地址，F0F38查询组网方式，10F1查询从节点数量，10F2查询节点信息，10F4查询路由运行状态，F0F40查询路由模块ID，F0F40查询路由芯片ID，F0F41查询SN，03F16查询频段，Wait_QueryCheckState_Finish；\r\n"
                                         );
    p_AbstractScriptHost->updateProgress(ProcessState_Start, "Test start!"+test_name_);
    emScriptRunState=ScriptInit;
    resultFlag=false;
    addrList.clear();

    if(needBuildNet==true)//场景2-4
    {
        p_BuildNetwork_GW->execute();//执行组网通用脚本
        emScriptRunState=Wait_BuildNetFinish_Whole;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "等待全网组网完成");
    }
    else//场景1
    {
        tryTimes=0;
        index=0;
        emScriptRunState=Wait_QueryInitialState_Finish;
        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryMasterAddr_03F4);
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--03F4查询主节点地址命令(execute)，等待--回复");
        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
    }
    p_maxAllowTimer->start((timerForReachThresld+timerAfterReachThresld)*1000);//此处要启动脚本的最大执行时间定时器
}
void Script_CcoHardWareInit_NotBuildFinish_1::stop()
{
    p_timer->stop();
    p_delayTimer->stop();
    p_maxAllowTimer->stop();
    if(p_AbstractScriptHost==nullptr)
        return;
    p_AbstractScriptHost->updateProgress(ProcessState_Error, QString("Test stop!")+test_name_);
    if(needPowerOff==true)
        powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost);
}
void Script_CcoHardWareInit_NotBuildFinish_1::processMsg(DvcType dvcType, int id, uchar *data, int datalen)
{
    if(p_CtrInfoList->size()==0)
        return;
    if(emScriptRunState==Wait_BuildNetFinish_Whole)//场景2-4
    {
        if(!p_BuildNetwork_GW->startBuildNetFlag)//场景2 关注p_BuildNetwork_GW->startBuildNetFlag标志,开始组网标志
            p_BuildNetwork_GW->processMsg(dvcType,id,data,datalen);
        else
        {
//            if(p_BuildNetwork_GW->emScriptRunState == BuildNetFinish && p_BuildNetwork_GW->buildNetworkResultFlag==true)
//            {
//                tryTimes=0;
//                index=0;
//                emScriptRunState=Wait_QueryInitialState_Finish;
//                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryMasterAddr_03F4);
//                p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--03F4查询主节点地址命令(processMsg)，等待--回复");
//                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
//            }
            tryTimes=0;
            index=0;
            emScriptRunState=Wait_QueryInitialState_Finish;
            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryMasterAddr_03F4);
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--03F4查询主节点地址命令(processMsg)，等待--回复");
            p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
        }
    }
    else//当测试脚本开始执行脚本自己的操作时，均从此进入
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
void Script_CcoHardWareInit_NotBuildFinish_1::processCtrlDvcRes(DvcType dvcType, QList<int> idList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params)
{
    if(isSucs==false)
        return;
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到控制命令回复，参数个数=%1").arg(QString::number(params.size())));
    QList<int> sendParams;
    switch(emScriptRunState)
    {
    case ScriptInit:
    {
        break;
    }
    case Wait_BuildNetFinish_Whole:
    {
        p_BuildNetwork_GW->processCtrlDvcRes(dvcType,idList,ctrlCmdType,isSucs,params);
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

void Script_CcoHardWareInit_NotBuildFinish_1::processMsgFromCCO(DvcType dvcType, int dvcId)
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

        uchar dtValue3762=get3762Dt(p_Frame3762Base->dt1_,p_Frame3762Base->dt2_);
        switch(emScriptRunState)
        {
        case ScriptInit:
        {
            break;
        }
        case Wait_BuildNetFinish_Whole:
        {
            break;
        }
        case Wait_QueryInitialState_Finish:
        {
            if(p_Frame3762Base->afn_ == 0x03 && dtValue3762==4 && p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                shared_ptr<Afn03F4> p_QueryMasterAddr_03F4_Up=dynamic_pointer_cast<Afn03F4>(p_Frame3762Base);
                master_addr_03F4_ =QString(QByteArray(p_QueryMasterAddr_03F4_Up->master_node_address_.addr,6).toHex());
                p_AbstractScriptHost->updateProgress(ProcessState_Processing,  QString(">>>收到主节点地址为：%1").arg(master_addr_03F4_));

                if(memcmp(p_QueryMasterAddr_03F4_Up->master_node_address_.addr,p_CtrInfoList->at(0)->ccoAddr,6)==0)
                {
                    emQueryState = Wait_QueryNodeNum_10F1;
                    tryTimes =0;
                    index =0;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--查询从节点数量（10F1），等待--回复");
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryNodeNum_10F1);
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                }
                else
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed,  QString(">>>Wait_QueryInitialState_Finish，Wait_QueryMasterAddr_03F4，接收到的主节点地址不为期望路由主节点地址测试失败,%1").arg(test_name_));
            }
            else  if(p_Frame3762Base->afn_ == 0x10 && dtValue3762==1 && p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                shared_ptr<Afn10F1> p_QueryNodeNum_10F1_Up=dynamic_pointer_cast<Afn10F1>(p_Frame3762Base);
                node_num_10F1_ = p_QueryNodeNum_10F1_Up->node_total_num_;
                p_AbstractScriptHost->updateProgress(ProcessState_Processing,  QString(">>>收到从节点数量为：%1").arg(node_num_10F1_));

                if(p_QueryNodeNum_10F1_Up->node_total_num_ == p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size())
                {
                    emQueryState = Wait_QueryNetNum_10F21;
                    tryTimes =0;
                    index =0;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--查询网络拓扑信息（10F21），等待--回复");
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryNetNum_10F21);
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                }
                else
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed,  QString(">>>Wait_QueryInitialState_Finish，Wait_QueryNodeNum_10F1，接收到的从节点数量不为期望从节点数量测试失败，1%").arg(test_name_));
            }
            else  if(p_Frame3762Base->afn_ == 0x10 && dtValue3762==21 && p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                shared_ptr<Afn10F21> p_QueryNetInfo_10F21_Up=dynamic_pointer_cast<Afn10F21>(p_Frame3762Base);
                int net_info_10F21 = int(p_QueryNetInfo_10F21_Up->network_typelogy_info_unit_.node_total_num_);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing,  QString(">>>收到节点总数量为：%1").arg(net_info_10F21));

                if(net_info_10F21 < node_num_10F1_+1)
                {

                    emQueryState = Wait_QueryModuleInfor_10F7;
                    tryTimes =0;
                    index =0;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--查询模块ID信息（10F7），等待--回复");
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryModuleInfor_10F7);
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                }
                else
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed,  QString(">>>Wait_QueryInitialState_Finish，Wait_QueryNetInfo_10F21，接收到的网络拓扑信息节点总数量不小于从节点数量+1测试失败，%1").arg(test_name_));
            }
            else if(p_Frame3762Base->afn_ == 0x10 && dtValue3762==7 && p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                shared_ptr<Afn10F7> p_QuerySlaveNodeIdInfo_10F7=dynamic_pointer_cast<Afn10F7>(p_Frame3762Base);
                int response_node  =p_QuerySlaveNodeIdInfo_10F7->node_id_info_unit_.this_node_num_;
                p_AbstractScriptHost->updateProgress(ProcessState_Processing,  QString(">>>收到从节点信息应答的数量为：%1,从节点总数为%2").arg(response_node).arg(p_QuerySlaveNodeIdInfo_10F7->node_id_info_unit_.node_total_num_));
                for(int i = 0;i < response_node;i++)
                {

                    QString  addr = QString::fromLatin1(QByteArray(Address(p_QuerySlaveNodeIdInfo_10F7->node_id_info_unit_.node_id_info_list_.at(i).node_address_).addr,6));
                    QString  id = QString::fromLatin1(p_QuerySlaveNodeIdInfo_10F7->node_id_info_unit_.node_id_info_list_.at(i).node_id_.toHex());
                    query_id_10F7_.append(addr+":"+id);
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,  QString(">>>收到从节点信息应答的信息为：%1,").arg(addr+":"+id));
                }

                emQueryState = Wait_QueryBulidChanle_F0F38;
                tryTimes =0;
                index =0;
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--查询双模组网方式F0F38，等待--回复");
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryBulidChanle);
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);

            }
            else if(p_Frame3762Base->afn_ == char(0xf0) && dtValue3762==38 && p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                shared_ptr<AfnF0F38> p_QueryBulidChanle = dynamic_pointer_cast<AfnF0F38>(p_Frame3762Base);
                bulid_chanle_ = p_QueryBulidChanle->freq_;
                p_AbstractScriptHost->updateProgress(ProcessState_Processing,  QString(">>>查询双模组网方式F0F38为：%1").arg(bulid_chanle_));

                emQueryState = Wait_QueryNodeInfo_10F2;
                tryTimes =0;
                index =0;
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--查询从节信息（10F2），等待--回复");
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryNodeInfo_10F2);
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            }
            else  if(p_Frame3762Base->afn_ == 0x10 && dtValue3762==2 && p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                shared_ptr<Afn10F2> p_QueryNodeInfo_10F2_Up=dynamic_pointer_cast<Afn10F2>(p_Frame3762Base);
                node_info_10F2_ = QString(QByteArray(Address(p_QueryNodeInfo_10F2_Up->node_info_data_unit_.node_info_group_list_.at(0).node_address_).addr,6).toHex());
                p_AbstractScriptHost->updateProgress(ProcessState_Processing,  QString(">>>收到节点信息为：%1").arg(node_info_10F2_));

                if(meterIsExist(p_QueryNodeInfo_10F2_Up->node_info_data_unit_.node_info_group_list_.at(0).node_address_)==true)
                {
                    emQueryState = Wait_QueryRouterID_F0F40;
                    tryTimes =0;
                    index =0;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--查询路由模块ID信息（F0F40），等待--回复");
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryRouterID_F0F40);
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                }
                else
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed,  QString(">>>Wait_QueryInitialState_Finish，Wait_QueryNodeInfo_10F2，接收到的节点信息不为期望节点信息测试失败，%1").arg(test_name_));
            }
            else  if(p_Frame3762Base->afn_ == char(0xf0) && dtValue3762==40 && p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                shared_ptr<AfnF0F40> p_QueryRouterID_F0F40_Up=dynamic_pointer_cast<AfnF0F40>(p_Frame3762Base);
                if(emQueryState == Wait_QueryRouterID_F0F40)
                {
                    if(memcmp(p_QueryRouterID_F0F40_Up->read_chip_id_unit_up_.mac_address_.addr,p_CtrInfoList->at(0)->ccoAddr,6)==0
                        &&p_QueryRouterID_F0F40_Up->read_chip_id_unit_up_.id_type_==0x02)
                    {
                        routerID_F0F40_ = QString(p_QueryRouterID_F0F40_Up->read_chip_id_unit_up_.id_content_.toHex());
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,  QString(">>>收到路由模块ID为：%1").arg(routerID_F0F40_));

                        emQueryState = Wait_QueryChipID_F0F40;
                        tryTimes =0;
                        index =0;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--查询路由芯片ID（F0F40），等待--回复");
                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryChipID_F0F40);
                        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed,  QString(">>>Wait_QueryInitialState_Finish，Wait_QueryRouterID_F0F40,接收到的路由模块ID时地址/ID类型与预期不符测试失败，%1").arg(test_name_));

                }
                else if (emQueryState==Wait_QueryChipID_F0F40)
                {
                    if(memcmp(p_QueryRouterID_F0F40_Up->read_chip_id_unit_up_.mac_address_.addr,p_CtrInfoList->at(0)->ccoAddr,6)==0
                        &&p_QueryRouterID_F0F40_Up->read_chip_id_unit_up_.id_type_==0x01)
                    {
                        chipID_F0F40_ = QString(p_QueryRouterID_F0F40_Up->read_chip_id_unit_up_.id_content_.toHex());
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,  QString(">>>收到路由芯片ID为：%1").arg(chipID_F0F40_));

                        emQueryState = Wait_QueryRouterSN_F0F41;
                        tryTimes =0;
                        index =0;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--查询路由SN（F0F41），等待--回复");
                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryRouterSN_F0F41);
                        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed,  QString(">>>Wait_QueryInitialState_Finish，Wait_QueryChipID_F0F40,接收到的路由芯片ID时地址/ID类型与预期不符测试失败，%1").arg(test_name_));
                }
                else
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed,  QString(">>>Wait_QueryInitialState_Finish，查询模块ID/芯片ID时收到非期望报文,%1").arg(test_name_));
            }
            else if(p_Frame3762Base->afn_ == char(0xF0) && dtValue3762==41 && p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                shared_ptr<AfnF0F41> p_QueryRouterSN_F0F41_Up=dynamic_pointer_cast<AfnF0F41>(p_Frame3762Base);

                if(memcmp(p_QueryRouterSN_F0F41_Up->mac_address_.addr,p_CtrInfoList->at(0)->ccoAddr,6)==0
                    &&p_QueryRouterSN_F0F41_Up->device_type_==0x02)
                {
                    router_sn_F0F41_ =QString(p_QueryRouterSN_F0F41_Up->sn_content_.toHex());
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,  QString(">>>收到路由SN为：%1").arg(router_sn_F0F41_));

                    emQueryState = Wait_QueryFreq_03F16;
                    tryTimes =0;
                    index =0;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--查询路由频段（03F16），等待--回复");
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryFreq_03F16);
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                }
                else
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed,  QString(">>>Wait_QueryInitialState_Finish，Wait_QueryRouterSN_F0F41，接收到的路由SN时地址/ID类型与预期不符测试失败，%1").arg(test_name_));
            }
            else if(p_Frame3762Base->afn_ == 0x03 && dtValue3762==16 && p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                shared_ptr<Afn03F16> p_QueryFreq_03F16_Up=dynamic_pointer_cast<Afn03F16>(p_Frame3762Base);
                freq_03F16_ = uchar(p_QueryFreq_03F16_Up->carrier_frequence_range_);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing,  QString(">>>收到路由频段为：%1").arg(freq_03F16_));

                emQueryState = Wait_QueryWorkSwitch_10F4;
                tryTimes =0;
                index =0;
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--查询路由工作状态（10F4），等待--回复");
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryWorkSwitch_10F4);
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            }
            else if(p_Frame3762Base->afn_ == 0x10 && dtValue3762==4 && p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                shared_ptr<Afn10F4> p_QueryWorkSwitch_10F4_Up=dynamic_pointer_cast<Afn10F4>(p_Frame3762Base);
                event_report_flag_10F4_ = p_QueryWorkSwitch_10F4_Up->router_operate_state_unit_.work_switch_.event_report_flag_;
                area_difference_flag_10F4_ = p_QueryWorkSwitch_10F4_Up->router_operate_state_unit_.work_switch_.area_difference_flag_;
                uchar router_complete_flag =p_QueryWorkSwitch_10F4_Up->router_operate_state_unit_.operate_state_word_.router_complete_flag_;
                p_AbstractScriptHost->updateProgress(ProcessState_Processing,  QString(">>>收到事件上报开关为：%1，期望值为：开启。台区区分开关为：%2，期望值为：关闭。").arg(event_report_flag_10F4_).arg(area_difference_flag_10F4_));
                p_AbstractScriptHost->updateProgress(ProcessState_Processing,  QString(">>>路由完成标志为：%1，期望值为：0x1学习完成").arg(router_complete_flag));

                emScriptRunState = Wait_QueryHardWareInit_Finish;
                tryTimes =0;
                index =0;
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--硬件初始化命令（01F1），等待--回复");
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_HardReset_01F1);
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            }
            else
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing,"收到其他376.2报文，等待处理");
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case Wait_QueryHardWareInit_Finish:
        {
            if(p_Frame3762Base->afn_== char(0x00) && dtValue3762==1 && p_Frame3762Base->ctrl_field_.dir==kDirUp)
            {

                p_timer->stop();
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>收到确认，01F1硬件初始化成功。");
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>进行30s等待，03F10上报路由工况。");
                p_delayTimer->start(30*1000);
            }
            else if(p_Frame3762Base->afn_ == 0x03 && dtValue3762==10 && p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_delayTimer->stop();
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>接收到03F10路由工况上报报文,发送确认帧");
                char seq = p_Frame3762Base->info_field_.info_field_up.msg_seq;
                sendMsg(dvcType,p_CtrInfoList->at(0)->ctrlID,seq,p_confirm_00F1);
                emScriptRunState = Wait_QueryCheckState_Finish;

                emQueryState = Wait_QueryNetNum_10F21;
                tryTimes =0;
                index =0;
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--查询网络拓扑信息（10F21），等待--回复");
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryNetNum_10F21);
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            }
            else
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到其他376.2报文，等待--回复");
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case Wait_QueryCheckState_Finish:
        {

            if(p_Frame3762Base->afn_ == 0x10 && dtValue3762==21 && p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                shared_ptr<Afn10F21> p_QueryNetInfo_10F21_Up=dynamic_pointer_cast<Afn10F21>(p_Frame3762Base);
                int net_info_10F21 = int(p_QueryNetInfo_10F21_Up->network_typelogy_info_unit_.node_total_num_);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing,  QString(">>>收到节点总数量为：%1").arg(net_info_10F21));

                if(net_info_10F21 ==1)
                {
                    emQueryState = Wait_QueryModuleInfor_10F7;
                    tryTimes =0;
                    index =0;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--查询模块ID信息（10F7），等待--回复");
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryModuleInfor_10F7);
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                }
                else
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed,  QString(">>>Wait_QueryCheckState_Finish，Wait_QueryNetInfo_10F21，接收到的网络拓扑信息节点总数量不为1测试失败(如果硬件复位后模块入网太快，此处预估有概率性失败)，%1").arg(test_name_));
            }
            else  if(p_Frame3762Base->afn_ == 0x10 && dtValue3762==7 && p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                shared_ptr<Afn10F7> p_QuerySlaveNodeIdInfo_10F7=dynamic_pointer_cast<Afn10F7>(p_Frame3762Base);
                int response_node  =p_QuerySlaveNodeIdInfo_10F7->node_id_info_unit_.this_node_num_;
                p_AbstractScriptHost->updateProgress(ProcessState_Processing,  QString(">>>收到从节点信息应答的数量为：%1,从节点总数为%2").arg(response_node).arg(p_QuerySlaveNodeIdInfo_10F7->node_id_info_unit_.node_total_num_));
                QStringList query_id_10F7;
                for (int i = 0;i < response_node;i++)
                {

                    QString  addr = QString::fromLatin1(QByteArray(Address(p_QuerySlaveNodeIdInfo_10F7->node_id_info_unit_.node_id_info_list_.at(i).node_address_).addr,6));
                    QString  id = QString::fromLatin1(p_QuerySlaveNodeIdInfo_10F7->node_id_info_unit_.node_id_info_list_.at(i).node_id_.toHex());
                    query_id_10F7.append(addr+":"+id);
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,  QString(">>>收到从节点信息应答的信息为：%1,").arg(addr+":"+id));
                }
                emQueryState = Wait_QueryMasterAddr_03F4;
                tryTimes =0;
                index =0;
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--查询主节点地址（03F4），等待--回复");
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryMasterAddr_03F4);
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            }
            else  if(p_Frame3762Base->afn_ == 0x03 && dtValue3762==4 && p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                shared_ptr<Afn03F4> p_QueryMasterAddr_03F4_Up=dynamic_pointer_cast<Afn03F4>(p_Frame3762Base);
                QString target_data =QString(QByteArray(p_QueryMasterAddr_03F4_Up->master_node_address_.addr,6).toHex());
                p_AbstractScriptHost->updateProgress(ProcessState_Processing,  QString(">>>收到主节点地址为：%1").arg(target_data));

                if(master_addr_03F4_ == target_data)
                {
                    emQueryState = Wait_QueryBulidChanle_F0F38;
                    tryTimes =0;
                    index =0;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--查询双模组网方式F0F38，等待--回复");
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryBulidChanle);
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                }
                else
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed,  QString(">>>Wait_QueryCheckState_Finish，Wait_QueryMasterAddr_03F4，期望主节点地址和接收主节点地址不一致测试失败，期望主节点地址：%1，接收主节点地址：%2,%3").arg(master_addr_03F4_).arg(target_data).arg(test_name_));
            }
            else  if(p_Frame3762Base->afn_ == char(0xf0) && dtValue3762==38 && p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                shared_ptr<AfnF0F38> p_QueryBulidChanle = dynamic_pointer_cast<AfnF0F38>(p_Frame3762Base);

                p_AbstractScriptHost->updateProgress(ProcessState_Processing,  QString(">>>查询双模组网方式F0F38为：%1").arg(p_QueryBulidChanle->freq_));
                if (bulid_chanle_ == p_QueryBulidChanle->freq_)
                {
                    emQueryState = Wait_QueryNodeNum_10F1;
                    tryTimes =0;
                    index =0;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--查询从节点数量（10F1），等待--回复");
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryNodeNum_10F1);
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                }
                else
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed,  QString(">>>Wait_QueryCheckState_Finish，Wait_QueryBulidChanle_F0F38，期望组网方式跟实际不一致测试失败，期望组网方式：%1，回复组网方式：%2,%3").arg(bulid_chanle_).arg(p_QueryBulidChanle->freq_).arg(test_name_));
            }
            else  if(p_Frame3762Base->afn_ == 0x10 && dtValue3762==1 && p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                shared_ptr<Afn10F1> p_QueryNodeNum_10F1_Up=dynamic_pointer_cast<Afn10F1>(p_Frame3762Base);
                int target_data = p_QueryNodeNum_10F1_Up->node_total_num_;
                p_AbstractScriptHost->updateProgress(ProcessState_Processing,  QString(">>>收到从节点数量为：%1").arg(target_data));

                if(node_num_10F1_ == target_data)
                {
                    emQueryState = Wait_QueryNodeInfo_10F2;
                    tryTimes =0;
                    index =0;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--查询从节信息（10F2），等待--回复");
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryNodeInfo_10F2);
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                }
                else
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed,  QString(">>>Wait_QueryCheckState_Finish，Wait_QueryNodeNum_10F1，期望从节点数量和接收从节点数量不一致测试失败，期望从节点数量：%1，接收从节点数量：%2,%3").arg(node_num_10F1_).arg(target_data).arg(test_name_));
            }
            else  if(p_Frame3762Base->afn_ == 0x10 && dtValue3762==2 && p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                shared_ptr<Afn10F2> p_QueryNodeInfo_10F2_Up=dynamic_pointer_cast<Afn10F2>(p_Frame3762Base);
                QString target_data  = QString(QByteArray(Address(p_QueryNodeInfo_10F2_Up->node_info_data_unit_.node_info_group_list_.at(0).node_address_).addr,6).toHex());

                p_AbstractScriptHost->updateProgress(ProcessState_Processing,  QString(">>>收到节点信息为：%1").arg(target_data));

                if(node_info_10F2_ ==target_data )
                {
                    emQueryState = Wait_QueryWorkSwitch_10F4;
                    tryTimes =0;
                    index =0;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--10F4查询路由运行状态，等待--确认\r\n");
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryWorkSwitch_10F4);
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                }
                else
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed,  QString(">>>Wait_QueryCheckState_Finish，Wait_QueryNodeInfo_10F2，期望节点信息和接收节点信息不一致测试失败，期望节点信息：%1，接收节点信息：%2").arg(node_info_10F2_).arg(target_data)+test_name_);
            }
            else  if(p_Frame3762Base->afn_ == 0x10 && dtValue3762==4 && p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                shared_ptr<Afn10F4> p_QueryWorkSwitch_10F4_Up=dynamic_pointer_cast<Afn10F4>(p_Frame3762Base);
                uchar event_report_flag_10F4 = uchar(p_QueryWorkSwitch_10F4_Up->router_operate_state_unit_.work_switch_.event_report_flag_);
                uchar area_difference_flag_10F4 = uchar(p_QueryWorkSwitch_10F4_Up->router_operate_state_unit_.work_switch_.area_difference_flag_);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString(">>>>收到10F40查询路由运行状态报文，事件上报开关为：%1，台区区分开关为：%2。").arg(event_report_flag_10F4).arg(area_difference_flag_10F4));
                if (event_report_flag_10F4 == event_report_flag_10F4_ && area_difference_flag_10F4 == area_difference_flag_10F4_)
                {
                    emQueryState = Wait_QueryRouterID_F0F40;
                    tryTimes =0;
                    index =0;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--查询路由模块ID（F0F40），等待--回复");
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryRouterID_F0F40);
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                }
                else
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString(">>>>Wait_QueryCheckState_Finish，10F4查回来状态与期望状态不一致测试失败，期望10F40查询回来路由运行状态，事件上报开关为：%1，台区区分开关为：%2，路由完成标志为：0，当前状态为：3,%3").arg(event_report_flag_10F4_).arg(area_difference_flag_10F4_).arg(test_name_));

            }
            else  if(p_Frame3762Base->afn_ == char(0xf0) && dtValue3762==40 && p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                shared_ptr<AfnF0F40> p_QueryRouterID_F0F40_Up=dynamic_pointer_cast<AfnF0F40>(p_Frame3762Base);
                if(emQueryState==Wait_QueryRouterID_F0F40)
                {
                    if(memcmp(p_QueryRouterID_F0F40_Up->read_chip_id_unit_up_.mac_address_.addr,p_CtrInfoList->at(0)->ccoAddr,6)==0
                        &&p_QueryRouterID_F0F40_Up->read_chip_id_unit_up_.id_type_==0x02)
                    {
                        QString target_data  = QString(p_QueryRouterID_F0F40_Up->read_chip_id_unit_up_.id_content_.toHex());
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,  QString(">>>收到路由模块ID为：%1").arg(target_data));
                        if (routerID_F0F40_ == target_data)
                        {
                            emQueryState = Wait_QueryChipID_F0F40;
                            tryTimes =0;
                            index =0;
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--查询路由芯片ID（F0F40），等待--回复");
                            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryChipID_F0F40);
                            p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                        }
                        else
                            p_AbstractScriptHost->updateProgress(ProcessState_Failed,  QString(">>>Wait_QueryCheckState_Finish，Wait_QueryRouterID_F0F40，期望路由模块ID和接收路由模块ID不一致测试失败，期望路由模块ID：%1，接收路由模块ID：%2").arg(routerID_F0F40_).arg(target_data)+test_name_);
                    }
                    else
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed,  QString(">>>Wait_QueryCheckState_Finish，Wait_QueryRouterID_F0F40，查询路由模块ID时接收到的节点地址/ID类型与期望不符测试失败，")+test_name_);

                }
                else if (emQueryState==Wait_QueryChipID_F0F40)
                {
                    if(memcmp(p_QueryRouterID_F0F40_Up->read_chip_id_unit_up_.mac_address_.addr,p_CtrInfoList->at(0)->ccoAddr,6)==0
                        &&p_QueryRouterID_F0F40_Up->read_chip_id_unit_up_.id_type_==0x01)
                    {
                        QString target_data  = QString(p_QueryRouterID_F0F40_Up->read_chip_id_unit_up_.id_content_.toHex());
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,  QString(">>>收到路由芯片ID为：%1").arg(target_data));
                        if (chipID_F0F40_ ==target_data)
                        {
                            emQueryState = Wait_QueryRouterSN_F0F41;
                            tryTimes =0;
                            index =0;
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--查询路由SN（F0F41），等待--回复");
                            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryRouterSN_F0F41);
                            p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                        }
                        else
                            p_AbstractScriptHost->updateProgress(ProcessState_Failed,  QString(">>>Wait_QueryCheckState_Finish，Wait_QueryChipID_F0F40，期望路由芯片ID和接收路由芯片ID不一致测试失败，期望路由芯片ID：%1，接收路由芯片ID：%2,").arg(chipID_F0F40_).arg(target_data)+test_name_);

                    }
                    else
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed,  QString(">>>Wait_QueryCheckState_Finish，Wait_QueryChipID_F0F40，查询路由芯片ID时接收到的节点地址/ID类型与期望不符测试失败，")+test_name_);
                }
                else
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed,  QString(">>>Wait_QueryCheckState_Finish，查询模块ID/芯片ID时收到非期望报文,")+test_name_);
            }
            else  if(p_Frame3762Base->afn_ == char(0xF0) && dtValue3762==41 && p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                shared_ptr<AfnF0F41> p_QueryRouterSN_F0F41_Up=dynamic_pointer_cast<AfnF0F41>(p_Frame3762Base);

                if(memcmp(p_QueryRouterSN_F0F41_Up->mac_address_.addr,p_CtrInfoList->at(0)->ccoAddr,6)==0
                    &&p_QueryRouterSN_F0F41_Up->device_type_==0x02)
                {
                    QString target_data =QString(p_QueryRouterSN_F0F41_Up->sn_content_.toHex());
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,  QString(">>>收到路由SN为：%1").arg(target_data));
                    if (router_sn_F0F41_ ==target_data)
                    {
                        emQueryState = Wait_QueryFreq_03F16;
                        tryTimes =0;
                        index =0;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--查询路由频段（03F16），等待--回复");
                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryFreq_03F16);
                        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed,  QString(">>>Wait_QueryCheckState_Finish，Wait_QueryRouterSN_F0F41，期望路由SN和接收路由SN不一致测试失败，期望路由SN：%1，接收路由SN：%2,").arg(router_sn_F0F41_).arg(target_data)+test_name_);

                }
                else
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed,  QString(">>>Wait_QueryCheckState_Finish，查询模块SN时收到错误报文")+test_name_);
            }
            else  if(p_Frame3762Base->afn_ == 0x03 && dtValue3762==16 && p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                shared_ptr<Afn03F16> p_QueryFreq_03F16_Up=dynamic_pointer_cast<Afn03F16>(p_Frame3762Base);
                uchar target_data  = uchar(p_QueryFreq_03F16_Up->carrier_frequence_range_);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing,  QString(">>>收到路由频段为：%1").arg(target_data));
                if(freq_03F16_ == target_data)
                {
                    p_maxAllowTimer->stop();
                    emScriptRunState = ScriptSuccess;
                    p_AbstractScriptHost->updateProgress(ProcessState_Success, "脚本执行成功，"+test_name_+"\r\n");
                }
                else
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed,  QString(">>>Wait_QueryCheckState_Finish，Wait_QueryFreq_03F16，期望路由频段和接收路由频段不一致测试失败，期望路由频段：%1，接收路由频N：%2").arg(freq_03F16_).arg(target_data)+test_name_);

            }
            else
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing,"收到其他376.2报文，等待处理");
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
void Script_CcoHardWareInit_NotBuildFinish_1::processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID)
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
        case ScriptInit:
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
void Script_CcoHardWareInit_NotBuildFinish_1::processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID)
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
        case ScriptInit:
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
void Script_CcoHardWareInit_NotBuildFinish_1::sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame)
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
    else if(frame==p_QueryBulidChanle)
    {
        p_QueryBulidChanle->ctrl_field_.dir=kDirDown;
        p_QueryBulidChanle->ctrl_field_.prm=kActive;
        p_QueryBulidChanle->ctrl_field_.comn_type=kHplc;

        p_QueryBulidChanle->info_field_.info_field_down.msg_seq = char(msgSeq++);
        p_QueryBulidChanle->info_field_.info_field_down.comu_rate = 0;
        p_QueryBulidChanle->info_field_.info_field_down.comu_module_ident = 0;

        sendMsgOct=p_QueryBulidChanle->EncodeFrame();
        sendMsgLog=QString("》》查询路由信道F0F38：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_QueryNetSize_10F9)
    {
        p_QueryNetSize_10F9->ctrl_field_.dir=kDirDown;
        p_QueryNetSize_10F9->ctrl_field_.prm=kActive;
        p_QueryNetSize_10F9->ctrl_field_.comn_type=kHplc;

        p_QueryNetSize_10F9->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_QueryNetSize_10F9->info_field_.info_field_down.comu_rate=0;
        p_QueryNetSize_10F9->info_field_.info_field_down.comu_module_ident=0;

        sendMsgOct=p_QueryNetSize_10F9->EncodeFrame();
        sendMsgLog=QString("》》查询网络规模10F9：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
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
    else if(frame==p_MonitorSlaveNode_13F1)
    {
        if(index>=p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size())
        {
            index=0;
        }
        uchar tmpAddr[6];
        memcpy(tmpAddr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(index)->mtrAddr,6);
        uchar comPrtclType=p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(index)->prtcl;

        if(comPrtclType==0x02)
        {
            uchar CrntPosEneTotal[4]={0x00,0x00,0x01,0x00}; //DI0_DI3
            shared_ptr<Rqst_ReadData_0x11> p_ReadData_0x11=make_shared<Rqst_ReadData_0x11>(addr,4,0);
            //reverseAddr(tmpAddr, 6);
            memcpy(p_ReadData_0x11->addr_,tmpAddr,6);
            memcpy(p_ReadData_0x11->di,CrntPosEneTotal,4);
            QByteArray msg645=p_ReadData_0x11->EncodeFrame();

            p_MonitorSlaveNode_13F1->data_field_down_.delay_tag_=0x00;
            p_MonitorSlaveNode_13F1->data_field_down_.sub_node_num_=0x00;
            p_MonitorSlaveNode_13F1->data_field_down_.protocol_type_=0x02;
            p_MonitorSlaveNode_13F1->data_field_down_.frame_content_=msg645;
            p_MonitorSlaveNode_13F1->data_field_down_.frame_length_=uchar(msg645.size());

            p_MonitorSlaveNode_13F1->ctrl_field_.dir=kDirDown;
            p_MonitorSlaveNode_13F1->ctrl_field_.prm=kActive;
            p_MonitorSlaveNode_13F1->ctrl_field_.comn_type=kHplc;

            p_MonitorSlaveNode_13F1->info_field_.info_field_down.msg_seq=char(msgSeq++);
            p_MonitorSlaveNode_13F1->info_field_.info_field_down.comu_rate=0;
            p_MonitorSlaveNode_13F1->info_field_.info_field_down.comu_module_ident=1;

            uchar tmpCcoAddr[6];
            memcpy(tmpCcoAddr,p_CtrInfoList->at(0)->ccoAddr,6);
            memcpy(p_MonitorSlaveNode_13F1->address_field_.src_addr,tmpCcoAddr,6);
            memcpy(p_MonitorSlaveNode_13F1->address_field_.dst_addr,tmpAddr,6);

            sendMsgOct=p_MonitorSlaveNode_13F1->EncodeFrame();
            sendMsgLog=QString("》》监控从节点13F1,抄读645电表：%1\n").arg(QString(sendMsgOct.toHex()));

            startTime=QDateTime::currentDateTime().toString(CURRENT_TIME_FORMAT);
        }
        else if(comPrtclType==0x03)
        {
            uchar tmpAddr[6];
            memcpy(tmpAddr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(index)->mtrAddr,6);

            shared_ptr<GetRequestNormal> p_GetRequestNormal_ReadData=make_shared<GetRequestNormal>();
            p_GetRequestNormal_ReadData->ctrl_field_.dir = 1;
            p_GetRequestNormal_ReadData->ctrl_field_.prm = 0;
            p_GetRequestNormal_ReadData->ctrl_field_.fra = 0;
            p_GetRequestNormal_ReadData->ctrl_field_.res = 0;
            p_GetRequestNormal_ReadData->ctrl_field_.sc = 0;
            p_GetRequestNormal_ReadData->ctrl_field_.func = 1;

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

            p_MonitorSlaveNode_13F1->info_field_.info_field_down.msg_seq=char(msgSeq++);
            p_MonitorSlaveNode_13F1->info_field_.info_field_down.comu_rate=0;
            p_MonitorSlaveNode_13F1->info_field_.info_field_down.comu_module_ident=1;

            uchar tmpCcoAddr[6];
            memcpy(tmpCcoAddr,p_CtrInfoList->at(0)->ccoAddr,6);
            memcpy(p_MonitorSlaveNode_13F1->address_field_.src_addr,tmpCcoAddr,6);
            memcpy(p_MonitorSlaveNode_13F1->address_field_.dst_addr,tmpAddr,6);

            sendMsgOct=p_MonitorSlaveNode_13F1->EncodeFrame();
            sendMsgLog=QString("》》监控从节点13F1,抄读OOP电表：%1\n").arg(QString(sendMsgOct.toHex()));

            startTime=QDateTime::currentDateTime().toString(CURRENT_TIME_FORMAT);
        }
        else
            return;
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

        memcpy(p_QueryRouterID_F0F40->read_chip_id_unit_down_.mac_address_.addr,p_CtrInfoList->at(0)->ccoAddr,6);
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

        memcpy(p_QueryChipID_F0F40->read_chip_id_unit_down_.mac_address_.addr,p_CtrInfoList->at(0)->ccoAddr,6);
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
    else if(frame==p_QueryNetNum_10F21)
    {
        p_QueryNetNum_10F21->ctrl_field_.dir=kDirDown;
        p_QueryNetNum_10F21->ctrl_field_.prm=kActive;
        p_QueryNetNum_10F21->ctrl_field_.comn_type=kHplc;

        p_QueryNetNum_10F21->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_QueryNetNum_10F21->info_field_.info_field_down.comu_rate=0;
        p_QueryNetNum_10F21->info_field_.info_field_down.comu_module_ident=0;

        p_QueryNetNum_10F21->node_start_no_=0;
        p_QueryNetNum_10F21->node_num_=5;

        sendMsgOct=p_QueryNetNum_10F21->EncodeFrame();
        sendMsgLog=QString("》》查询网络拓扑信息10F21：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_QueryModuleInfor_10F7)
    {
        p_QueryModuleInfor_10F7->ctrl_field_.dir=kDirDown;
        p_QueryModuleInfor_10F7->ctrl_field_.prm=kActive;
        p_QueryModuleInfor_10F7->ctrl_field_.comn_type=kHplc;

        p_QueryModuleInfor_10F7->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_QueryModuleInfor_10F7->info_field_.info_field_down.comu_rate=0;
        p_QueryModuleInfor_10F7->info_field_.info_field_down.comu_module_ident=0;

        p_QueryModuleInfor_10F7->node_start_no_=0;
        p_QueryModuleInfor_10F7->node_num_=topoCntPerTime;

        sendMsgOct=p_QueryModuleInfor_10F7->EncodeFrame();
        sendMsgLog=QString("》》查询STA模块ID 10F7：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_ResetRouter_12F1)
    {
        p_ResetRouter_12F1->ctrl_field_.dir=kDirDown;
        p_ResetRouter_12F1->ctrl_field_.prm=kActive;
        p_ResetRouter_12F1->ctrl_field_.comn_type=kHplc;

        p_ResetRouter_12F1->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_ResetRouter_12F1->info_field_.info_field_down.comu_rate=0;
        p_ResetRouter_12F1->info_field_.info_field_down.comu_module_ident=0;

        sendMsgOct=p_ResetRouter_12F1->EncodeFrame();
        sendMsgLog=QString("》》设置路由复位12F1：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_MeterAddrResp_93)
    {
        memcpy(p_MeterAddrResp_93->addr,(*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[meterID]->mtrAddr,6);
        memcpy(p_MeterAddrResp_93->addr_,(*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[meterID]->mtrAddr,6);

        sendMsgOct=p_MeterAddrResp_93->EncodeFrame();
        sendMsgLog=QString("》》 读通信地址应答(0x93)：%1\n").arg(QString(sendMsgOct.toHex()));
    }
    else if(frame==p_RouterRequestRead_14F1)
    {
        p_RouterRequestRead_14F1->ctrl_field_.dir=kDirDown;
        p_RouterRequestRead_14F1->ctrl_field_.prm=kPassive;
        p_RouterRequestRead_14F1->ctrl_field_.comn_type=kHplc;

        p_RouterRequestRead_14F1->info_field_.info_field_down.msg_seq=char(msgSeq);
        p_RouterRequestRead_14F1->info_field_.info_field_down.comu_rate=0;
        p_RouterRequestRead_14F1->info_field_.info_field_down.comu_module_ident=1;

        memcpy(p_RouterRequestRead_14F1->address_field_.dst_addr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(meterID)->mtrAddr,6);
        memcpy(p_RouterRequestRead_14F1->address_field_.src_addr,p_CtrInfoList->at(0)->ccoAddr,6);

        p_RouterRequestRead_14F1->router_request_read_unit_.read_flag_=char(0x01);
        p_RouterRequestRead_14F1->router_request_read_unit_.delay_related_flag_=0x00;
        p_RouterRequestRead_14F1->router_request_read_unit_.subsidiary_node_num_=0x00;
        p_RouterRequestRead_14F1->router_request_read_unit_.frame_length_=uchar(0x00);

        QByteArray msg645;
        p_RouterRequestRead_14F1->router_request_read_unit_.frame_content_=msg645;

        sendMsgOct.clear();
        sendMsgOct=p_RouterRequestRead_14F1->EncodeFrame();
        sendMsgLog=QString("》》路由请求抄读14F1,抄读645电表：%1\n").arg(QString(sendMsgOct.toHex()));
        startTime=QDateTime::currentDateTime().toString(CURRENT_TIME_FORMAT);

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
void Script_CcoHardWareInit_NotBuildFinish_1::sendSrcMsg(DvcType dvcType, int dvcId, QByteArray msg)
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

void Script_CcoHardWareInit_NotBuildFinish_1::timer_timeoutProc()
{
    switch(emScriptRunState)
    {
    case Wait_BuildNetFinish_Whole:
    {
        p_CtrInfoList->at(0)->inNetResult=false;
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("全网组网成功率：%1%").arg(p_CtrInfoList->at(0)->inNetSuccessRate*100));
        break;
    }
    case Wait_QueryInitialState_Finish:
    {
        p_timer->stop();
        switch(emQueryState)
        {
        case Wait_QueryMasterAddr_03F4:
        {
            if(++tryTimes<3)
            {
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryMasterAddr_03F4);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--03F4查询主节点地址命令(execute)，等待--回复");
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            }
            else
                p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("Wait_QueryInitialState_Finish,Wait_QueryMasterAddr_03F4，03F4查询主节点3次超时失败")+test_name_);
            break;
        }
        case Wait_QueryBulidChanle_F0F38:
        {
            if(++tryTimes<3)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--查询双模组网方式F0F38，等待--回复");
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryBulidChanle);
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            }
            else
                p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("Wait_QueryInitialState_Finish,Wait_QueryBulidChanle_F0F38，F0F38组网方式3次超时失败")+test_name_);
            break;
        }
        case Wait_QueryNodeNum_10F1:
        {
            if(++tryTimes<3)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--查询从节点数量（10F1），等待--回复");
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryNodeNum_10F1);
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            }
            else
                p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("Wait_QueryInitialState_Finish,Wait_QueryNodeNum_10F1，查询从节点数量（10F1）3次超时失败")+test_name_);
            break;
        }
        case Wait_QueryNodeInfo_10F2:
        {
            if(++tryTimes<3)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--查询从节信息（10F2），等待--回复");
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryNodeInfo_10F2);
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            }
            else
                p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("Wait_QueryInitialState_Finish,Wait_QueryNodeInfo_10F2，查询从节信息（10F2）3次超时失败")+test_name_);
            break;
        }
        case Wait_QueryNetNum_10F21:
        {
            if(++tryTimes<3)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--查询网络拓扑信息（10F21），等待--回复");
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryNetNum_10F21);
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            }
            else
                p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("Wait_QueryInitialState_Finish,Wait_QueryNetNum_10F21，查询网络规模（10F9）3次超时失败")+test_name_);
            break;
        }
        case Wait_QueryModuleInfor_10F7:
        {
            if(++tryTimes<3)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--查询模块ID信息（10F7），等待--回复");
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryModuleInfor_10F7);
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            }
            else
                p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("Wait_QueryInitialState_Finish,Wait_QueryModuleInfor_10F7，查询网络规模（10F9）3次超时失败")+test_name_);
            break;
        }
        case Wait_QueryRouterID_F0F40:
        {
            if(++tryTimes<3)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--查询路由模块ID（F0F40），等待--回复");
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryRouterID_F0F40);
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            }
            else
                p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("Wait_QueryInitialState_Finish,Wait_QueryRouterID_F0F40，查询路由模块ID（F0F40）3次超时失败")+test_name_);
            break;
        }
        case Wait_QueryChipID_F0F40:
        {
            if(++tryTimes<3)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--查询路由芯片ID（F0F40），等待--回复");
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryChipID_F0F40);
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            }
            else
                p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("Wait_QueryInitialState_Finish,Wait_QueryChipID_F0F40，查询路由芯片ID（F0F40）3次超时失败")+test_name_);
            break;
        }
        case Wait_QueryRouterSN_F0F41:
        {
            if(++tryTimes<3)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--查询路由SN（F0F41），等待--回复");
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryRouterSN_F0F41);
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            }
            else
                p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("Wait_QueryInitialState_Finish,Wait_QueryRouterSN_F0F41，查询路由SN（F0F41）3次超时失败")+test_name_);
            break;
        }
        case Wait_QueryFreq_03F16:
        {
            if(++tryTimes<3)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--查询路由频段（03F16），等待--回复");
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryFreq_03F16);
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            }
            else
                p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("Wait_QueryInitialState_Finish,Wait_QueryFreq_03F16，查询路由频段（03F16）3次超时失败")+test_name_);
            break;
        }
        case Wait_QueryWorkSwitch_10F4:
        {
            if(++tryTimes<3)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--查询路由工作状态（10F4），等待--回复");
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryWorkSwitch_10F4);
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            }
            else
                p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("Wait_QueryInitialState_Finish,Wait_QueryWorkSwitch_10F4，查询路由工作状态（10F4）3次超时失败")+test_name_);
            break;
        }
        }
        break;
    }
    case Wait_QueryHardWareInit_Finish:
    {
        p_timer->stop();
        if(++tryTimes<3)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--硬件初始化命令（01F1），等待--回复");
            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_HardReset_01F1);
            p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
        }
        else
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("Wait_QueryHardWareInit_Finish，硬件初始化命令（01F1）3次超时失败")+test_name_);
        break;
    }
    case Wait_QueryCheckState_Finish:
    {
        p_timer->stop();
        switch(emQueryState)
        {
        case Wait_QueryMasterAddr_03F4:
        {
            if(++tryTimes<3)
            {
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryMasterAddr_03F4);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--03F4查询主节点地址命令(execute)，等待--回复");
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            }
            else
                p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("Wait_QueryCheckState_Finish,Wait_QueryMasterAddr_03F4，03F4查询主节点3次超时失败")+test_name_);
            break;
        }
        case Wait_QueryBulidChanle_F0F38:
        {
            if(++tryTimes<3)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--查询双模组网方式F0F38，等待--回复");
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryBulidChanle);
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            }
            else
                p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("Wait_QueryCheckState_Finish,Wait_QueryBulidChanle_F0F38，F0F38组网方式3次超时失败")+test_name_);
            break;
        }
        case Wait_QueryNodeNum_10F1:
        {
            if(++tryTimes<3)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--查询从节点数量（10F1），等待--回复");
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryNodeNum_10F1);
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            }
            else
                p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("Wait_QueryCheckState_Finish,Wait_QueryNodeNum_10F1，查询从节点数量（10F1）3次超时失败")+test_name_);
            break;
        }
        case Wait_QueryNodeInfo_10F2:
        {
            if(++tryTimes<3)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--查询从节信息（10F2），等待--回复");
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryNodeInfo_10F2);
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            }
            else
                p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("Wait_QueryCheckState_Finish,Wait_QueryNodeInfo_10F2，查询从节信息（10F2）3次超时失败")+test_name_);
            break;
        }
        case Wait_QueryNetNum_10F21:
        {
            if(++tryTimes<3)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--查询网络拓扑信息（10F21），等待--回复");
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryNetNum_10F21);
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            }
            else
                p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("Wait_QueryCheckState_Finish,Wait_QueryNetSize_10F9，查询网络规模（10F9）3次超时失败")+test_name_);
            break;
        }
        case Wait_QueryModuleInfor_10F7:
        {
            if(++tryTimes<3)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--查询模块ID信息（10F7），等待--回复");
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryModuleInfor_10F7);
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            }
            else
                p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("Wait_QueryCheckState_Finish,Wait_QueryModuleInfor_10F7，查询网络规模（10F9）3次超时失败")+test_name_);
            break;
        }
        case Wait_QueryRouterID_F0F40:
        {
            if(++tryTimes<3)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--查询路由模块ID（F0F40），等待--回复");
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryRouterID_F0F40);
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            }
            else
                p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("Wait_QueryCheckState_Finish,Wait_QueryRouterID_F0F40，查询路由模块ID（F0F40）3次超时失败")+test_name_);
            break;
        }
        case Wait_QueryChipID_F0F40:
        {
            if(++tryTimes<3)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--查询路由芯片ID（F0F40），等待--回复");
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryChipID_F0F40);
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            }
            else
                p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("Wait_QueryCheckState_Finish,Wait_QueryChipID_F0F40，查询路由芯片ID（F0F40）3次超时失败")+test_name_);
            break;
        }
        case Wait_QueryRouterSN_F0F41:
        {
            if(++tryTimes<3)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--查询路由SN（F0F41），等待--回复");
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryRouterSN_F0F41);
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            }
            else
                p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("Wait_QueryCheckState_Finish,Wait_QueryRouterSN_F0F41，查询路由SN（F0F41）3次超时失败")+test_name_);
            break;
        }
        case Wait_QueryFreq_03F16:
        {
            if(++tryTimes<3)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--查询路由频段（03F16），等待--回复");
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryFreq_03F16);
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            }
            else
                p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("Wait_QueryCheckState_Finish,Wait_QueryFreq_03F16，查询路由频段（03F16）3次超时失败")+test_name_);
            break;
        }
        case Wait_QueryWorkSwitch_10F4:
        {
            if(++tryTimes<3)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--查询路由工作状态（10F4），等待--回复");
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryWorkSwitch_10F4);
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            }
            else
                p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("Wait_QueryCheckState_Finish,Wait_QueryWorkSwitch_10F4，查询路由工作状态（10F4）3次超时失败")+test_name_);
            break;
        }
        }
        break;
    }
    default:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==p_timer");
        break;
    }
    }
}

void Script_CcoHardWareInit_NotBuildFinish_1::maxAllowTimer_timeoutProc()
{
    p_delayTimer->stop();
    switch(emScriptRunState)
    {
    default:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Error, QString("State machine run error!!!==p_maxAllowTimer,")+test_name_);
        break;
    }
    }
}

void Script_CcoHardWareInit_NotBuildFinish_1::delayTimer_timeoutProc()
{
    p_delayTimer->stop();
    switch(emScriptRunState)
    {
    case Wait_BuildNetFinish_Whole:
    {
        break;
    }
    case Wait_QueryInitialState_Finish:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString(">>>>Wait_QueryInitialState_Finish，延时时间到，未收到期望报文测试失败，")+test_name_);
        break;
    }
    case Wait_QueryHardWareInit_Finish:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString(">>>>Wait_QueryHardWareInit_Finish，延时时间到，未收到期望报文测试失败，")+test_name_);
        break;
    }
    case Wait_QueryCheckState_Finish:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString(">>>>Wait_QueryCheckState_Finish，延时时间到，未收到期望03F10上报路由工况报文测试失败，")+test_name_);
        break;
    }
    default:
    {
        break;
    }
    }
}


bool Script_CcoHardWareInit_NotBuildFinish_1::meterIsExist(Address meterAddr)
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
