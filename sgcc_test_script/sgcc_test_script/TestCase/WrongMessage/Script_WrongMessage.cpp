#include "Script_WrongMessage.h"

Script_WrongMessage::Script_WrongMessage(QObject *parent) : QObject(parent)
{
    emScriptRunState=TestInit;
    resultFlag=false;
    sendMsgOct.clear();

    p_CtrInfoList=make_shared<QList<shared_ptr<CtrInfo>>>();
    p_BuildNetwork_GW=new BuildNetwork_GW();

    p_Frame3762Helper=make_shared<Frame3762Helper>();
    p_QueryNetScale_10F9=make_shared<Afn10F9>();
    p_ChkCcoAddr_03F4_Down=make_shared<Afn03F4>();
    p_ChkNodeNum_10F1_Down=make_shared<Afn10F1>();
    p_MonitorSlaveNode_13F1=make_shared<Afn13F1>();
    p_ParallelReadMeter_F1F1=make_shared<AfnF1F1>();

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
Script_WrongMessage::~Script_WrongMessage()
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
void Script_WrongMessage::execute()
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
    }
    p_maxAllowTimer->start((timerForReachThresld+timerAfterReachThresld)*1000);
}
void Script_WrongMessage::stop()
{
    if(p_AbstractScriptHost==nullptr)
        return;
    p_AbstractScriptHost->updateProgress(ProcessState_Error, "Test stop!");
    if(needPowerOff==true)
        powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost);
}
void Script_WrongMessage::addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq)
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
void Script_WrongMessage::setHost(AbstractScriptHost *host)
{
    p_AbstractScriptHost=host;
    p_BuildNetwork_GW->setHost(host);
}
bool Script_WrongMessage::config(const QMap<QString,QString> *paraDic)
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
void Script_WrongMessage::processMsg(DvcType dvcType, int id, uchar *data, int datalen)
{
    if(p_CtrInfoList->size()==0)
        return;
    if(emScriptRunState==Wait_BuildNetFinish_Whole)
    {
        if(!p_BuildNetwork_GW->buildNetworkResultFlag)
            p_BuildNetwork_GW->processMsg(dvcType,id,data,datalen);
        else
        {
            emScriptRunState=SendWrongFrame;
            sendWrongFrame();
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
void Script_WrongMessage::processCtrlDvcRes(DvcType dvcType, QList<int> idList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params)
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
    default:
        {
            break;
        }
    }
}

void Script_WrongMessage::processMsgFromCCO(DvcType dvcType, int dvcId)
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
        case SendWrongFrame:
        {
            uchar dtValue3762 = get3762Dt(p_Frame3762Base->dt1_,p_Frame3762Base->dt2_);
            if(p_Frame3762Base->afn_==0x00 && dtValue3762==1 && p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Failed, "收到确认");
            }
            else if(p_Frame3762Base->afn_==0x00 && dtValue3762==2 && p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                shared_ptr<Afn00F2> p_Deny_Up=dynamic_pointer_cast<Afn00F2>(p_Frame3762Base);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到否认，错误状态字"+QString::number(p_Deny_Up->error_code_));
            }
            else if(p_Frame3762Base->afn_==0x03 && dtValue3762==10 && p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("收到03F10上报，可能发生复位"));
            }
            else if(p_Frame3762Base->afn_==0x13 && dtValue3762==1 && p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Failed, "收到13F1回复");
            }
            else if(p_Frame3762Base->afn_==char(0xF1) && dtValue3762==1 && p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Failed, "收到F1F1回复");
            }
            else if(p_Frame3762Base->afn_ == 0x03 && dtValue3762==4 && p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                shared_ptr<Afn03F4> p_ChkCcoAddr_03F4_Up=std::dynamic_pointer_cast<Afn03F4>(p_Frame3762Base);
                if(isArrayEqual(p_CtrInfoList->at(0)->ccoAddr,reinterpret_cast<uchar*>(p_ChkCcoAddr_03F4_Up->master_node_address_.addr),6))
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("主节点地址无问题，继续发送下一条错误报文"));
                    sendWrongFrame();
                }
                else
                {
                    QByteArray ccoAddr;
                    for(int i=0;i<6;i++)
                    {
                        ccoAddr.append(p_ChkCcoAddr_03F4_Up->master_node_address_.addr[i]);
                    }
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("发送错误报文后主节点地址有误："+ccoAddr.toHex()));
                }
            }
            else if(p_Frame3762Base->afn_==0x10 && dtValue3762==1 && p_Frame3762Base->ctrl_field_.dir==kDirUp)
            {
                p_timer->stop();
                shared_ptr<Afn10F1> p_ChkNodeNum_10F1_Up=std::dynamic_pointer_cast<Afn10F1>(p_Frame3762Base);
                if(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size()==p_ChkNodeNum_10F1_Up->node_total_num_)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("从节点数量无问题，查询网络规模"));
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryNetScale_10F9);
                    p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("发送异常报文后从节点数量有误：%1").arg(p_ChkNodeNum_10F1_Up->node_total_num_));
                }
            }
            else if(p_Frame3762Base->afn_ ==0x10 && dtValue3762==9 && p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                shared_ptr<Afn10F9> p_QueryNetScale_10F9_Up=std::dynamic_pointer_cast<Afn10F9>(p_Frame3762Base);
                if(p_QueryNetScale_10F9_Up->network_scale_==p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size()+1)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Success, "网络规模无问题，路由未发现异常");
                    emScriptRunState=ScriptSuccess;
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("发送异常报文后网络规模有误：%0").arg(p_QueryNetScale_10F9_Up->network_scale_));
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

void Script_WrongMessage::processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID)
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
void Script_WrongMessage::processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID)
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

QByteArray Script_WrongMessage::encodeWrongFrame(shared_ptr<void> frame,QString error)
{
    if(frame==p_MonitorSlaveNode_13F1)
    {
        uchar tmpAddr[6];
        memcpy(tmpAddr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(0)->mtrAddr,6);
        uchar comPrtclType=p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(0)->prtcl;

        if(comPrtclType==DLT645_2007)
        {
            uchar CrntPosEneTotal[4]={0x00,0x00,0x01,0x00};
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

            if(error=="CS68_13F1")
            {
                sendMsgOct[sendMsgOct.length()-2]=0x68;
            }
            else if(error=="Length+1_13F1")
            {
                sendMsgOct[1]=sendMsgOct.at(1)+1;
            }
            else if(error=="CS16&Length-1_13F1")
            {
                sendMsgOct[sendMsgOct.length()-2]=0x16;
                sendMsgOct[1]=sendMsgOct.at(1)-1;
            }
            else if(error=="end68_13F1")
            {
                sendMsgOct[sendMsgOct.length()-1]=0x68;
            }
            else if(error=="NoAddrField_13F1")
            {
                sendMsgOct.remove(10,12);

                ushort len = static_cast<ushort>(sendMsgOct.length());
                sendMsgOct[1] = static_cast<char>(len&0xff);
                sendMsgOct[2] = static_cast<char>(len>>8);

                char cs = 0x00;
                for(int index=3;index<sendMsgOct.length()-2;index++)
                {
                    cs += sendMsgOct[index];
                }
                sendMsgOct[sendMsgOct.length()-2] = cs;
            }
            else
            {}
        }
    }
    else if(frame==nullptr)
    {
        uchar tmpAddr[6];
        memcpy(tmpAddr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(0)->mtrAddr,6);
        if(error=="645")
        {
            uchar CrntPosEneTotal[4]={0x00,0x00,0x01,0x00};
            shared_ptr<Rqst_ReadData_0x11> p_ReadData_0x11=make_shared<Rqst_ReadData_0x11>(addr,4,0);
            memcpy(p_ReadData_0x11->addr_,tmpAddr,6);
            memcpy(p_ReadData_0x11->di,CrntPosEneTotal,4);
            sendMsgOct=p_ReadData_0x11->EncodeFrame();
        }
        else if(error=="698")
        {
            shared_ptr<GetRequestNormal> p_GetRequestNormal_ReadData=make_shared<GetRequestNormal>();
            p_GetRequestNormal_ReadData->ctrl_field_.dir = 0;
            p_GetRequestNormal_ReadData->ctrl_field_.prm = 1;
            p_GetRequestNormal_ReadData->ctrl_field_.fra = 0;
            p_GetRequestNormal_ReadData->ctrl_field_.res = 0;
            p_GetRequestNormal_ReadData->ctrl_field_.sc = 0;
            p_GetRequestNormal_ReadData->ctrl_field_.func = 3;

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

            sendMsgOct=p_GetRequestNormal_ReadData->EncodeFrame();
        }
        else if(error=="All68")
        {
            sendMsgOct = QByteArray::fromHex("686868686868686868686868686868686868686868686868686868686868");
        }
        else if(error=="NWmsg")
        {
            sendMsgOct = QByteArray::fromHex("68 30 00 60 20 14 00 20 95 51 93 73 14 09 14 01 02 23 01 02 02 E8 00 00 82 2C 01 12 68 99 99 99 99 99 99 68 08 06 7A 3A 47 4A 36 55 44 16 40 16");
        }
        else if(error=="LongMsg2000")
        {
            sendMsgOct = QByteArray::fromHex("68 d0 07 43");
            for(int i=1;i<=1994;i++)
            {
                static char a = 0;
                sendMsgOct.append(a++);
              //  msg.append(QString("%1").arg(a++,2,16,QChar('0')));
            }
            char cs = 0;
            for(int index=3;index<sendMsgOct.length();index++)
            {
                cs += sendMsgOct[index];
            }
            sendMsgOct.append(cs).append(0x16);
        }
        else if(error=="ShortMsg6")
        {
            sendMsgOct = QByteArray::fromHex("68 06 00 43 43 16");
        }
        else
        {}
    }
    else if(frame==p_ParallelReadMeter_F1F1)
    {
        p_ParallelReadMeter_F1F1->ctrl_field_.dir=kDirDown;
        p_ParallelReadMeter_F1F1->ctrl_field_.prm=kActive;
        p_ParallelReadMeter_F1F1->ctrl_field_.comn_type=kHplc;

        p_ParallelReadMeter_F1F1->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_ParallelReadMeter_F1F1->info_field_.info_field_down.comu_rate=0;
        p_ParallelReadMeter_F1F1->info_field_.info_field_down.comu_module_ident=1;

        uchar tmpAddr[6];
        memcpy(tmpAddr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(0)->mtrAddr,6);
        uchar tmpCcoAddr[6];
        memcpy(tmpCcoAddr,p_CtrInfoList->at(0)->ccoAddr,6);
        memcpy(p_ParallelReadMeter_F1F1->address_field_.src_addr,tmpCcoAddr,6);
        memcpy(p_ParallelReadMeter_F1F1->address_field_.dst_addr,tmpAddr,6);

        uchar comPrtclType=p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(0)->prtcl;
        if(comPrtclType==DLT645_2007)
        {
            QByteArray msg645;
            shared_ptr<Rqst_ReadData_0x11> p_ReadData_0x11=make_shared<Rqst_ReadData_0x11>(addr,4,0);
        //    memcpy(p_ReadData_0x11->addr_,tmpAddr,6);
            memcpy(p_ReadData_0x11->di,PosActEne_Blck,4);
            msg645.append(p_ReadData_0x11->EncodeFrame());

            p_ParallelReadMeter_F1F1->unit_down_.protocol_type_=0x02;
            p_ParallelReadMeter_F1F1->unit_down_.subsidiary_node_num_=0x00;
            p_ParallelReadMeter_F1F1->unit_down_.frame_content_=msg645;
            p_ParallelReadMeter_F1F1->unit_down_.frame_length_=ushort(msg645.size());
            sendMsgOct=p_ParallelReadMeter_F1F1->EncodeFrame();
        }
    }
    else
    {}
    return sendMsgOct;
}

void Script_WrongMessage::sendWrongFrame()
{
    emSendFrameState = afterSendWrongFrame;
    switch(sendNo)
    {
    case 1:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "开始发送错误报文--645，等5s");
        sendSrcMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,encodeWrongFrame(nullptr,"645"));
        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
        sendNo++;
        break;
    }
    case 2:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送错误报文--698，等5s");
        sendSrcMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,encodeWrongFrame(nullptr,"698"));
        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
        sendNo++;
        break;
    }
    case 3:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送13F1长度+1报文，等5s");
        sendSrcMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,encodeWrongFrame(p_MonitorSlaveNode_13F1,"Length+1_13F1"));
        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
        sendNo++;
        break;
    }
    case 4:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送13F1校验码68报文，等5s");
        sendSrcMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,encodeWrongFrame(p_MonitorSlaveNode_13F1,"CS68_13F1"));
        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
        sendNo++;
        break;
    }
    case 5:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送13F1校验码16且长度-1报文，等5s");
        sendSrcMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,encodeWrongFrame(p_MonitorSlaveNode_13F1,"CS16&Length-1_13F1"));
        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
        sendNo++;
        break;
    }
    case 6:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送13F1无地址域报文，等5s");
        sendSrcMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,encodeWrongFrame(p_MonitorSlaveNode_13F1,"NoAddrField_13F1"));
        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
        sendNo++;
        break;
    }
    case 7:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送13F1结束符68报文，等5s");
        sendSrcMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,encodeWrongFrame(p_MonitorSlaveNode_13F1,"end68_13F1"));
        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
        sendNo++;
        break;
    }
    case 8:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送30个字节全68报文，等5s");
        sendSrcMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,encodeWrongFrame(nullptr,"All68"));
        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
        sendNo++;
        break;
    }
    case 9:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送南网本地报文，等5s");
        sendSrcMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,encodeWrongFrame(nullptr,"NWmsg"));
        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
        sendNo++;
        break;
    }
    case 10:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送2000字节长报文，等5s");
        sendSrcMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,encodeWrongFrame(nullptr,"LongMsg2000"));
        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
        sendNo++;
        break;
    }
    case 11:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送无数据域短报文，等5s");
        sendSrcMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,encodeWrongFrame(nullptr,"ShortMsg6"));
        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
        sendNo++;
        break;
    }
    case 12:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送F1F1中645地址域全0报文，等5s");
        sendSrcMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,encodeWrongFrame(p_ParallelReadMeter_F1F1,nullptr));
        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
        sendNo=0;
        break;
    }
    default:
    {
        emSendFrameState=CheckRouter;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "错误报文发送完毕，查询从节点数量");
        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_ChkNodeNum_10F1_Down);
        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
    }
    }
}

void Script_WrongMessage::sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame)
{
    if(frame==p_QueryNetScale_10F9)
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
    else if(frame==p_ChkCcoAddr_03F4_Down)
    {
        p_ChkCcoAddr_03F4_Down->ctrl_field_={kHplc,kActive,kDirDown};
        p_ChkCcoAddr_03F4_Down->info_field_.info_field_down.comu_module_ident=0;
        p_ChkCcoAddr_03F4_Down->info_field_.info_field_down.msg_seq=char((msgSeq>=255)?0:++msgSeq);

        sendMsgOct=p_ChkCcoAddr_03F4_Down->EncodeFrame();
        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("》》查询主节点地址03F4：%1\n").arg(QString(sendMsgOct.toHex())));
    }
    else if(frame==p_ChkNodeNum_10F1_Down)
    {
        p_ChkNodeNum_10F1_Down->ctrl_field_={kHplc,kActive,kDirDown};
        p_ChkNodeNum_10F1_Down->info_field_.info_field_down.comu_module_ident=0;
        p_ChkNodeNum_10F1_Down->info_field_.info_field_down.msg_seq=char((msgSeq>=255)?0:++msgSeq);

        sendMsgOct=p_ChkNodeNum_10F1_Down->EncodeFrame();
        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("》》查询从节点数量10F1：%1\n").arg(QString(sendMsgOct.toHex())));
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
void Script_WrongMessage::sendSrcMsg(DvcType dvcType, int dvcId, QByteArray msg)
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

void Script_WrongMessage::timer_timeoutProc()
{
    p_timer->stop();
    switch(emScriptRunState)
    {
    case Wait_BuildNetFinish_Whole:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_BuildNetFinish_Whole timeout!!!");
        break;
    }
    case SendWrongFrame:
    {
        if(emSendFrameState == afterSendWrongFrame)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "查询主节点地址");
            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_ChkCcoAddr_03F4_Down);
            emSendFrameState = wait_firstQueryCcoAddrRes;
            p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
        }
        else if(emSendFrameState == wait_firstQueryCcoAddrRes)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "查主节点地址未收到回复，等6min后再查");
            p_delayTimer->start(360*1000);
        }
        else if(emSendFrameState == wait_secondQueryCcoAddrRes)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "等6min查主节点地址仍未收到回复");
//            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "等5.5min查主节点地址未回复，再次尝试");
//            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_ChkCcoAddr_03F4_Down);
//            emSendFrameState = wait_thirdQueryCcoAddrRes;
//            p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
        }
//        else if(emSendFrameState == wait_thirdQueryCcoAddrRes)
//        {
//            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "等5.5min查主节点地址2次仍未收到回复");
//        }
        else if(emSendFrameState == CheckRouter)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "回复超时:p_timer");
        }
        else
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Error, QString("emSendFrameState error:%1").arg(emSendFrameState));
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

void Script_WrongMessage::maxAllowTimer_timeoutProc()
{
    p_maxAllowTimer->stop();
    switch(emScriptRunState)
    {
    default:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==p_maxAllowTimer");
        break;
    }
    }
}

void Script_WrongMessage::delayTimer_timeoutProc()
{
    p_delayTimer->stop();
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "查询主节点地址");
    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_ChkCcoAddr_03F4_Down);
    emSendFrameState = wait_secondQueryCcoAddrRes;
    p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
}
