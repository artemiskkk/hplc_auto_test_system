#include "Script_MinuteFreeze_Reset_Hunan.h"
Script_MinuteFreeze_Reset_Hunan::Script_MinuteFreeze_Reset_Hunan(QObject *parent) : QObject(parent)
{
    emScriptRunState=TestInit;
    resultFlag=false;
    sendMsgOct.clear();

    p_CtrInfoList=make_shared<QList<shared_ptr<CtrInfo>>>();
    p_BuildNetwork_GW=new BuildNetwork_GW();

    p_Frame3762Helper=make_shared<Frame3762Helper>();
    p_SetRouterTimePeriod_05F102=make_shared<Afn05F102>();
    p_SetCurveConfig_STA_05F103=make_shared<Afn05F103>();
    p_QueryCurveData_CCO_03F102=make_shared<Afn03F102>();
    p_QueryConfigStatus_STA_10f103=make_shared<Afn10F103_Hunan>();
    p_ReadCurveData_STA_F1F100=make_shared<AfnF1F100>();
    p_HardReset_01F1=make_shared<Afn01F1>();
    p_QueryNetScale_10F9=make_shared<Afn10F9>();

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
Script_MinuteFreeze_Reset_Hunan::~Script_MinuteFreeze_Reset_Hunan()
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
void Script_MinuteFreeze_Reset_Hunan::execute()
{
    p_AbstractScriptHost->updateProgress(ProcessState_Start, "Test start!");

    emScriptRunState=TestInit;
    resultFlag=false;
    addrList.clear();
    meterInfoListInit();

    if(needBuildNet==true)
    {
        p_BuildNetwork_GW->execute();
        emScriptRunState=Wait_BuildNetFinish_Whole;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "等待全网组网完成");

    }
    else
    {
        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_SetRouterTimePeriod_05F102);
        emScriptRunState=Wait_00F1_For_05F102_Finish;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--设置路由校时周期，等待--确认");
        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
    }
    p_maxAllowTimer->start((timerForReachThresld+timerAfterReachThresld)*1000);
}
void Script_MinuteFreeze_Reset_Hunan::stop()
{
    if(p_AbstractScriptHost==nullptr)
        return;
    p_AbstractScriptHost->updateProgress(ProcessState_Error, "Test stop!");
    if(needPowerOff==true)
        powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost);
}
void Script_MinuteFreeze_Reset_Hunan::addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq)
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
void Script_MinuteFreeze_Reset_Hunan::setHost(AbstractScriptHost *host)
{
    p_AbstractScriptHost=host;
    p_BuildNetwork_GW->setHost(host);
}
bool Script_MinuteFreeze_Reset_Hunan::config(const QMap<QString,QString> *paraDic)
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
void Script_MinuteFreeze_Reset_Hunan::processMsg(DvcType dvcType, int id, uchar *data, int datalen)
{
    if(p_CtrInfoList->size()==0)
        return;
    if(emScriptRunState==Wait_BuildNetFinish_Whole)
    {
        if(!p_BuildNetwork_GW->buildNetworkResultFlag)
            p_BuildNetwork_GW->processMsg(dvcType,id,data,datalen);
        else
        {
            if(p_BuildNetwork_GW->emScriptRunState==BuildNetFinish&&p_BuildNetwork_GW->buildNetworkResultFlag==true)
            {
                tryTimes=0;
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_SetRouterTimePeriod_05F102);
                emScriptRunState=Wait_00F1_For_05F102_Finish;
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--设置路由校时周期，等待--确认");
                p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
            }
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
void Script_MinuteFreeze_Reset_Hunan::processCtrlDvcRes(DvcType dvcType, QList<int> idList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params)
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
        case Wait_00F1_For_05F102_Finish:
        {
            break;
        }
        case Wait_SetCurveConfig_STA_05F103_Finish:
        {
            break;
        }
        case Wait_Delay_Finish:
        {
            break;
        }
        case Wait_QueryCurveData_CCO_03F102_Finish:
        {
            break;
        }
        case Wait_SetCurveConfig_STA_05F103_2_Finish:
        {
            break;
        }
        case Wait_Delay_2_Finish:
        {
            break;
        }
        case Wait_QueryCurveData_CCO_03F102_2_Finish:
        {
            break;
        }
        case Wait_QueryConfigStatus_STA_10f103_Finish:
        {
            break;
        }
        case Wait_Delay_3_Finish:
        {
            break;
        }
        case Wait_Delay_4_Finish:
        {
            break;
        }
        case Wait_ReadCurveData_STA_F1F100_Finish:
        {
            break;
        }
        case Wait_SetCurveConfig_STA_05F103_3_Finish:
        {
            break;
        }
        case Wait_SetCurveConfig_STA_05F103_4_Finish:
        {
            break;
        }
        case Wait_Delay_5_Finish:
        {
            break;
        }
        case Wait_ReadCurveData_STA_F1F100_2_Finish:
        {
            break;
        }
        case Wait_QueryTaskConfig_STA_10F103_Finish:
        {
            break;
        }
        case ScriptSuccess:
        {
            break;
        }
    }
}

void Script_MinuteFreeze_Reset_Hunan::processMsgFromCCO(DvcType dvcType, int dvcId)
{
    if(dvcId!=p_CtrInfoList->at(0)->ctrlID)
        return;
    bool haveCompleteMsg=true;
    while(haveCompleteMsg)
    {
        qInfo()<<QString("CCO-解析前 buf3762=%1").arg(QString((p_CtrInfoList->at(0)->buf.toHex())));
        shared_ptr<Frame3762Base> p_Frame3762Base = p_Frame3762Helper->DecodeLocalMsg(&(p_CtrInfoList->at(0)->buf),haveCompleteMsg,Hunan);
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
        case Wait_Delay_Finish:
        {
            break;
        }
        case Wait_Delay_2_Finish:
        {
            break;
        }
        case Wait_Delay_3_Finish:
        {
            break;
        }
        case Wait_Delay_4_Finish:
        {
            break;
        }
        case Wait_Delay_5_Finish:
        {
            break;
        }
        case Wait_QueryTaskConfig_STA_10F103_Finish:
        {
            break;
        }
        case Wait_00F1_For_05F102_Finish:
        {
            if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到设置路由校时周期，确认回复");
                emScriptRunState=Wait_SetCurveConfig_STA_05F103_Finish;
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_SetCurveConfig_STA_05F103);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--设置STA模块曲线配置（05F103），等待--确认");
                p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case Wait_SetCurveConfig_STA_05F103_Finish:
        {
            if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到设置STA模块曲线配置（05F103），确认回复");
                emScriptRunState=Wait_Delay_Finish;
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "延时2min等待任务配置");
                p_delayTimer->start(2*60*1000);
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case Wait_QueryCurveData_CCO_03F102_Finish:
        {
            if(p_Frame3762Base->afn_ == 0x03&&p_Frame3762Base->dt1_==0x20&&p_Frame3762Base->dt2_==0x0C&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                shared_ptr<Afn03F102> p_QueryCurveData_CCO_03F102_Up=dynamic_pointer_cast<Afn03F102>(p_Frame3762Base);
                if(p_QueryCurveData_CCO_03F102_Up->data_unit_up_.enable_flag==0x01 &&p_QueryCurveData_CCO_03F102_Up->data_unit_up_.sample_cycle==realPeriod)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到设置CCO模块曲线数据（03F102）");
                    emScriptRunState=Wait_SetCurveConfig_STA_05F103_2_Finish;
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_SetCurveConfig_STA_05F103);
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--设置STA模块曲线配置（05F103_oop），等待--确认");
                    p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                }
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case Wait_SetCurveConfig_STA_05F103_2_Finish:
        {
            if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到设置STA模块曲线配置（05F103_oop），确认回复");
                emScriptRunState=Wait_Delay_2_Finish;
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "延时10s等待任务配置");
                p_delayTimer->start(10*1000);
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case Wait_QueryCurveData_CCO_03F102_2_Finish:
        {
            if(p_Frame3762Base->afn_ == 0x03&&p_Frame3762Base->dt1_==0x20&&p_Frame3762Base->dt2_==0x0C&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                shared_ptr<Afn03F102> p_QueryCurveData_CCO_03F102_2_Up=dynamic_pointer_cast<Afn03F102>(p_Frame3762Base);
                if(p_QueryCurveData_CCO_03F102_2_Up->data_unit_up_.enable_flag==0x01 &&p_QueryCurveData_CCO_03F102_2_Up->data_unit_up_.sample_cycle==realPeriod)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到设置CCO模块曲线数据（03F102）");
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("等待%1s,开始查询STA任务配置状态").arg(2));
                    p_delayTimer->start(2*1000);
                    emScriptRunState=Wait_QueryTaskConfig_STA_10F103_Finish;
                }
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case Wait_QueryConfigStatus_STA_10f103_Finish:
        {
            if(p_Frame3762Base->afn_ == 0x10&&p_Frame3762Base->dt1_==0x40&&p_Frame3762Base->dt2_==0x0C&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                shared_ptr<Afn10F103_Hunan> p_QueryConfigStatus_STA_10f103_Up=dynamic_pointer_cast<Afn10F103_Hunan>(p_Frame3762Base);
                index+=node_num;
                for(int n=0;n<p_QueryConfigStatus_STA_10f103_Up->module_config_unit_.node_info_list_.size();n++)
                {
                    if(p_QueryConfigStatus_STA_10f103_Up->module_config_unit_.node_info_list_.at(n).config_state_==1)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("节点%1状态配置成功").arg(QString(QByteArray(p_QueryConfigStatus_STA_10f103_Up->module_config_unit_.node_info_list_.at(n).node_address_.addr,6).toHex())));
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("节点%1状态配置失败").arg(QString(QByteArray(p_QueryConfigStatus_STA_10f103_Up->module_config_unit_.node_info_list_.at(n).node_address_.addr,6).toHex())));
                    }
                }
                if(index<p_QueryConfigStatus_STA_10f103_Up->module_config_unit_.node_total_num_)
                {
                    emScriptRunState=Wait_QueryConfigStatus_STA_10f103_Finish;
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryConfigStatus_STA_10f103);
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询STA模块配置状态（10F103），等待--回复");
                    p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                }
                else
                {
                    emScriptRunState=Wait_Delay_3_Finish;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "延时2min，确保每个节点开始冻结");
                    p_delayTimer->start(2*60*1000);
                }
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case Wait_ReadCurveData_STA_F1F100_Finish:
        {
            if(p_Frame3762Base->afn_ == char(0xF1)&&p_Frame3762Base->dt1_==0x08&&p_Frame3762Base->dt2_==0x0C&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到F1F100上行报文。");
                p_timer->stop();
                shared_ptr<AfnF1F100> p_ReadCurveData_STA_F1F100_Up=dynamic_pointer_cast<AfnF1F100>(p_Frame3762Base);
                Address srcAddr;
                memcpy(srcAddr.addr,p_ReadCurveData_STA_F1F100_Up->address_field_.src_addr,6);

                int currentMeterIndex=getMeterNoIndex(srcAddr);
                if(currentMeterIndex!=-1)
                {
                    if(p_ReadCurveData_STA_F1F100_Up->unit_up_.frame_length_==0)
                        continue;
                    //返回的数据内容，包含多个645或oop
                    QByteArray msgBuf=p_ReadCurveData_STA_F1F100_Up->unit_up_.frame_content_;
                    //判断返回数据类型
                    if(p_ReadCurveData_STA_F1F100_Up->unit_up_.protocol_type_==DLT645_2007)
                    {
                        //循环判断每条645报文
                        while(msgBuf.size()>0)
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "解析收到的上行645报文。");
                            bool res=true;
                            shared_ptr<Frame645Base> MsgBase_645_ptr = Frame645Helper::DecodeLocalMsg(&msgBuf,res);
                            if(MsgBase_645_ptr==nullptr)
                                continue;
                            if(MsgBase_645_ptr->ctrlCode_==NORMAL_RESP)
                            {
                                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到645从站正常应答上行报文。");
                                /////////////
                                shared_ptr<RspsNormal_ReadData_0x91> p_ReadData_0x91=dynamic_pointer_cast<RspsNormal_ReadData_0x91>(MsgBase_645_ptr);
                                if(memcmp(p_ReadData_0x91->addr_,srcAddr.addr,6)!=0)
                                    continue;

//                                    for(int i=0;i<allMeterList.at(currentMeterIndex).dataUnitList.size();i++)//数据项个数
//                                    {
//                                        if(memcmp(allMeterList.at(currentMeterIndex).dataUnitList.at(i).dataIdLen.data_id_,p_ReadData_0x91->di,4)==0)
//                                        {
//                                            QByteArray dateTime=p_ReadData_0x91->data.mid(0,5);//时间
//                                            uchar readNum=uchar(p_ReadData_0x91->data.at(6));  //数据点数
//                                            QList<QByteArray> dataList;
//                                            //取出n点对应时间点的数据
//                                            for(int j=7;p_ReadData_0x91->data.size();j+=allMeterList.at(currentMeterIndex).dataUnitList.at(i).dataIdLen.reply_data_len_)
//                                            {
//                                                QByteArray data=p_ReadData_0x91->data.mid(j,allMeterList.at(currentMeterIndex).dataUnitList.at(i).dataIdLen.reply_data_len_);
//                                                dataList.append(data);
//                                            }
//                                            //当前返回数据项确认
//    //                                        for(int j=0;j<allMeterList.at(currentMeterIndex).dataUnitList.at(i).roundList.size();j++)
//                                            {
//                                                //数据时间相等,,,,,,不判定时间
//    //                                            if(memcmp(allMeterList.at(currentMeterIndex).dataUnitList.at(i).roundList.at(j).dateTime,dateTime,5)==0)
//                                                {
//                                                    //判定所有的数据点的数据正确
//                                                    for(int n=0;n<readNum;n++)
//                                                    {
//                                                        //返回数据存储
//    //                                                    allMeterList[currentMeterIndex].dataUnitList[i].roundList[j+n].replyData=dataList.at(n);
//                                                        allMeterList[currentMeterIndex].dataUnitList[i].roundList[n].replyData=dataList.at(n);
//                                                        //判断返回数据是否为全f
//                                                        if(isNotReplyEmpty(dataList.at(n))==true)
//    //                                                        allMeterList[currentMeterIndex].dataUnitList[i].roundList[j+n].readFlag=true;
//                                                            allMeterList[currentMeterIndex].dataUnitList[i].roundList[n].readFlag=true;
//                                                        else
//                                                            p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("返回数据为空，曲线抄读数据失败！"));
//                                                    }
//                                                    break;
//                                                }
//                                            }
//    //                                        break;
//                                        }
//                                        else
//                                            continue;
//                                    }
                                ////////////////
                            }
                            else if(MsgBase_645_ptr->ctrlCode_==AbNORMAL_RESP_ReadData)
                            {
                                p_AbstractScriptHost->updateProgress(ProcessState_Failed, "收到从站异常应答。");
                                shared_ptr<RspsAbNormal_ReadData_0xD1> p_RspsAbNormal_0xD1=dynamic_pointer_cast<RspsAbNormal_ReadData_0xD1>(MsgBase_645_ptr);
                                if(memcmp(p_RspsAbNormal_0xD1->addr_,srcAddr.addr,6)!=0)
                                    continue;
                            }
                            else
                                continue;
                        }
                    }
                    else if(p_ReadCurveData_STA_F1F100_Up->unit_up_.protocol_type_==OOP)
                    {
                        //循环判断每条oop报文
                        while(msgBuf.size()>0)
                        {
                            bool res=true;
                            shared_ptr<FrameOOPBase> MsgBase_OOP_ptr = FrameOOPHelper::DecodeMsg(&msgBuf,res);
                            if(MsgBase_OOP_ptr==nullptr)
                                continue;
                            if(0!=memcmp(QByteArray::fromHex(MsgBase_OOP_ptr->address_field_.sa.address.toLatin1()),allMeterList.at(currentMeterIndex).meterNo.addr,6))
                                continue;
                            if(MsgBase_OOP_ptr->service_type_!=GET_RESPONSE_SERVER && MsgBase_OOP_ptr->service_sub_type_!=uchar(GetResponseType::kGetResponseNormal))
                                continue;
                            shared_ptr<GetResponseNormal> p_GetResponseNormal=dynamic_pointer_cast<GetResponseNormal>(MsgBase_OOP_ptr);
//                            待修改？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？
                            for(int i=0;i<allMeterList.at(currentMeterIndex).dataUnitList.size();i++)
                            {
                                char oi[2];
                                oi[0]=allMeterList.at(currentMeterIndex).dataUnitList.at(i).dataIdLen.data_id_[0];
                                oi[1]=allMeterList.at(currentMeterIndex).dataUnitList.at(i).dataIdLen.data_id_[1];

                                OAD oad;
                                oad.OI=uchar(oi[1])+ushort(short(oi[0])<<8);
                                oad.attribute.feature=(allMeterList.at(currentMeterIndex).dataUnitList.at(i).dataIdLen.data_id_[2]>>4)&0x0f;
                                oad.attribute.seq=allMeterList.at(currentMeterIndex).dataUnitList.at(i).dataIdLen.data_id_[2]&0x0f;
                                oad.element_index=uchar(allMeterList.at(currentMeterIndex).dataUnitList.at(i).dataIdLen.data_id_[3]);

                                if(p_GetResponseNormal->a_result_normal_.oad==oad)
                                {
//                                    allMeterList[currentMeterIndex].dataUnitList[i].successRound++;
                                    if(p_GetResponseNormal->a_result_normal_.get_result_ptr->choice_!=GetResultChoice(0))
                                    {
//                                        p_GetResponseNormal->follow_report_field_.optional_
//                                        p_GetResponseNormal->follow_report_field_.follow_report_
//                                        p_GetResponseNormal->follow_report_field_.
                                        std::shared_ptr<FollowReportNormal> value=dynamic_pointer_cast<FollowReportNormal>(p_GetResponseNormal);

                                        std::shared_ptr<GetResultData> value_ptr_=dynamic_pointer_cast<GetResultData>(p_GetResponseNormal);

                                        std::shared_ptr<DataBasic> value_ptr=dynamic_pointer_cast<DataBasic>(p_GetResponseNormal);
//                                        value_ptr_->value_ptr_.
                                        value_ptr->data_.size();

//                                        QByteArray data=p_GetResponseNormal->a_result_normal_.get_result_ptr;
//                                        AResultNormal::DecodeFrame(QByteArray *data)
                                    }
                                    break;
                                }
                                else
                                    continue;
                            }
                        }
                    }
                }

                parallelCount--;
                if(allMeterList[currentMeterIndex].dataIDNumIndex<MaxDataIDNum) //回复的表冻结项如果没抄读完，继续抄读
                {
                    emScriptRunState=Wait_ReadCurveData_STA_F1F100_Finish;
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,currentMeterIndex,p_ReadCurveData_STA_F1F100);
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--并发抄读STA曲线数据（F1F100），等待--回复");
                    p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
                    parallelCount++;
                }
                else if(index<allMeterList.size())//如果还有下一个表，继续抄读下一项电表的数据项
                {
                    emScriptRunState=Wait_ReadCurveData_STA_F1F100_Finish;
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,index,p_ReadCurveData_STA_F1F100);
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--并发抄读STA曲线数据（F1F100），等待--回复");
                    p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
                    parallelCount++;
                    index++;

                    allMeterList[currentMeterIndex].dataIDNumIndex=0;//对抄读完毕的电表的数据项索引复位
                }
                else
                {
                    allMeterList[currentMeterIndex].dataIDNumIndex=0;

                    if(parallelCount==0)//等待所有数据抄读完毕
                    {
                        pointNumIndex+=pointNum;//已经读完的点数
                        //更新下次读取点数的开始时间
                        curTime=QByteArray::fromHex(startDateTime.addSecs(pointNumIndex*60).toString("mmhhddMMyy").toLatin1());
                        if(pointNumIndex<MaxPointNum)//判断当前的抄读数据点数
                        {
                            index=0;
                            parallelCount=0;
                            while(parallelCount<maxParallelNum)
                            {
                                if(index>=allMeterList.size())
                                    break;
                                emScriptRunState=Wait_ReadCurveData_STA_F1F100_Finish;
                                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,index,p_ReadCurveData_STA_F1F100);
                                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--并发抄读STA曲线数据（F1F100_2），等待--回复");
                                p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
                                parallelCount++;
                                index++;
                           }
                        }
                        else
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "F1F100抄读曲线数据结束！");
                            index=0;

                            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_HardReset_01F1);
                            emScriptRunState=Wait_HardResetTest_Finish;
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--路由硬件复位（00F1），等待--确认");
                            p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                        }
                    }
                }
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case Wait_HardResetTest_Finish:
        {
            if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                emScriptRunState=Wait_QueryNetScale_10F9_Finish;
                p_delayTimer->start(30*1000);
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case Wait_QueryNetScale_10F9_2_Finish:
        {
            if(p_Frame3762Base->afn_ == 0x10&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x01&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();//接收到一条完整报文
                shared_ptr<Afn10F9> p_QueryNetScale_10F9_Up=dynamic_pointer_cast<Afn10F9>(p_Frame3762Base);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("当前网络规模为%1").arg(p_QueryNetScale_10F9_Up->network_scale_));
                if(p_QueryNetScale_10F9_Up->network_scale_==p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size()+1)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "组网完成");
                    emScriptRunState=Wait_Delay_5_Finish;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "延时2min，确保每个节点开始冻结");
                    p_delayTimer->start(2*60*1000);
                }
                else
                {
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryNetScale_10F9);
                    emScriptRunState=Wait_QueryNetScale_10F9_2_Finish;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询网络规模（10F9），等待--回复");
                    p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
                }
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case Wait_ReadCurveData_STA_F1F100_2_Finish:
        {
            if(p_Frame3762Base->afn_ == char(0xF1)&&p_Frame3762Base->dt1_==0x08&&p_Frame3762Base->dt2_==0x0C&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到F1F100上行报文。");
                p_timer->stop();
                shared_ptr<AfnF1F100> p_ReadCurveData_STA_F1F100_Up=dynamic_pointer_cast<AfnF1F100>(p_Frame3762Base);
                Address srcAddr;
                memcpy(srcAddr.addr,p_ReadCurveData_STA_F1F100_Up->address_field_.src_addr,6);

                int currentMeterIndex=getMeterNoIndex(srcAddr);
                if(currentMeterIndex!=-1)
                {
                    if(p_ReadCurveData_STA_F1F100_Up->unit_up_.frame_length_==0)
                        continue;
                    //返回的数据内容，包含多个645或oop
                    QByteArray msgBuf=p_ReadCurveData_STA_F1F100_Up->unit_up_.frame_content_;
                    //判断返回数据类型
                    if(p_ReadCurveData_STA_F1F100_Up->unit_up_.protocol_type_==DLT645_2007)
                    {
                        //循环判断每条645报文
                        while(msgBuf.size()>0)
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "解析收到的上行645报文。");
                            bool res=true;
                            shared_ptr<Frame645Base> MsgBase_645_ptr = Frame645Helper::DecodeLocalMsg(&msgBuf,res);
                            if(MsgBase_645_ptr==nullptr)
                                continue;
                            if(MsgBase_645_ptr->ctrlCode_==NORMAL_RESP)
                            {
                                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到645从站正常应答上行报文。");
                                /////////////
                                shared_ptr<RspsNormal_ReadData_0x91> p_ReadData_0x91=dynamic_pointer_cast<RspsNormal_ReadData_0x91>(MsgBase_645_ptr);
                                if(memcmp(p_ReadData_0x91->addr_,srcAddr.addr,6)!=0)
                                    continue;
                                ////////////////
                            }
                            else if(MsgBase_645_ptr->ctrlCode_==AbNORMAL_RESP_ReadData)
                            {
                                p_AbstractScriptHost->updateProgress(ProcessState_Failed, "收到从站异常应答。");
                                shared_ptr<RspsAbNormal_ReadData_0xD1> p_RspsAbNormal_0xD1=dynamic_pointer_cast<RspsAbNormal_ReadData_0xD1>(MsgBase_645_ptr);
                                if(memcmp(p_RspsAbNormal_0xD1->addr_,srcAddr.addr,6)!=0)
                                    continue;
                            }
                            else
                                continue;
                        }
                    }
                    else if(p_ReadCurveData_STA_F1F100_Up->unit_up_.protocol_type_==OOP)
                    {
                        //循环判断每条oop报文
                        while(msgBuf.size()>0)
                        {
                            bool res=true;
                            shared_ptr<FrameOOPBase> MsgBase_OOP_ptr = FrameOOPHelper::DecodeMsg(&msgBuf,res);
                            if(MsgBase_OOP_ptr==nullptr)
                                continue;
                            if(0!=memcmp(QByteArray::fromHex(MsgBase_OOP_ptr->address_field_.sa.address.toLatin1()),allMeterList.at(currentMeterIndex).meterNo.addr,6))
                                continue;
                            if(MsgBase_OOP_ptr->service_type_!=GET_RESPONSE_SERVER && MsgBase_OOP_ptr->service_sub_type_!=uchar(GetResponseType::kGetResponseNormal))
                                continue;
                            shared_ptr<GetResponseNormal> p_GetResponseNormal=dynamic_pointer_cast<GetResponseNormal>(MsgBase_OOP_ptr);
//                            待修改？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？
                            for(int i=0;i<allMeterList.at(currentMeterIndex).dataUnitList.size();i++)
                            {
                                char oi[2];
                                oi[0]=allMeterList.at(currentMeterIndex).dataUnitList.at(i).dataIdLen.data_id_[0];
                                oi[1]=allMeterList.at(currentMeterIndex).dataUnitList.at(i).dataIdLen.data_id_[1];

                                OAD oad;
                                oad.OI=uchar(oi[1])+ushort(short(oi[0])<<8);
                                oad.attribute.feature=(allMeterList.at(currentMeterIndex).dataUnitList.at(i).dataIdLen.data_id_[2]>>4)&0x0f;
                                oad.attribute.seq=allMeterList.at(currentMeterIndex).dataUnitList.at(i).dataIdLen.data_id_[2]&0x0f;
                                oad.element_index=uchar(allMeterList.at(currentMeterIndex).dataUnitList.at(i).dataIdLen.data_id_[3]);

                                if(p_GetResponseNormal->a_result_normal_.oad==oad)
                                {
//                                    allMeterList[currentMeterIndex].dataUnitList[i].successRound++;
                                    if(p_GetResponseNormal->a_result_normal_.get_result_ptr->choice_!=GetResultChoice(0))
                                    {
//                                        p_GetResponseNormal->follow_report_field_.optional_
//                                        p_GetResponseNormal->follow_report_field_.follow_report_
//                                        p_GetResponseNormal->follow_report_field_.
                                        std::shared_ptr<FollowReportNormal> value=dynamic_pointer_cast<FollowReportNormal>(p_GetResponseNormal);

                                        std::shared_ptr<GetResultData> value_ptr_=dynamic_pointer_cast<GetResultData>(p_GetResponseNormal);

                                        std::shared_ptr<DataBasic> value_ptr=dynamic_pointer_cast<DataBasic>(p_GetResponseNormal);
//                                        value_ptr_->value_ptr_.
                                        value_ptr->data_.size();

//                                        QByteArray data=p_GetResponseNormal->a_result_normal_.get_result_ptr;
//                                        AResultNormal::DecodeFrame(QByteArray *data)
                                    }
                                    break;
                                }
                                else
                                    continue;
                            }
                        }
                    }
                }

                parallelCount--;
                if(allMeterList[currentMeterIndex].dataIDNumIndex<MaxDataIDNum) //回复的表冻结项如果没抄读完，继续抄读
                {
                    emScriptRunState=Wait_ReadCurveData_STA_F1F100_2_Finish;
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,currentMeterIndex,p_ReadCurveData_STA_F1F100);
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--并发抄读STA曲线数据（F1F100），等待--回复");
                    p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
                    parallelCount++;
                }
                else if(index<allMeterList.size())//如果还有下一个表，继续抄读下一项电表的数据项
                {
                    emScriptRunState=Wait_ReadCurveData_STA_F1F100_2_Finish;
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,index,p_ReadCurveData_STA_F1F100);
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--并发抄读STA曲线数据（F1F100），等待--回复");
                    p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
                    parallelCount++;
                    index++;

                    allMeterList[currentMeterIndex].dataIDNumIndex=0;//对抄读完毕的电表的数据项索引复位
                }
                else
                {
                    allMeterList[currentMeterIndex].dataIDNumIndex=0;

                    if(parallelCount==0)//等待所有数据抄读完毕
                    {
                        pointNumIndex+=pointNum;//已经读完的点数
                        //更新下次读取点数的开始时间
                        curTime=QByteArray::fromHex(startDateTime.addSecs(pointNumIndex*60).toString("mmhhddMMyy").toLatin1());
                        if(pointNumIndex<MaxPointNum)//判断当前的抄读数据点数
                        {
                            index=0;
                            parallelCount=0;
                            while(parallelCount<maxParallelNum)
                            {
                                if(index>=allMeterList.size())
                                    break;
                                emScriptRunState=Wait_ReadCurveData_STA_F1F100_2_Finish;
                                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,index,p_ReadCurveData_STA_F1F100);
                                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--并发抄读STA曲线数据（F1F100_2），等待--回复");
                                p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
                                parallelCount++;
                                index++;
                           }
                        }
                        else
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "F1F100抄读曲线数据结束！");
                            index=0;
                            emScriptRunState=Wait_SetCurveConfig_STA_05F103_3_Finish;
                            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_SetCurveConfig_STA_05F103);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--设置STA模块曲线配置（05F103_645_00），等待--确认");
                            p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                        }
                    }
                }
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case Wait_SetCurveConfig_STA_05F103_3_Finish:
        {
            if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到设置STA模块曲线配置（05F103_645_00），确认回复");

                emScriptRunState=Wait_SetCurveConfig_STA_05F103_4_Finish;
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_SetCurveConfig_STA_05F103);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--设置STA模块曲线配置（05F103_oop_00），等待--确认");
                p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case Wait_SetCurveConfig_STA_05F103_4_Finish:
        {
            if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                readInfoClear();
                resultFlag=true;
                emScriptRunState=ScriptSuccess;
                p_AbstractScriptHost->updateProgress(ProcessState_Success, QString("测试成功"));
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
void Script_MinuteFreeze_Reset_Hunan::processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID)
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
void Script_MinuteFreeze_Reset_Hunan::processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID)
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
void Script_MinuteFreeze_Reset_Hunan::sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame)
{
    if(frame==p_SetRouterTimePeriod_05F102)
    {
        p_SetRouterTimePeriod_05F102->ctrl_field_.dir=kDirDown;
        p_SetRouterTimePeriod_05F102->ctrl_field_.prm=kActive;
        p_SetRouterTimePeriod_05F102->ctrl_field_.comn_type=kHplc;

        p_SetRouterTimePeriod_05F102->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_SetRouterTimePeriod_05F102->info_field_.info_field_down.comu_rate=0;
        p_SetRouterTimePeriod_05F102->info_field_.info_field_down.comu_module_ident=0;

        p_SetRouterTimePeriod_05F102->period_value_=0x01;
        p_SetRouterTimePeriod_05F102->period_unit_=0x02;

        sendMsgOct=p_SetRouterTimePeriod_05F102->EncodeFrame();
        sendMsgLog=QString("》》设置路由校时周期05F102：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_QueryNetScale_10F9)
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
    else if(frame==p_HardReset_01F1)
    {
        p_HardReset_01F1->ctrl_field_.dir=kDirDown;
        p_HardReset_01F1->ctrl_field_.prm=kActive;
        p_HardReset_01F1->ctrl_field_.comn_type=kHplc;

        p_HardReset_01F1->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_HardReset_01F1->info_field_.info_field_down.comu_rate=0;
        p_HardReset_01F1->info_field_.info_field_down.comu_module_ident=0;

        sendMsgOct=p_HardReset_01F1->EncodeFrame();
        sendMsgLog=QString("》》路由硬件复位01F1：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_SetCurveConfig_STA_05F103)
    {
        p_SetCurveConfig_STA_05F103->ctrl_field_.dir=kDirDown;
        p_SetCurveConfig_STA_05F103->ctrl_field_.prm=kActive;
        p_SetCurveConfig_STA_05F103->ctrl_field_.comn_type=kHplc;

        p_SetCurveConfig_STA_05F103->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_SetCurveConfig_STA_05F103->info_field_.info_field_down.comu_rate=0;
        p_SetCurveConfig_STA_05F103->info_field_.info_field_down.comu_module_ident=0;

        p_SetCurveConfig_STA_05F103->curve_config_data_.read_period_=realPeriod;

        if(emScriptRunState==Wait_SetCurveConfig_STA_05F103_Finish||emScriptRunState==Wait_SetCurveConfig_STA_05F103_3_Finish)//配置645表，单三相，冻结01或禁止冻结00
        {
            if(emScriptRunState==Wait_SetCurveConfig_STA_05F103_Finish)
            {
                p_SetCurveConfig_STA_05F103->curve_config_data_.set_flag_=0x01;
            }
            else if(emScriptRunState==Wait_SetCurveConfig_STA_05F103_3_Finish)
            {
                p_SetCurveConfig_STA_05F103->curve_config_data_.set_flag_=0x00;
            }
//            p_SetCurveConfig_STA_05F103->curve_config_data_.read_period_=1;
            p_SetCurveConfig_STA_05F103->curve_config_data_.protocol=DLT645_2007;
            //单相表
            p_SetCurveConfig_STA_05F103->curve_config_data_.single_meter_=0x00;
            p_SetCurveConfig_STA_05F103->curve_config_data_.single_data_id_num_=uchar(MaxDataIDNum);

            p_SetCurveConfig_STA_05F103->curve_config_data_.single_data_id_len_list_.clear();
            p_SetCurveConfig_STA_05F103->curve_config_data_.single_reply_data_len_=0;
            for(int i=0;i<MaxDataIDNum;i++)
            {
                memcpy(dataID.data_id_,singleDataId_645_List.at(i).data_id_,4);
                dataID.reply_data_len_=singleDataId_645_List.at(i).reply_data_len_;
                p_SetCurveConfig_STA_05F103->curve_config_data_.single_data_id_len_list_.append(dataID);
                p_SetCurveConfig_STA_05F103->curve_config_data_.single_reply_data_len_+=singleDataId_645_List.at(i).reply_data_len_;
            }
            //三相表
            p_SetCurveConfig_STA_05F103->curve_config_data_.tri_meter_=0x01;
            p_SetCurveConfig_STA_05F103->curve_config_data_.tri_data_id_num_=uchar(MaxDataIDNum);

            p_SetCurveConfig_STA_05F103->curve_config_data_.tri_data_id_len_list_.clear();
            p_SetCurveConfig_STA_05F103->curve_config_data_.tri_reply_data_len_=0;
            for(int i=0;i<MaxDataIDNum;i++)
            {
                memcpy(dataID.data_id_,triDataId_645_List.at(i).data_id_,4);
                dataID.reply_data_len_=triDataId_645_List.at(i).reply_data_len_;
                p_SetCurveConfig_STA_05F103->curve_config_data_.tri_data_id_len_list_.append(dataID);
                p_SetCurveConfig_STA_05F103->curve_config_data_.tri_reply_data_len_+=triDataId_645_List.at(i).reply_data_len_;
            }
        }
        else if(emScriptRunState==Wait_SetCurveConfig_STA_05F103_2_Finish||emScriptRunState==Wait_SetCurveConfig_STA_05F103_4_Finish)//配置oop表，单三相，冻结01
        {
            if(emScriptRunState==Wait_SetCurveConfig_STA_05F103_2_Finish)
            {
                p_SetCurveConfig_STA_05F103->curve_config_data_.set_flag_=0x01;
            }
            else if(emScriptRunState==Wait_SetCurveConfig_STA_05F103_4_Finish)
            {
                p_SetCurveConfig_STA_05F103->curve_config_data_.set_flag_=0x00;
            }

            p_SetCurveConfig_STA_05F103->curve_config_data_.protocol=OOP;
            //单相表
            p_SetCurveConfig_STA_05F103->curve_config_data_.single_meter_=0x00;
            p_SetCurveConfig_STA_05F103->curve_config_data_.single_data_id_num_=uchar(MaxDataIDNum);

            p_SetCurveConfig_STA_05F103->curve_config_data_.single_data_id_len_list_.clear();
            p_SetCurveConfig_STA_05F103->curve_config_data_.single_reply_data_len_=0;
            for(int i=0;i<MaxDataIDNum;i++)
            {
                memcpy(dataID.data_id_,singleDataId_OOP_List.at(i).data_id_,4);
                dataID.reply_data_len_=singleDataId_OOP_List.at(i).reply_data_len_;
                p_SetCurveConfig_STA_05F103->curve_config_data_.single_data_id_len_list_.append(dataID);
                p_SetCurveConfig_STA_05F103->curve_config_data_.single_reply_data_len_+=singleDataId_OOP_List.at(i).reply_data_len_;
            }
            //三相表
            p_SetCurveConfig_STA_05F103->curve_config_data_.tri_meter_=0x01;
            p_SetCurveConfig_STA_05F103->curve_config_data_.tri_data_id_num_=uchar(MaxDataIDNum);

            p_SetCurveConfig_STA_05F103->curve_config_data_.tri_data_id_len_list_.clear();
            p_SetCurveConfig_STA_05F103->curve_config_data_.tri_reply_data_len_=0;
            for(int i=0;i<MaxDataIDNum;i++)
            {
                memcpy(dataID.data_id_,triDataId_OOP_List.at(i).data_id_,4);
                dataID.reply_data_len_=triDataId_OOP_List.at(i).reply_data_len_;
                p_SetCurveConfig_STA_05F103->curve_config_data_.tri_data_id_len_list_.append(dataID);
                p_SetCurveConfig_STA_05F103->curve_config_data_.tri_reply_data_len_+=triDataId_OOP_List.at(i).reply_data_len_;
            }
        }
        sendMsgOct=p_SetCurveConfig_STA_05F103->EncodeFrame();
        sendMsgLog=QString("》》设置STA模块曲线配置05F103：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_QueryCurveData_CCO_03F102)
    {
        p_QueryCurveData_CCO_03F102->ctrl_field_.dir=kDirDown;
        p_QueryCurveData_CCO_03F102->ctrl_field_.prm=kActive;
        p_QueryCurveData_CCO_03F102->ctrl_field_.comn_type=kHplc;

        p_QueryCurveData_CCO_03F102->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_QueryCurveData_CCO_03F102->info_field_.info_field_down.comu_rate=0;
        p_QueryCurveData_CCO_03F102->info_field_.info_field_down.comu_module_ident=0;

        if(emScriptRunState==Wait_QueryCurveData_CCO_03F102_Finish)
        {
            p_QueryCurveData_CCO_03F102->protocol_=DLT645_2007;
        }
        else if(emScriptRunState==Wait_QueryCurveData_CCO_03F102_2_Finish)
        {
            p_QueryCurveData_CCO_03F102->protocol_=OOP;
        }

        sendMsgOct=p_QueryCurveData_CCO_03F102->EncodeFrame();
        sendMsgLog=QString("》》查询CCO模块曲线数据03F102：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_QueryConfigStatus_STA_10f103)
    {
        p_QueryConfigStatus_STA_10f103->ctrl_field_.dir=kDirDown;
        p_QueryConfigStatus_STA_10f103->ctrl_field_.prm=kActive;
        p_QueryConfigStatus_STA_10f103->ctrl_field_.comn_type=kHplc;

        p_QueryConfigStatus_STA_10f103->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_QueryConfigStatus_STA_10f103->info_field_.info_field_down.comu_rate=0;
        p_QueryConfigStatus_STA_10f103->info_field_.info_field_down.comu_module_ident=0;

        if((index+node_num)<=times)
        {
            p_QueryConfigStatus_STA_10f103->node_num_=node_num;
            p_QueryConfigStatus_STA_10f103->node_start_no_=index;
        }
        else
        {
            p_QueryConfigStatus_STA_10f103->node_num_=(times%node_num==0)?node_num:(times%node_num);
            p_QueryConfigStatus_STA_10f103->node_start_no_=index;
        }

        sendMsgOct=p_QueryConfigStatus_STA_10f103->EncodeFrame();
        sendMsgLog=QString("》》查询STA模块配置状态10F103：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_ReadCurveData_STA_F1F100)
    {
        p_ReadCurveData_STA_F1F100->ctrl_field_.dir=kDirDown;
        p_ReadCurveData_STA_F1F100->ctrl_field_.prm=kActive;
        p_ReadCurveData_STA_F1F100->ctrl_field_.comn_type=kHplc;

        p_ReadCurveData_STA_F1F100->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_ReadCurveData_STA_F1F100->info_field_.info_field_down.comu_rate=0;
        p_ReadCurveData_STA_F1F100->info_field_.info_field_down.comu_module_ident=1;

        uchar tmpAddr[6];
//        memcpy(tmpAddr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(index)->mtrAddr,6);
        memcpy(tmpAddr,allMeterList.at(meterID).meterNo.addr,6);

        uchar tmpCcoAddr[6];
        memcpy(tmpCcoAddr,p_CtrInfoList->at(0)->ccoAddr,6);
        memcpy(p_ReadCurveData_STA_F1F100->address_field_.src_addr,tmpCcoAddr,6);
        memcpy(p_ReadCurveData_STA_F1F100->address_field_.dst_addr,tmpAddr,6);

        if(allMeterList.at(meterID).protocolType==DLT645_2007)
        {
            shared_ptr<Rqst_ReadData_0x11> p_Rqst_ReadData_0x11=make_shared<Rqst_ReadData_0x11>(addr,4,0);
            memcpy(p_Rqst_ReadData_0x11->addr_,tmpAddr,6);
            QByteArray msg645;
//            if(emScriptRunState==Wait_ReadCurveData_STA_F1F100_Finish)
//            {
                msg645.clear();
                for(int i=0;i<dataIDNum;i++)//每次抄读数据项数
                {
                    if(allMeterList.at(meterID).dataIDNumIndex<MaxDataIDNum)
                    {

                        p_Rqst_ReadData_0x11->m_DataLen=0x06;
                        memcpy(p_Rqst_ReadData_0x11->di,allMeterList.at(meterID).dataUnitList.at(allMeterList[meterID].dataIDNumIndex).dataIdLen.data_id_,4);//变量
                        allMeterList[meterID].dataIDNumIndex+=1;

                        memcpy(p_Rqst_ReadData_0x11->dateTime,curTime,5);

                        if(pointNumIndex+pointNum<=MaxPointNum)
                        {
                            p_Rqst_ReadData_0x11->blockOfRcrd=uchar(pointNum);
                        }
                        else
                        {
                            p_Rqst_ReadData_0x11->blockOfRcrd=uchar(MaxPointNum%pointNum);
                        }
//                        p_Rqst_ReadData_0x11->blockOfRcrd=uchar(pointNum);

                        QByteArray dataId=p_Rqst_ReadData_0x11->EncodeFrame();
                        msg645.append(dataId);

                    }
                    else
                    {
                        break;
                    }


//                    p_Rqst_ReadData_0x11->m_DataLen=0x06;
//                    memcpy(p_Rqst_ReadData_0x11->di,allMeterList.at(index).dataUnitList.at(i).dataIdLen.data_id_,4);//变量
//                    memcpy(p_Rqst_ReadData_0x11->dateTime,curTime,5);
//                    p_Rqst_ReadData_0x11->blockOfRcrd=uchar(pointNum);
//                    QByteArray dataId=p_Rqst_ReadData_0x11->EncodeFrame();
//                    msg645.append(dataId);
                }
//            }
            p_ReadCurveData_STA_F1F100->unit_down_.protocol_type_=0X02;
            p_ReadCurveData_STA_F1F100->unit_down_.preserve_=0X00;
            p_ReadCurveData_STA_F1F100->unit_down_.frame_content_=msg645;
            p_ReadCurveData_STA_F1F100->unit_down_.frame_length_=uchar(msg645.size());

            sendMsgOct=p_ReadCurveData_STA_F1F100->EncodeFrame();
            sendMsgLog=QString("》》并发抄读曲线数据F1F100_645：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
        }

        else if(allMeterList.at(index).protocolType==OOP)
        {
            shared_ptr<GetRequestRecord> p_GetRequestNormal_ReadData=make_shared<GetRequestRecord>();
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

//            p_GetRequestNormal_ReadData->oad_.OI = PosActEne_OI;
//            p_GetRequestNormal_ReadData->oad_.attribute.feature = 0;
//            p_GetRequestNormal_ReadData->oad_.attribute.seq = 2;//低四位
//            p_GetRequestNormal_ReadData->oad_.element_index = 0;

            p_GetRequestNormal_ReadData->piid_.reserve = 0;
            p_GetRequestNormal_ReadData->piid_.serve_priority = 0;
            p_GetRequestNormal_ReadData->piid_.serve_seq = 5;

            p_GetRequestNormal_ReadData->time_tag_field_.optional_ = 0;

            p_GetRequestNormal_ReadData->get_record_.oad.OI=0x5002;
            p_GetRequestNormal_ReadData->get_record_.oad.attribute.feature=0x00;
            p_GetRequestNormal_ReadData->get_record_.oad.attribute.seq=2;
            p_GetRequestNormal_ReadData->get_record_.oad.element_index=0x00;


            //-----rsd
            p_GetRequestNormal_ReadData->get_record_.rsd_ptr->choice_=kSelector2;

            shared_ptr<RsdSelector2> rsd=make_shared<RsdSelector2>();
            //oad
            rsd->oad_.OI=0x2021;
            rsd->oad_.attribute.feature=0;
            rsd->oad_.attribute.seq=2;
            rsd->oad_.element_index=0;
            //data
            shared_ptr<DataBasic> startTime=make_shared<DataBasic>();
            startTime->type_=kDate_time_s;
            startTime->data_=curTime_start_oop;
            rsd->start_value_ptr_=startTime;

            shared_ptr<DataBasic> endTime=make_shared<DataBasic>();
            endTime->type_=kDate_time_s;
            endTime->data_=curTime_end_oop;
            rsd->end_value_ptr_=endTime;
            //data_TI
            QByteArray data;
            data.append(char(0x01));
            data.append(char(realPeriod>>8));
            data.append(char(realPeriod));

            shared_ptr<DataBasic> dataTI=make_shared<DataBasic>();
            dataTI->type_=kTI;
            dataTI->data_=data;
            rsd->interval_value_ptr_=dataTI;

//            QByteArray dataRsd=rsd->EncodeFrame();
//            p_GetRequestNormal_ReadData->get_record_.rcsd_ptr->GetCsdSize();

            shared_ptr<RCSD> rcsd=make_shared<RCSD>();

            shared_ptr<CsdOad> oad=make_shared<CsdOad>();
            for(int i=0;i<dataIDNum;i++)
            {
                oad->choice_=CsdChoice(0);

                char oi[2];
                oi[0]=allMeterList.at(index).dataUnitList.at(i).dataIdLen.data_id_[0];
                oi[1]=allMeterList.at(index).dataUnitList.at(i).dataIdLen.data_id_[1];
                oad->oad_.OI=uchar(oi[1])+ushort(short(oi[0])<<8);//QByteArray::fromHex(oi).toHex().toUShort();
                oad->oad_.attribute.feature = (allMeterList.at(index).dataUnitList.at(i).dataIdLen.data_id_[2]>>4)&0x0f;
                oad->oad_.attribute.seq = allMeterList.at(index).dataUnitList.at(i).dataIdLen.data_id_[2]&0x0f;
                oad->oad_.element_index = uchar(allMeterList.at(index).dataUnitList.at(i).dataIdLen.data_id_[3]);
//                QByteArray dataOad=oad->EncodeFrame();
                rcsd->list_csd_.append(oad);
//                rcsd->list_csd_.append(dataOad);
            }
//            QByteArray dataRcsd=rcsd->EncodeFrame();
            p_GetRequestNormal_ReadData->get_record_.rsd_ptr=rsd;
            p_GetRequestNormal_ReadData->get_record_.rcsd_ptr=rcsd;

            QByteArray msg698=p_GetRequestNormal_ReadData->EncodeFrame();

            p_ReadCurveData_STA_F1F100->unit_down_.preserve_=0x00;
            p_ReadCurveData_STA_F1F100->unit_down_.protocol_type_=0x03;
            p_ReadCurveData_STA_F1F100->unit_down_.frame_content_=msg698;
            p_ReadCurveData_STA_F1F100->unit_down_.frame_length_=uchar(msg698.size());

            sendMsgOct=p_ReadCurveData_STA_F1F100->EncodeFrame();
            sendMsgLog=QString("》》并发抄读曲线数据F1F100_oop：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
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
void Script_MinuteFreeze_Reset_Hunan::sendSrcMsg(DvcType dvcType, int dvcId, QByteArray msg)
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
void Script_MinuteFreeze_Reset_Hunan::timer_timeoutProc()
{
    switch(emScriptRunState)
    {
        case Wait_BuildNetFinish_Whole:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_BuildNetFinish_Whole timeout!!!");
            break;
        }
        case Wait_00F1_For_05F102_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_QueryAfn_Finish timeout!!!");
            break;
        }
        default:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==p_timer");
            break;
        }
    }
}
void Script_MinuteFreeze_Reset_Hunan::maxAllowTimer_timeoutProc()
{
    switch(emScriptRunState)
    {
        case Wait_00F1_For_05F102_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_00F1_For_05F12_Finish timeout!!!");
            break;
        }
        default:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==p_maxAllowTimer");
            break;
        }
    }
}
void Script_MinuteFreeze_Reset_Hunan::delayTimer_timeoutProc()
{
    p_delayTimer->stop();
    switch(emScriptRunState)
    {
    case TestInit:
    {
        break;
    }
    case Wait_00F1_For_05F102_Finish:
    {
        break;
    }
    case Wait_SetCurveConfig_STA_05F103_Finish:
    {
        break;
    }
    case Wait_QueryCurveData_CCO_03F102_Finish:
    {
        break;
    }
    case Wait_SetCurveConfig_STA_05F103_2_Finish:
    {
        break;
    }
    case Wait_QueryCurveData_CCO_03F102_2_Finish:
    {
        break;
    }
    case Wait_QueryConfigStatus_STA_10f103_Finish:
    {
        break;
    }
    case Wait_ReadCurveData_STA_F1F100_Finish:
    {
        break;
    }
    case Wait_SetCurveConfig_STA_05F103_3_Finish:
    {
        break;
    }
    case Wait_SetCurveConfig_STA_05F103_4_Finish:
    {
        break;
    }
    case Wait_ReadCurveData_STA_F1F100_2_Finish:
    {
        break;
    }
    case ScriptSuccess:
    {
        break;
    }
    case Wait_QueryTaskConfig_STA_10F103_Finish:
    {
        index=1;
        times=ushort(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size());
        emScriptRunState=Wait_QueryConfigStatus_STA_10f103_Finish;
        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryConfigStatus_STA_10f103);
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询STA模块配置状态（10F103），等待--回复");
        p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
        break;
    }
    case Wait_BuildNetFinish_Whole:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_BuildNetFinish_Whole timeout!!!");
        break;
    }
    case Wait_Delay_Finish:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "延时时间到");
        emScriptRunState=Wait_QueryCurveData_CCO_03F102_Finish;
        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryCurveData_CCO_03F102);
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询CCO模块曲线数据（03F102），等待--回复");
        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
        break;
    }
    case Wait_Delay_2_Finish:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "延时时间到");
        emScriptRunState=Wait_QueryCurveData_CCO_03F102_2_Finish;
        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryCurveData_CCO_03F102);
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询CCO模块曲线数据（03F102_oop），等待--回复");
        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
        break;
    }
    case Wait_Delay_3_Finish:
    {
//            curTime=QDateTime::currentDateTime().toString("yyMMddhhmm");
        startDateTime=QDateTime::currentDateTime();
        curTime=QByteArray::fromHex(startDateTime.toString("mmhhddMMyy").toLatin1());

        curTime_start_oop=QByteArray::fromHex(QDateTime::currentDateTime().toString("yyyyMMddhhmmss").toLatin1());
        curTime_end_oop=QByteArray::fromHex(QDateTime::currentDateTime().addSecs(realPeriod*60).toString("yyyyMMddhhmmss").toLatin1());
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("延时2min时间到,记录当前时间为：%1").arg(QString(QDateTime::currentDateTime().toString("mmhhddMMyy"))));
        emScriptRunState=Wait_Delay_4_Finish;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("等待环境运行%1min").arg(delayTimer));
        p_delayTimer->start(delayTimer*60*1000);
        break;
    }
    case Wait_Delay_4_Finish:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "延时15分钟时间到");
        index=0;
        parallelCount=0;
        while(parallelCount<maxParallelNum)
        {
            if(index>=allMeterList.size())
                break;
            emScriptRunState=Wait_ReadCurveData_STA_F1F100_Finish;
            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,index,p_ReadCurveData_STA_F1F100);
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--并发抄读STA曲线数据（F1F100），等待--回复");
            p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
            index++;
            parallelCount++;
        }
        break;
    }
    case Wait_QueryNetScale_10F9_Finish:
    {
        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryNetScale_10F9);
        emScriptRunState=Wait_QueryNetScale_10F9_2_Finish;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询网络规模（10F9），等待--回复");
        p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
        break;
    }
    case Wait_Delay_5_Finish:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "延时2分钟时间到");
        startDateTime=QDateTime::currentDateTime();
        curTime=QByteArray::fromHex(startDateTime.toString("mmhhddMMyy").toLatin1());

        curTime_start_oop=QByteArray::fromHex(QDateTime::currentDateTime().toString("yyyyMMddhhmmss").toLatin1());
        curTime_end_oop=QByteArray::fromHex(QDateTime::currentDateTime().addSecs(realPeriod*60).toString("yyyyMMddhhmmss").toLatin1());
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("延时2min时间到,记录当前时间为：%1").arg(QString(QDateTime::currentDateTime().toString("mmhhddMMyy"))));
        emScriptRunState=Wait_Delay_6_Finish;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("等待环境运行%1min").arg(delayTimer));
        p_delayTimer->start(delayTimer*60*1000);
        break;
    }
    case Wait_Delay_6_Finish:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "第二次延时15分钟时间到");
        index=0;
        parallelCount=0;

        MaxPointNum=12;
        pointNumIndex=0;
        readInfoClear();
        meterInfoListInit();

        while(parallelCount<maxParallelNum)
        {
            if(index>=allMeterList.size())
                break;
            emScriptRunState=Wait_ReadCurveData_STA_F1F100_2_Finish;
            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,index,p_ReadCurveData_STA_F1F100);
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--并发抄读STA曲线数据（F1F100），等待--回复");
            p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
            index++;
            parallelCount++;
        }
        break;
    }
    }
//    p_maxAllowTimer->stop();
}
void Script_MinuteFreeze_Reset_Hunan::meterInfoListInit()
{
    singleDataId_645_List.clear();
    triDataId_645_List.clear();
    singleDataId_OOP_List.clear();
    triDataId_OOP_List.clear();
    //645数据项
    singleDataId_645_List=dataIdInit(single645);
    triDataId_645_List=dataIdInit(tri645);
    //OOP OAD
    singleDataId_OOP_List=dataIdInit(singleOOP);
    triDataId_OOP_List=dataIdInit(triOOP);

//    singleSTA_645List.clear();
//    threeSTA_645List.clear();
//    singleSTA_oopList.clear();
//    threeSTA_oopList.clear();

    allMeterList.clear();
    ReadModeFrame meterMode;
    for(int i=0;i<p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size();i++)
    {
        if(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(i)->prtcl==DLT645_2007)
        {
            if(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(i)->slotPosition==SingleSTA||
                    p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(i)->slotPosition==SingleMeter)
            {
                memcpy(meterMode.meterNo.addr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(i)->mtrAddr,6);
                meterMode.protocolType=DLT645_2007;
//                singleSTA_645List.append(meterMode);
                for(int n=0;n<MaxDataIDNum;n++)//与数据项个数对应
                {
                    ReadDataUnit readData;
                    readData.dataIdLen=singleDataId_645_List.at(n);
//                    readData.roundList.append(roundList);
                    meterMode.dataUnitList.append(readData);
                }
                allMeterList.append(meterMode);
            }
            else if(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(i)->slotPosition==ThreeSTA||
                    p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(i)->slotPosition==ThreeMeter)
            {
                memcpy(meterMode.meterNo.addr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(i)->mtrAddr,6);
                meterMode.protocolType=DLT645_2007;
//                threeSTA_645List.append(meterMode);
                for(int n=0;n<MaxDataIDNum;n++)//与数据项个数对应
                {
                    ReadDataUnit readData;
                    readData.dataIdLen=triDataId_645_List.at(n);
//                    readData.roundList.append(roundList);
                    meterMode.dataUnitList.append(readData);
                }
                allMeterList.append(meterMode);
            }

        }
        else if(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(i)->prtcl==OOP)
        {
            if(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(i)->slotPosition==SingleSTA||
                    p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(i)->slotPosition==SingleMeter)
            {
                memcpy(meterMode.meterNo.addr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(i)->mtrAddr,6);
                meterMode.protocolType=OOP;
//                singleSTA_oopList.append(meterMode);
                for(int n=0;n<MaxDataIDNum;n++)//与数据项个数对应
                {
                    ReadDataUnit readData;
                    readData.dataIdLen=singleDataId_OOP_List.at(n);
//                    readData.roundList.append(roundList);
                    meterMode.dataUnitList.append(readData);
                }
                allMeterList.append(meterMode);
            }
            else if(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(i)->slotPosition==ThreeSTA||
                    p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(i)->slotPosition==ThreeMeter)
            {
                memcpy(meterMode.meterNo.addr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(i)->mtrAddr,6);
                meterMode.protocolType=OOP;
//                threeSTA_oopList.append(meterMode);
                for(int n=0;n<MaxDataIDNum;n++)//与数据项个数对应
                {
                    ReadDataUnit readData;
                    readData.dataIdLen=triDataId_OOP_List.at(n);
//                    readData.roundList.append(roundList);
                    meterMode.dataUnitList.append(readData);
                }
                allMeterList.append(meterMode);
            }
        }
    }
}

bool Script_MinuteFreeze_Reset_Hunan::isSuccess(QList<ModuleConfig10F103_Hunan> moduleConfig10F103_Hunan)
{
    for(int i=0;i<moduleConfig10F103_Hunan.size();i++)
    {
        if(moduleConfig10F103_Hunan.at(i).config_state_==0x00)
        {
            return false;
        }
    }
    return true;
}
QString Script_MinuteFreeze_Reset_Hunan::getFailMeterNo()
{
    QString failMeterNo;
    for(int i=0;i<moduleConfig10F103_Hunan.size();i++)
    {
        if(moduleConfig10F103_Hunan.at(i).config_state_==0x00)
            failMeterNo+=QString(QByteArray(moduleConfig10F103_Hunan.at(i).node_address_.addr,6).toHex())+";";
    }
    return failMeterNo;
}
int Script_MinuteFreeze_Reset_Hunan::getMeterNoIndex(Address address)
{
    for(int i=0;i<allMeterList.size();i++)
    {
        if(address==allMeterList.at(i).meterNo)
        {
            return i;
        }
    }
    return -1;
}
bool Script_MinuteFreeze_Reset_Hunan::isNotReplyEmpty(QByteArray data)
{
    int count=0;
    for(int i=0;i<data.size();i++)
    {
        if(data.at(i)==char(0xff))
            count++;
    }
    if(count==data.size())
        return false;
    else
        return true;
}
void Script_MinuteFreeze_Reset_Hunan::readInfoClear()
{
    for(int i=0;i<allMeterList.size();i++)
    {
        allMeterList[i].dataIDNumIndex=0;
        for(int j=0;j<allMeterList.at(i).dataUnitList.size();j++)
        {
            for(int n=0;n<allMeterList.at(i).dataUnitList.at(j).roundList.size();n++)
            {
                allMeterList[n].dataUnitList[j].roundList[n].readFlag=false;
                allMeterList[n].dataUnitList[j].roundList[n].replyData.clear();
            }
        }
    }
}
QList<DataIdLen> Script_MinuteFreeze_Reset_Hunan::dataIdInit(QString dataId)
{
    QList<DataIdLen> dataIdUnitList;
    QStringList dataIdList=dataId.split(',');
    for(int i=0;i<dataIdList.size();i++)
    {
        QStringList dataIdAndLen=dataIdList.at(i).split(' ');
        DataIdLen dataIdST;
        memcpy(dataIdST.data_id_,QByteArray::fromHex(dataIdAndLen.at(0).toLatin1()),4);
        dataIdST.reply_data_len_=uchar(dataIdAndLen.at(1).toUInt(nullptr,16));
        dataIdUnitList.append(dataIdST);
    }
    return dataIdUnitList;
}
