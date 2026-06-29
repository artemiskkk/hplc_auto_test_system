#include "Script_ParameterManage_DeleteNotInRouter.h"

Script_ParameterManage_DeleteNotInRouter::Script_ParameterManage_DeleteNotInRouter(QObject *parent) : QObject(parent)
{
    p_BuildNetwork_GW=new BuildNetwork_GW();
    p_CtrInfoList=make_shared<QList<shared_ptr<CtrInfo>>>();
    p_Frame3762Helper=make_shared<Frame3762Helper>();
    p_Afn11F2=make_shared<Afn11F2>();
    p_Afn10F1=make_shared<Afn10F1>();
    p_Afn10F2=make_shared<Afn10F2>();
    p_Afn01F2=make_shared<Afn01F2>();
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
Script_ParameterManage_DeleteNotInRouter::~Script_ParameterManage_DeleteNotInRouter()
{
    p_BuildNetwork_GW->initBuildNetWork();
    if(needPowerOff==true)//断电处理
        powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost);
}
void Script_ParameterManage_DeleteNotInRouter::setHost(AbstractScriptHost *host)
{
    p_AbstractScriptHost=host;
    p_BuildNetwork_GW->setHost(host);
}
void Script_ParameterManage_DeleteNotInRouter::addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq)
{
    p_CtrInfoList->clear();
    comnAddAddrsInfo(p_CtrInfoList, p_ConcentratorInfoList, p_MeterInfoList, p_SchemeCfgInfoList);
    uchar dstFreq=freq&0x0f;

    uchar dstPrtcl=uchar(freq)>>4;
    if(dstPrtcl==0x03)
        setMeterAddrsPrtcl(p_CtrInfoList,dstPrtcl);

    p_BuildNetwork_GW->setCtrInfoListAndFreq(p_CtrInfoList, dstFreq);
    for(auto i:p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values())
    {
        allMeters<<Address(i->mtrAddr).toString();
    }
}
bool Script_ParameterManage_DeleteNotInRouter::config(const QMap<QString,QString> *paraDic)
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
    QSettings property(QString("PropertyConfig.ini"),QSettings::IniFormat);
    chipType=property.value("SYSTEM_PROPERTY/ChipType","GY").toString();
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("chipType=%1").arg(chipType));
    return result;
}

///几种测试场景
/// ***********************************************************************************
/// 场景1.不需要组网，且不需要添加档案或重新设置CCO，这种一般像抄控器升级，不涉及CCO的操作
/// 直接在execute()中执行测试脚本的第一步操作即可
/// ***********************************************************************************
/// 场景2.不需要组网，但是需要添加档案或重新设置CCO，设置needBuildNet=true;
/// 结合p_BuildNetwork_GW->startBuildNetFlag标志，startBuildNetFlag初始是false
/// 当组网通用脚本执行完添加档案之后，startBuildNetFlag标志置为true
/// 据此判断执行测试脚本的新操作即可
/// ***********************************************************************************
/// 场景3.需要组网，但如果CCO已组网完成且档案也全部匹配则不需要重新组网，如抄表脚本，
/// 设置needBuildNet=true;
/// 组网通用脚本默认会首先执行组网探测脚本，具体可查看组网通用脚本代码
/// 如果探测已组网完成且拓扑档案与设定档案一样，会提示组网完成，否则继续执行组网通用脚本，完成组网
/// 组网完成后，p_BuildNetwork_GW->buildNetworkResultFlag标志为true
/// 执行测试脚本的新操作即可
/// ***********************************************************************************
/// 场景4.需要组网，需要重新组网，如组网脚本，设置needBuildNet=true;
/// 设置p_BuildNetwork_GW->needRebuildNetwork=true;
/// 组网通用脚本默认会跳过组网探测脚本，直接执行组网通用脚本后续操作，完成组网
/// 组网完成后，p_BuildNetwork_GW->buildNetworkResultFlag标志为true
/// 执行测试脚本的新操作即可
void Script_ParameterManage_DeleteNotInRouter::execute()
{
    p_AbstractScriptHost->updateProgress(ProcessState_Start, "Test start!");

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
        tryTimes=0;
        index=0;
        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,index,p_Afn11F2);
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送删除从节点命令，等待--回复");
        emScriptRunState=Wait_DeleteNode_11F2_Finish;
        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
    }
    p_maxAllowTimer->start((timerForReachThresld+timerAfterReachThresld)*1000);//此处要启动脚本的最大执行时间定时器
}
void Script_ParameterManage_DeleteNotInRouter::stop()
{
    p_timer->stop();
    p_delayTimer->stop();
    p_maxAllowTimer->stop();
    if(p_AbstractScriptHost==nullptr)
        return;
    p_AbstractScriptHost->updateProgress(ProcessState_Error, "Test stop!");
    if(needPowerOff==true)
        powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost);
}
void Script_ParameterManage_DeleteNotInRouter::processMsg(DvcType dvcType, int id, uchar *data, int datalen)
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
            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,index,p_Afn11F2);
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送删除从节点命令，等待--回复");
            emScriptRunState=Wait_DeleteNode_11F2_Finish;
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
void Script_ParameterManage_DeleteNotInRouter::processCtrlDvcRes(DvcType dvcType, QList<int> idList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params)
{
    //处理工装操作命令，如12V断上电，复位，设置波特率之类的
    Q_UNUSED(dvcType)
    Q_UNUSED(idList)
    Q_UNUSED(ctrlCmdType)
    Q_UNUSED(isSucs)
    Q_UNUSED(params)

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
        {
            break;
        }
    }
}

void Script_ParameterManage_DeleteNotInRouter::processMsgFromCCO(DvcType dvcType, int dvcId)
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
        //检查点处理
        if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x02&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp&&uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq)==msgSeq)
        {
            if(!(emScriptRunState==Wait_DeleteNode_11F2_Finish||emScriptRunState==Wait_DeleteNode_11F2_1_Finish))
            {
                p_timer->stop();
                p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("收到CCO回复否认，运行状态为%1").arg(runStateMetaEnum.valueToKey(emScriptRunState)));
                p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("收到CCO回复否认，运行状态为%1"+testCase).arg(runStateMetaEnum.valueToKey(emScriptRunState)));
                return;
            }
        }
        else if(p_Frame3762Base->afn_ == 0x03&&p_Frame3762Base->dt1_==0x02&&p_Frame3762Base->dt2_==0x01&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("收到CCO上报03F10，疑似复位，运行状态为%1").arg(runStateMetaEnum.valueToKey(emScriptRunState)));
            p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("收到CCO上报03F10，疑似复位，运行状态为%1"+testCase).arg(runStateMetaEnum.valueToKey(emScriptRunState)));
            return;
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
            case Wait_DeleteNode_11F2_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp&&uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq)==msgSeq)
                {
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("收到删除从节点回复确认"));
                    if(chipType=="GY")
                    {
                        emScriptRunState=Wait_QueryNodeInfo_10F2_Finish;
                        index=0;
                        tryTimes=0;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("查询从节点信息，等待回复"));
                        sendMsg(dvcType,p_CtrInfoList->at(0)->ctrlID,index,p_Afn10F2);
                        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("%0芯片CCO，收到删除从节点回复确认，不符合要求，运行状态为%1"+testCase).arg(chipType).arg(runStateMetaEnum.valueToKey(emScriptRunState)));
                    }
                }
                else if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x02&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)//&&uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq)==msgSeq)
                {
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("收到删除从节点回复否认"));
                    shared_ptr<Afn00F2> ptr=dynamic_pointer_cast<Afn00F2>(p_Frame3762Base);
                    if((chipType=="V3"&&uchar(ptr->error_code_)==0x00)||(chipType=="V3B"&&(uchar(ptr->error_code_)==0x01 || uchar(ptr->error_code_)==0x07)))//07 表号不存在
                    {
                        emScriptRunState=Wait_QueryNodeInfo_10F2_Finish;
                        index=1;
                        tryTimes=0;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("查询从节点信息，等待回复"));
                        sendMsg(dvcType,p_CtrInfoList->at(0)->ctrlID,index,p_Afn10F2);
                        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("%1芯片CCO，收到删除从节点回复否认，否认码%2，不符合要求，运行状态为%3"+testCase).arg(chipType).arg(QString::number(uchar(ptr->error_code_))).arg(runStateMetaEnum.valueToKey(emScriptRunState)));
                    }
                }
                else//收到其它报文的处理
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_QueryNodeInfo_10F2_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x10&&p_Frame3762Base->dt1_==0x02&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)//&&uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq)==msgSeq)
                {
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("收到查询从节点信息回复"));
                    shared_ptr<Afn10F2> ptr=dynamic_pointer_cast<Afn10F2>(p_Frame3762Base);
                    for(auto i:ptr->node_info_data_unit_.node_info_group_list_)
                    {
                        queryInfoMeters<<i.node_address_.toString();
                    }
                    index+=addCntPerTime;
                    if(index<ptr->node_info_data_unit_.node_total_num_)
                    {
                        emScriptRunState=Wait_QueryNodeInfo_10F2_Finish;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("查询从节点信息，等待回复"));
                        sendMsg(dvcType,p_CtrInfoList->at(0)->ctrlID,index,p_Afn10F2);
                        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                    {
                        if(queryInfoMeters.size()!=allMeters.size())
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("从节点信息与档案信息数量不一致"));
                            p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("从节点信息存在异常"+testCase));
                            return;
                        }
                        for(auto i:queryInfoMeters)
                        {
                            if(!allMeters.contains(i))
                            {
                                p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("存在节点地址不在档案内，表号%1").arg(i));
                                p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("从节点信息存在异常"+testCase));
                                return;
                            }
                        }
                        index=0;
                        tryTimes=0;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("从节点信息正常"));
                        emScriptRunState=Wait_ParameterInit_01F2_Finish;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("参数初始化，等待回复"));
                        sendMsg(dvcType,p_CtrInfoList->at(0)->ctrlID,index,p_Afn01F2);
                        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                    }
                }
                else//收到其它报文的处理
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_ParameterInit_01F2_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)//&&uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq)==msgSeq)
                {
                    p_timer->stop();
                    tryTimes=0;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("收到参数初始化回复确认"));
                    emScriptRunState=Wait_QueryNodeNum_10F1_Finish;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("查询从节点数量，等待回复"));
                    sendMsg(dvcType,p_CtrInfoList->at(0)->ctrlID,index,p_Afn10F1);
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                }
                else//收到其它报文的处理
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_QueryNodeNum_10F1_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x10&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)//&&uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq)==msgSeq)
                {
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("收到查询从节点数量回复"));
                    if(dynamic_pointer_cast<Afn10F1>(p_Frame3762Base)->node_total_num_==0)
                    {
                        tryTimes=0;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("从节点数量为0，符合要求"));
                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,index,p_Afn11F2);
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送删除从节点命令，等待--回复");
                        emScriptRunState=Wait_DeleteNode_11F2_1_Finish;
                        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("从节点数量不为0，不符合要求"+testCase));
                    }
                }
                else//收到其它报文的处理
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_DeleteNode_11F2_1_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)//&&uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq)==msgSeq)
                {
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("收到删除从节点回复确认"));
                    if(chipType=="GY")
                    {
                        tryTimes=0;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("收到删除从节点回复确认"));
                        emScriptRunState=Wait_QueryNodeNum_10F1_1_Finish;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("查询从节点数量，等待回复"));
                        sendMsg(dvcType,p_CtrInfoList->at(0)->ctrlID,index,p_Afn10F1);
                        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("%0芯片CCO，收到删除从节点回复确认，不符合要求，运行状态为%1"+testCase).arg(chipType).arg(runStateMetaEnum.valueToKey(emScriptRunState)));
                    }
                }
                else if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x02&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)//&&uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq)==msgSeq)
                {
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("收到删除从节点回复否认"));
                    shared_ptr<Afn00F2> ptr=dynamic_pointer_cast<Afn00F2>(p_Frame3762Base);
                    if((chipType=="V3"&&uchar(ptr->error_code_)==0x00)||(chipType=="V3B"&&(uchar(ptr->error_code_)==0x01 || uchar(ptr->error_code_)==0x07)))//07 表号不存在
                    {
                        tryTimes=0;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("收到删除从节点回复否认"));
                        emScriptRunState=Wait_QueryNodeNum_10F1_1_Finish;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("查询从节点数量，等待回复"));
                        sendMsg(dvcType,p_CtrInfoList->at(0)->ctrlID,index,p_Afn10F1);
                        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("%1芯片CCO，收到删除从节点回复否认，否认码%2，不符合要求，运行状态为%3"+testCase).arg(chipType).arg(QString::number(uchar(ptr->error_code_))).arg(runStateMetaEnum.valueToKey(emScriptRunState)));
                    }
                }
                else//收到其它报文的处理
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_QueryNodeNum_10F1_1_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x10&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)//&&uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq)==msgSeq)
                {
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("收到查询从节点数量回复"));
                    if(dynamic_pointer_cast<Afn10F1>(p_Frame3762Base)->node_total_num_==0)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("从节点数量为0，符合要求"));
                        p_AbstractScriptHost->updateProgress(ProcessState_Success,QString("脚本执行成功"+testCase));
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("从节点数量不为0，不符合要求"+testCase));
                    }
                }
                else//收到其它报文的处理
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
void Script_ParameterManage_DeleteNotInRouter::processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID)
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
void Script_ParameterManage_DeleteNotInRouter::processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID)
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
void Script_ParameterManage_DeleteNotInRouter::sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame)
{
    if(frame==p_Afn11F2)
    {
        p_Afn11F2->ctrl_field_.dir=kDirDown;
        p_Afn11F2->ctrl_field_.prm=kActive;
        p_Afn11F2->ctrl_field_.comn_type=kHplc;
        msgSeq++;
        p_Afn11F2->info_field_.info_field_down.msg_seq=char(msgSeq);
        p_Afn11F2->info_field_.info_field_down.comu_rate=0;
        p_Afn11F2->info_field_.info_field_down.comu_module_ident=0;

        p_Afn11F2->node_address_list_.clear();
        p_Afn11F2->node_num_=uchar(deleteMeters.size());
        for(auto i:deleteMeters)
        {
            p_Afn11F2->node_address_list_<<Address(QByteArray::fromHex(i.toLatin1()));
        }
        sendMsgOct=p_Afn11F2->EncodeFrame();
        sendMsgLog=QString("》》删除从节点11F2：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_Afn10F2)
    {
        p_Afn10F2->ctrl_field_.dir=kDirDown;
        p_Afn10F2->ctrl_field_.prm=kActive;
        p_Afn10F2->ctrl_field_.comn_type=kHplc;
        msgSeq++;
        p_Afn10F2->info_field_.info_field_down.msg_seq=char(msgSeq);
        p_Afn10F2->info_field_.info_field_down.comu_rate=0;
        p_Afn10F2->info_field_.info_field_down.comu_module_ident=0;

        p_Afn10F2->node_start_no_=ushort(index);
        p_Afn10F2->node_num_=addCntPerTime;

        sendMsgOct=p_Afn10F2->EncodeFrame();
        sendMsgLog=QString("》》查询从节点信息10F2：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_Afn01F2)
    {
        p_Afn01F2->ctrl_field_.dir=kDirDown;
        p_Afn01F2->ctrl_field_.prm=kActive;
        p_Afn01F2->ctrl_field_.comn_type=kHplc;
        msgSeq++;
        p_Afn01F2->info_field_.info_field_down.msg_seq=char(msgSeq);
        p_Afn01F2->info_field_.info_field_down.comu_rate=0;
        p_Afn01F2->info_field_.info_field_down.comu_module_ident=0;

        sendMsgOct=p_Afn01F2->EncodeFrame();
        sendMsgLog=QString("》》参数初始化01F2：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_Afn10F1)
    {
        p_Afn10F1->ctrl_field_.dir=kDirDown;
        p_Afn10F1->ctrl_field_.prm=kActive;
        p_Afn10F1->ctrl_field_.comn_type=kHplc;
        msgSeq++;
        p_Afn10F1->info_field_.info_field_down.msg_seq=char(msgSeq);
        p_Afn10F1->info_field_.info_field_down.comu_rate=0;
        p_Afn10F1->info_field_.info_field_down.comu_module_ident=0;

        sendMsgOct=p_Afn10F1->EncodeFrame();
        sendMsgLog=QString("》》查询从节点数量10F1：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
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
void Script_ParameterManage_DeleteNotInRouter::sendSrcMsg(DvcType dvcType, int dvcId, QByteArray msg)
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


void Script_ParameterManage_DeleteNotInRouter::timer_timeoutProc()
{
    switch(emScriptRunState)
    {
        case Wait_BuildNetFinish_Whole:
        {
            p_CtrInfoList->at(0)->inNetResult=false;
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("全网组网成功率：%1%").arg(p_CtrInfoList->at(0)->inNetSuccessRate*100));
            break;
        }
        case Wait_DeleteNode_11F2_Finish:
        case Wait_QueryNodeInfo_10F2_Finish:
        case Wait_ParameterInit_01F2_Finish:
        case Wait_QueryNodeNum_10F1_Finish:
        case Wait_DeleteNode_11F2_1_Finish:
        case Wait_QueryNodeNum_10F1_1_Finish:
        {
            p_timer->stop();
            if(++tryTimes>=3)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("%1 timeout!!!").arg(runStateMetaEnum.valueToKey(emScriptRunState)));
            }
            else
            {
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,nullptr);
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

void Script_ParameterManage_DeleteNotInRouter::maxAllowTimer_timeoutProc()
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

void Script_ParameterManage_DeleteNotInRouter::delayTimer_timeoutProc()
{
    switch(emScriptRunState)
    {
        case Wait_BuildNetFinish_Whole:
        {
            break;
        }
        default:
        {
            break;
        }
    }
}
