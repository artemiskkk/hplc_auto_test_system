#include "Script_ReadMeter_13F1_ContrlArea.h"


Script_ReadMeter_13F1_ContrlArea::Script_ReadMeter_13F1_ContrlArea(QObject *parent) : QObject(parent)
{
    p_BuildNetwork_GW=new BuildNetwork_GW();
    p_CtrInfoList=make_shared<QList<shared_ptr<CtrInfo>>>();
    p_Frame3762Helper=make_shared<Frame3762Helper>();
    p_Frame645Helper=make_shared<Frame645Helper>();
    p_MeterAddrResp_93=make_shared<RspsNormal_ReadAddr_0x93>(addr,6);
    p_FrameOOPHelper=make_shared<FrameOOPHelper>();
    p_GetResponseNormal_ReadAddr=make_shared<GetResponseNormal>();

    p_RouterPause_12F2=make_shared<Afn12F2>();
    p_MonitorSlaveNode_13F1=make_shared<Afn13F1>();

    p_timer=new QTimer(this);
    p_maxAllowTimer=new QTimer(this);
    p_delayTimer=new QTimer(this);
    connect(p_timer,SIGNAL(timeout()),this,SLOT(timer_timeoutProc()));
    connect(p_maxAllowTimer,SIGNAL(timeout()),this,SLOT(maxAllowTimer_timeoutProc()));
    connect(p_delayTimer,SIGNAL(timeout()),this,SLOT(delayTimer_timeoutProc()));
}
Script_ReadMeter_13F1_ContrlArea::~Script_ReadMeter_13F1_ContrlArea()
{
    p_BuildNetwork_GW->initBuildNetWork();
    if(needPowerOff==true)//断电处理
        powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost,true);
}
void Script_ReadMeter_13F1_ContrlArea::setHost(AbstractScriptHost *host)
{
    p_AbstractScriptHost=host;
    p_BuildNetwork_GW->setHost(host);
}
void Script_ReadMeter_13F1_ContrlArea::addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq)
{
    p_CtrInfoList->clear();
    comnAddAddrsInfo(p_CtrInfoList, p_ConcentratorInfoList, p_MeterInfoList, p_SchemeCfgInfoList);
    uchar dstFreq=freq&0x0f;

    uchar dstPrtcl=uchar(freq)>>4;
    if(dstPrtcl==0x03)
        setMeterAddrsPrtcl(p_CtrInfoList,dstPrtcl);

    p_BuildNetwork_GW->setCtrInfoListAndFreq(p_CtrInfoList, dstFreq);

}
bool Script_ReadMeter_13F1_ContrlArea::config(const QMap<QString,QString> *paraDic)
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
void Script_ReadMeter_13F1_ContrlArea::execute()
{
    p_AbstractScriptHost->updateProgress(ProcessState_Start, "Test start!");
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "测试编号：【GW-CCO-F003-0007-V01】\n"
                                                                  "测试流程：\n"
                                                                  "1：组网，Wait_11F1_BuildNet_Finish；\n"
                                                                  "2：给路由下发12F2，暂停路由命令，Wait_12F12_RouterPause_Finish；\n"
                                                                  "3：下发13F1监控从节点命令，通信协议类型0x02，传输标志位0，启动标志位1，采用645协议，抄读645电表数据，Wait_13F1_MeterRead01_Finish；\n"
                                                                  "4：下发13F1监控从节点命令，通信协议类型0x02，传输标志位0，启动标志位0，采用645协议，抄读645电表数据，Wait_13F1_MeterRead00_Finish；\n"
                                                                  "5：下发13F1监控从节点命令，通信协议类型0x02，传输标志位1，启动标志位0，采用645协议，抄读645电表数据，Wait_13F1_MeterRead10_Finish；\n"
                                                                  "6：下发13F1监控从节点命令，通信协议类型0x02，传输标志位1，启动标志位1，采用645协议，抄读645电表数据，Wait_13F1_MeterRead11_Finish；");

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
        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_RouterPause_12F2);
        emScriptRunState=Wait_00F1_for_12F2_Pause;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--路由暂停（12F2），等待--确认");
        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
    }
    p_maxAllowTimer->start((timerForReachThresld+timerAfterReachThresld)*1000);//此处要启动脚本的最大执行时间定时器
}
void Script_ReadMeter_13F1_ContrlArea::stop()
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
void Script_ReadMeter_13F1_ContrlArea::processMsg(DvcType dvcType, int id, uchar *data, int datalen)
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
            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,id,p_RouterPause_12F2);
            emScriptRunState=Wait_00F1_for_12F2_Pause;
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--路由暂停（12F2），等待--确认");
            p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
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
void Script_ReadMeter_13F1_ContrlArea::processCtrlDvcRes(DvcType dvcType, QList<int> idList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params)
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
//    case Wait_CommandProcess_Finish:
//    {
//        break;
//    }
    case ScriptSuccess:
    {
        break;
    }
    }
}

void Script_ReadMeter_13F1_ContrlArea::processMsgFromCCO(DvcType dvcType, int dvcId)
{
    if(dvcId!=p_CtrInfoList->at(0)->ctrlID)
        return;
    
    QByteArray completeFrame;
    QByteArray& buf = p_CtrInfoList->at(0)->buf;
    bool haveCompleteMsg=extractAndProcess3762Frame(buf,completeFrame);

    while(haveCompleteMsg)
    {
        shared_ptr<Frame3762Base> p_Frame3762Base = nullptr;
        
        try {
            p_Frame3762Base = p_Frame3762Helper->DecodeLocalMsg(&completeFrame,haveCompleteMsg);
        }
        catch(...) {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, 
                QString("3762帧解析异常，跳过此帧: %1").arg(QString(completeFrame.toHex())));
            // 继续处理下一帧
            haveCompleteMsg = extractAndProcess3762Frame(buf, completeFrame);
            continue;
        }
        
        if(p_Frame3762Base==nullptr)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, 
                QString("3762帧解析失败，跳过此帧: %1").arg(QString(completeFrame.toHex())));
            // 继续处理下一帧
            haveCompleteMsg = extractAndProcess3762Frame(buf, completeFrame);
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
        case Wait_00F1_for_12F2_Pause:
        {
            if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();//接收到一条完整报文
                tryTimes=0;
                index=0;
                //times=ushort(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size());
                emScriptRunState=Wait_13F1_MeterRead01_Finish;
                sendMsg(CCO_GW,dvcId,index,p_MonitorSlaveNode_13F1);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--监控从节点（13F1），等待--点抄完成");
                p_timer->start(90*1000);
            }
            else if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x02&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp&&uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq)==msgSeq)
            {
                p_timer->stop();
                p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("Wait_12F12_RouterPause_Finish，12F1回复异常，期望回复确认，实际回复否认，【GW-CCO-F003-0007-V01】"));
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case Wait_13F1_MeterRead01_Finish:
        {
            if(p_Frame3762Base->afn_==0x13&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                shared_ptr<Afn13F1> p_MonitorSlaveNode_13F1_Up=dynamic_pointer_cast<Afn13F1>(p_Frame3762Base);
                if(p_MonitorSlaveNode_13F1_Up->data_field_up_.frame_content_.size()!=0)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "Wait_13F1_MeterRead01_Finish，13F1正常回复，测试继续");
                    emScriptRunState=Wait_13F1_MeterRead00_Finish;
                    sendMsg(CCO_GW,dvcId,index,p_MonitorSlaveNode_13F1);
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--监控从节点（13F1），等待--点抄完成");
                    p_timer->start(90*1000);
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_13F1_MeterRead01_Finish，13F1回复异常，期望正常上报，实际无正常上报，【GW-CCO-F003-0007-V01】");

                }
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case Wait_13F1_MeterRead00_Finish:
        {
            if(p_Frame3762Base->afn_==0x13&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                shared_ptr<Afn13F1> p_MonitorSlaveNode_13F1_Up=dynamic_pointer_cast<Afn13F1>(p_Frame3762Base);
                if(p_MonitorSlaveNode_13F1_Up->data_field_up_.frame_content_.size()!=0)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "Wait_13F1_MeterRead00_Finish，13F1正常回复，测试继续");
                    emScriptRunState=Wait_13F1_MeterRead10_Finish;
                    sendMsg(CCO_GW,dvcId,index,p_MonitorSlaveNode_13F1);
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--监控从节点（13F1），等待--点抄完成");
                    p_timer->start(90*1000);
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_13F1_MeterRead00_Finish，13F1回复异常，期望正常上报，实际无正常上报，【GW-CCO-F003-0007-V01】");

                }
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case Wait_13F1_MeterRead10_Finish:
        {
            if(p_Frame3762Base->afn_==0x13&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();

                p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_13F1_MeterRead10_Finish，13F1回复异常，期望无13F1上报，实际13F1有上报，【GW-CCO-F003-0007-V01】");

//                shared_ptr<Afn13F1> p_MonitorSlaveNode_13F1_Up=dynamic_pointer_cast<Afn13F1>(p_Frame3762Base);
//                if(p_MonitorSlaveNode_13F1_Up->data_field_up_.frame_content_.size()==0)
//                {
//                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "Wait_13F1_MeterRead10_Finish，13F1上报空数据域，符合期望，测试继续！");
//                    emScriptRunState = Wait_13F1_MeterRead11_Finish;
//                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,index,p_MonitorSlaveNode_13F1);
//                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--监控从节点（13F1），等待--点抄完成");
//                    p_timer->start(90*1000);
//                }
//                else
//                {
//                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_13F1_MeterRead10_Finish，13F1回复异常，期望无13F1上报，实际13F1有上报，【GW-CCO-F003-0007-V01】");
//                }
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }

            break;
        }
        case Wait_13F1_MeterRead11_Finish:
        {
            if(p_Frame3762Base->afn_==0x13&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();

                p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_13F1_MeterRead11_Finish，13F1回复异常，期望无13F1上报，实际13F1有上报，【GW-CCO-F003-0007-V01】");

//                shared_ptr<Afn13F1> p_MonitorSlaveNode_13F1_Up=dynamic_pointer_cast<Afn13F1>(p_Frame3762Base);
//                if(p_MonitorSlaveNode_13F1_Up->data_field_up_.frame_content_.size()==0)
//                {
//                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "Wait_13F1_MeterRead11_Finish，13F1上报空数据域，符合预期，测试通过！");
//                    emScriptRunState = ScriptSuccess;
//                    p_AbstractScriptHost->updateProgress(ProcessState_Success, "脚本执行成功，【GW-CCO-F003-0007-V01】");
//                }
//                else
//                {
//                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_13F1_MeterRead11_Finish，13F1回复异常，期望无13F1上报，实际13F1有上报，【GW-CCO-F003-0007-V01】");
//                }
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
        
        // 继续处理下一帧
        haveCompleteMsg = extractAndProcess3762Frame(buf, completeFrame);
    }
}
void Script_ReadMeter_13F1_ContrlArea::processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID)
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
void Script_ReadMeter_13F1_ContrlArea::processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID)
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
void Script_ReadMeter_13F1_ContrlArea::sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame)
{
    if(frame==p_RouterPause_12F2)
    {
        p_RouterPause_12F2->ctrl_field_.dir=kDirDown;
        p_RouterPause_12F2->ctrl_field_.prm=kActive;
        p_RouterPause_12F2->ctrl_field_.comn_type=kHplc;

        p_RouterPause_12F2->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_RouterPause_12F2->info_field_.info_field_down.comu_rate=0;
        p_RouterPause_12F2->info_field_.info_field_down.comu_module_ident=0;

        sendMsgOct=p_RouterPause_12F2->EncodeFrame();
        sendMsgLog=QString("》》路由暂停12F2：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_MonitorSlaveNode_13F1)
    {
        uchar tmpAddr[6];
        uchar tmp645Addr[6];
        uchar tmp698Addr[6];
        uchar comPrtclType = 0x02;
        bool state_645_meter = false;
        //        memcpy(tmpAddr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(1)->mtrAddr,6);
        //        uchar comPrtclType=p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(1)->prtcl;

        for(int i=0; i<p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().size(); i++)
        {
            if(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(i)->prtcl == 0x02
                && p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(i)->slotPosition == SingleMeter)
            {
                memcpy(tmp645Addr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(i)->mtrAddr,6);
                comPrtclType = 0x02;
                state_645_meter = true;
                break;
            }
        }

        memcpy(tmpAddr,tmp645Addr,6);

        if(state_645_meter == false)
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "档案中无645表，测试停止！");

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

            if(emScriptRunState == Wait_13F1_MeterRead01_Finish)
            {
                sendMsgOct[3] = 0x43;
            }
            else if(emScriptRunState == Wait_13F1_MeterRead00_Finish)
            {
                sendMsgOct[3] = 0x03;
                int tmp_len = sendMsgOct.size() - 2;
                char tmp_cs = sendMsgOct[tmp_len];
                sendMsgOct[tmp_len] = tmp_cs - 0x40;
            }
            else if(emScriptRunState == Wait_13F1_MeterRead10_Finish)
            {
                sendMsgOct[3] = char(0x83);
                int tmp_len = sendMsgOct.size() - 2;
                char tmp_cs = sendMsgOct[tmp_len];
                sendMsgOct[tmp_len] = tmp_cs + 0x40;
            }
            else if(emScriptRunState == Wait_13F1_MeterRead11_Finish)
            {
                sendMsgOct[3] = char(0xc3);
                int tmp_len = sendMsgOct.size() - 2;
                char tmp_cs = sendMsgOct[tmp_len];
                sendMsgOct[tmp_len] = tmp_cs + 0x80;
            }


            sendMsgLog=QString("》》监控从节点13F1,抄读645电表：%1\n").arg(QString(sendMsgOct.toHex()));

            //startTime=QDateTime::currentDateTime().toString(CURRENT_TIME_FORMAT);
        }
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
void Script_ReadMeter_13F1_ContrlArea::sendSrcMsg(DvcType dvcType, int dvcId, QByteArray msg)
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


void Script_ReadMeter_13F1_ContrlArea::timer_timeoutProc()
{
    switch(emScriptRunState)
    {
        case Wait_BuildNetFinish_Whole:
        {
            p_CtrInfoList->at(0)->inNetResult=false;
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("全网组网成功率：%1%").arg(p_CtrInfoList->at(0)->inNetSuccessRate*100));
            break;
        }
        case Wait_00F1_for_12F2_Pause:
        {
            if(++tryTimes>=3)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_00F1_for_12F2_Pause1st timeout!!!");
            }
            else
            {
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_RouterPause_12F2);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--路由暂停命令（12F2），等待--确认");
            }
            break;
        }
        case Wait_13F1_MeterRead01_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_13F1_MeterRead01_Finish，“timeout！”，【GW-CCO-F003-0007-V01】");
            break;
        }
        case Wait_13F1_MeterRead00_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_13F1_MeterRead00_Finish，“timeout！”，【GW-CCO-F003-0007-V01】");
            break;
        }
        case Wait_13F1_MeterRead10_Finish:
        {
            p_timer->stop();
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "Wait_13F1_MeterRead10_Finish，无上行，不回复，符合期望，测试继续！");
            emScriptRunState = Wait_13F1_MeterRead11_Finish;
            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,index,p_MonitorSlaveNode_13F1);
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--监控从节点（13F1），等待--点抄完成");
            p_timer->start(90*1000);

            break;
        }
        case Wait_13F1_MeterRead11_Finish:
        {
            p_timer->stop();
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "Wait_13F1_MeterRead11_Finish，无上行，不回复，符合预期，测试通过！");
            if(p_maxAllowTimer!=nullptr)
                p_maxAllowTimer->stop();
            emScriptRunState = ScriptSuccess;
            p_AbstractScriptHost->updateProgress(ProcessState_Success, "脚本执行成功，【GW-CCO-F003-0007-V01】");
            break;
        }
        default:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==p_timer");
            break;
        }
    }
}

void Script_ReadMeter_13F1_ContrlArea::maxAllowTimer_timeoutProc()
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

void Script_ReadMeter_13F1_ContrlArea::delayTimer_timeoutProc()
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

bool Script_ReadMeter_13F1_ContrlArea::extractAndProcess3762Frame(QByteArray& buf,QByteArray& completeFrame)
{
    completeFrame.clear();

    // 1. 去除前导的 0xFE/0x1E 字符
    while (buf.size() > 0 && ((unsigned char)buf[0] == 0xFE || (unsigned char)buf[0] == 0x1E)) {
        buf.remove(0, 1);
    }

    if (buf.size() < 6) {
        return false;
    }

    // 2. 寻找有效的帧起始符（同时验证帧长度合理性）
    int frameStart = -1;
    for (int i = 0; i < buf.size() - 5; i++) {
        if ((unsigned char)buf[i] == 0x68) {
            uint16_t potentialLength = 0;
            memcpy(&potentialLength, buf.constData() + i + 1, 2);
            // 帧长度合理性检查：4-500字节
            if (potentialLength > 3 && potentialLength < 500) {
                if (buf.size() >= i + potentialLength) {
                    // 验证帧结束标志
                    if ((unsigned char)buf[i + potentialLength - 1] == 0x16) {
                        frameStart = i;
                        break;
                    }
                } else {
                    // 数据还不完整，等待更多数据
                    frameStart = i;
                    break;
                }
            }
        }
    }

    if (frameStart == -1) {
        // 没找到有效帧，保留最后5个字节等待下次拼接
        if (buf.size() > 5) buf.remove(0, buf.size() - 5);
        return false;
    }

    // 3. 移除帧起始之前的无效数据
    if (frameStart > 0) buf.remove(0, frameStart);
    if (buf.size() < 6) return false;

    // 4. 解析帧长度
    uint16_t frameLength = 0;
    memcpy(&frameLength, buf.constData() + 1, 2);

    // 帧长度再次校验
    if (frameLength < 4 || frameLength > 500) {
        buf.remove(0, 1);
        return extractAndProcess3762Frame(buf, completeFrame);
    }

    // 5. 检查数据是否完整
    if (buf.size() < frameLength) {
        p_AbstractScriptHost->updateProgress(ProcessState_Processing,
            QString("等待完整帧，需要%1字节，当前%2字节").arg(frameLength).arg(buf.size()));
        return false;
    }

    // 6. 验证帧结束标志
    if ((unsigned char)buf[frameLength - 1] != 0x16) {
        buf.remove(0, 1);
        return extractAndProcess3762Frame(buf, completeFrame);
    }

    // 7. 提取完整帧
    completeFrame = buf.left(frameLength);
    buf.remove(0, frameLength);

    p_AbstractScriptHost->updateProgress(ProcessState_Processing,
        QString("提取到完整3762帧: %1").arg(QString(completeFrame.toHex())));


    return true;
}
