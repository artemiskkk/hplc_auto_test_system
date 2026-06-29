#include "Script_SnManage_CCO.h"

Script_SnManage_CCO::Script_SnManage_CCO(QObject *parent) : QObject(parent)
{
    emScriptRunState=TestInit;
    resultFlag=false;
    sendMsgOct.clear();

    p_CtrInfoList=make_shared<QList<shared_ptr<CtrInfo>>>();
    p_BuildNetwork_GW=new BuildNetwork_GW();

    p_Frame3762Helper=make_shared<Frame3762Helper>();
    p_ParaInit_01F2=make_shared<Afn01F2>();
    p_QueryRouterSn_F0F41=make_shared<AfnF0F41>();
    p_SetRouterSn_F0F42=make_shared<AfnF0F42>();

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

    snHeader=QDateTime::currentDateTime().toString("yyMMddhhmmss");
    snList<<snHeader+sn_0<<snHeader+sn_1;//<<sn_2
}
Script_SnManage_CCO::~Script_SnManage_CCO()
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
void Script_SnManage_CCO::execute()
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
        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,snIndex,p_SetRouterSn_F0F42);
        emScriptRunState=Wait_SetRouterSn_Finish;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--设置CCO生产序列号Sn，等待--确认");
        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
    }
    p_maxAllowTimer->start((timerForReachThresld+timerAfterReachThresld)*1000);

}
void Script_SnManage_CCO::stop()
{
    if(p_AbstractScriptHost==nullptr)
        return;
    p_AbstractScriptHost->updateProgress(ProcessState_Error, "Test stop!");
    if(needPowerOff==true)
        powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost);
}
void Script_SnManage_CCO::addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq)
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
void Script_SnManage_CCO::setHost(AbstractScriptHost *host)
{
    p_AbstractScriptHost=host;
    p_BuildNetwork_GW->setHost(host);
}
bool Script_SnManage_CCO::config(const QMap<QString,QString> *paraDic)
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
void Script_SnManage_CCO::processMsg(DvcType dvcType, int id, uchar *data, int datalen)
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
            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,snIndex,p_SetRouterSn_F0F42);
            emScriptRunState=Wait_SetRouterSn_Finish;
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--设置CCO生产序列号Sn（F0F42），等待--确认");
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
void Script_SnManage_CCO::processCtrlDvcRes(DvcType dvcType, QList<int> snList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params)
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
            p_BuildNetwork_GW->processCtrlDvcRes(dvcType,snList,ctrlCmdType,isSucs,params);
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

void Script_SnManage_CCO::processMsgFromCCO(DvcType dvcType, int dvcId)
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
            case Wait_SetRouterSn_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp&&p_Frame3762Base->info_field_.info_field_up.msg_seq==char(msgSeq-1))
                {
                    p_timer->stop();
//                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_ParaInit_01F2);
//                    emScriptRunState=Wait_ParaInit_Finish;
//                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--CCO参数初始化（01F2），等待--确认");
//                    p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "设置CCO生产序列号Sn（F0F41）回复确认");

                    emScriptRunState=Wait_QueryRouterSn_Finish;
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryRouterSn_F0F41);
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询CCO生产序列号Sn（F0F41），等待--回复");
                    p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                }
                else if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x02&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp&&p_Frame3762Base->info_field_.info_field_up.msg_seq==char(msgSeq-1))
                {
                    p_timer->stop();
                    if(snIndex!=0)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "设置CCO生产序列号Sn（F0F41）回复否认");
                        emScriptRunState=Wait_QueryRouterSn_Finish;
                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryRouterSn_F0F41);
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询CCO生产序列号Sn（F0F41），等待--回复");
                        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "设置CCO生产序列号Sn（F0F41）回复否认");
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "设置CCO生产序列号Sn（F0F41）回复否认");
                    }
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_ParaInit_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryRouterSn_F0F41);
                    emScriptRunState=Wait_QueryRouterSn_Finish;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询CCO生产序列号Sn（F0F41），等待--回复");
                    p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_QueryRouterSn_Finish:
            {
                if(p_Frame3762Base->afn_ == char(0xF0)&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x05&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    shared_ptr<AfnF0F41> p_QueryRouterSn_F0F41_Up=dynamic_pointer_cast<AfnF0F41>(p_Frame3762Base);

                    if(memcmp(p_QueryRouterSn_F0F41_Up->mac_address_.addr,p_CtrInfoList->at(0)->ccoAddr,6)==0
                            &&p_QueryRouterSn_F0F41_Up->device_type_==0x02)
                    {
                        if(dstSn==QString(p_QueryRouterSn_F0F41_Up->sn_content_.toHex()).toUpper())
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("路由生产序列号Sn为%1，符合要求").arg(QString(p_QueryRouterSn_F0F41_Up->sn_content_.toHex())));
                            snIndex++;
                            if(snIndex<snList.size())
                            {
                                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,snIndex,p_SetRouterSn_F0F42);
                                emScriptRunState=Wait_SetRouterSn_Finish;
                                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--设置CCO生产序列号Sn，等待--确认");
                                p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                            }
                            else
                            {
                                emScriptRunState=ScriptSuccess;
                                resultFlag=true;
                                p_AbstractScriptHost->updateProgress(ProcessState_Success, QString("CCO生产序列号Sn管理测试成功;"));
                            }
                        }
                        else
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("CCO生产序列号Sn为%1，不符合要求").arg(QString(p_QueryRouterSn_F0F41_Up->sn_content_.toHex())));
                        }
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("CCO生产序列号Sn为%1，不符合要求").arg(QString(p_QueryRouterSn_F0F41_Up->sn_content_.toHex())));
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
void Script_SnManage_CCO::processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID)
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
void Script_SnManage_CCO::processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID)
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
void Script_SnManage_CCO::sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame)
{
    if(frame==p_SetRouterSn_F0F42)
    {
        p_SetRouterSn_F0F42->ctrl_field_.dir=kDirDown;
        p_SetRouterSn_F0F42->ctrl_field_.prm=kActive;
        p_SetRouterSn_F0F42->ctrl_field_.comn_type=kHplc;

        p_SetRouterSn_F0F42->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_SetRouterSn_F0F42->info_field_.info_field_down.comu_rate=0;
        p_SetRouterSn_F0F42->info_field_.info_field_down.comu_module_ident=0;

        memcpy(p_SetRouterSn_F0F42->mac_address_.addr,p_CtrInfoList->at(0)->ccoAddr,6);
        p_SetRouterSn_F0F42->device_type_=0x02;
        p_SetRouterSn_F0F42->sn_content_=QByteArray::fromHex(snList.at(meterID).toLatin1());

        sendMsgOct=p_SetRouterSn_F0F42->EncodeFrame();
        sendMsgLog=QString("》》设置CCO生产序列号Sn F0F42：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
        /////
        dstSn=snList.at(0);
//        if(meterID==1)
//            dstSn=dstSn.right(100);
    }
    else if(frame==p_ParaInit_01F2)
    {
        p_ParaInit_01F2->ctrl_field_.dir=kDirDown;
        p_ParaInit_01F2->ctrl_field_.prm=kActive;
        p_ParaInit_01F2->ctrl_field_.comn_type=kHplc;

        p_ParaInit_01F2->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_ParaInit_01F2->info_field_.info_field_down.comu_rate=0;
        p_ParaInit_01F2->info_field_.info_field_down.comu_module_ident=0;

        sendMsgOct=p_ParaInit_01F2->EncodeFrame();
        sendMsgLog=QString("》》路由参数初始化01F2：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_QueryRouterSn_F0F41)
    {
        p_QueryRouterSn_F0F41->ctrl_field_.dir=kDirDown;
        p_QueryRouterSn_F0F41->ctrl_field_.prm=kActive;
        p_QueryRouterSn_F0F41->ctrl_field_.comn_type=kHplc;

        p_QueryRouterSn_F0F41->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_QueryRouterSn_F0F41->info_field_.info_field_down.comu_rate=0;
        p_QueryRouterSn_F0F41->info_field_.info_field_down.comu_module_ident=0;

        memcpy(p_QueryRouterSn_F0F41->mac_address_.addr,p_CtrInfoList->at(0)->ccoAddr,6);
        p_QueryRouterSn_F0F41->device_type_=0x02;

        sendMsgOct=p_QueryRouterSn_F0F41->EncodeFrame();
        sendMsgLog=QString("》》查询CCO生产序列号Sn F0F41：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
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
void Script_SnManage_CCO::sendSrcMsg(DvcType dvcType, int dvcId, QByteArray msg)
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

void Script_SnManage_CCO::timer_timeoutProc()
{
    switch(emScriptRunState)
    {
        case Wait_BuildNetFinish_Whole:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_BuildNetFinish_Whole timeout!!!");
            break;
        }
        case Wait_SetRouterSn_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_SetRouterSn_Finish timeout!!!");
            break;
        }
        case Wait_QueryRouterSn_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_QueryRouterSn_Finish timeout!!!");
            break;
        }
        default:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==p_timer");
            break;
        }
    }
}
void Script_SnManage_CCO::maxAllowTimer_timeoutProc()
{
    switch(emScriptRunState)
    {
        case Wait_SetRouterSn_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_SetRouterSn_Finish maxtimeout!!!");
            break;
        }
        case Wait_QueryRouterSn_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_QueryRouterSn_Finish maxtimeout!!!");
            break;
        }
        default:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==p_maxAllowTimer");
            break;
        }
    }
}
void Script_SnManage_CCO::delayTimer_timeoutProc()
{
    p_delayTimer->stop();
    index=0;
    p_maxAllowTimer->stop();
}

