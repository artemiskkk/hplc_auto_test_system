#include "Script_CcoParaInit_RouteUpgrade.h"
#include <QApplication>

Script_CcoParaInit_RouteUpgrade::Script_CcoParaInit_RouteUpgrade(QObject *parent) : QObject(parent)
{
    p_BuildNetwork_GW=new BuildNetwork_GW();
    p_CtrInfoList=make_shared<QList<shared_ptr<CtrInfo>>>();
    p_Frame3762Helper=make_shared<Frame3762Helper>();
    p_Frame645Helper=make_shared<Frame645Helper>();
    p_MeterAddrResp_93=make_shared<RspsNormal_ReadAddr_0x93>(addr,6);
    p_FrameOOPHelper=make_shared<FrameOOPHelper>();
    p_GetResponseNormal_ReadAddr=make_shared<GetResponseNormal>();
    p_ParameterInit_01F2=make_shared<Afn01F2>();

    p_timer=new QTimer(this);
    p_maxAllowTimer=new QTimer(this);
    p_delayTimer=new QTimer(this);
    connect(p_timer,SIGNAL(timeout()),this,SLOT(timer_timeoutProc()));
    connect(p_maxAllowTimer,SIGNAL(timeout()),this,SLOT(maxAllowTimer_timeoutProc()));
    connect(p_delayTimer,SIGNAL(timeout()),this,SLOT(delayTimer_timeoutProc()));
}
Script_CcoParaInit_RouteUpgrade::~Script_CcoParaInit_RouteUpgrade()
{
    if(p_FileTransfer!=nullptr)
        delete p_FileTransfer;
    if(p_QueryVersion!=nullptr)
        delete p_QueryVersion;
    p_BuildNetwork_GW->initBuildNetWork();
    if(needPowerOff==true)//断电处理
        powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost);
}
void Script_CcoParaInit_RouteUpgrade::setHost(AbstractScriptHost *host)
{
    p_AbstractScriptHost=host;
    p_BuildNetwork_GW->setHost(host);
}
void Script_CcoParaInit_RouteUpgrade::addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq)
{
    p_CtrInfoList->clear();
    comnAddAddrsInfo(p_CtrInfoList, p_ConcentratorInfoList, p_MeterInfoList, p_SchemeCfgInfoList);
    uchar dstFreq=freq&0x0f;

    uchar dstPrtcl=uchar(freq)>>4;
    if(dstPrtcl==0x03)
        setMeterAddrsPrtcl(p_CtrInfoList,dstPrtcl);

    p_BuildNetwork_GW->setCtrInfoListAndFreq(p_CtrInfoList, dstFreq);

}
bool Script_CcoParaInit_RouteUpgrade::config(const QMap<QString,QString> *paraDic)
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
void Script_CcoParaInit_RouteUpgrade::execute()
{
    p_AbstractScriptHost->updateProgress(ProcessState_Start, "Test start!");
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "脚本执行流程描述打印：-----whc(2024年1月20)\r\n"
                                                                  "1.组网通用流程，SENCE2(需要配置CCO，但不需要组网) Wait_BuildNetFinish_Whole\n"
                                                                  "2.升级前查询内外部版本 Wait_QueryVersion_Finish\n"
                                                                  "3.文件传输到只剩一段 Wait_FileTransfer_15F1_Finish\n"
                                                                  "4.参数初始化路由  Wait_ParameterInit_01F2_Finish\n"
                                                                  "5.查询内外部版本 Wait_QueryVersion_Finish\n");
    if(needBuildNet==true)//场景2-4
    {
#ifdef SENCE4
        p_BuildNetwork_GW->needRebuildNetwork=true;
#endif
        p_BuildNetwork_GW->execute();//执行组网通用脚本
        emScriptRunState=Wait_BuildNetFinish_Whole;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "等待全网组网完成");
    }
    else//场景1
    {
        //        tryTimes=0;
        //        index=0;
        ////        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,index,make_shared<void>());//make_shared<void>()应该替换成实际的376.2命令
        //        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--命令，等待--回复");
        //        emScriptRunState=Wait;
        //        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
    }
    p_maxAllowTimer->start((timerForReachThresld+timerAfterReachThresld)*1000);//此处要启动脚本的最大执行时间定时器
}
void Script_CcoParaInit_RouteUpgrade::stop()
{
    p_timer->stop();
    p_delayTimer->stop();
    if(p_maxAllowTimer != nullptr) {
        p_maxAllowTimer->stop();
    }
    if(p_AbstractScriptHost==nullptr)
        return;
    p_AbstractScriptHost->updateProgress(ProcessState_Error, "Test stop!");
    if(needPowerOff==true)
        powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost);
}
void Script_CcoParaInit_RouteUpgrade::processMsg(DvcType dvcType, int id, uchar *data, int datalen)
{
    if(p_CtrInfoList->size()==0)
        return;
    if(emScriptRunState==Wait_BuildNetFinish_Whole)//场景2-4
    {
#ifdef SENCE2
        if(!p_BuildNetwork_GW->startBuildNetFlag)//场景2 关注p_BuildNetwork_GW->startBuildNetFlag标志,开始组网标志
            p_BuildNetwork_GW->processMsg(dvcType,id,data,datalen);
#elif defined(SENCE3)||defined(SENCE4)
        if(!p_BuildNetwork_GW->buildNetworkResultFlag)//场景3、4 关注p_BuildNetwork_GW->buildNetworkResultFlag标志，组网完成标志
            p_BuildNetwork_GW->processMsg(dvcType,id,data,datalen);
#elif defined SENCE1
        if(true){}
#endif
        else
        {
            p_BuildNetwork_GW->initBuildNetWork();
            tryTimes=0;
            index=0;
            emScriptRunState=Wait_QueryVersion_Before_Finish;
            p_QueryVersion=new QueryVersion(p_CtrInfoList,p_AbstractScriptHost);
            connect(p_QueryVersion,&QueryVersion::signalNoticeQueryVersionState,this,&Script_CcoParaInit_RouteUpgrade::slotNoticeQueryVersionState);
            connect(p_QueryVersion,&QueryVersion::signalNoticeQueryVersionResult,this,&Script_CcoParaInit_RouteUpgrade::slotNoticeQueryVersionResult);
            p_QueryVersion->execute();
        }
    }
    else if(emScriptRunState==Wait_QueryVersion_Finish||emScriptRunState==Wait_QueryVersion_Before_Finish)
    {
        p_QueryVersion->processMsg(dvcType,id,data,datalen);
    }
    else if(emScriptRunState==Wait_FileTransfer_15F1_Finish)
    {
        p_FileTransfer->processMsg(dvcType,id,data,datalen);
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
void Script_CcoParaInit_RouteUpgrade::processCtrlDvcRes(DvcType dvcType, QList<int> idList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params)
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
    case Wait_QueryVersion_Before_Finish:
    case Wait_QueryVersion_Finish:
    {
        p_QueryVersion->processCtrlDvcRes(dvcType,idList,ctrlCmdType,isSucs,params);
        break;
    }
    case Wait_FileTransfer_15F1_Finish:
    {
        p_FileTransfer->processCtrlDvcRes(dvcType,idList,ctrlCmdType,isSucs,params);
        break;
    }
    default:
    {
        break;
    }
    }
}

void Script_CcoParaInit_RouteUpgrade::processMsgFromCCO(DvcType dvcType, int dvcId)
{
    if(dvcId!=p_CtrInfoList->at(0)->ctrlID)
        return;
    bool haveCompleteMsg=true;
    while(haveCompleteMsg)
    {
        shared_ptr<Frame3762Base> p_Frame3762Base = p_Frame3762Helper->DecodeLocalMsg(&(p_CtrInfoList->at(0)->buf),haveCompleteMsg);
        if(p_Frame3762Base==nullptr)
        {
            continue;
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
        case Wait_QueryVersion_Before_Finish:
        {
            break;
        }
        case Wait_FileTransfer_15F1_Finish:
        {
            break;
        }
        case Wait_ParameterInit_01F2_Finish:
        {
            if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp&&uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq)==0)
            {
                p_timer->stop();
                QString state=QString("%1,CCO参数初始化收到确认").arg(metaEnum.valueToKey(emScriptRunState));
                p_AbstractScriptHost->updateProgress(ProcessState_Processing,state);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing,"等待10s，开始查询版本");
                FileTransfer::delay(CCO_CMD_TIMEOUT_TIME*1000);
                emScriptRunState=Wait_QueryVersion_Finish;
                delete p_QueryVersion;
                p_QueryVersion=nullptr;
                p_QueryVersion=new QueryVersion(p_CtrInfoList,p_AbstractScriptHost,parameter.baud);
                connect(p_QueryVersion,&QueryVersion::signalNoticeQueryVersionState,this,&Script_CcoParaInit_RouteUpgrade::slotNoticeQueryVersionState);
                connect(p_QueryVersion,&QueryVersion::signalNoticeQueryVersionResult,this,&Script_CcoParaInit_RouteUpgrade::slotNoticeQueryVersionResult);
                p_QueryVersion->execute();
            }
            else if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x02&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp&&uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq)==0)
            {
                p_timer->stop();
                QString state=QString("%1,CCO参数初始化回复异常，实际回复否认，期望回复确认").arg(metaEnum.valueToKey(emScriptRunState));
                p_AbstractScriptHost->updateProgress(ProcessState_Processing,state);
                p_AbstractScriptHost->updateProgress(ProcessState_Failed,state+testCase);
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case Wait_QueryVersion_Finish:
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

void Script_CcoParaInit_RouteUpgrade::slotNoticeQueryVersionState(ProcessState pState, const QString &state)
{
    switch(emScriptRunState)
    {
    case Wait_QueryVersion_Before_Finish:
    {
        p_AbstractScriptHost->updateProgress(pState,QString(metaEnum.valueToKey(emScriptRunState))+"查询版本异常，"+state+testCase);
        break;
    }
    case Wait_QueryVersion_Finish:
    {
        p_AbstractScriptHost->updateProgress(pState,QString(metaEnum.valueToKey(emScriptRunState))+"查询版本异常，"+state+testCase);
        break;
    }
    default:
    {
        break;
    }
    }
}
void Script_CcoParaInit_RouteUpgrade::slotNoticeQueryVersionResult(bool isCCO, QMap<QString, QString> versionInfo)
{
    switch(emScriptRunState)
    {
    case Wait_QueryVersion_Before_Finish:
    {
        if(isCCO)
        {
            outVersionBefore=versionInfo.value("OutVer");
            innerVersionBefore=versionInfo.value("InnerVer");
            p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString(metaEnum.valueToKey(emScriptRunState))+"升级前外部版本"+outVersionBefore+"，内部版本"+innerVersionBefore);

            emScriptRunState=Wait_FileTransfer_15F1_Finish;
            p_FileTransfer=new FileTransfer(p_CtrInfoList,p_AbstractScriptHost);
            connect(p_FileTransfer,&FileTransfer::signalNoticeFileTransState,this,&Script_CcoParaInit_RouteUpgrade::slotNoticeFileTransState);
            ST_FileTransferParameter para;
            para.upgradeFile=selectUpgradeFile();
            if(!QFile(para.upgradeFile).exists())
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString(metaEnum.valueToKey(emScriptRunState))+para.upgradeFile);
                p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString(metaEnum.valueToKey(emScriptRunState))+para.upgradeFile+testCase);
                return;
            }
            para.isSpecial=true;
            para.missFileNo=ushort(QFile(para.upgradeFile).size()/para.len)+(QFile(para.upgradeFile).size()%para.len==0?0:1)-1;
            p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString(metaEnum.valueToKey(emScriptRunState))+para.upgradeFile);
            p_FileTransfer->setFileTransferParameter(para);
            this->parameter=para;
            p_FileTransfer->execute();
        }
        break;
    }
    case Wait_QueryVersion_Finish:
    {
        QString outVersion=versionInfo.value("OutVer");
        QString innerVersion=versionInfo.value("InnerVer");
        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString(metaEnum.valueToKey(emScriptRunState))+"升级前外部版本"+outVersionBefore+"，内部版本"+innerVersionBefore+"，升级后外部版本"+outVersion+"，内部版本"+innerVersion);
        if(outVersion==outVersionBefore&&innerVersion==innerVersionBefore)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString(metaEnum.valueToKey(emScriptRunState))+"升级过程CCO参数初始化版本不变，符合要求"+testCase);
            if(p_maxAllowTimer != nullptr) p_maxAllowTimer->stop();
            emScriptRunState = ScriptSuccess;
            p_AbstractScriptHost->updateProgress(ProcessState_Success,QString("脚本执行成功，"+testCase));
        }
        else
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString(metaEnum.valueToKey(emScriptRunState))+"升级过程CCO参数初始化版本改变，不符合要求"+testCase);
            p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString(metaEnum.valueToKey(emScriptRunState))+"升级过程CCO参数初始化版本改变，不符合要求"+testCase);
        }
        break;
    }
    default:
    {
        break;
    }
    }
}
void Script_CcoParaInit_RouteUpgrade::slotNoticeFileTransState(ProcessState pState, const QString &state)
{
    switch(emScriptRunState)
    {
    case Wait_FileTransfer_15F1_Finish:
    {
        if(pState==ProcessState_Processing)
        {
            p_AbstractScriptHost->updateProgress(pState,QString(metaEnum.valueToKey(emScriptRunState))+"文件传输流程，"+state);
            emScriptRunState=Wait_ParameterInit_01F2_Finish;
            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_ParameterInit_01F2);
            p_AbstractScriptHost->updateProgress(pState, "发送--CCO参数初始化（01F2），等待--确认");
            p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
        }
        else
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString(metaEnum.valueToKey(emScriptRunState))+"文件传输流程异常，"+state);
            p_AbstractScriptHost->updateProgress(pState,QString(metaEnum.valueToKey(emScriptRunState))+"文件传输流程异常，"+state+testCase);
        }

        break;
    }
    default:
    {
        break;
    }
    }
}

void Script_CcoParaInit_RouteUpgrade::processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID)
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
void Script_CcoParaInit_RouteUpgrade::processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID)
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
void Script_CcoParaInit_RouteUpgrade::sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame)
{
    if(frame==p_ParameterInit_01F2)
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
void Script_CcoParaInit_RouteUpgrade::sendSrcMsg(DvcType dvcType, int dvcId, QByteArray msg)
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

void Script_CcoParaInit_RouteUpgrade::timer_timeoutProc()
{
    switch(emScriptRunState)
    {
    case Wait_BuildNetFinish_Whole:
    {
        p_CtrInfoList->at(0)->inNetResult=false;
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("全网组网成功率：%1%").arg(p_CtrInfoList->at(0)->inNetSuccessRate*100));
        break;
    }
    case Wait_ParameterInit_01F2_Finish:
    {
        p_timer->stop();
        if(++tryTimes>=3)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString(metaEnum.valueToKey(emScriptRunState))+"硬件初始化后回复超时");
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString(metaEnum.valueToKey(emScriptRunState))+"硬件初始化后回复超时"+testCase);
        }
        else
        {
            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_ParameterInit_01F2);
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--CCO参数初始化（01F2），等待--确认");
            p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
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
void Script_CcoParaInit_RouteUpgrade::maxAllowTimer_timeoutProc()
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
void Script_CcoParaInit_RouteUpgrade::delayTimer_timeoutProc()
{
    switch(emScriptRunState)
    {
    default:
    {
        break;
    }
    }
}
QString Script_CcoParaInit_RouteUpgrade::selectUpgradeFile()
{
    QStringList fileNameFilters;
    QStringList fileList;
    QString fileDir=QApplication::applicationDirPath()+"/DataBase/Upgrade/";
    QStringList paths;
    QString error;
    paths<<fileDir+"路由程序(新-新)"<<fileDir+"路由程序(旧-新)";
    for(auto path:paths)
    {
        QDir updateFileDir(path);
        QFileInfoList files=updateFileDir.entryInfoList(QStringList()<<"*",QDir::Files,QDir::NoSort);
        if(files.count() != 1)
        {
            error="获取到的CCO升级文件数不为1："+QString::number(files.count());
            p_AbstractScriptHost->updateProgress(ProcessState_Processing,error);
            continue;
        }
        QString fileName=files.first().fileName();
        if(fileName.right(3)!="bin"&&fileName.right(3)!="dat")
        {
            error="获取到的文件后缀名不是dat或bin："+fileName.right(3);
            p_AbstractScriptHost->updateProgress(ProcessState_Processing,error);
            continue;
        }
        if(fileName.left(7)!="TCRS091" && fileName.left(7)!="TCRS053" && fileName.left(6)!="TCC0A1" && fileName.left(6)!="TCD0A1"&& fileName.left(7)!="TCRS0B1"&& fileName.left(7)!="GY2301J")//fileName.left(7)!="TCRS0A1"
        {
            error="获取到的文件名头不是TCRS091或TCRS053或TCC0A1或TCD0A1或TCRS0B1或GY2301J："+fileName.left(7);
            p_AbstractScriptHost->updateProgress(ProcessState_Processing,error);
            continue;
        }
        QString outVer,innerVer;
        QStringList list=fileName.split("_");
        
        // 支持两种文件名格式:
        // 格式1: TCRS091_日期1_日期2_V版本号.bin (4个部分)
        // 格式2: GY2301JC(GW)_日期V版本号(V内部版本号).bin (2个部分)
        if(list.size()>=4)
        {
            // 格式1: TCRS091_日期1_日期2_V版本号.bin
            outVer=list.at(1);//外部日期
            innerVer=list.at(2);//内部日期
            list=list.at(3).split("V");
            if(list.size()<2)
            {
                error="获取到的文件名版本号格式不对："+fileName;
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, error);
                continue;
            }
            outVer+=list[1].remove('.').remove('(').remove(')').rightJustified(4,'0');//外部版本号
            innerVer+=list[2].remove('.').remove('(').remove(')').rightJustified(4,'0');//内部版本号
        }
        else if(list.size()==2 && fileName.contains("GY2301"))
        {
            // 格式2: GY2301JC(GW)_202601041801V03.00(V03.16).bin
            // 解析: 日期202601041801 + V03.00(V03.16)
            QString part2 = list.at(1); // 202601041801V03.00(V03.16).bin
            int vIndex = part2.indexOf('V');
            if(vIndex < 0 || vIndex < 6)
            {
                error="获取到的文件名格式不对(找不到V)："+fileName;
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, error);
                continue;
            }
            QString dateStr = part2.left(vIndex); // 202601041801
            // 外部日期取前6位(年月日): 260104
            outVer = dateStr.left(6);
            // 内部日期取后6位(时分秒): 041801 -> 需要转换为日期格式
            innerVer = dateStr.left(6); // 使用相同的日期
            
            // 解析版本号部分: V03.00(V03.16).bin
            QString verPart = part2.mid(vIndex); // V03.00(V03.16).bin
            verPart = verPart.left(verPart.lastIndexOf('.')); // V03.00(V03.16)
            
            QStringList verList = verPart.split("(V");
            if(verList.size() < 2)
            {
                error="获取到的文件名版本号格式不对："+fileName;
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, error);
                continue;
            }
            // 外部版本: V03.00 -> 0300
            QString outVerNum = verList[0].remove('V').remove('.');
            outVer += outVerNum.rightJustified(4,'0');
            // 内部版本: 03.16) -> 0316
            QString innerVerNum = verList[1].remove('.').remove(')');
            innerVer += innerVerNum.rightJustified(4,'0');
        }
        else
        {
            error="获取到的文件名日期格式不对："+fileName;
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, error);
            continue;
        }
        if(outVer!=outVersionBefore||innerVer!=innerVersionBefore)
        {
            return files.first().filePath();
        }
    }
    return error;
}
