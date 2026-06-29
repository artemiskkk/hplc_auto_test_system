#include "Script_EventActiveReport_645.h"

Script_EventActiveReport_645::Script_EventActiveReport_645(QObject *parent) : InteroperateBase_GW(parent)
{

}
Script_EventActiveReport_645::~Script_EventActiveReport_645()
{
    stop();

    QList<int> idList;
    idList<<p_CtrInfoList->at(0)->dvcId;
    QList<double> sendParams;
    p_AbstractScriptHost->controlDvc(SingleSTA,idList,CtrlCmd_EventPinLow,sendParams);
    InteroperateInit_GW::delayTime(100);
    p_AbstractScriptHost->controlDvc(ThreeSTA,idList,CtrlCmd_EventPinLow,sendParams);

    powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost);
}
void Script_EventActiveReport_645::setHost(AbstractScriptHost *host)
{
    p_AbstractScriptHost=host;
    p_InteroperateInit_GW->setHost(host);
}
void Script_EventActiveReport_645::addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq)
{
    p_CtrInfoList->clear();
    comnAddAddrsInfo(p_CtrInfoList, p_ConcentratorInfoList, p_MeterInfoList, p_SchemeCfgInfoList);
    uchar dstPrtcl=uchar(freq)>>4;
    if(dstPrtcl==0x03)
        setMeterAddrsPrtcl(p_CtrInfoList,dstPrtcl);
    p_InteroperateInit_GW->setCtrInfoList(p_CtrInfoList);
    for(int i=0; i<p_CtrInfoList->at(0)->keyList.size(); i++)
    {
        if(SingleSTA == p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->value(p_CtrInfoList->at(0)->keyList.at(i))->slotPosition
        || ThreeSTA==p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->value(p_CtrInfoList->at(0)->keyList.at(i))->slotPosition)
        {
            protocol=(*p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList)[p_CtrInfoList->at(0)->keyList.at(i)]->prtcl;
                break;
        }
    }
}
bool Script_EventActiveReport_645::config(const QMap<QString,QString> *paraDic)
{
    bool result = true;
    if(paraDic!=nullptr)
    {
//        p_BuildNetwork_GW->config(paraDic);

//        if(paraDic->keys().contains("timerForReachThresld"))
//        {
//            this->timerForReachThresld = (*paraDic)["timerForReachThresld"].toUShort();
//        }
//        if(paraDic->keys().contains("timerAfterReachThresld"))
//        {
//            this->timerAfterReachThresld = (*paraDic)["timerAfterReachThresld"].toUShort();
//        }
//        if(paraDic->keys().contains("netSucRateThresld"))
//        {
//            this->netSucRateThresld = (*paraDic)["netSucRateThresld"].toDouble();
//        }
//        if(paraDic->keys().contains("needBuildNet"))
//        {
//            (*paraDic)["needBuildNet"].toLower()=="true"?this->needBuildNet=true:this->needBuildNet=false;
//        }
//        if(paraDic->keys().contains("needPowerOff"))
//        {
//            (*paraDic)["needPowerOff"].toLower()=="true"?this->needPowerOff=true:this->needPowerOff=false;
//        }
//        result = true;
    }
    return result;
}

void Script_EventActiveReport_645::execute()
{
    p_AbstractScriptHost->updateProgress(ProcessState_Start, "事件主动上报流程开始测试!");
    emScriptRunState=ScriptInit;
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "等待检测台体初始化完成...");
    emScriptRunState=Wait_IdentityFlow_Finish;
    QList<int> idList;
    idList<<p_CtrInfoList->at(0)->dvcId;
    QList<double> sendParams;
    p_AbstractScriptHost->controlDvc(CCO_GW,idList,CtrlCmd_ModuleRST,sendParams);
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("开始复位CCO"));
    p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
    p_maxAllowTimer->start((timerForReachThresld+timerAfterReachThresld)*1000);
}
void Script_EventActiveReport_645::slotStartFollowTest()
{

}
void Script_EventActiveReport_645::stop()
{
    p_timer->stop();
    p_maxAllowTimer->stop();
    p_delayTimer->stop();
}

void Script_EventActiveReport_645::processMsg(DvcType dvcType, int id, uchar *data, int datalen)
{
    if(p_CtrInfoList->size()==0)
        return;

    switch(emScriptRunState)
    {
        default:
        {
            QByteArray tmpRecvTempData=QByteArray::fromRawData(reinterpret_cast<char*>(data),datalen);
            QByteArray recvTempData;
            recvTempData.append(tmpRecvTempData);
            delete[]data;

//            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到报文：%1").arg(QString(recvTempData.toHex())));

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
            break;
        }
    }
}

void Script_EventActiveReport_645::processCtrlDvcRes(DvcType dvcType, QList<int> , CtrlCmdType , bool isSucs, QList<double> params)
{
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到工装命令回复，参数个数=%1").arg(QString::number(params.size())));
    if(isSucs==false)
        return;
    switch(emScriptRunState)
    {
        case Wait_IdentityFlow_Finish:
        {
            if(dvcType == CCO_GW)
            {
                p_timer->stop();
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("集中器复位路由 成功"));
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            }
            break;
        }
        default:
        {
            break;
        }
    }
}

void Script_EventActiveReport_645::processMsgFromCCO(DvcType dvcType, int dvcId)
{
    bool haveCompleteMsg=true;
    while(haveCompleteMsg)
    {
        qInfo()<<QString("CCO-解析前 buf3762=%1").arg(QString(p_CtrInfoList->at(0)->buf.toHex()));
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到CCO数据：%1").arg(QString(p_CtrInfoList->at(0)->buf.toHex())));
        shared_ptr<Frame3762Base> p_Frame3762Base = p_Frame3762Helper->DecodeLocalMsg(&(p_CtrInfoList->at(0)->buf),haveCompleteMsg);
        qInfo()<<QString("CCO-解析后 buf3762=%1").arg(QString((p_CtrInfoList->at(0)->buf.toHex())));

        if(p_Frame3762Base==nullptr)
            continue;
        switch(emScriptRunState)
        {
            case Wait_IdentityFlow_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x03&&p_Frame3762Base->dt1_==0x02&&p_Frame3762Base->dt2_==0x01&&p_Frame3762Base->ctrl_field_.dir == kDirUp )
                {
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("通信单元已上报信息(AFN=03H-F10)"));
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("设置主节点地址 开始"));
                    QString msg=QString("68 15 00 43 00 00 28 32 00 4B 05 01 00 88 88 88 99 99 04 BC 16").replace(" ","");
                    sendSrcMsg(dvcType,dvcId,QByteArray::fromHex(msg.toLatin1()));
                    msgSeq=0x4B;
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                }
                else if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp&&uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq)==msgSeq)
                {
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("设置主节点地址 结束"));
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("事件主动上报流程: 主节点识别流程成功\n\n\n"));
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "事件主动上报流程: 档案同步开始");
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "档案同步流程: 开始测试!");
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "使用“暂停”命令暂停路由工作 开始...");
                    emScriptRunState=Wait_ParameterSynchronousFlow_Finish;
                    QString msg=QString("68 0F 00 43 00 00 28 32 00 4C 12 02 00 FD 16 ").replace(" ","");
                    sendSrcMsg(CCO_GW,p_CtrInfoList->at(0)->dvcId,QByteArray::fromHex(msg.toLatin1()));
                    msgSeq=0x4C;
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }

                break;
            }
            case Wait_ParameterSynchronousFlow_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp&&uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq)==msgSeq)
                {
                    if(msgSeq==0x4C)
                    {
                        p_timer->stop();
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("使用“暂停”命令暂停路由工作 结束"));
                        if(flagStop)
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("事件主动上报流程:测试结束."));
                            p_AbstractScriptHost->updateProgress(ProcessState_Success, QString("事件主动上报流程:测试结束."));
                            return;
                        }
                        emScriptRunState=Wait_ParameterSynchronousFlow_Finish;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("参数区初始化 开始"));
                        QString msg=QString("68 0F 00 43 00 00 28 32 00 4D 01 02 00 ED 16 ").replace(" ","");
                        sendSrcMsg(dvcType,dvcId,QByteArray::fromHex(msg.toLatin1()));
                        msgSeq=0x4D;
                        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                    }
                    else if(msgSeq==0x4D)
                    {
                        p_timer->stop();
                        InteroperateInit_GW::delayTime(30*1000);
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("参数区初始化 结束"));
                        emScriptRunState=Wait_ParameterSynchronousFlow_Finish;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("下发从节点数量，查询路由从节点数量 开始"));
                        QString msg=QString("68 0F 00 43 00 00 28 32 00 4E 10 01 00 FC 16").replace(" ","");
                        sendSrcMsg(dvcType,dvcId,QByteArray::fromHex(msg.toLatin1()));
                        msgSeq=0x4E;
                        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                    }
                    else if(msgSeq==0x4F)
                    {
                        p_timer->stop();
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("添加从节点，添加电表地址 结束"));
                        emScriptRunState=Wait_ParameterSynchronousFlow_Finish;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("下发从节点信息，读取路由模块的表档案地址 开始"));
                        QString msg=QString("68 12 00 43 00 00 28 32 00 50 10 02 00 01 00 01 01 16").replace(" ","");
                        sendSrcMsg(dvcType,dvcId,QByteArray::fromHex(msg.toLatin1()));
                        msgSeq=0x50;
                        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                    }
                }
                else if(p_Frame3762Base->afn_ == 0x10&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp&&uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq)==msgSeq)
                {
                    p_timer->stop();
                    if(dynamic_pointer_cast<Afn10F1>(p_Frame3762Base)->node_total_num_==0)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("下发从节点数量，查询路由从节点数量 结束"));
                        emScriptRunState=Wait_ParameterSynchronousFlow_Finish;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("添加从节点，添加电表地址 开始"));
                        QString msg=(protocol==0x02?QString("68 17 00 43 00 00 28 32 00 4F 11 01 00 01 04 33 00 00 00 00 02 38 16").replace(" ",""):QString("68 17 00 43 00 00 28 32 00 4F 11 01 00 01 04 33 00 00 00 00 03 39 16").replace(" ",""));
                        sendSrcMsg(dvcType,dvcId,QByteArray::fromHex(msg.toLatin1()));
                        msgSeq=0x4F;
                        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("查询路由从节点数量不为0"));
                        p_AbstractScriptHost->updateProgress(ProcessState_Error, QString("查询路由从节点数量不为0"));
                    }
                }
                else if(p_Frame3762Base->afn_ == 0x10&&p_Frame3762Base->dt1_==0x02&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp&&uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq)==msgSeq)
                {
                    p_timer->stop();
                    shared_ptr<Afn10F2> ptr=dynamic_pointer_cast<Afn10F2>(p_Frame3762Base);
                    uchar addr[6]={0x00,0x00,0x00,0x00,0x33,0x04};
                    if(ptr->node_info_data_unit_.node_total_num_==1
                        &&ptr->node_info_data_unit_.this_node_num_==1
                        &&memcmp(ptr->node_info_data_unit_.node_info_group_list_.first().node_address_.addr,addr,6)==0
                        &&uchar(ptr->node_info_data_unit_.node_info_group_list_.first().node_info_.protocol_type_)==protocol)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("下发从节点信息，读取路由模块的表档案地址 结束"));
                        emScriptRunState=Wait_ParameterSynchronousFlow_Finish;
                        QString msg=QString("68 0F 00 43 00 00 28 32 00 51 10 08 00 06 16").replace(" ","");
                        sendSrcMsg(CCO_GW,p_CtrInfoList->at(0)->dvcId,QByteArray::fromHex(msg.toLatin1()));
                        msgSeq=0x51;
                        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("下发“路由运行状态” 开始"));
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("查询从节点信息存在异常"));
                        p_AbstractScriptHost->updateProgress(ProcessState_Error, QString("查询从节点信息存在异常"));
                    }
                }
                else if(p_Frame3762Base->afn_ == 0x10&&p_Frame3762Base->dt1_==0x08&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp&&uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq)==msgSeq)
                {
                    p_timer->stop();
                    shared_ptr<Afn10F4> ptr=dynamic_pointer_cast<Afn10F4>(p_Frame3762Base);
                    if(ptr->router_operate_state_unit_.node_total_num_==1)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("下发“路由运行状态” 结束"));
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("事件主动上报流程: 主节点档案同步成功\n\n\n"));
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "设置从节点模块(表/采集器)事件管脚状态为 [有事件] 开始!");
                        emScriptRunState=Wait_EventReport_Finish;
                        QList<int> idList;
                        idList<<p_CtrInfoList->at(0)->dvcId;
                        QList<double> sendParams;
                        p_AbstractScriptHost->controlDvc(SingleSTA,idList,CtrlCmd_EventPinHigh,sendParams);
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("开始对单相模块事件管脚拉高..."));
                        InteroperateInit_GW::delayTime(100);
                        p_AbstractScriptHost->controlDvc(ThreeSTA,idList,CtrlCmd_EventPinHigh,sendParams);
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("开始对三相模块事件管脚拉高..."));
                        p_timer->start(6*60*1000);
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("开始等待从节点本地通信模块抄读\"电能表主动上报状态字\",超时时间为6分钟"));
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("查询路由运行状态存在异常"));
                        p_AbstractScriptHost->updateProgress(ProcessState_Error, QString("档案同步流程:流程测试异常"));
                    }
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }

                break;
            }
            case Wait_EventReport_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x06&&p_Frame3762Base->dt1_==0x10&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    shared_ptr<Afn06F5> ptr=dynamic_pointer_cast<Afn06F5>(p_Frame3762Base);
                    if(ptr->report_event_unit_.frame_content_==QByteArray::fromHex(QString("68 04 33 00 00 00 00 68 91 14 34 48 33 37 33 73 37 33 33 33 33 33 33 33 33 33 DD 34 34 DD 5C 16").replace(" ","").toLatin1()))
                    {
                        p_timer->stop();
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到主节点本地通信模块抄读\"电能表主动上报状态字\"命令R2L10"));
                        QByteArray msg=QByteArray::fromHex(QString("68 15 00 01 00 00 00 00 00 00 00 01 00 FF FF FF FF 00 00 FE 16").replace(" ","").toLatin1());
                        InteroperateInit_GW::calNewMsg(p_Frame3762Base->info_field_.info_field_up.msg_seq,msg);
                        sendSrcMsg(dvcType,dvcId,msg);
                        emScriptRunState=Wait_EventReport_Finish;
                        QString msg1=QString("68 2F 00 41 05 00 C8 00 00 0E 88 88 88 99 99 04 04 33 00 00 00 00 13 01 00 02 00 00 10 68 04 33 00 00 00 00 68 11 04 34 34 34 45 FD 16 57 16").replace(" ","");
                        sendSrcMsg(dvcType,dvcId,QByteArray::fromHex(msg1.toLatin1()));
                        msgSeq=0x0E;
                        p_timer->start(90*1000);
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("抄读第1个事件[645帧开头不带FE](\"A相过压\"事件)R2L10 开始"));
                    }
                    else
                    {
                        QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                        sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                    }
                }
                else if(p_Frame3762Base->afn_ == 0x13&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp&&uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq)==msgSeq)
                {
                    shared_ptr<Afn13F1> ptr=dynamic_pointer_cast<Afn13F1>(p_Frame3762Base);
                    if(msgSeq==0x0E)
                    {
                        p_timer->stop();
                        if(!ptr->data_field_up_.frame_content_.isEmpty())
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("抄读第1个事件[645帧开头不带FE](\"A相过压\"事件)R2L10 结束"));
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("\"A相过压\"事件抄读成功"));
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString(ptr->data_field_up_.frame_content_.toHex()));
                            emScriptRunState=Wait_EventReport_Finish;
                            QString msg1=QString("68 2F 00 41 05 00 C8 00 00 0F 88 88 88 99 99 04 04 33 00 00 00 00 13 01 00 02 00 00 10 68 04 33 00 00 00 00 68 11 04 34 34 33 50 07 16 6C 16").replace(" ","");
                            sendSrcMsg(dvcType,dvcId,QByteArray::fromHex(msg1.toLatin1()));
                            msgSeq=0x0F;
                            p_timer->start(90*1000);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("抄读第2个事件[645帧开头不带FE](\"跳闸成功\"事件)R2L10 开始"));
                        }
                        else
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("事件主动上报流程:测试失败"));
                            p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("事件主动上报流程:测试失败"));
                        }
                    }
                    else if(msgSeq==0x0F)
                    {
                        p_timer->stop();
                        if(!ptr->data_field_up_.frame_content_.isEmpty())
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("抄读第2个事件[645帧开头不带FE](\"跳闸成功\"事件)R2L10 结束"));
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("\"跳闸成功\"事件抄读成功"));
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString(ptr->data_field_up_.frame_content_.toHex()));
                            emScriptRunState=Wait_EventReport_Finish;
                            QString msg1=QString("68 33 00 41 05 00 C8 00 00 10 88 88 88 99 99 04 04 33 00 00 00 00 13 01 00 02 00 00 14 FE FE FE FE 68 04 33 00 00 00 00 68 11 04 34 34 34 45 FD 16 55 16").replace(" ","");
                            sendSrcMsg(dvcType,dvcId,QByteArray::fromHex(msg1.toLatin1()));
                            msgSeq=0x10;
                            p_timer->start(90*1000);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("抄读第1个事件[645帧开头带FE](\"A相过压\"事件)R2L10 开始"));
                        }
                        else
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("事件主动上报流程:测试失败"));
                            p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("事件主动上报流程:测试失败"));
                        }
                    }
                    else if(msgSeq==0x10)
                    {
                        p_timer->stop();
                        if(!ptr->data_field_up_.frame_content_.isEmpty())
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("抄读第1个事件[645帧开头带FE](\"A相过压\"事件)R2L10 结束"));
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("\"A相过压\"事件抄读成功"));
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString(ptr->data_field_up_.frame_content_.toHex()));
                            emScriptRunState=Wait_EventReport_Finish;
                            QString msg1=QString("68 33 00 41 05 00 C8 00 00 11 88 88 88 99 99 04 04 33 00 00 00 00 13 01 00 02 00 00 14 FE FE FE FE 68 04 33 00 00 00 00 68 11 04 34 34 33 50 07 16 6A 16").replace(" ","");
                            sendSrcMsg(dvcType,dvcId,QByteArray::fromHex(msg1.toLatin1()));
                            msgSeq=0x11;
                            p_timer->start(90*1000);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("抄读第2个事件[645帧开头带FE](\"跳闸成功\"事件)R2L10 开始"));
                        }
                        else
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("事件主动上报流程:测试失败"));
                            p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("事件主动上报流程:测试失败"));
                        }
                    }
                    else if(msgSeq==0x11)
                    {
                        p_timer->stop();
                        if(!ptr->data_field_up_.frame_content_.isEmpty())
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("抄读第2个事件[645帧开头带FE](\"跳闸成功\"事件)R2L10 结束"));
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("\"跳闸成功\"事件抄读成功"));
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString(ptr->data_field_up_.frame_content_.toHex()));
                            emScriptRunState=Wait_EventReport_Finish;
                            QString msg1=QString("68 43 00 41 05 00 C8 00 00 12 88 88 88 99 99 04 04 33 00 00 00 00 13 01 00 02 00 00 24 68 04 33 00 00 00 00 68 14 18 36 48 33 37 35 33 33 33 AB 89 67 45 32 F2 2E 32 32 32 32 32 32 32 32 32 DD 16 2F 16").replace(" ","");
                            sendSrcMsg(dvcType,dvcId,QByteArray::fromHex(msg1.toLatin1()));
                            msgSeq=0x12;
                            p_timer->start(90*1000);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("复位主动上报状态字R2L10 开始"));
                        }
                        else
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("事件主动上报流程:测试失败"));
                            p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("事件主动上报流程:测试失败"));
                        }
                    }
                    else if(msgSeq==0x12)
                    {
                        p_timer->stop();
                        if(!ptr->data_field_up_.frame_content_.isEmpty())
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("复位主动上报状态字\"R2L10 结束"));
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("事件主动上报流程:\"复位主动上报状态字\"成功."));
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString(ptr->data_field_up_.frame_content_.toHex()));
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("通过EVENT管脚产生无事件状态[成功]"));
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("\n\n\n\n"));
                            InteroperateInit_GW::delayTime(5*1000);
                            emScriptRunState=Wait_EventReport1_Finish;
                            QList<int> idList;
                            idList<<p_CtrInfoList->at(0)->dvcId;
                            QList<double> sendParams;
                            p_AbstractScriptHost->controlDvc(SingleSTA,idList,CtrlCmd_EventPinHigh,sendParams);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("开始对单相模块事件管脚拉高..."));
                            InteroperateInit_GW::delayTime(100);
                            p_AbstractScriptHost->controlDvc(ThreeSTA,idList,CtrlCmd_EventPinHigh,sendParams);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("开始对三相模块事件管脚拉高..."));
                            p_timer->start(6*60*1000);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("开始等待从节点本地通信模块抄读\"电能表主动上报状态字\",超时时间为6分钟"));
                        }
                        else
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("事件主动上报流程:测试失败"));
                            p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("事件主动上报流程:测试失败"));
                        }
                    }
                    else
                    {
                        QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                        sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                    }
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_EventReport1_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x06&&p_Frame3762Base->dt1_==0x10&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    shared_ptr<Afn06F5> ptr=dynamic_pointer_cast<Afn06F5>(p_Frame3762Base);
                    if(ptr->report_event_unit_.frame_content_==QByteArray::fromHex(QString("68 04 33 00 00 00 00 68 91 13 34 48 33 37 43 33 33 33 33 33 33 33 33 33 33 33 DD 32 DD F1 16").replace(" ","").toLatin1()))
                    {
                        p_timer->stop();
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到主节点本地通信模块抄读\"电能表主动上报状态字\"命令R1"));
                        QByteArray msg=QByteArray::fromHex(QString("68 15 00 01 00 00 00 00 00 00 00 01 00 FF FF FF FF 00 00 FE 16").replace(" ","").toLatin1());
                        InteroperateInit_GW::calNewMsg(p_Frame3762Base->info_field_.info_field_up.msg_seq,msg);
                        sendSrcMsg(dvcType,dvcId,msg);
                        emScriptRunState=Wait_EventReport1_Finish;
                        QString msg1=QString("68 43 00 41 05 00 C8 00 00 13 88 88 88 99 99 04 04 33 00 00 00 00 13 01 00 02 00 00 24 68 04 33 00 00 00 00 68 14 18 36 48 33 37 35 33 33 33 AB 89 67 45 22 32 32 32 32 32 32 32 32 32 32 32 11 16 98 16").replace(" ","");
                        sendSrcMsg(dvcType,dvcId,QByteArray::fromHex(msg1.toLatin1()));
                        msgSeq=0x13;
                        p_timer->start(90*1000);
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("复位主动上报状态字R1 开始"));
                    }
                    else
                    {
                        QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                        sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                    }
                }
                else if(p_Frame3762Base->afn_ == 0x13&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp&&uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq)==msgSeq)
                {
                    shared_ptr<Afn13F1> ptr=dynamic_pointer_cast<Afn13F1>(p_Frame3762Base);
                    if(msgSeq==0x13)
                    {
                        p_timer->stop();
                        if(!ptr->data_field_up_.frame_content_.isEmpty())
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("复位主动上报状态字\"R1 结束"));
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("事件主动上报流程:\"复位主动上报状态字\"成功."));
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString(ptr->data_field_up_.frame_content_.toHex()));
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("通过EVENT管脚产生无事件状态[成功]"));
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("\n\n\n\n"));
                            emScriptRunState=Wait_EventReport1_Finish;
                            QList<int> idList;
                            idList<<p_CtrInfoList->at(0)->dvcId;
                            QList<double> sendParams;
                            InteroperateInit_GW::delayTime(5000);
                            p_AbstractScriptHost->controlDvc(SingleSTA,idList,CtrlCmd_PowerOff_12V,sendParams);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("开始对单相模块断电..."));
                            InteroperateInit_GW::delayTime(100);
                            p_AbstractScriptHost->controlDvc(ThreeSTA,idList,CtrlCmd_PowerOff_12V,sendParams);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("开始对三相模块断电..."));
                            InteroperateInit_GW::delayTime(1000);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("终端本地通信单元保持供电,电能表本地通信单元掉电[成功]"));

                            emScriptRunState=Wait_EventReport2_Finish;
                            InteroperateInit_GW::delayTime(100);
                            p_AbstractScriptHost->controlDvc(SingleSTA,idList,CtrlCmd_EventPinHigh,sendParams);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("开始对单相模块事件管脚拉高..."));
                            InteroperateInit_GW::delayTime(100);
                            p_AbstractScriptHost->controlDvc(ThreeSTA,idList,CtrlCmd_EventPinHigh,sendParams);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("开始对三相模块事件管脚拉高..."));

                            InteroperateInit_GW::delayTime(100);
                            p_AbstractScriptHost->controlDvc(SingleSTA,idList,CtrlCmd_PowerOn_12V,sendParams);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("开始对单相模块上电..."));
                            InteroperateInit_GW::delayTime(100);
                            p_AbstractScriptHost->controlDvc(ThreeSTA,idList,CtrlCmd_PowerOn_12V,sendParams);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("开始对三相模块上电..."));

                            InteroperateInit_GW::delayTime(100);
                            p_AbstractScriptHost->controlDvc(SingleSTA,idList,CtrlCmd_ModuleRST,sendParams);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("开始对单相模块复位..."));
                            InteroperateInit_GW::delayTime(100);
                            p_AbstractScriptHost->controlDvc(ThreeSTA,idList,CtrlCmd_ModuleRST,sendParams);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("开始对三相模块复位..."));

                            p_timer->start(6*60*1000);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("电能表本地通信单元在EventOut管脚持续高阻态的情况下上电"));
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("开始等待从节点本地通信模块抄读\"电能表主动上报状态字\",超时时间为6分钟"));
                        }
                        else
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("事件主动上报流程:测试失败"));
                            p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("事件主动上报流程:测试失败"));
                        }
                    }
                    else
                    {
                        QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                        sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                    }
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_EventReport2_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x06&&p_Frame3762Base->dt1_==0x10&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    shared_ptr<Afn06F5> ptr=dynamic_pointer_cast<Afn06F5>(p_Frame3762Base);
                    if(ptr->report_event_unit_.frame_content_==QByteArray::fromHex(QString("68 04 33 00 00 00 00 68 91 14 34 48 33 37 34 33 33 33 33 33 33 33 53 33 33 33 DD 34 34 DD 39 16").replace(" ","").toLatin1()))
                    {
                        p_timer->stop();
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到主节点本地通信模块上报\"电能表主动上报状态字\"命令POR1"));
                        QByteArray msg=QByteArray::fromHex(QString("68 15 00 01 00 00 00 00 00 00 00 01 00 FF FF FF FF 00 00 FE 16").replace(" ","").toLatin1());
                        InteroperateInit_GW::calNewMsg(p_Frame3762Base->info_field_.info_field_up.msg_seq,msg);
                        sendSrcMsg(dvcType,dvcId,msg);
                        emScriptRunState=Wait_EventReport2_Finish;
                        QString msg1=QString("68 43 00 41 05 00 C8 00 00 14 88 88 88 99 99 04 04 33 00 00 00 00 13 01 00 02 00 00 24 68 04 33 00 00 00 00 68 14 18 36 48 33 37 35 33 33 33 AB 89 67 45 31 32 32 32 32 32 32 32 12 32 32 32 00 16 77 16").replace(" ","");
                        sendSrcMsg(dvcType,dvcId,QByteArray::fromHex(msg1.toLatin1()));
                        msgSeq=0x14;
                        p_timer->start(90*1000);
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("复位主动上报状态字POR1 开始"));
                    }
                    else
                    {
                        QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                        sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                    }
                }
                else if(p_Frame3762Base->afn_ == 0x13&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp&&uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq)==msgSeq)
                {
                    shared_ptr<Afn13F1> ptr=dynamic_pointer_cast<Afn13F1>(p_Frame3762Base);
                    if(msgSeq==0x14)
                    {
                        p_timer->stop();
                        if(!ptr->data_field_up_.frame_content_.isEmpty())
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("复位主动上报状态字\"POR1 结束"));
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("事件主动上报流程:\"复位主动上报状态字\"成功."));
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString(ptr->data_field_up_.frame_content_.toHex()));
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("通过EVENT管脚产生无事件状态[成功]"));
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("\n\n\n\n"));
                            emScriptRunState=Wait_EventReport2_Finish;
                            QList<int> idList;
                            idList<<p_CtrInfoList->at(0)->dvcId;
                            QList<double> sendParams;
                            InteroperateInit_GW::delayTime(5000);
                            p_AbstractScriptHost->controlDvc(CCO_GW,idList,CtrlCmd_PowerOff_12V,sendParams);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("开始对路由模块断电..."));
                            InteroperateInit_GW::delayTime(100);
                            p_AbstractScriptHost->controlDvc(SingleSTA,idList,CtrlCmd_PowerOff_12V,sendParams);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("开始对单相模块断电..."));
                            InteroperateInit_GW::delayTime(100);
                            p_AbstractScriptHost->controlDvc(ThreeSTA,idList,CtrlCmd_PowerOff_12V,sendParams);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("开始对三相模块断电..."));
                            InteroperateInit_GW::delayTime(1000);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("终端本地通信单元和电能表本地通信单元同时掉电[成功]"));

                            emScriptRunState=Wait_EventReport3_Finish;
                            InteroperateInit_GW::delayTime(100);
                            p_AbstractScriptHost->controlDvc(SingleSTA,idList,CtrlCmd_EventPinHigh,sendParams);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("开始对单相模块事件管脚拉高..."));
                            InteroperateInit_GW::delayTime(100);
                            p_AbstractScriptHost->controlDvc(ThreeSTA,idList,CtrlCmd_EventPinHigh,sendParams);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("开始对三相模块事件管脚拉高..."));

                            InteroperateInit_GW::delayTime(100);
                            p_AbstractScriptHost->controlDvc(CCO_GW,idList,CtrlCmd_PowerOn_12V,sendParams);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("开始对路由模块上电..."));
                            InteroperateInit_GW::delayTime(100);
                            p_AbstractScriptHost->controlDvc(SingleSTA,idList,CtrlCmd_PowerOn_12V,sendParams);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("开始对单相模块上电..."));
                            InteroperateInit_GW::delayTime(100);
                            p_AbstractScriptHost->controlDvc(ThreeSTA,idList,CtrlCmd_PowerOn_12V,sendParams);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("开始对三相模块上电..."));

                            InteroperateInit_GW::delayTime(100);
                            p_AbstractScriptHost->controlDvc(CCO_GW,idList,CtrlCmd_ModuleRST,sendParams);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("开始对路由模块复位..."));
                            InteroperateInit_GW::delayTime(100);
                            p_AbstractScriptHost->controlDvc(SingleSTA,idList,CtrlCmd_ModuleRST,sendParams);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("开始对单相模块复位..."));
                            InteroperateInit_GW::delayTime(100);
                            p_AbstractScriptHost->controlDvc(ThreeSTA,idList,CtrlCmd_ModuleRST,sendParams);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("开始对三相模块复位..."));

                            p_timer->start(6*60*1000);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("在电能表本地通信单元EventOut管脚持续高阻态的情况下,终端本地通信单元和电能表本地通信单元同时上电[成功]."));
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("开始等待从节点本地通信模块抄读\"电能表主动上报状态字\",超时时间为6分钟"));
                        }
                        else
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("事件主动上报流程:测试失败"));
                            p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("事件主动上报流程:测试失败"));
                        }
                    }
                    else
                    {
                        QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                        sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                    }
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_EventReport3_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x03&&p_Frame3762Base->dt1_==0x02&&p_Frame3762Base->dt2_==0x01&&p_Frame3762Base->ctrl_field_.dir == kDirUp )
                {
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("通信单元已上报信息(AFN=03H-F10)"));
                    flagRecv03F10=true;
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

void Script_EventActiveReport_645::processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID)
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
        QByteArray tmp=(*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[mtrlID]->buf645;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到645数据123：%1").arg(QString(tmp.toHex())));
        shared_ptr<dlt_645_Protocol::Frame645Base> MsgBase_645_ptr = dlt_645_Protocol::Frame645Helper::DecodeLocalMsg(&((*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[mtrlID]->buf645),haveCompleteMsg);

        if(MsgBase_645_ptr==nullptr)
            break;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到645数据：%1").arg(QString(tmp.toHex())));
        switch(emScriptRunState)
        {
            case Wait_EventReport_Finish:
            {
                if(MsgBase_645_ptr->ctrlCode_==0x11)
                {
                    uchar state[4]={0x01,0x15,0x00,0x04};
                    uchar AVol[4]={0x01,0x01,0x01,0x12};
                    uchar powerOff[4]={0x01,0x01,0x00,0x1D};
                    shared_ptr<Rqst_ReadData_0x11> ptr=dynamic_pointer_cast<Rqst_ReadData_0x11>(MsgBase_645_ptr);
                    if(memcmp(ptr->di,state,4)==0)
                    {
                        QByteArray tmpSendMsg=QByteArray::fromHex(QString("68 04 33 00 00 00 00 68 91 14 34 48 33 37 33 73 37 33 33 33 33 33 33 33 33 33 DD 34 34 DD 5C 16").replace(" ","").toLatin1());
                        sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                    }
                    else if(memcmp(ptr->di,AVol,4)==0)
                    {
                        QByteArray tmpSendMsg=QByteArray::fromHex(QString("FE FE FE FE 68 04 33 00 00 00 00 68 91 0A 34 34 34 45 7A 55 44 48 36 56 6A 16").replace(" ","").toLatin1());
                        sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                    }
                    else if(memcmp(ptr->di,powerOff,4)==0)
                    {
                        QByteArray tmpSendMsg=QByteArray::fromHex(QString("FE FE FE FE 68 04 33 00 00 00 00 68 91 0A 34 34 33 50 7A 55 44 48 36 56 74 16").replace(" ","").toLatin1());
                        sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                    }
                }
                else if(MsgBase_645_ptr->ctrlCode_==0x14)
                {
                    QByteArray tmpSendMsg=QByteArray::fromHex(QString("68 04 33 00 00 00 00 68 94 00 9B 16").replace(" ","").toLatin1());
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                    QList<int> idList;
                    idList<<p_CtrInfoList->at(0)->dvcId;
                    QList<double> sendParams;
                    p_AbstractScriptHost->controlDvc(SingleSTA,idList,CtrlCmd_EventPinLow,sendParams);
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("开始对单相模块事件管脚拉低..."));
                    InteroperateInit_GW::delayTime(100);
                    p_AbstractScriptHost->controlDvc(ThreeSTA,idList,CtrlCmd_EventPinLow,sendParams);
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("开始对三相模块事件管脚拉低..."));
                    break;
                }
                else
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
                }
                break;
            }
            case Wait_EventReport1_Finish:
            {
                if(MsgBase_645_ptr->ctrlCode_==0x11)
                {
                    uchar state[4]={0x01,0x15,0x00,0x04};
                    if(memcmp(dynamic_pointer_cast<Rqst_ReadData_0x11>(MsgBase_645_ptr)->di,state,4)==0)
                    {
                        QByteArray tmpSendMsg=QByteArray::fromHex(QString("68 04 33 00 00 00 00 68 91 13 34 48 33 37 43 33 33 33 33 33 33 33 33 33 33 33 DD 32 DD F1 16").replace(" ","").toLatin1());
                        sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                    }
                }
                else if(MsgBase_645_ptr->ctrlCode_==0x14)
                {
                    QByteArray tmpSendMsg=QByteArray::fromHex(QString("68 04 33 00 00 00 00 68 94 00 9B 16").replace(" ","").toLatin1());
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                    QList<int> idList;
                    idList<<p_CtrInfoList->at(0)->dvcId;
                    QList<double> sendParams;
                    p_AbstractScriptHost->controlDvc(SingleSTA,idList,CtrlCmd_EventPinLow,sendParams);
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("开始对单相模块事件管脚拉低..."));
                    InteroperateInit_GW::delayTime(100);
                    p_AbstractScriptHost->controlDvc(ThreeSTA,idList,CtrlCmd_EventPinLow,sendParams);
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("开始对三相模块事件管脚拉低..."));
                    break;
                }
                else
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
                }
                break;
            }
            case Wait_EventReport2_Finish:
            {
                if(MsgBase_645_ptr->ctrlCode_==0x11)
                {
                    uchar state[4]={0x01,0x15,0x00,0x04};
                    if(memcmp(dynamic_pointer_cast<Rqst_ReadData_0x11>(MsgBase_645_ptr)->di,state,4)==0)
                    {
                        QByteArray tmpSendMsg=QByteArray::fromHex(QString("68 04 33 00 00 00 00 68 91 14 34 48 33 37 34 33 33 33 33 33 33 33 53 33 33 33 DD 34 34 DD 39 16").replace(" ","").toLatin1());
                        sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                    }
                }
                else if(MsgBase_645_ptr->ctrlCode_==0x14)
                {
                    QByteArray tmpSendMsg=QByteArray::fromHex(QString("68 04 33 00 00 00 00 68 94 00 9B 16").replace(" ","").toLatin1());
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                    QList<int> idList;
                    idList<<p_CtrInfoList->at(0)->dvcId;
                    QList<double> sendParams;
                    p_AbstractScriptHost->controlDvc(SingleSTA,idList,CtrlCmd_EventPinLow,sendParams);
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("开始对单相模块事件管脚拉低..."));
                    InteroperateInit_GW::delayTime(100);
                    p_AbstractScriptHost->controlDvc(ThreeSTA,idList,CtrlCmd_EventPinLow,sendParams);
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("开始对三相模块事件管脚拉低..."));
                    break;
                }
                else
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
                }
                break;
            }
            case Wait_EventReport3_Finish:
            {
                if(MsgBase_645_ptr->ctrlCode_==0x11)
                {
                    uchar state[4]={0x01,0x15,0x00,0x04};
                    if(memcmp(dynamic_pointer_cast<Rqst_ReadData_0x11>(MsgBase_645_ptr)->di,state,4)==0)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到从节点本地通信模块抄读\"电能表主动上报状态字\"命令APOR1"));
                        QList<int> idList;
                        idList<<p_CtrInfoList->at(0)->dvcId;
                        QList<double> sendParams;
                        p_AbstractScriptHost->controlDvc(SingleSTA,idList,CtrlCmd_EventPinLow,sendParams);
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("开始对单相模块事件管脚拉低..."));
                        InteroperateInit_GW::delayTime(100);
                        p_AbstractScriptHost->controlDvc(ThreeSTA,idList,CtrlCmd_EventPinLow,sendParams);
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("开始对三相模块事件管脚拉低..."));
                        emScriptRunState=Wait_IdentityFlow_Finish;
                        flagStop=true;
                        if(flagRecv03F10)
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("设置主节点地址 开始"));
                            QString msg=QString("68 15 00 43 00 00 28 32 00 4B 05 01 00 88 88 88 99 99 04 BC 16").replace(" ","");
                            sendSrcMsg(CCO_GW,dvcId,QByteArray::fromHex(msg.toLatin1()));
                            msgSeq=0x4B;
                            p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                        }
                    }
                }
                else
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

void Script_EventActiveReport_645::processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID)
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
        QByteArray tmp=(*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[mtrlID]->buf698;
        shared_ptr<FrameOOPBase> MsgBase_OOP_ptr = FrameOOPHelper::DecodeMsg(&((*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[mtrlID]->buf698),haveCompleteMsg);

        if(MsgBase_OOP_ptr==nullptr)
            continue;
        switch(emScriptRunState)
        {
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

void Script_EventActiveReport_645::sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame)
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

void Script_EventActiveReport_645::timer_timeout()
{
    switch(emScriptRunState)
    {
        case Wait_IdentityFlow_Finish:
        {
            p_timer->stop();
            p_AbstractScriptHost->updateProgress(ProcessState_Error, QString("timeout Wait_IdentityFlow_Finish!!!"));
            break;
        }
        default:
        {
            p_timer->stop();
            p_AbstractScriptHost->updateProgress(ProcessState_Error, QString("timeout !!!"));
            break;
        }
    }
}

void Script_EventActiveReport_645::maxAllowTimer_timeout()
{
    p_maxAllowTimer->stop();
    p_AbstractScriptHost->updateProgress(ProcessState_Error, QString("timeout p_maxAllowTimer!!!"));
}

void Script_EventActiveReport_645::delayTimer_timeout()
{

}
