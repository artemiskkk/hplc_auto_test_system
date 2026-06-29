#include "Script_VersionHead_QueryRecordCco.h"

Script_VersionHead_QueryRecordCco::Script_VersionHead_QueryRecordCco(QObject *parent) : QObject(parent)
{
    emScriptRunState=TestInit;
    resultFlag=false;
    sendMsgOct.clear();

    p_CtrInfoList=make_shared<QList<shared_ptr<CtrInfo>>>();
    p_BuildNetwork_GW=new BuildNetwork_GW();

    p_Frame3762Helper=make_shared<Frame3762Helper>();
    p_Confirm_00F1=make_shared<Afn00F1>();

    p_QueryRecordF0F7_Down=make_shared<AfnF0F7>();
    p_HardInit_01F1_Down=make_shared<Afn01F1>();
    p_ParaInit_01F2_Down=make_shared<Afn01F2>();

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
Script_VersionHead_QueryRecordCco::~Script_VersionHead_QueryRecordCco()
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
void Script_VersionHead_QueryRecordCco::execute()
{
    p_AbstractScriptHost->updateProgress(ProcessState_Start, "Test start!");

    if(needBuildNet==true)
    {
        p_BuildNetwork_GW->execute();
        emScriptRunState=Wait_BuildNetFinish_Whole;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "等待全网组网完成");
        p_timer->start((timerForReachThresld+timerAfterReachThresld)*1000);
    }
    p_maxAllowTimer->start((timerForReachThresld+timerAfterReachThresld)*1000);
}
void Script_VersionHead_QueryRecordCco::stop()
{
    if(p_AbstractScriptHost==nullptr)
        return;
    p_AbstractScriptHost->updateProgress(ProcessState_Error, "Test stop!");
    if(needPowerOff==true)
        powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost);
}
void Script_VersionHead_QueryRecordCco::addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq)
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
void Script_VersionHead_QueryRecordCco::setHost(AbstractScriptHost *host)
{
    p_AbstractScriptHost=host;
    p_BuildNetwork_GW->setHost(host);
}
bool Script_VersionHead_QueryRecordCco::config(const QMap<QString,QString> *paraDic)
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
void Script_VersionHead_QueryRecordCco::processMsg(DvcType dvcType, int id, uchar *data, int datalen)
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

            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "读路由初始工厂参数配置记录");
            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryRecordF0F7_Down);
            emScriptRunState = ReadRecordBefore;
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
void Script_VersionHead_QueryRecordCco::processCtrlDvcRes(DvcType dvcType, QList<int> idList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params)
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
    case ScriptSuccess:
    {
        break;
    }
    default:
        break;
    }
}

void Script_VersionHead_QueryRecordCco::processMsgFromCCO(DvcType dvcType, int dvcId)
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
        case ReadRecordBefore:
        {
            dtValue3762=get3762Dt(p_Frame3762Base->dt1_,p_Frame3762Base->dt2_);
            if(p_Frame3762Base->afn_==char(0xf0) && dtValue3762==7 && p_Frame3762Base->ctrl_field_.dir==kDirUp)
            {
                p_timer->stop();
                shared_ptr<AfnF0F7> p_QueryVersionHeadF0F7_Up=dynamic_pointer_cast<AfnF0F7>(p_Frame3762Base);

                ccoRecord_init = p_QueryVersionHeadF0F7_Up->data_unit_up_.parameter_content_;
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "路由配置记录：\n版本头"+ccoRecord_init.left(40).toHex()
                             +"\n版本切换日期"+ccoRecord_init.mid(40,8).toHex()+"\nMAC地址"+ccoRecord_init.mid(48,6).toHex()+"\n记录次数"+ccoRecord_init.mid(54,1).toHex());

                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_ParaInit_01F2_Down);
                p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "路由参数初始化");
            }
            else if(p_Frame3762Base->afn_==0x00 && dtValue3762==1 && p_Frame3762Base->ctrl_field_.dir==kDirUp)
            {
                p_timer->stop();
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "路由参数初始化回复确认，再读路由工厂参数配置记录");
                p_delayTimer->start(1*1000);
                emScriptRunState = ReadRecordAfterParainit;
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case ReadRecordAfterParainit:
        {
            dtValue3762=get3762Dt(p_Frame3762Base->dt1_,p_Frame3762Base->dt2_);
            if(p_Frame3762Base->afn_==char(0xf0) && dtValue3762==7 && p_Frame3762Base->ctrl_field_.dir==kDirUp)
            {
                p_timer->stop();
                shared_ptr<AfnF0F7> p_QueryVersionHeadF0F7_Up=dynamic_pointer_cast<AfnF0F7>(p_Frame3762Base);

                QByteArray ccoRecord = p_QueryVersionHeadF0F7_Up->data_unit_up_.parameter_content_;
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "路由配置记录：\n版本头"+ccoRecord_init.left(40).toHex()
                             +"\n版本切换日期"+ccoRecord_init.mid(40,8).toHex()+"\nMAC地址"+ccoRecord_init.mid(48,6).toHex()+"\n记录次数"+ccoRecord_init.mid(54,1).toHex());
                if(ccoRecord == ccoRecord_init)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "参数初始化后路由工厂参数配置记录未改变，路由硬件初始化");
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_HardInit_01F1_Down);
                    p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, "参数初始化后路由工厂参数配置记录发生改变");
                }
            }
            else if(p_Frame3762Base->afn_==0x00 && dtValue3762==1 && p_Frame3762Base->ctrl_field_.dir==kDirUp)
            {
                p_timer->stop();
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "路由硬件初始化回复确认，等待03F10上报");
                p_delayTimer->start(10*1000);
                emScriptRunState = ReadRecordAfterHardinit;
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case ReadRecordAfterHardinit:
        {
            dtValue3762=get3762Dt(p_Frame3762Base->dt1_,p_Frame3762Base->dt2_);
            if(p_Frame3762Base->afn_==0x03 && dtValue3762==10 && p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_delayTimer->stop();
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "路由硬件初始化后收到03F10运行模式信息上报，再读路由工厂参数配置记录");
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryRecordF0F7_Down);
                p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
            }
            else if(p_Frame3762Base->afn_==char(0xf0) && dtValue3762==7 && p_Frame3762Base->ctrl_field_.dir==kDirUp)
            {
                p_timer->stop();
                shared_ptr<AfnF0F7> p_QueryVersionHeadF0F7_Up=dynamic_pointer_cast<AfnF0F7>(p_Frame3762Base);

                QByteArray ccoRecord = p_QueryVersionHeadF0F7_Up->data_unit_up_.parameter_content_;
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "路由配置记录：\n版本头"+ccoRecord_init.left(40).toHex()
                             +"\n版本切换日期"+ccoRecord_init.mid(40,8).toHex()+"\nMAC地址"+ccoRecord_init.mid(48,6).toHex()+"\n记录次数"+ccoRecord_init.mid(54,1).toHex());
                if(ccoRecord == ccoRecord_init)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Success, "硬件初始化后路由工厂参数配置记录未改变");
                    emScriptRunState = ScriptSuccess;
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, "硬件初始化后路由工厂参数配置记录发生改变");
                }
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        default:
            break;
        }
    }
}
void Script_VersionHead_QueryRecordCco::processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID)
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
void Script_VersionHead_QueryRecordCco::processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID)
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
void Script_VersionHead_QueryRecordCco::sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame)
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
    else if(frame==p_QueryRecordF0F7_Down)
    {
        p_QueryRecordF0F7_Down->ctrl_field_.dir=kDirDown;
        p_QueryRecordF0F7_Down->ctrl_field_.prm=kActive;
        p_QueryRecordF0F7_Down->ctrl_field_.comn_type=kHplc;

        p_QueryRecordF0F7_Down->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_QueryRecordF0F7_Down->info_field_.info_field_down.comu_rate=0;
        p_QueryRecordF0F7_Down->info_field_.info_field_down.comu_module_ident=0;

        p_QueryRecordF0F7_Down->data_unit_down_.parameter_id_[0]=0x02;
        p_QueryRecordF0F7_Down->data_unit_down_.parameter_id_[1]=0x5e;

        sendMsgOct=p_QueryRecordF0F7_Down->EncodeFrame();
        sendMsgLog=QString("》》查询工厂参数配置记录F0F7：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_HardInit_01F1_Down)
    {
        p_HardInit_01F1_Down->ctrl_field_.dir=kDirDown;
        p_HardInit_01F1_Down->ctrl_field_.prm=kActive;
        p_HardInit_01F1_Down->ctrl_field_.comn_type=kHplc;

        p_HardInit_01F1_Down->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_HardInit_01F1_Down->info_field_.info_field_down.comu_rate=0;
        p_HardInit_01F1_Down->info_field_.info_field_down.comu_module_ident=0;

        sendMsgOct=p_HardInit_01F1_Down->EncodeFrame();
        sendMsgLog=QString("》》路由硬件初始化01F1：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_ParaInit_01F2_Down)
    {
        p_ParaInit_01F2_Down->ctrl_field_.dir=kDirDown;
        p_ParaInit_01F2_Down->ctrl_field_.prm=kActive;
        p_ParaInit_01F2_Down->ctrl_field_.comn_type=kHplc;

        p_ParaInit_01F2_Down->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_ParaInit_01F2_Down->info_field_.info_field_down.comu_rate=0;
        p_ParaInit_01F2_Down->info_field_.info_field_down.comu_module_ident=0;

        sendMsgOct=p_ParaInit_01F2_Down->EncodeFrame();
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
void Script_VersionHead_QueryRecordCco::sendSrcMsg(DvcType dvcType, int dvcId, QByteArray msg)
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

void Script_VersionHead_QueryRecordCco::timer_timeoutProc()
{
    p_timer->stop();
    switch(emScriptRunState)
    {
    default:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Error, "超时未回复");
        break;
    }
    }
}
void Script_VersionHead_QueryRecordCco::maxAllowTimer_timeoutProc()
{
    p_maxAllowTimer->stop();
    switch(emScriptRunState)
    {
//    case Wait_EventReport_Finish:
//    {
//        if(receiveEventFlag==true)
//        {
//            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "组网到达最大等待时间，普通电表事件汇总：\n"+eventMsgList);
//            p_AbstractScriptHost->updateProgress(ProcessState_Success, "收到电表事件上报");
//        }
//        else
//        {
//            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "组网到达最大等待时间，未收到电表事件上报");
//        }
//        break;
//    }
    default:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==p_maxAllowTimer");
        break;
    }
    }
}
void Script_VersionHead_QueryRecordCco::delayTimer_timeoutProc()
{
    p_delayTimer->stop();
    if(emScriptRunState == ReadRecordAfterParainit)
    {
        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryRecordF0F7_Down);
        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
    }
    else if(emScriptRunState == ReadRecordAfterHardinit)
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "路由硬件初始化后10s内未收到03F10上报");
    }
    else
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine error:p_delayTimer");
    }
}

void Script_VersionHead_QueryRecordCco::calPowerOnReportRate()
{
    QString powerOnFailMeter;
    QString powerOffFailMeter;
    QString otherEventReportMeter;
    struct MeterStruct
    {
        Address meterNo;
        bool powerOnReportFlag=false;
        bool powerOffReportFlag=false;
    };
    QList<MeterStruct> allMeterList;
    for(int i=0;i<p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().size();i++)
    {
        if(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(i)->slotPosition==SingleSTA||p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(i)->slotPosition==ThreeSTA)
            continue;
        MeterStruct meter;
        memcpy(meter.meterNo.addr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(i)->mtrAddr,6);
        allMeterList.append(meter);
    }
    for(int i=0;i<allMeterList.size();i++)
    {
        for(int j=0;j<powerOnReportList.size();j++)
        {
            if(allMeterList.at(i).meterNo==powerOnReportList.at(j).reportNodeAddress)
            {
                allMeterList[i].powerOnReportFlag=true;
            }
        }
        for(int j=0;j<powerOffReportList.size();j++)
        {
            if(allMeterList.at(i).meterNo==powerOffReportList.at(j).reportNodeAddress)
            {
                allMeterList[i].powerOffReportFlag=true;
            }
        }
    }
    for(int i=0;i<otherEventReportList.size();i++)
    {
        otherEventReportMeter.append(QString(QByteArray(otherEventReportList.at(i).reportNodeAddress.addr,6).toHex())+";");
    }
    for(int i=0;i<allMeterList.size();i++)
    {
        if(allMeterList.at(i).powerOnReportFlag==false)
            powerOnFailMeter.append(QString(QByteArray(allMeterList.at(i).meterNo.addr,6).toHex())+";");
        if(allMeterList.at(i).powerOffReportFlag==false)
            powerOffFailMeter.append(QString(QByteArray(allMeterList.at(i).meterNo.addr,6).toHex())+";");
    }
    if(!otherEventReportMeter.isEmpty())
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("上报其它事件的电表如下：\n%1").arg(powerOffFailMeter));
    }
    if(powerOnFailMeter.isEmpty()==true&&powerOffFailMeter.isEmpty()==true)
    {
        emScriptRunState=ScriptSuccess;
        resultFlag=true;
        p_AbstractScriptHost->updateProgress(ProcessState_Success, QString("停复电事件上报测试成功！"));
    }
    else if(powerOnFailMeter.isEmpty()!=true&&powerOffFailMeter.isEmpty()!=true)
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("停电事件上报不全！\n失败电表如下：\n%1").arg(powerOffFailMeter));
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("上电事件上报不全！\n失败电表如下：\n%1").arg(powerOnFailMeter));
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("停复电事件均上报不全！"));
    }
    else if(powerOnFailMeter.isEmpty()!=true&&powerOffFailMeter.isEmpty()==true)
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("上电事件上报不全！\n失败电表如下：\n%1").arg(powerOnFailMeter));
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("复电事件上报不全！"));
    }
    else if(powerOnFailMeter.isEmpty()==true&&powerOffFailMeter.isEmpty()!=true)
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("停电事件上报不全！\n失败电表如下：\n%1").arg(powerOffFailMeter));
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("停电事件上报不全！"));
    }
}
