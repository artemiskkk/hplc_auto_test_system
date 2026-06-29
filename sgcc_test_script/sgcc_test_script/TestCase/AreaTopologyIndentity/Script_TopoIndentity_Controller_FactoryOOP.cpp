#include "Script_TopoIndentity_Controller_FactoryOOP.h"

Script_TopoIndentity_Controller_FactoryOOP::Script_TopoIndentity_Controller_FactoryOOP(QObject *parent) : QObject(parent)
{
    emScriptRunState=ReadMeterInit;
    resultFlag=false;
    sendMsgOct.clear();

    p_CtrInfoList=make_shared<QList<shared_ptr<CtrInfo>>>();
    p_BuildNetwork_GW=new BuildNetwork_GW();

    p_MsgBase_3762=make_shared<Frame3762Helper>();
    p_RouterPause_12F2=make_shared<Afn12F2>();
    p_RouterRecover_12F3=make_shared<Afn12F3>();
    p_MonitorSlaveNode_13F1=make_shared<Afn13F1>();

    p_Frame645Helper=make_shared<Frame645Helper>();
    p_MeterAddrResp_93=make_shared<RspsNormal_ReadAddr_0x93>(addr,6);
    p_FrameOOPHelper=make_shared<FrameOOPHelper>();
    p_GetResponseNormal_ReadAddr=make_shared<GetResponseNormal>();
    p_ActionRequest=make_shared<ActionRequest>();
//    p_MstrStnNrmlRqst=new MstrStnNrmlRqst();
//    p_MeterAddrResp_93=new MeterAddrResp_93();
//    p_SlaveNodeNormalResp=new SlaveNodeNormalResp();

    p_timer=new QTimer();
    p_maxAllowTimer=new QTimer();
    p_delayTimer=new QTimer();
    connect(p_timer,SIGNAL(timeout()),this,SLOT(timer_timeoutProc()));
    connect(p_maxAllowTimer,SIGNAL(timeout()),this,SLOT(maxAllowTimer_timeoutProc()));
    connect(p_delayTimer,SIGNAL(timeout()),this,SLOT(delayTimer_timeoutProc()));
}
Script_TopoIndentity_Controller_FactoryOOP::~Script_TopoIndentity_Controller_FactoryOOP()
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

void Script_TopoIndentity_Controller_FactoryOOP::execute()
{
    p_AbstractScriptHost->updateProgress(ProcessState_Start, "Test start!");

    emScriptRunState=ReadMeterInit;
    resultFlag=false;
    addrList.clear();
    readInfoInit();

    if(needBuildNet==true)
    {
        p_BuildNetwork_GW->execute();
        emScriptRunState=Wait_BuildNetFinish_Whole;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "等待全网组网完成");
        p_timer->start((timerForReachThresld+timerAfterReachThresld)*1000);
    }
    else
    {
        sendMsg(ReadCtrlDvc,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_ActionRequest);
        emScriptRunState=Wait_Finish_SendOOPData;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--OOP命令，等待--回复");
        p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
    }
    p_maxAllowTimer->start((timerForReachThresld+timerAfterReachThresld)*1000);

}
void Script_TopoIndentity_Controller_FactoryOOP::stop()
{
    if(p_AbstractScriptHost==nullptr)
        return;
    p_AbstractScriptHost->updateProgress(ProcessState_Error, "Test stop!");
    if(needPowerOff==true)
        powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost);
}
void Script_TopoIndentity_Controller_FactoryOOP::addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq)
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
void Script_TopoIndentity_Controller_FactoryOOP::setHost(AbstractScriptHost *host)
{
    p_AbstractScriptHost=host;
    p_BuildNetwork_GW->setHost(host);
}
bool Script_TopoIndentity_Controller_FactoryOOP::config(const QMap<QString,QString> *paraDic)
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
void Script_TopoIndentity_Controller_FactoryOOP::processMsg(DvcType dvcType, int id, uchar *data, int datalen)
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
            times=ushort(oiStructList.size());
            sendMsg(ReadCtrlDvc,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_ActionRequest);
            emScriptRunState=Wait_Finish_SendOOPData;
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--OOP命令，等待--回复");
            p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
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
        else if(dvcType==ReadCtrlDvc)
        {
            for(int i=0; i<p_CtrInfoList->at(0)->keyList.size(); i++)
            {
                if(id == p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->value(p_CtrInfoList->at(0)->keyList.at(i))->dvcId)
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
void Script_TopoIndentity_Controller_FactoryOOP::processCtrlDvcRes(DvcType dvcType, QList<int> idList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params)
{
    if(isSucs==false)
        return;
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到控制命令回复，参数个数=%1").arg(QString::number(params.size())));
    //    if(emScriptRunState!=Wait_BuildNetFinish_Whole)
    //        Refresh_CtrInfo_Result_for_CtrlCmdRes(p_CtrInfoList->at(0), dvcType, idList.at(0), ctrlCmdType);
    QList<int> sendParams;
    switch(emScriptRunState)
    {
        case ReadMeterInit:
        {
            break;
        }
        case Wait_BuildNetFinish_Whole:
        {
            p_BuildNetwork_GW->processCtrlDvcRes(dvcType,idList,ctrlCmdType,isSucs,params);
            break;
        }
        case Wait_Finish_SendOOPData:
        {
            break;
        }
        case ScriptSuccess:
        {
            break;
        }
    }
}
void Script_TopoIndentity_Controller_FactoryOOP::processMsgFromCCO(DvcType dvcType, int dvcId)
{
    if(dvcId!=p_CtrInfoList->at(0)->ctrlID)
        return;
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
            case ReadMeterInit:
            {
                break;
            }
            case Wait_BuildNetFinish_Whole:
            {
                break;
            }
            case Wait_Finish_SendOOPData:
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
void Script_TopoIndentity_Controller_FactoryOOP::processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID)
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
            case ReadMeterInit:
            {
                break;
            }
            case Wait_BuildNetFinish_Whole:
            {
                break;
            }
            case Wait_Finish_SendOOPData:
            {
                if(MsgBase_645_ptr->ctrlCode_==READ_ADDR)
                {
                    p_timer->stop();
                    sendMsg(dvcType,dvcId,mtrlID,p_MeterAddrResp_93);
                }
                else
                {
                    p_timer->stop();
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
            case ScriptSuccess:
            {
                break;
            }
        }
    }
}
void Script_TopoIndentity_Controller_FactoryOOP::processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID)
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
            case ReadMeterInit:
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
            case Wait_Finish_SendOOPData:
            {
                if(MsgBase_OOP_ptr->service_type_==ACTION_RESPONSE_SERVER && MsgBase_OOP_ptr->service_sub_type_==uchar(ActionResponseType::kActionResponseNormal))
                {
                    p_timer->stop();
                    char tempAddr[6];
                    memcpy(tempAddr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(0)->mtrAddr,6);

                    if(!QByteArray::fromHex(MsgBase_OOP_ptr->address_field_.sa.address.toLatin1()).contains(QByteArray(tempAddr,6)))
                        continue;
                    shared_ptr<ActionResponseNormal> p_ActionResponseNormal=dynamic_pointer_cast<ActionResponseNormal>(MsgBase_OOP_ptr);
                    if(p_ActionResponseNormal==nullptr)
                        continue;
                    if(p_ActionResponseNormal->a_action_result_.dar==kSuccess&&p_ActionResponseNormal->a_action_result_.omd.OI==oiStructList.at(index).oi)
                    {
                        oiStructList[index].readFlag="正常";
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("回复正常！"));
                        index++;
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("回复异常！"));
                        index++;
                    }
                    if(index>=times)
                    {
                        index=0;
                        int denyCount=0;
                        QString info;
                        for(int i=0;i<oiStructList.size();i++)
                        {
                            if(oiStructList.at(i).readFlag=="异常")
                                denyCount++;
                            info+=QString("序号%1 OI为%2 特征信号频率为%3Hz 回复%4\n").arg(i+1).arg(QString::number(oiStructList.at(i).oi,16)).arg(QString::number(oiStructList.at(i).frequence)).arg(oiStructList.at(i).readFlag);
                        }
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, info);
                        emScriptRunState=ScriptSuccess;
                        if(denyCount==0)
                            p_AbstractScriptHost->updateProgress(ProcessState_Success, QString("户变识别测试完毕;"));
                        else
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("存在%1次异常回复！").arg(denyCount));
                            p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("户变识别测试完毕，存在%1次异常回复！").arg(denyCount));
                        }
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("等待200s，开始下一次发送！"));
                        p_delayTimer->start(200*1000);
                    }
                }
                else if(MsgBase_OOP_ptr->service_type_==GET_REQUEST_CLIENT && MsgBase_OOP_ptr->service_sub_type_==uchar(GetRequestType::kGetRequestNormal))
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

void Script_TopoIndentity_Controller_FactoryOOP::sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame)
{
    if(frame==p_ActionRequest)
    {
        uchar tmpAddr[6];
        memcpy(tmpAddr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(0)->mtrAddr,6);

        p_ActionRequest->ctrl_field_.dir = 0;
        p_ActionRequest->ctrl_field_.prm = 1;
        p_ActionRequest->ctrl_field_.fra = 0;
        p_ActionRequest->ctrl_field_.res = 0;
        p_ActionRequest->ctrl_field_.sc = 0;
        p_ActionRequest->ctrl_field_.func = 3;

        p_ActionRequest->address_field_.sa.addr_type = 0;
        p_ActionRequest->address_field_.sa.logic_addr = 2;
        ///////
        if(oiStructList.at(index).oi==0x4E05)
        {
            p_ActionRequest->address_field_.sa.logic_addr = 0;
            p_ActionRequest->address_field_.sa.addr_len = 5;
            p_ActionRequest->address_field_.sa.address = QString((QByteArray(reinterpret_cast<char*>(tmpAddr),6).toHex()));
        }
        else if(oiStructList.at(index).oi==0x4E06||oiStructList.at(index).oi==0x4E01)
        {
            p_ActionRequest->address_field_.sa.addr_len = 6;
            p_ActionRequest->address_field_.sa.address = QString((QByteArray(reinterpret_cast<char*>(tmpAddr),6).toHex()))+"63";
        }

        p_ActionRequest->address_field_.ca.address = 0x10;

        p_ActionRequest->piid_.reserve = 0;
        p_ActionRequest->piid_.serve_priority = 0;
        p_ActionRequest->piid_.serve_seq = 0;

        p_ActionRequest->a_action_.omd.OI=oiStructList.at(index).oi;
        p_ActionRequest->a_action_.omd.method_index=0x7f;
        p_ActionRequest->a_action_.omd.operate_mode=0x00;

        p_ActionRequest->a_action_.data_ptr=make_shared<DataList>();
        p_ActionRequest->a_action_.data_ptr->type_=kStructure;
        if(oiStructList.at(index).oi==0x4E05)
        {
            shared_ptr<DataBasic> device_type_ptr=make_shared<DataBasic>();
            device_type_ptr->type_=kEnum;
            device_type_ptr->data_.append(char(oiStructList.at(index).currentST.device_type_));
            dynamic_pointer_cast<DataList>(p_ActionRequest->a_action_.data_ptr)->list_data_member_.append(device_type_ptr);
            shared_ptr<DataBasic> send_style_ptr=make_shared<DataBasic>();
            send_style_ptr->type_=kEnum;
            send_style_ptr->data_.append(char(oiStructList.at(index).currentST.send_style_));
            dynamic_pointer_cast<DataList>(p_ActionRequest->a_action_.data_ptr)->list_data_member_.append(send_style_ptr);
        }
        shared_ptr<DataBasic> bit_width_ptr=make_shared<DataBasic>();
        bit_width_ptr->type_=kLong_unsigned;
        bit_width_ptr->data_.append(char(oiStructList.at(index).currentST.bit_width_>>8));
        bit_width_ptr->data_.append(char(oiStructList.at(index).currentST.bit_width_));
        dynamic_pointer_cast<DataList>(p_ActionRequest->a_action_.data_ptr)->list_data_member_.append(bit_width_ptr);
        shared_ptr<DataBasic> high_pulse_width_ptr=make_shared<DataBasic>();
        high_pulse_width_ptr->type_=kLong_unsigned;
        high_pulse_width_ptr->data_.append(char(oiStructList.at(index).currentST.high_pulse_width_>>8));
        high_pulse_width_ptr->data_.append(char(oiStructList.at(index).currentST.high_pulse_width_));
        dynamic_pointer_cast<DataList>(p_ActionRequest->a_action_.data_ptr)->list_data_member_.append(high_pulse_width_ptr);
        shared_ptr<DataBasic> low_pulse_width_ptr=make_shared<DataBasic>();
        low_pulse_width_ptr->type_=kLong_unsigned;
        low_pulse_width_ptr->data_.append(char(oiStructList.at(index).currentST.low_pulse_width_>>8));
        low_pulse_width_ptr->data_.append(char(oiStructList.at(index).currentST.low_pulse_width_));
        dynamic_pointer_cast<DataList>(p_ActionRequest->a_action_.data_ptr)->list_data_member_.append(low_pulse_width_ptr);
        shared_ptr<DataString> signal_ptr=make_shared<DataString>();
        signal_ptr->type_=kBit_string;
        signal_ptr->data_.append(oiStructList.at(index).currentST.word_info_);
        dynamic_pointer_cast<DataList>(p_ActionRequest->a_action_.data_ptr)->list_data_member_.append(signal_ptr);
        if(oiStructList.at(index).oi==0x4E06)
        {
            shared_ptr<DataBasic> frequence_ptr=make_shared<DataBasic>();
            frequence_ptr->type_=kDouble_long_unsigned;
            QString frequence_=QString::number(int(oiStructList.at(index).currentST.frequence_*100),16).rightJustified(8,'0');
            frequence_ptr->data_=QByteArray::fromHex(frequence_.toLatin1());
            dynamic_pointer_cast<DataList>(p_ActionRequest->a_action_.data_ptr)->list_data_member_.append(frequence_ptr);
            shared_ptr<DataString> extend_ptr=make_shared<DataString>();
            extend_ptr->type_=kOctet_string;
            dynamic_pointer_cast<DataList>(p_ActionRequest->a_action_.data_ptr)->list_data_member_.append(extend_ptr);
        }
        else if(oiStructList.at(index).oi==0x4E05)
        {
            shared_ptr<DataBasic> send_time_ptr=make_shared<DataBasic>();
            send_time_ptr->type_=kDate_time_s;
            send_time_ptr->data_.append(QByteArray::fromHex(QString("00000000000000").toLatin1()));
            dynamic_pointer_cast<DataList>(p_ActionRequest->a_action_.data_ptr)->list_data_member_.append(send_time_ptr);
            shared_ptr<DataBasic> send_span_ptr=make_shared<DataBasic>();
            send_span_ptr->type_=kLong_unsigned;
            send_span_ptr->data_.append(char(oiStructList.at(index).currentST.send_span_>>8));
            send_span_ptr->data_.append(char(oiStructList.at(index).currentST.send_span_));
            dynamic_pointer_cast<DataList>(p_ActionRequest->a_action_.data_ptr)->list_data_member_.append(send_span_ptr);
            shared_ptr<DataBasic> first_send_time_ptr=make_shared<DataBasic>();
            first_send_time_ptr->type_=kDate_time_s;
            first_send_time_ptr->data_.append(QByteArray::fromHex(QString("00000000000000").toLatin1()));
            dynamic_pointer_cast<DataList>(p_ActionRequest->a_action_.data_ptr)->list_data_member_.append(first_send_time_ptr);
            shared_ptr<DataBasic> device_num_ptr=make_shared<DataBasic>();
            device_num_ptr->type_=kLong_unsigned;
            device_num_ptr->data_.append(char(oiStructList.at(index).currentST.device_num_>>8));
            device_num_ptr->data_.append(char(oiStructList.at(index).currentST.device_num_));
            dynamic_pointer_cast<DataList>(p_ActionRequest->a_action_.data_ptr)->list_data_member_.append(device_num_ptr);
        }
        p_ActionRequest->time_tag_field_.optional_ = 0;

        QByteArray msg698=p_ActionRequest->EncodeFrame();

        sendMsgOct=p_ActionRequest->EncodeFrame();
        sendMsgLog=QString("》》OOP电表命令：%1\n").arg(QString(sendMsgOct.toHex()));
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
void Script_TopoIndentity_Controller_FactoryOOP::sendSrcMsg(DvcType dvcType, int dvcId, QByteArray msg)
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

void Script_TopoIndentity_Controller_FactoryOOP::timer_timeoutProc()
{
    switch(emScriptRunState)
    {
        case Wait_BuildNetFinish_Whole:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait build whole net finish timeout!!!");
            break;
        }
        case Wait_Finish_SendOOPData:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_Finish_WriteData timeout!!!");
            break;
        }
        default:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==p_timer");
            break;
        }
    }
}
void Script_TopoIndentity_Controller_FactoryOOP::maxAllowTimer_timeoutProc()
{
    switch(emScriptRunState)
    {
        case Wait_Finish_SendOOPData:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "Wait_Finish_13F1 timeout!!!"+QString("  13F1抄表成功率：%1%").arg(p_CtrInfoList->at(0)->successRate[1]*100));
            QString failedMeter = GenerateFailedMeterStr_ReadMeter(p_CtrInfoList->at(0),1);
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, failedMeter);

            p_CtrInfoList->at(0)->successConsume[1]=double(timerForReachThresld)/double(p_CtrInfoList->at(0)->successCnt[1]);
            p_maxAllowTimer->stop();

            p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==p_maxAllowTimer");
            break;
        }
        default:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==p_maxAllowTimer");
            break;
        }
    }
}
void Script_TopoIndentity_Controller_FactoryOOP::delayTimer_timeoutProc()
{
    p_delayTimer->stop();

    sendMsg(ReadCtrlDvc,p_CtrInfoList->at(0)->ctrlID,index,p_ActionRequest);
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--OOP命令，等待--完成");
    p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
}


void Script_TopoIndentity_Controller_FactoryOOP::readInfoInit()
{
    currentStructList.clear();
    OOPCurrentStruct currentST1;
    currentST1.device_type_=0x00;
    currentST1.send_style_=0x00;
    currentST1.bit_width_=600;
    currentST1.frequence_=833.33;
    currentST1.word_info_length_=2;
    currentST1.word_info_=QByteArray::fromHex("AAE9");
    currentST1.high_pulse_width_=400;
    currentST1.low_pulse_width_=800;
    currentST1.device_num_=0x00;
    currentStructList.append(currentST1);

    OOPCurrentStruct currentST2;
    currentST1.device_type_=0x00;
    currentST1.send_style_=0x00;
    currentST2.bit_width_=600;
    currentST2.frequence_=1666.66;
    currentST2.word_info_length_=2;
    currentST2.word_info_=QByteArray::fromHex("AAE9");
    currentST2.high_pulse_width_=200;
    currentST2.low_pulse_width_=400;
    currentST2.device_num_=0x00;
    currentStructList.append(currentST2);

    oiStructList.clear();
    ushort OI_Array[3]={0x4E06,0x4E05,0x4E01};
    for(int i=0;i<currentStructList.size();i++)
    {
        for(int j=0;j<3;j++)
        {
            if(i!=0&&j>0)
                continue;
            OIStruct oiST;
            oiST.oi=OI_Array[j];
            oiST.currentST=currentStructList.at(i);
            oiST.frequence=currentStructList.at(i).frequence_;
            if(j==0)
                oiST.currentST.need_Frequence_=true;
            oiStructList.append(oiST);
        }
    }
}
