#include "Script_ReadMeterTestFlow.h"

Script_ReadMeterTestFlow::Script_ReadMeterTestFlow(QObject *parent) : InteroperateBase_GW(parent)
{

}
Script_ReadMeterTestFlow::~Script_ReadMeterTestFlow()
{
    stop();
    powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost);
}
void Script_ReadMeterTestFlow::setHost(AbstractScriptHost *host)
{
    p_AbstractScriptHost=host;
    p_InteroperateInit_GW->setHost(host);
}
void Script_ReadMeterTestFlow::addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq)
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
bool Script_ReadMeterTestFlow::config(const QMap<QString,QString> *paraDic)
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

void Script_ReadMeterTestFlow::execute()
{
    p_AbstractScriptHost->updateProgress(ProcessState_Start, "抄表测试开始测试!");
    emScriptRunState=ScriptInit;
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "等待检测台体初始化完成...");
    p_InteroperateInit_GW->execute();
    emScriptRunState=Wait_InteroperateInit_Finish;
    p_maxAllowTimer->start((timerForReachThresld+timerAfterReachThresld)*1000);
}
void Script_ReadMeterTestFlow::slotStartFollowTest()
{
    emScriptRunState=Wait_IdentityFlow_Finish;
    QList<int> idList;
    idList<<p_CtrInfoList->at(0)->dvcId;
    QList<double> sendParams;
    p_AbstractScriptHost->controlDvc(CCO_GW,idList,CtrlCmd_ModuleRST,sendParams);
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("开始复位CCO"));
    p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
}
void Script_ReadMeterTestFlow::stop()
{
    p_timer->stop();
    p_maxAllowTimer->stop();
    p_delayTimer->stop();
}

void Script_ReadMeterTestFlow::processMsg(DvcType dvcType, int id, uchar *data, int datalen)
{
    if(p_CtrInfoList->size()==0)
        return;

    switch(emScriptRunState)
    {
        case Wait_InteroperateInit_Finish:
        {
            p_InteroperateInit_GW->processMsg(dvcType, id, data, datalen);
            break;
        }
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

void Script_ReadMeterTestFlow::processCtrlDvcRes(DvcType dvcType, QList<int> idList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params)
{
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到工装命令回复，参数个数=%1").arg(QString::number(params.size())));
    if(isSucs==false)
        return;
    switch(emScriptRunState)
    {
        case Wait_InteroperateInit_Finish:
        {
            p_InteroperateInit_GW->processCtrlDvcRes(dvcType,idList,ctrlCmdType,isSucs,params);
            break;
        }
        case Wait_IdentityFlow_Finish:
        {
            if(dvcType == CCO_GW)
            {
                p_timer->stop();
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("集中器复位路由 成功").arg(dvcType).arg(ctrlCmdType).arg(QString::number(params.size())));
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

void Script_ReadMeterTestFlow::processMsgFromCCO(DvcType dvcType, int dvcId)
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
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("识别流程:流程测试成功\n\n\n"));

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
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("档案同步流程:流程测试成功\n\n\n"));
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "点抄流程: 开始测试!");
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "使用“暂停”命令暂停路由工作 开始...");
                        emScriptRunState=Wait_ReadMeter13F1Flow_Finish;
                        QString msg=QString("68 0F 00 43 00 00 28 32 00 52 12 02 00 03 16").replace(" ","");
                        sendSrcMsg(CCO_GW,p_CtrInfoList->at(0)->dvcId,QByteArray::fromHex(msg.toLatin1()));
                        msgSeq=0x52;
                        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
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
            case Wait_ReadMeter13F1Flow_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp&&uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq)==msgSeq)
                {
                    if(msgSeq==0x52)
                    {
                        p_timer->stop();
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("使用“暂停”命令暂停路由工作 结束"));
                        InteroperateInit_GW::delayTime(10*1000);
                        emScriptRunState=Wait_ReadMeter13F1Flow_Finish;
                        QString msg=(protocol==0x02?QString("68 2F 00 43 04 00 28 32 00 53 88 88 88 99 99 04 04 33 00 00 00 00 13 01 00 02 01 00 10 68 04 33 00 00 00 00 68 11 04 33 33 34 33 E9 16 08 16").replace(" ",""):QString("68 39 00 43 04 00 28 32 00 53 88 88 88 99 99 04 04 33 00 00 00 00 13 01 00 03 01 00 1A 68 18 00 43 05 04 33 00 00 00 00 10 64 D1 05 02 00 01 00 10 02 00 00 91 04 16 34 16").replace(" ",""));
                        sendSrcMsg(dvcType,dvcId,QByteArray::fromHex(msg.toLatin1()));
                        msgSeq=0x53;
                        p_timer->start(90*1000);
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("13F1点抄 开始"));
                    }
                }
                else if(p_Frame3762Base->afn_ == 0x14&&p_Frame3762Base->dt1_==0x04&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到14F3"));
                    emScriptRunState=Wait_ReadMeter13F1Flow_Finish;
                    QByteArray msg=QByteArray::fromHex(protocol==0x02?QString("68 20 00 03 00 00 28 32 00 01 14 04 00 10 68 04 33 00 00 00 00 68 11 04 33 33 34 33 E9 16 6E 16").replace(" ","").toLatin1():QString("68 2A 00 03 00 00 28 32 00 01 14 04 00 1A 68 18 00 43 05 04 33 00 00 00 00 10 64 D1 05 02 00 01 00 10 02 00 00 91 04 16 99 16").replace(" ","").toLatin1());
                    InteroperateInit_GW::calNewMsg(p_Frame3762Base->info_field_.info_field_up.msg_seq,msg);
                    sendSrcMsg(dvcType,dvcId,msg);
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("回复14F3"));
                }
                else if(p_Frame3762Base->afn_ == 0x13&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)//&&uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq)==msgSeq)
                {
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("13F1点抄 结束"));
                    emScriptRunState=Wait_ReadMeter13F1Flow_Finish;
                    QString msg=QString("68 0F 00 43 00 00 28 32 00 54 12 04 00 07 16 ").replace(" ","");
                    sendSrcMsg(dvcType,dvcId,QByteArray::fromHex(msg.toLatin1()));
                    msgSeq=0x54;
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("点抄流程:流程测试成功\n\n\n"));
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "集中器主动抄表: 开始测试!");
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "使用“暂停”命令暂停路由工作 开始...");
                    emScriptRunState=Wait_ConcentratorActiveReadMeterFlow_Finish;
                    msg=QString("68 0F 00 43 00 00 28 32 00 59 12 02 00 0A 16").replace(" ","");
                    sendSrcMsg(CCO_GW,p_CtrInfoList->at(0)->dvcId,QByteArray::fromHex(msg.toLatin1()));
                    msgSeq=0x59;
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_ConcentratorActiveReadMeterFlow_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp&&uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq)==msgSeq)
                {
                    if(msgSeq==0x59)
                    {
                        p_timer->stop();
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("使用“暂停”命令暂停路由工作 结束"));
                        InteroperateInit_GW::delayTime(10*1000);
                        emScriptRunState=Wait_ConcentratorActiveReadMeterFlow_Finish;
                        QString msg=(protocol==0x02?QString("68 2F 00 43 04 00 28 32 00 5A 88 88 88 99 99 04 04 33 00 00 00 00 13 01 00 02 01 00 10 68 04 33 00 00 00 00 68 11 04 33 33 34 33 E9 16 0F 16").replace(" ","")
                                                   :QString("68 39 00 43 04 00 28 32 00 5A 88 88 88 99 99 04 04 33 00 00 00 00 13 01 00 03 01 00 1A 68 18 00 43 05 04 33 00 00 00 00 10 64 D1 05 02 00 01 00 10 02 00 00 91 04 16 3B 16").replace(" ",""));
                        sendSrcMsg(dvcType,dvcId,QByteArray::fromHex(msg.toLatin1()));
                        msgSeq=0x5A;
                        p_timer->start(90*1000);
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("集中器主动抄表 开始"));
                    }
                }
                else if(p_Frame3762Base->afn_ == 0x14&&p_Frame3762Base->dt1_==0x04&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到14F3"));
                    emScriptRunState=Wait_ConcentratorActiveReadMeterFlow_Finish;
                    QByteArray msg=QByteArray::fromHex(protocol==0x02?QString("68 28 00 C3 00 00 00 00 00 04 14 04 00 04 33 00 00 00 00 0A 00 10 68 04 33 00 00 00 00 68 11 04 33 33 34 33 E9 16 18 16").replace(" ","").toLatin1()
                                                                    :QString("68 32 00 C3 00 00 00 00 00 04 14 04 00 04 33 00 00 00 00 0A 00 1A 68 18 00 43 05 04 33 00 00 00 00 10 64 D1 05 02 00 01 00 10 02 00 00 91 04 16 43 16").replace(" ","").toLatin1());
                    InteroperateInit_GW::calNewMsg(p_Frame3762Base->info_field_.info_field_up.msg_seq,msg);
                    sendSrcMsg(dvcType,dvcId,msg);
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("回复14F3"));
                }
                else if(p_Frame3762Base->afn_ == 0x13&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)//&&uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq)==msgSeq)
                {
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("13F1点抄 结束"));
                    emScriptRunState=Wait_ConcentratorActiveReadMeterFlow_Finish;
                    QString msg=QString("68 0F 00 43 00 00 28 32 00 5B 12 04 00 0E 16").replace(" ","");
                    sendSrcMsg(dvcType,dvcId,QByteArray::fromHex(msg.toLatin1()));
                    msgSeq=0x5B;
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("集中器主动抄表:流程测试成功\n\n\n"));
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "路由模块主动抄表流程: 开始测试!");
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "重启 开始...");
                    emScriptRunState=Wait_RouterActiveReadMeterFlow_Finish;
                    msg=QString("68 0F 00 43 00 00 28 32 00 5D 12 01 00 0D 16").replace(" ","");
                    sendSrcMsg(CCO_GW,p_CtrInfoList->at(0)->dvcId,QByteArray::fromHex(msg.toLatin1()));
                    msgSeq=0x5D;
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_RouterActiveReadMeterFlow_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp&&uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq)==msgSeq)
                {
                    if(msgSeq==0x5D)
                    {
                        p_timer->stop();
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("重启 结束"));
                    }
                }
                else if(p_Frame3762Base->afn_ == 0x14&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("路由请求抄读内容 接收结束"));
                    emScriptRunState=Wait_RouterActiveReadMeterFlow_Finish;
                    QByteArray msg=QByteArray::fromHex(protocol==0x02?QString("68 2F 00 03 04 01 28 32 00 05 88 88 88 99 99 04 04 33 00 00 00 00 14 01 00 02 01 10 68 04 33 00 00 00 00 68 11 04 33 33 34 33 E9 16 00 7C 16").replace(" ","").toLatin1()
                                                                    :QString("68 39 00 03 04 01 28 32 00 05 88 88 88 99 99 04 04 33 00 00 00 00 14 01 00 02 01 1A 68 18 00 43 05 04 33 00 00 00 00 10 64 D1 05 02 00 01 00 10 02 00 00 91 04 16 00 A7 16").replace(" ","").toLatin1());
                    InteroperateInit_GW::calNewMsg(p_Frame3762Base->info_field_.info_field_up.msg_seq,msg);
                    sendSrcMsg(dvcType,dvcId,msg);
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("发送“路由请求抄读内容”"));
                }
                else if(p_Frame3762Base->afn_ == 0x14&&p_Frame3762Base->dt1_==0x04&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到14F3"));
                    emScriptRunState=Wait_RouterActiveReadMeterFlow_Finish;
                    QByteArray msg=QByteArray::fromHex(protocol==0x02?QString("68 20 00 03 00 01 28 32 00 06 14 04 00 10 68 04 33 00 00 00 00 68 11 04 33 33 34 33 E9 16 74 16 ").replace(" ","").toLatin1()
                                                                    :QString("68 2A 00 03 00 01 28 32 00 06 14 04 00 1A 68 18 00 43 05 04 33 00 00 00 00 10 64 D1 05 02 00 01 00 10 02 00 00 91 04 16 9F 16 ").replace(" ","").toLatin1());
                    InteroperateInit_GW::calNewMsg(p_Frame3762Base->info_field_.info_field_up.msg_seq,msg);
                    sendSrcMsg(dvcType,dvcId,msg);
                    p_timer->start(90*1000);
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("发送“延时修正通信报文”"));
                }
                else if(p_Frame3762Base->afn_ == 0x06&&p_Frame3762Base->dt1_==0x02&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到上报抄读数据"));
                    QByteArray msg=QByteArray::fromHex(QString("68 15 00 03 00 00 00 00 00 02 00 01 00 FF FF FF FF 00 00 02 16").replace(" ","").toLatin1());
                    InteroperateInit_GW::calNewMsg(p_Frame3762Base->info_field_.info_field_up.msg_seq,msg);
                    sendSrcMsg(dvcType,dvcId,msg);
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("路由模块主动抄表流程:流程测试成功\n\n\n"));
                    p_AbstractScriptHost->updateProgress(ProcessState_Success, QString("抄表测试流程:流程测试成功"));
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

void Script_ReadMeterTestFlow::processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID)
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

void Script_ReadMeterTestFlow::processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID)
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
            case Wait_ReadMeter13F1Flow_Finish:
            case Wait_ConcentratorActiveReadMeterFlow_Finish:
            case Wait_RouterActiveReadMeterFlow_Finish:
            {
                QByteArray tmpSendMsg=QByteArray::fromHex(QString("68 35 00 C3 05 04 33 00 00 00 00 10 57 40 85 02 00 01 00 10 02 00 01 01 05 06 00 00 02 BB 06 00 00 00 C6 06 00 00 00 CB 06 00 00 00 AE 06 00 00 00 7C 00 00 55 43 16").replace(" ","").toLatin1());
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
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

void Script_ReadMeterTestFlow::sendMsg(DvcType , int , int , shared_ptr<void> )
{

}

void Script_ReadMeterTestFlow::timer_timeout()
{
    switch(emScriptRunState)
    {
        case Wait_IdentityFlow_Finish:
        {
            p_timer->stop();
            p_AbstractScriptHost->updateProgress(ProcessState_Error, QString("timeout Wait_CommandProcess_Finish!!!"));
            break;
        }
        default:
            break;
    }
}

void Script_ReadMeterTestFlow::maxAllowTimer_timeout()
{
    p_maxAllowTimer->stop();
    p_AbstractScriptHost->updateProgress(ProcessState_Error, QString("timeout p_maxAllowTimer!!!"));
}

void Script_ReadMeterTestFlow::delayTimer_timeout()
{

}
