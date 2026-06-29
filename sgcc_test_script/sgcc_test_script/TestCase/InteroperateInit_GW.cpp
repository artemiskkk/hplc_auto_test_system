#include "InteroperateInit_GW.h"

InteroperateInit_GW::InteroperateInit_GW(QObject *parent) : QObject(parent)
{
    p_timer=make_shared<QTimer>();
    p_maxAllowTimer=make_shared<QTimer>();
    p_delayTimer=make_shared<QTimer>();
    connect(p_timer.get(),&QTimer::timeout,this,&InteroperateInit_GW::timer_timeout);
    connect(p_maxAllowTimer.get(),&QTimer::timeout,this,&InteroperateInit_GW::maxAllowTimer_timeout);
    connect(p_delayTimer.get(),&QTimer::timeout,this,&InteroperateInit_GW::delayTimer_timeout);

    p_MsgBase_3762 = make_shared<qgdw_3762_protocol::Frame3762Helper>();
    p_MsgBase_645 = make_shared<dlt_645_Protocol::Frame645Helper>();
    p_MeterAddrResp_93 = make_shared<dlt_645_Protocol::RspsNormal_ReadAddr_0x93>(addr,6);
    p_GetResponseNormal_ReadAddr=make_shared<GetResponseNormal>();
}
void InteroperateInit_GW::execute()
{
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("互操作检测开始！！！"));
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("检测设备初始化！！！"));
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("开始设置检测工装波特率..."));

    emDvcType=CCO_GW;
    setBaudRate("CCO","115200");
}
void InteroperateInit_GW::stop()
{
    p_timer->stop();
    p_delayTimer->stop();
    p_maxAllowTimer->stop();
}
void InteroperateInit_GW::setHost(AbstractScriptHost *host)
{
    p_AbstractScriptHost=host;
}
bool InteroperateInit_GW::config(const QMap<QString,QString> *paraDic)
{
    bool result = true;
    if(paraDic!=nullptr)
    {

    }
    return result;
}
void InteroperateInit_GW::setCtrInfoList(shared_ptr<QList<shared_ptr<CtrInfo>>> p_CtrInfoList)
{
    this->p_CtrInfoList=p_CtrInfoList;
}

void InteroperateInit_GW::processCtrlDvcRes(DvcType dvcType, QList<int> idList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params)
{
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("互操作初始化收到控制命令回复，设备类型=%1，命令类型=%2，参数个数=%3").arg(dvcType).arg(ctrlCmdType).arg(QString::number(params.size())));

    if(isSucs==false)
        return;


    Refresh_CtrInfo_Result_for_CtrlCmdRes(p_CtrInfoList->at(0), dvcType, idList.at(0), ctrlCmdType);    //该接口屏蔽

    switch(emScriptRunState)
    {
    case Init:
    {
        break;
    }
    case Wait_SetBaudRate_Finish:
    {
        p_timer->stop();
        if(dvcType == CCO_GW && emDvcType == CCO_GW)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("国网CCO槽位_设置波特率成功"));
            emDvcType = SingleSTA;
            setBaudRate("单相STA","115200");
        }
        else if(dvcType == SingleSTA && emDvcType == SingleSTA)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("单通槽位_设置波特率成功"));
            emDvcType = ThreeSTA;
            setBaudRate("三相STA","115200");
        }
        else if (dvcType == ThreeSTA && emDvcType == ThreeSTA)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("三通槽位_设置波特率成功"));

            delayTime(100);
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("开始对检测工装断电操作..."));
            emDvcType=CCO_GW;
            setPowerOff12V("CCO");
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
        break;
    }
    case Wait_PowerOff12V_Finish:
    {
        p_timer->stop();
        if(dvcType == CCO_GW && emDvcType == CCO_GW)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("国网路由槽位_12V断电成功"));
            emDvcType = SingleSTA;
            setPowerOff12V("单相STA");
        }
        else if(dvcType == SingleSTA && emDvcType == SingleSTA)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("单通槽位_12V断电成功"));
            emDvcType = ThreeSTA;
            setPowerOff12V("三相STA");
        }
        else if (dvcType == ThreeSTA && emDvcType == ThreeSTA)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("三通槽位_12V断电成功"));

            delayTime(100);
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("开始对检测工装上电操作..."));
            emDvcType=CCO_GW;
            setPowerOn12V("CCO");
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
        break;
    }
    case Wait_PowerOn12V_Finish:
    {
        p_timer->stop();
        if(dvcType == CCO_GW && emDvcType == CCO_GW)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("国网路由槽位_12V上电成功"));
            if(flagPowerOnSTA)
            {
                emDvcType = SingleSTA;
                setPowerOn12V("单相STA");
            }
            else
            {
                stop();
                emit signalStartFollowTest();
            }
        }
        else if(dvcType == SingleSTA && emDvcType == SingleSTA)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("单通槽位_12V上电成功"));
            emDvcType = ThreeSTA;
            setPowerOn12V("三相STA");
        }
        else if (dvcType == ThreeSTA && emDvcType == ThreeSTA)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("三通槽位_12V上电成功"));

            delayTime(100);
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("开始对检测模块进行复位操作..."));
            emDvcType=CCO_GW;
            setResetModule("CCO");
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
        break;
    }
    case Wait_ResetModule_Finish:
    {
        p_timer->stop();
        if(dvcType == CCO_GW && emDvcType == CCO_GW)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("国网路由槽位_复位成功"));
            emDvcType = SingleSTA;
            setResetModule("单相STA");
        }
        else if(dvcType == SingleSTA && emDvcType == SingleSTA)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("单通槽位_复位成功"));
            emDvcType = ThreeSTA;
            setResetModule("三相STA");
        }
        else if (dvcType == ThreeSTA && emDvcType == ThreeSTA)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("三通槽位_复位成功"));

            delayTime(100);
            emScriptRunState=Wait_AssignAddrsFinish;
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("模块复位操作完成，等待STA读表号..."));
            p_timer->start(120*1000);
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
        break;
    }
    case Wait_AssignAddrsFinish:
    {
        break;
    }
    default:
    {
        break;
    }
    }
}

void InteroperateInit_GW::timer_timeout()
{
    p_timer->stop();
    p_AbstractScriptHost->updateProgress(ProcessState_Error, QString("p_timer timeout!!!"));
}

void InteroperateInit_GW::maxAllowTimer_timeout()
{

}

void InteroperateInit_GW::delayTimer_timeout()
{

}

void InteroperateInit_GW::processMsg(DvcType dvcType, int id, uchar *data, int datalen)
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

    }
    else
    {
        return;
    }
}
void InteroperateInit_GW::processMsgFromCco(DvcType dvcType, int dvcId)
{
    bool haveCompleteMsg=true;
    while(haveCompleteMsg)
    {
        qInfo()<<QString("CCO-解析前 buf3762=%1").arg(QString((p_CtrInfoList->at(0)->buf.toHex())));
        shared_ptr<Frame3762Base> p_Frame3762Base = p_MsgBase_3762->DecodeLocalMsg(&(p_CtrInfoList->at(0)->buf),haveCompleteMsg);
        qInfo()<<QString("CCO-解析后 buf3762=%1").arg(QString((p_CtrInfoList->at(0)->buf.toHex())));

        if(p_Frame3762Base==nullptr)
            continue;
        switch(emScriptRunState)
        {
            case Wait_ResetModule_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x03&&p_Frame3762Base->dt1_==0x2&&p_Frame3762Base->dt2_==0x01&&p_Frame3762Base->ctrl_field_.dir == kDirUp )
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到CCO复位上报的03F10"));
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }

                break;
            }
            default:
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                break;
            }
        }
    }
}
void InteroperateInit_GW::processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID)
{
    QString stringDvcType;
    if(dvcType == SingleSTA)
    {
        stringDvcType = QString("单通;");
    }
    else if(dvcType == ThreeSTA)
    {
        stringDvcType = QString("三通;");
    }

    bool haveCompleteMsg=true;
    while(haveCompleteMsg)
    {
        shared_ptr<dlt_645_Protocol::Frame645Base> MsgBase_645_ptr = dlt_645_Protocol::Frame645Helper::DecodeLocalMsg(&((*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[mtrlID]->buf645),haveCompleteMsg);

        if(MsgBase_645_ptr==nullptr)
            break;

        switch(emScriptRunState)
        {
            case Init:
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
                        emScriptRunState=InitFinish;
                        emit signalStartFollowTest();
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
                                    emScriptRunState=InitFinish;
                                    emit signalStartFollowTest();
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
void InteroperateInit_GW::processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID)
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
        shared_ptr<FrameOOPBase> MsgBase_OOP_ptr = FrameOOPHelper::DecodeMsg(&((*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[mtrlID]->buf698),haveCompleteMsg);

        if(MsgBase_OOP_ptr==nullptr)
            continue;

        switch(emScriptRunState)
        {
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
                        emScriptRunState=InitFinish;
                        emit signalStartFollowTest();
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
void InteroperateInit_GW::processMsgFromCJQ(DvcType dvcType,int dvcId)
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
        case Wait_AssignAddrsFinish:
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
void InteroperateInit_GW::sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame)
{
    QByteArray sendMsgOct;
    QString sendMsgLog;
    if(frame==p_MeterAddrResp_93)
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
void InteroperateInit_GW::sendSrcMsg(DvcType dvcType, int dvcId, QByteArray msg)
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

void InteroperateInit_GW::setBaudRate(QString dvcType, QString baudRate)
{
    emScriptRunState=Wait_SetBaudRate_Finish;
    QList<int> idList = getDvcIdList(emDvcType);
    QList<double> sendParams;
    sendParams.append(baudRate.toDouble());
    p_AbstractScriptHost->controlDvc(emDvcType,idList,CtrlCmd_SetBaudRate,sendParams);
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("开始设置%1槽位波特率为%2bps...").arg(dvcType).arg(baudRate));
    p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
}
void InteroperateInit_GW::setPowerOff12V(QString dvcType)
{
    emScriptRunState=Wait_PowerOff12V_Finish;
    QList<int> idList = getDvcIdList(emDvcType);
    QList<double> sendParams;
    p_AbstractScriptHost->controlDvc(emDvcType,idList,CtrlCmd_PowerOff_12V,sendParams);
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("开始对%1槽位12V断电...").arg(dvcType));
    p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
}
void InteroperateInit_GW::setPowerOn12V(QString dvcType)
{
    emScriptRunState=Wait_PowerOn12V_Finish;
    QList<int> idList = getDvcIdList(emDvcType);
    QList<double> sendParams;
    p_AbstractScriptHost->controlDvc(emDvcType,idList,CtrlCmd_PowerOn_12V,sendParams);
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("开始对%1槽位12V上电...").arg(dvcType));
    p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
}
void InteroperateInit_GW::setResetModule(QString dvcType)
{
    emScriptRunState=Wait_ResetModule_Finish;
    QList<int> idList = getDvcIdList(emDvcType);
    QList<double> sendParams;
    p_AbstractScriptHost->controlDvc(emDvcType,idList,CtrlCmd_ModuleRST,sendParams);
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("开始对%1模块复位...").arg(dvcType));
    p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
}
QList<int> InteroperateInit_GW::getDvcIdList(DvcType dvcType)
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
