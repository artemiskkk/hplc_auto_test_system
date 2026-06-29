#include "Script_CollectImmediately_Zhejiang.h"

Script_CollectImmediately_Zhejiang::Script_CollectImmediately_Zhejiang(QObject *parent) : QObject(parent)
{
    emScriptRunState=TestInit;
    resultFlag=false;
    sendMsgOct.clear();

    p_CtrInfoList=make_shared<QList<shared_ptr<CtrInfo>>>();
    p_BuildNetwork_Zhejiang=make_shared<BuildNetwork_Zhejiang>();

    p_Frame3762Helper=make_shared<Frame3762Helper>();
    p_QueryReportNode_10F150=make_shared<Afn10F150_Zhejiang>();
    p_Confirm_00F1=make_shared<Afn00F1>();

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
Script_CollectImmediately_Zhejiang::~Script_CollectImmediately_Zhejiang()
{
    p_BuildNetwork_Zhejiang->initBuildNetWork();
    if(needPowerOff==true)
        powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost);
    p_BuildNetwork_Zhejiang->initBuildNetWork();

    if(p_timer==nullptr)
        return;
    p_timer->stop();
    delete p_timer;
    p_timer=nullptr;

    delete p_maxAllowTimer;
}
void Script_CollectImmediately_Zhejiang::execute()
{
    p_AbstractScriptHost->updateProgress(ProcessState_Start, "Test start!");

    resultFlag=false;
    p_BuildNetwork_Zhejiang->flagNeedAddMeter=false;

    if(needBuildNet==true)
    {
        p_BuildNetwork_Zhejiang->execute();
        emScriptRunState=Wait_BuildNetFinish_Whole;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "等待全网组网完成");
        p_timer->start(timerForReachThresld*1000);
    }
    else
    {
        p_delayTimer->start(waitTime*1000);
        emScriptRunState=Wait_CollectImmediately_Finish;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "等待即装即采上报");
    }
    p_maxAllowTimer->start((timerForReachThresld+timerAfterReachThresld)*1000);

}
void Script_CollectImmediately_Zhejiang::stop()
{
    if(p_AbstractScriptHost==nullptr)
        return;
    p_AbstractScriptHost->updateProgress(ProcessState_Error, "Test stop!");
    if(needPowerOff==true)
        powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost);
}
void Script_CollectImmediately_Zhejiang::addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq)
{
    p_CtrInfoList->clear();
    comnAddAddrsInfo(p_CtrInfoList, p_ConcentratorInfoList, p_MeterInfoList, p_SchemeCfgInfoList);
    concentratorCnt=ushort(p_CtrInfoList->size());
    uchar dstFreq=freq&0x0f;
    uchar dstPrtcl=uchar(freq)>>4;
    if(dstPrtcl==0x03)
        setMeterAddrsPrtcl(p_CtrInfoList,dstPrtcl);

    p_BuildNetwork_Zhejiang->setCtrInfoListAndFreq(p_CtrInfoList, dstFreq);
}
void Script_CollectImmediately_Zhejiang::setHost(AbstractScriptHost *host)
{
    p_AbstractScriptHost=host;
    p_BuildNetwork_Zhejiang->setHost(host);
}
bool Script_CollectImmediately_Zhejiang::config(const QMap<QString,QString> *paraDic)
{
    bool result = false;
    if(paraDic!=nullptr)
    {
        p_BuildNetwork_Zhejiang->config(paraDic);

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
void Script_CollectImmediately_Zhejiang::processMsg(DvcType dvcType, int id, uchar *data, int datalen)
{
    if(p_CtrInfoList->size()==0)
        return;
    if(emScriptRunState==Wait_BuildNetFinish_Whole)
    {
        if(!p_BuildNetwork_Zhejiang->resultFlag)
            p_BuildNetwork_Zhejiang->processMsg(dvcType,id,data,datalen);
        else
        {
            p_BuildNetwork_Zhejiang->initBuildNetWork();
            p_delayTimer->start(waitTime*1000);
            emScriptRunState=Wait_CollectImmediately_Finish;
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "等待即装即采上报");
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
void Script_CollectImmediately_Zhejiang::processCtrlDvcRes(DvcType dvcType, QList<int> idList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params)
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
            p_BuildNetwork_Zhejiang->processCtrlDvcRes(dvcType,idList,ctrlCmdType,isSucs,params);
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

void Script_CollectImmediately_Zhejiang::processMsgFromCCO(DvcType dvcType, int dvcId)
{
    if(dvcId!=p_CtrInfoList->at(0)->ctrlID)
        return;
    bool haveCompleteMsg=true;
    while(haveCompleteMsg)
    {
        qInfo()<<QString("CCO-解析前 buf3762=%1").arg(QString((p_CtrInfoList->at(0)->buf.toHex())));
        shared_ptr<Frame3762Base> p_Frame3762Base = p_Frame3762Helper->DecodeLocalMsg(&(p_CtrInfoList->at(0)->buf),haveCompleteMsg,Zhejiang);
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
            case Wait_CollectImmediately_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x06&&p_Frame3762Base->dt1_==0x08&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
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
                            p_BuildNetwork_Zhejiang->reportNodeList_06F4.append(node);
                        }
                        for(int i=0;i<p_ReportNode_06F4_Up->report_node_info_unit_.node_info_list_.size();i++)
                        {
                            NodeInfo node;
                            node.node_address_=p_ReportNode_06F4_Up->report_node_info_unit_.node_info_list_.at(i).node_address_;
                            node.node_protocol_=p_ReportNode_06F4_Up->report_node_info_unit_.node_info_list_.at(i).node_protocol_;
                            node.deviceType=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_device_type_;
                            p_BuildNetwork_Zhejiang->reportNodeList_06F4.append(node);
                        }
                    }
                    else if(p_ReportNode_06F4_Up->report_node_info_unit_.report_node_device_type_==0x00)
                    {
                        NodeInfo node;
                        node.node_address_=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_address_;
                        node.node_protocol_=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_protocol_;
                        node.deviceType=p_ReportNode_06F4_Up->report_node_info_unit_.report_node_device_type_;
                        p_BuildNetwork_Zhejiang->reportNodeList_06F4.append(node);
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
            case ScriptSuccess:
            {
                break;
            }
        }
    }
}
void Script_CollectImmediately_Zhejiang::processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID)
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
void Script_CollectImmediately_Zhejiang::processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID)
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
void Script_CollectImmediately_Zhejiang::sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame)
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
void Script_CollectImmediately_Zhejiang::sendSrcMsg(DvcType dvcType, int dvcId, QByteArray msg)
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

void Script_CollectImmediately_Zhejiang::timer_timeoutProc()
{
    switch(emScriptRunState)
    {
        case Wait_BuildNetFinish_Whole:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_BuildNetFinish_Whole timeout!!!");
            break;
        }
        default:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==p_timer");
            break;
        }
    }
}
void Script_CollectImmediately_Zhejiang::maxAllowTimer_timeoutProc()
{
    switch(emScriptRunState)
    {
        default:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==p_maxAllowTimer");
            break;
        }
    }
}
void Script_CollectImmediately_Zhejiang::delayTimer_timeoutProc()
{
    p_delayTimer->stop();
    statisticsNode();
}

void Script_CollectImmediately_Zhejiang::statisticsNode()
{
    nodeInfoStatisticsList.clear();
    QList<NodeInfoStatistics> tempNodeInfoStatisticsList;
    for(int i=0;i<p_BuildNetwork_Zhejiang->reportNodeList_06F4.size();i++)
    {
        bool isExist=false;
        for(int j=0;j<tempNodeInfoStatisticsList.size();j++)
        {
            if(p_BuildNetwork_Zhejiang->reportNodeList_06F4.at(i)==tempNodeInfoStatisticsList.at(j).nodeInfo)
            {
                isExist=true;
                tempNodeInfoStatisticsList[j].count++;
                break;
            }
            else
                continue;
        }
        if(isExist==false)
        {
            NodeInfoStatistics nodeInfoSts;
            nodeInfoSts.nodeInfo.node_address_=p_BuildNetwork_Zhejiang->reportNodeList_06F4.at(i).node_address_;
            nodeInfoSts.nodeInfo.node_protocol_=p_BuildNetwork_Zhejiang->reportNodeList_06F4.at(i).node_protocol_;
            nodeInfoSts.nodeInfo.deviceType=p_BuildNetwork_Zhejiang->reportNodeList_06F4.at(i).deviceType;
            nodeInfoSts.count++;
            tempNodeInfoStatisticsList.append(nodeInfoSts);
        }
    }
    while(tempNodeInfoStatisticsList.size()>0)
    {
        nodeInfoStatisticsList.append(tempNodeInfoStatisticsList.at(0));
        for(int j=1;j<tempNodeInfoStatisticsList.size();j++)
        {
            if(tempNodeInfoStatisticsList.at(0).nodeInfo.node_address_==tempNodeInfoStatisticsList.at(j).nodeInfo.node_address_)
            {
                nodeInfoStatisticsList.append(tempNodeInfoStatisticsList.at(j));
                tempNodeInfoStatisticsList.removeAt(j);
            }
        }
        tempNodeInfoStatisticsList.removeAt(0);
    }
    QString result="06F4上报情况如下\n";
    for(int i=0;i<nodeInfoStatisticsList.size();i++)
    {
        result+=QString("表号：%1 协议：%2 设备类型：%3 上报次数：%4次\n").arg(QString(QByteArray(nodeInfoStatisticsList.at(i).nodeInfo.node_address_.addr,6).toHex()))
                .arg(QString(nodeInfoStatisticsList.at(i).nodeInfo.node_protocol_).rightJustified(2,'0'))
                .arg(QString(nodeInfoStatisticsList.at(i).nodeInfo.deviceType).rightJustified(2,'0'))
                .arg(nodeInfoStatisticsList.at(i).count);
    }
    //可扩展比较方案
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, result);
    p_AbstractScriptHost->updateProgress(ProcessState_Success, "即装即采测试成功");

}

