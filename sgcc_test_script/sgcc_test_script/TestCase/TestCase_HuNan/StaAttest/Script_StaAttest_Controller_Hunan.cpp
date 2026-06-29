#include "Script_StaAttest_Controller_Hunan.h"

Script_StaAttest_Controller_Hunan::Script_StaAttest_Controller_Hunan(QObject *parent) : QObject(parent)
{
    emScriptRunState=TestInit;
    resultFlag=false;
    sendMsgOct.clear();

    p_CtrInfoList=make_shared<QList<shared_ptr<CtrInfo>>>();
    p_BuildNetwork_GW=new BuildNetwork_GW();

    p_Frame3762Helper=make_shared<Frame3762Helper>();
    p_TransparentTransmit_02F1=make_shared<Afn02F1>();
    p_ParameterInit_01F2=make_shared<Afn01F2>();

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
Script_StaAttest_Controller_Hunan::~Script_StaAttest_Controller_Hunan()
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
void Script_StaAttest_Controller_Hunan::execute()
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
    }
    else
    {
        emScriptRunState=Wait_SetStateDisable_02F1_Finish;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--设置STA认证状态禁止，等待--回复");
        sendMsg(ReadCtrlDvc,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_TransparentTransmit_02F1);
        p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
    }
    p_maxAllowTimer->start((timerForReachThresld+timerAfterReachThresld)*1000);
}
void Script_StaAttest_Controller_Hunan::stop()
{
    if(p_AbstractScriptHost==nullptr)
        return;
    p_AbstractScriptHost->updateProgress(ProcessState_Error, "Test stop!");
    if(needPowerOff==true)
        powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost);
}
void Script_StaAttest_Controller_Hunan::addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq)
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
void Script_StaAttest_Controller_Hunan::setHost(AbstractScriptHost *host)
{
    p_AbstractScriptHost=host;
    p_BuildNetwork_GW->setHost(host);
}
bool Script_StaAttest_Controller_Hunan::config(const QMap<QString,QString> *paraDic)
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
void Script_StaAttest_Controller_Hunan::processMsg(DvcType dvcType, int id, uchar *data, int datalen)
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

            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--路由参数初始化（01F2），等待--确认");
            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_ParameterInit_01F2);
            emScriptRunState=Wait_InitBeforeTest_Finish;
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
        else if(dvcType==ReadCtrlDvc)
        {
            p_CtrInfoList->at(0)->buf.append(recvTempData);
            processMsgFromCCO(dvcType,id);
        }
        else
        {
            return;
        }
    }
}
void Script_StaAttest_Controller_Hunan::processCtrlDvcRes(DvcType dvcType, QList<int> idList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params)
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
    case Wait_InitBeforeTest_Finish:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("路由断电"));
        QList<double> sendParams;
        p_AbstractScriptHost->controlDvc(CCO_GW,getDvcIdList(CCO_GW),CtrlCmd_PowerOff_12V,sendParams);
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

void Script_StaAttest_Controller_Hunan::processMsgFromCCO(DvcType dvcType, int dvcId)
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
        case Wait_InitBeforeTest_Finish:
        {
            if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "参数初始化收到确认，拉模块复位管脚，等待30s模块读表号");
                p_delayTimer->start(30*1000);

                QList<double> sendParams;
                p_AbstractScriptHost->controlDvc(SingleSTA,getDvcIdList(SingleSTA),CtrlCmd_ModuleRST,sendParams);
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
            case Wait_SetStateDisable_02F1_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x02&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    shared_ptr<Afn02F1> p_TransmitData_02F1_Up=dynamic_pointer_cast<Afn02F1>(p_Frame3762Base);
                    shared_ptr<SetParam_Up> p_SetParam_Up=SetParam_Up::decode_SetParam_Up(&p_TransmitData_02F1_Up->frame_content_);
                    if(p_SetParam_Up->paramSetRlstInfoList.at(0).setRes==0x00)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "STA认证禁止设置成功");
                        tryTimes=0;
                        emScriptRunState=Wait_QueryStateDisable_02F1_Finish;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询STA认证状态（02F1），等待--回复");
                        sendMsg(ReadCtrlDvc,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_TransparentTransmit_02F1);
                        p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "STA认证禁止设置失败");
                    }
                    break;
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_QueryStateDisable_02F1_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x02&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    shared_ptr<Afn02F1> p_TransmitData_02F1_Up=dynamic_pointer_cast<Afn02F1>(p_Frame3762Base);
                    shared_ptr<ChkParam_Up> p_ChkParam_Up=ChkParam_Up::decode_ChkParam_Up(&p_TransmitData_02F1_Up->frame_content_);
                    if(p_ChkParam_Up->paramInfoList.at(0).id==46&&p_ChkParam_Up->paramInfoList.at(0).idCntnt.at(0)==0x00)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "STA认证状态为禁止，符合要求");
                        tryTimes=0;
                        emScriptRunState=Wait_SetStateEnable_02F1_Finish;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--设置STA认证状态开启，等待--回复");
                        sendMsg(ReadCtrlDvc,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_TransparentTransmit_02F1);
                        p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "STA认证-STA状态为开启，不符合要求");
                    }
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_SetStateEnable_02F1_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x02&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    shared_ptr<Afn02F1> p_TransmitData_02F1_Up=dynamic_pointer_cast<Afn02F1>(p_Frame3762Base);
                    shared_ptr<SetParam_Up> p_SetParam_Up=SetParam_Up::decode_SetParam_Up(&p_TransmitData_02F1_Up->frame_content_);
                    if(p_SetParam_Up->paramSetRlstInfoList.at(0).setRes==0x00)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "STA认证开启设置成功");
                        tryTimes=0;
                        emScriptRunState=Wait_QueryStateEnable_02F1_Finish;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询STA认证状态（02F1），等待--回复");
                        sendMsg(ReadCtrlDvc,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_TransparentTransmit_02F1);
                        p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "STA认证开启设置失败");
                    }
                    break;
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_QueryStateEnable_02F1_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x02&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    shared_ptr<Afn02F1> p_TransmitData_02F1_Up=dynamic_pointer_cast<Afn02F1>(p_Frame3762Base);
                    shared_ptr<ChkParam_Up> p_ChkParam_Up=ChkParam_Up::decode_ChkParam_Up(&p_TransmitData_02F1_Up->frame_content_);
                    if(p_ChkParam_Up->paramInfoList.at(0).id==46&&p_ChkParam_Up->paramInfoList.at(0).idCntnt.at(0)==0x01)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "STA认证状态为开启，符合要求");
                        emScriptRunState=ScriptSuccess;
                        p_AbstractScriptHost->updateProgress(ProcessState_Success, "STA认证状态-抄控器测试成功");
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "STA认证状态为禁止，不符合要求");
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
void Script_StaAttest_Controller_Hunan::processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID)
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
void Script_StaAttest_Controller_Hunan::processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID)
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
void Script_StaAttest_Controller_Hunan::sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame)
{
    if(frame==p_TransparentTransmit_02F1)
    {
        p_TransparentTransmit_02F1->ctrl_field_.dir=kDirDown;
        p_TransparentTransmit_02F1->ctrl_field_.prm=kActive;
        p_TransparentTransmit_02F1->ctrl_field_.comn_type=kHplc;

        p_TransparentTransmit_02F1->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_TransparentTransmit_02F1->info_field_.info_field_down.comu_rate=0;
        p_TransparentTransmit_02F1->info_field_.info_field_down.comu_module_ident=1;


        p_TransparentTransmit_02F1->protocol_type_=0x04;

        char tmpCcoAddr[6];
        char tmpStaAddr[6];
        memcpy(tmpCcoAddr,p_CtrInfoList->at(0)->ccoAddr,6);
        if(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().size()>=3)
            memcpy(tmpStaAddr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(2)->mtrAddr,6);
        else
            memcpy(tmpStaAddr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(0)->mtrAddr,6);

        if(emScriptRunState==Wait_QueryStateEnable_02F1_Finish||emScriptRunState==Wait_QueryStateDisable_02F1_Finish)
        {
            QByteArray msg;
            shared_ptr<ChkParam_Down> p_ChkParam_Down=make_shared<ChkParam_Down>();
            QList<ushort> chkParamIdList;

            chkParamIdList.append(46);
            p_ChkParam_Down->msgSeq=msgSeq;
            memcpy(p_ChkParam_Down->srcAddr,reverseArray(QByteArray(tmpCcoAddr,6)),6);
            memcpy(p_ChkParam_Down->dstAddr,reverseArray(QByteArray(tmpStaAddr,6)),6);
            p_ChkParam_Down->idList=chkParamIdList;
            p_ChkParam_Down->idCnt=ushort(chkParamIdList.size());

            msg=ChkParam_Down::encode_ChkParam_Down(p_ChkParam_Down);
            p_TransparentTransmit_02F1->frame_content_=msg;
        }
        else if(emScriptRunState==Wait_SetStateEnable_02F1_Finish||emScriptRunState==Wait_SetStateDisable_02F1_Finish)
        {
            QByteArray msg;
            shared_ptr<SetParam_Down> p_SetParam_Down=make_shared<SetParam_Down>();
            QList<ParamInfo> setParamInfoList;
            ParamInfo tmpParamInfo;
            tmpParamInfo.id=46;
            tmpParamInfo.idLen=1;
            if(emScriptRunState==Wait_SetStateEnable_02F1_Finish)
                tmpParamInfo.idCntnt.append(0x01);
            else if(emScriptRunState==Wait_SetStateDisable_02F1_Finish)
                tmpParamInfo.idCntnt.append(char(0x00));
            setParamInfoList.append(tmpParamInfo);


            p_SetParam_Down->msgSeq=msgSeq;
            memcpy(p_SetParam_Down->srcAddr,reverseArray(QByteArray(tmpCcoAddr,6)),6);
            memcpy(p_SetParam_Down->dstAddr,reverseArray(QByteArray(tmpStaAddr,6)),6);
            p_SetParam_Down->paramInfoList=setParamInfoList;
            p_SetParam_Down->idCnt=ushort(setParamInfoList.size());

            msg=SetParam_Down::encode_SetParam_Down(p_SetParam_Down);
            p_TransparentTransmit_02F1->frame_content_=msg;
        }

        p_TransparentTransmit_02F1->frame_length_=uchar(p_TransparentTransmit_02F1->frame_content_.size());

        memcpy(p_TransparentTransmit_02F1->address_field_.src_addr,tmpCcoAddr,6);
        memcpy(p_TransparentTransmit_02F1->address_field_.dst_addr,tmpStaAddr,6);

        sendMsgOct=p_TransparentTransmit_02F1->EncodeFrame();
        sendMsgLog=QString("》》转发数据02F1：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
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
void Script_StaAttest_Controller_Hunan::sendSrcMsg(DvcType dvcType, int dvcId, QByteArray msg)
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

void Script_StaAttest_Controller_Hunan::timer_timeoutProc()
{
    switch(emScriptRunState)
    {
    case Wait_BuildNetFinish_Whole:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_BuildNetFinish_Whole timeout!!!");
        break;
    }
    case Wait_SetStateDisable_02F1_Finish:
    case Wait_QueryStateDisable_02F1_Finish:
    case Wait_SetStateEnable_02F1_Finish:
    case Wait_QueryStateEnable_02F1_Finish:
    {
        if(++tryTimes<3)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("超时未回复，重发--转发数据（02F1），等待--回复"));
            sendMsg(ReadCtrlDvc,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_TransparentTransmit_02F1);
            p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
        }
        else
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("抄控器发送3次，超时未回复"));
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
void Script_StaAttest_Controller_Hunan::maxAllowTimer_timeoutProc()
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
void Script_StaAttest_Controller_Hunan::delayTimer_timeoutProc()
{
    p_delayTimer->stop();
    tryTimes=0;
    emScriptRunState=Wait_SetStateDisable_02F1_Finish;
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--设置STA认证状态禁止，等待--回复");
    sendMsg(ReadCtrlDvc,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_TransparentTransmit_02F1);
    p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
}

QList<int> Script_StaAttest_Controller_Hunan::getDvcIdList(DvcType dvcType)
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
    case CCO_GW:
    {
        dvcIdList.append(p_CtrInfoList->at(0)->dvcId);
        break;
    }
    default:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Error, "dvcType error:getDvcIdList()");
        break;
    }
    }
    if(dvcIdList.size()==0)
    {
        dvcIdList.append(p_CtrInfoList->at(0)->dvcId);
    }
    return dvcIdList;
}
