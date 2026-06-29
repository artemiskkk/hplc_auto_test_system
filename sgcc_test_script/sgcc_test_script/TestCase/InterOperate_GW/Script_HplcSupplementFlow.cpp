#include "Script_HplcSupplementFlow.h"

Script_HplcSupplementFlow::Script_HplcSupplementFlow(QObject *parent) : InteroperateBase_GW(parent)
{

}
Script_HplcSupplementFlow::~Script_HplcSupplementFlow()
{
    stop();
    powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost);
}
void Script_HplcSupplementFlow::setHost(AbstractScriptHost *host)
{
    p_AbstractScriptHost=host;
    p_InteroperateInit_GW->setHost(host);
}
void Script_HplcSupplementFlow::addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq)
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
    protocolMap.insert(0x00,"透明传输");
    protocolMap.insert(0x01,"DL/T 645-1997");
    protocolMap.insert(0x02,"DL/T 645-2007");
    protocolMap.insert(0x03,"DL/T 698.45");
    if(protocol==0x02)
    {
        readMeters=QList<QByteArray>()<<QByteArray::fromHex(QString("00 68 2F 00 43 04 00 00 00 00 69 88 88 88 99 99 04 04 33 00 00 00 00 F1 01 00 00 00 10 00 AB CD 20 23 03 15 11 19 53 19 11 15 03 23 20 EF 7B 16").replace(" ","").toLatin1())
                                <<QByteArray::fromHex(QString("01 68 2D 00 43 04 00 00 00 00 6A 88 88 88 99 99 04 04 33 00 00 00 00 F1 01 00 01 00 0E 00 68 04 33 00 00 00 00 68 01 02 43 C3 10 16 ED 16").replace(" ","").toLatin1())
                                <<QByteArray::fromHex(QString("02 68 2F 00 43 04 00 00 00 00 6B 88 88 88 99 99 04 04 33 00 00 00 00 F1 01 00 02 00 10 00 68 04 33 00 00 00 00 68 11 04 33 33 34 33 E9 16 A3 16").replace(" ","").toLatin1());
    }
    else if(protocol==0x03)
    {
        readMeters=QList<QByteArray>()<<QByteArray::fromHex(QString("00 68 2F 00 43 04 00 00 00 00 69 88 88 88 99 99 04 04 33 00 00 00 00 F1 01 00 00 00 10 00 AB CD 20 23 03 15 11 19 53 19 11 15 03 23 20 EF 7B 16").replace(" ","").toLatin1())
                               <<QByteArray::fromHex(QString("03 68 39 00 43 04 00 00 00 00 6A 88 88 88 99 99 04 04 33 00 00 00 00 F1 01 00 03 00 1A 00 68 18 00 43 05 04 33 00 00 00 00 10 64 D1 05 02 00 01 00 10 02 00 00 91 04 16 CE 16").replace(" ","").toLatin1());
    }
}
bool Script_HplcSupplementFlow::config(const QMap<QString,QString> *paraDic)
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

void Script_HplcSupplementFlow::execute()
{
    p_AbstractScriptHost->updateProgress(ProcessState_Start, "宽带增补内容测试流程: 开始测试!");
    emScriptRunState=ScriptInit;
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "使用“暂停”命令暂停路由工作 开始...");
    emScriptRunState=Wait_PauseRouter_Finish;
    QString msg=QString("68 0F 00 43 00 00 28 32 00 63 12 02 00 14 16").replace(" ","");
    sendSrcMsg(CCO_GW,p_CtrInfoList->at(0)->dvcId,QByteArray::fromHex(msg.toLatin1()));
    msgSeq=0x63;
    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
    p_maxAllowTimer->start((timerForReachThresld+timerAfterReachThresld)*1000);
}
void Script_HplcSupplementFlow::slotStartFollowTest()
{

}
void Script_HplcSupplementFlow::stop()
{
    p_timer->stop();
    p_maxAllowTimer->stop();
    p_delayTimer->stop();
}

void Script_HplcSupplementFlow::processMsg(DvcType dvcType, int id, uchar *data, int datalen)
{
    if(p_CtrInfoList->size()==0)
        return;

    switch(emScriptRunState)
    {
        case ScriptInit:
            break;
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
                if(protocol==0x00)
                {
                    processMsgFromMeter0x00(dvcType,id,recvTempData);
                    break;
                }
                else if(protocol==0x01)
                {
                    processMsgFromMeter0x01(dvcType,id,recvTempData);
                    break;
                }
            }
            else if(dvcType==CJQ)
            {

            }
            break;
        }
    }
}

void Script_HplcSupplementFlow::processCtrlDvcRes(DvcType dvcType, QList<int> idList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params)
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
        default:
        {
            break;
        }
    }
}

void Script_HplcSupplementFlow::processMsgFromCCO(DvcType dvcType, int dvcId)
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
            case Wait_PauseRouter_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp&&uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq)==msgSeq)
                {
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("使用“暂停”命令暂停路由工作 结束"));
                    InteroperateInit_GW::delayTime(10*1000);
                    emScriptRunState=Wait_QueryInfo_Finish;
                    QString msg=QString("68 12 00 43 00 00 28 32 00 64 10 10 02 01 00 02 26 16").replace(" ","");
                    sendSrcMsg(dvcType,dvcId,QByteArray::fromHex(msg.toLatin1()));
                    msgSeq=0x64;
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("查询网络拓扑信息 开始"));
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_QueryInfo_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x10&&p_Frame3762Base->dt1_==0x10&&p_Frame3762Base->dt2_==0x02&&p_Frame3762Base->ctrl_field_.dir == kDirUp&&uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq)==msgSeq)
                {
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("查询网络拓扑信息 结束"));
                    emScriptRunState=Wait_QueryInfo_Finish;
                    QString msg=QString("68 0F 00 43 00 00 28 32 00 65 10 40 0D 5F 16").replace(" ","");
                    sendSrcMsg(dvcType,dvcId,QByteArray::fromHex(msg.toLatin1()));
                    msgSeq=0x65;
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("查询多网络信息 开始"));
                }
                else if(p_Frame3762Base->afn_ == 0x10&&p_Frame3762Base->dt1_==0x40&&p_Frame3762Base->dt2_==0x0D&&p_Frame3762Base->ctrl_field_.dir == kDirUp&&uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq)==msgSeq)
                {
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("查询多网络信息 结束"));
                    emScriptRunState=Wait_QueryInfo_Finish;
                    QString msg=QString("68 12 00 43 00 00 28 32 00 66 10 80 0D 01 00 02 A3 16 ").replace(" ","");
                    sendSrcMsg(dvcType,dvcId,QByteArray::fromHex(msg.toLatin1()));
                    msgSeq=0x66;
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("查询宽带载波芯片信息 开始"));
                }
                else if(p_Frame3762Base->afn_ == 0x10&&p_Frame3762Base->dt1_==char(0x80)&&p_Frame3762Base->dt2_==0x0D&&p_Frame3762Base->ctrl_field_.dir == kDirUp&&uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq)==msgSeq)
                {
                    p_timer->stop();
                    shared_ptr<Afn10F112> ptr=dynamic_pointer_cast<Afn10F112>(p_Frame3762Base);
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("表号%1的芯片ID为%2")
                                                         .arg(ptr->node_chip_info_unit_.node_chip_info_list_[1].node_address_.toString())
                                                         .arg(QString(ptr->node_chip_info_unit_.node_chip_info_list_[1].node_chip_id_.toHex()).toUpper()));
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("查询宽带载波芯片信息 结束"));
                    emScriptRunState=Wait_F1F1_Finish;
                    QString msg=QString("68 0F 00 43 00 00 28 32 00 67 03 80 01 88 16").replace(" ","");
                    sendSrcMsg(dvcType,dvcId,QByteArray::fromHex(msg.toLatin1()));
                    msgSeq=0x67;
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("查询宽带载波通信参数 开始"));
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_F1F1_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x03&&p_Frame3762Base->dt1_==char(0x80)&&p_Frame3762Base->dt2_==0x01&&p_Frame3762Base->ctrl_field_.dir == kDirUp&&uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq)==msgSeq)
                {
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("查询宽带载波通信参数 结束"));
                    emScriptRunState=Wait_F1F1_Finish;
                    QString msg=QString("68 10 00 43 00 00 28 32 00 68 05 80 01 00 8B 16").replace(" ","");
                    sendSrcMsg(dvcType,dvcId,QByteArray::fromHex(msg.toLatin1()));
                    msgSeq=0x68;
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("设置宽带载波通信参数 开始"));
                }
                else if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp&&uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq)==msgSeq)
                {
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("设置宽带载波通信参数 结束"));
                    emScriptRunState=Wait_F1F1_Finish;
                    if(readMeters.isEmpty())
                        return;
                    protocol=uchar(readMeters.first().at(0));
                    readMeters.first().remove(0,1);
                    sendSrcMsg(dvcType,dvcId,readMeters.first());
                    msgSeq=uchar(readMeters.first().at(9));
                    readMeters.removeFirst();
                    p_timer->start(90*1000);
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("并发抄表-(%1) 开始").arg(protocolMap.value(protocol)));
                }
                else if(p_Frame3762Base->afn_ == char(0xF1)&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp&&uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq)==msgSeq)
                {
                    p_timer->stop();
                    shared_ptr<AfnF1F1> ptr=dynamic_pointer_cast<AfnF1F1>(p_Frame3762Base);
                    if(ptr->unit_up_.frame_length_>0)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("回复内容%1 %2").arg(QString(ptr->unit_up_.frame_content_.toHex())).arg(uchar(protocol)));
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("并发抄表-(%1) 结束").arg(protocolMap.value(protocol)));
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("并发抄表-(%1) 回复内容为空").arg(protocolMap.value(protocol)));
                    }
                    if(!readMeters.isEmpty())
                    {
                        protocol=uchar(readMeters.first().at(0));
                        readMeters.first().remove(0,1);
                        sendSrcMsg(dvcType,dvcId,readMeters.first());
                        msgSeq=uchar(readMeters.first().at(9));
                        readMeters.removeFirst();
                        p_timer->start(90*1000);
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("并发抄表-(%1) 开始").arg(protocolMap.value(protocol)));
                        emScriptRunState=Wait_F1F1_Finish;
                    }
                    else
                    {
                        QString msg=QString("68 FF 00 43 04 00 00 00 00 6C 88 88 88 99 99 04 04 33 00 00 00 00 F1 01 00 02 00 E0 00 "
                                    "68 04 33 00 00 00 00 68 11 04 33 33 34 33 E9 16"
                                    "68 04 33 00 00 00 00 68 11 04 33 33 34 33 E9 16"
                                    "68 04 33 00 00 00 00 68 11 04 33 33 34 33 E9 16"
                                    "68 04 33 00 00 00 00 68 11 04 33 33 34 33 E9 16"
                                    "68 04 33 00 00 00 00 68 11 04 33 33 34 33 E9 16"
                                    "68 04 33 00 00 00 00 68 11 04 33 33 34 33 E9 16"
                                    "68 04 33 00 00 00 00 68 11 04 33 33 34 33 E9 16"
                                    "68 04 33 00 00 00 00 68 11 04 33 33 34 33 E9 16"
                                    "68 04 33 00 00 00 00 68 11 04 33 33 34 33 E9 16"
                                    "68 04 33 00 00 00 00 68 11 04 33 33 34 33 E9 16"
                                    "68 04 33 00 00 00 00 68 11 04 33 33 34 33 E9 16"
                                    "68 04 33 00 00 00 00 68 11 04 33 33 34 33 E9 16"
                                    "68 04 33 00 00 00 00 68 11 04 33 33 34 33 E9 16"
                                    "68 04 33 00 00 00 00 68 11 04 33 33 34 33 E9 16 3C 16").replace(" ","");
                        msgSeq=0x6C;
                        emScriptRunState=Wait_F1F1_0x6E_Finish;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("并发抄表-Err0x6E 开始"));
                        sendSrcMsg(dvcType,dvcId,QByteArray::fromHex(msg.toLatin1()));
                        p_timer->start(90*1000);
                    }
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_F1F1_0x6E_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x02&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp&&uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq)==msgSeq)
                {
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("并发抄表-Err0x6E 结束"));
                    emScriptRunState=Wait_F1F1_0x6F_Finish;
                    QString msg=QString("68 2F 00 43 04 00 00 00 00 6D 88 88 88 99 99 04 04 33 00 00 00 00 F1 01 00 02 00 10 00 68 04 33 00 00 00 00 68 11 04 33 33 34 33 E9 16 A5 16"
                                        "68 2F 00 43 04 00 00 00 00 6E 88 88 88 99 99 04 04 33 00 00 00 00 F1 01 00 02 00 10 00 68 04 33 00 00 00 00 68 11 04 33 33 35 33 EA 16 A8 16").replace(" ","");
                    sendSrcMsg(dvcType,dvcId,QByteArray::fromHex(msg.toLatin1()));
                    msgSeq=0x6E;
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("并发抄表-Err0x6F 开始"));
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_F1F1_0x6F_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x02&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp&&uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq)==msgSeq)
                {
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("并发抄表-Err0x6F 结束"));
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "使用“暂停”命令暂停路由工作 开始...");
                    emScriptRunState=Wait_PauseRouter1_Finish;
                    QString msg=QString("68 0F 00 43 00 00 28 32 00 6F 12 02 00 20 16").replace(" ","");
                    sendSrcMsg(dvcType,dvcId,QByteArray::fromHex(msg.toLatin1()));
                    msgSeq=0x6F;
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_PauseRouter1_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp&&uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq)==msgSeq)
                {
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("使用“暂停”命令暂停路由工作 结束"));
                    InteroperateInit_GW::delayTime(10*1000);
                    emScriptRunState=Wait_ParameterInit_Finish;
                    QString msg=QString("68 0F 00 43 00 00 28 32 00 70 01 02 00 10 16").replace(" ","");
                    sendSrcMsg(dvcType,dvcId,QByteArray::fromHex(msg.toLatin1()));
                    msgSeq=0x70;
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("参数区初始化 开始"));
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_ParameterInit_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp&&uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq)==msgSeq)
                {
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("参数区初始化 结束"));
                    emScriptRunState=Wait_AddMeter_Finish;
                    QString msg=addMeters.first().replace(" ","");
                    addMeters.removeFirst();
                    sendSrcMsg(dvcType,dvcId,QByteArray::fromHex(msg.toLatin1()));
                    msgSeq=uchar(QByteArray::fromHex(msg.toLatin1()).at(9));
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("添加从节点，添加电表地址 开始"));
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_AddMeter_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp&&uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq)==msgSeq)
                {
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("添加从节点，添加电表地址 结束"));
                    if(addMeters.size()!=0)
                    {
                        emScriptRunState=Wait_AddMeter_Finish;
                        QString msg=addMeters.first().replace(" ","");
                        addMeters.removeFirst();
                        sendSrcMsg(dvcType,dvcId,QByteArray::fromHex(msg.toLatin1()));
                        msgSeq=uchar(QByteArray::fromHex(msg.toLatin1()).at(9));
                        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("添加从节点，添加电表地址 开始"));
                    }
                    else
                    {
                        emScriptRunState=Wait_F1F1_0x6D_Finish;
                        QStringList msgs=QStringList()<<"68 2F 00 43 04 00 00 00 00 CB 88 88 88 99 99 04 02 00 00 00 00 00 F1 01 00 02 00 10 00 68 01 00 00 00 00 00 68 11 04 33 33 34 33 B3 16 61 16"
                                                     <<"68 2F 00 43 04 00 00 00 00 CC 88 88 88 99 99 04 02 00 00 00 00 00 F1 01 00 02 00 10 00 68 02 00 00 00 00 00 68 11 04 33 33 34 33 B4 16 65 16"
                                                     <<"68 2F 00 43 04 00 00 00 00 CD 88 88 88 99 99 04 03 00 00 00 00 00 F1 01 00 02 00 10 00 68 03 00 00 00 00 00 68 11 04 33 33 34 33 B5 16 69 16"
                                                     <<"68 2F 00 43 04 00 00 00 00 CE 88 88 88 99 99 04 04 00 00 00 00 00 F1 01 00 02 00 10 00 68 04 00 00 00 00 00 68 11 04 33 33 34 33 B6 16 6D 16"
                                                     <<"68 2F 00 43 04 00 00 00 00 CF 88 88 88 99 99 04 05 00 00 00 00 00 F1 01 00 02 00 10 00 68 05 00 00 00 00 00 68 11 04 33 33 34 33 B7 16 71 16"
                                                     <<"68 2F 00 43 04 00 00 00 00 D0 88 88 88 99 99 04 06 00 00 00 00 00 F1 01 00 02 00 10 00 68 06 00 00 00 00 00 68 11 04 33 33 34 33 B8 16 75 16"
                                                     <<"68 2F 00 43 04 00 00 00 00 D1 88 88 88 99 99 04 07 00 00 00 00 00 F1 01 00 02 00 10 00 68 07 00 00 00 00 00 68 11 04 33 33 34 33 B9 16 79 16"
                                                     <<"68 2F 00 43 04 00 00 00 00 D2 88 88 88 99 99 04 08 00 00 00 00 00 F1 01 00 02 00 10 00 68 08 00 00 00 00 00 68 11 04 33 33 34 33 BA 16 7D 16"
                                                     <<"68 2F 00 43 04 00 00 00 00 D3 88 88 88 99 99 04 09 00 00 00 00 00 F1 01 00 02 00 10 00 68 09 00 00 00 00 00 68 11 04 33 33 34 33 BB 16 81 16"
                                                     <<"68 2F 00 43 04 00 00 00 00 D4 88 88 88 99 99 04 10 00 00 00 00 00 F1 01 00 02 00 10 00 68 10 00 00 00 00 00 68 11 04 33 33 34 33 C2 16 97 16"
                                                     <<"68 2F 00 43 04 00 00 00 00 D5 88 88 88 99 99 04 11 00 00 00 00 00 F1 01 00 02 00 10 00 68 11 00 00 00 00 00 68 11 04 33 33 34 33 C3 16 9B 16"
                                                     <<"68 2F 00 43 04 00 00 00 00 D6 88 88 88 99 99 04 12 00 00 00 00 00 F1 01 00 02 00 10 00 68 12 00 00 00 00 00 68 11 04 33 33 34 33 C4 16 9F 16"
                                                     <<"68 2F 00 43 04 00 00 00 00 D7 88 88 88 99 99 04 13 00 00 00 00 00 F1 01 00 02 00 10 00 68 13 00 00 00 00 00 68 11 04 33 33 34 33 C5 16 A3 16"
                                                     <<"68 2F 00 43 04 00 00 00 00 D8 88 88 88 99 99 04 14 00 00 00 00 00 F1 01 00 02 00 10 00 68 14 00 00 00 00 00 68 11 04 33 33 34 33 C6 16 A7 16"
                                                     <<"68 2F 00 43 04 00 00 00 00 D9 88 88 88 99 99 04 15 00 00 00 00 00 F1 01 00 02 00 10 00 68 15 00 00 00 00 00 68 11 04 33 33 34 33 C7 16 AB 16"
                                                     <<"68 2F 00 43 04 00 00 00 00 DA 88 88 88 99 99 04 16 00 00 00 00 00 F1 01 00 02 00 10 00 68 16 00 00 00 00 00 68 11 04 33 33 34 33 C8 16 AF 16"
                                                     <<"68 2F 00 43 04 00 00 00 00 DB 88 88 88 99 99 04 17 00 00 00 00 00 F1 01 00 02 00 10 00 68 17 00 00 00 00 00 68 11 04 33 33 34 33 C9 16 B3 16"
                                                     <<"68 2F 00 43 04 00 00 00 00 DC 88 88 88 99 99 04 18 00 00 00 00 00 F1 01 00 02 00 10 00 68 18 00 00 00 00 00 68 11 04 33 33 34 33 CA 16 B7 16"
                                                     <<"68 2F 00 43 04 00 00 00 00 DD 88 88 88 99 99 04 19 00 00 00 00 00 F1 01 00 02 00 10 00 68 19 00 00 00 00 00 68 11 04 33 33 34 33 CB 16 BB 16"
                                                     <<"68 2F 00 43 04 00 00 00 00 DE 88 88 88 99 99 04 20 00 00 00 00 00 F1 01 00 02 00 10 00 68 20 00 00 00 00 00 68 11 04 33 33 34 33 D2 16 D1 16"
                                                     <<"68 2F 00 43 04 00 00 00 00 DF 88 88 88 99 99 04 21 00 00 00 00 00 F1 01 00 02 00 10 00 68 21 00 00 00 00 00 68 11 04 33 33 34 33 D3 16 D5 16"
                                                     <<"68 2F 00 43 04 00 00 00 00 E0 88 88 88 99 99 04 22 00 00 00 00 00 F1 01 00 02 00 10 00 68 22 00 00 00 00 00 68 11 04 33 33 34 33 D4 16 D9 16"
                                                     <<"68 2F 00 43 04 00 00 00 00 E1 88 88 88 99 99 04 23 00 00 00 00 00 F1 01 00 02 00 10 00 68 23 00 00 00 00 00 68 11 04 33 33 34 33 D5 16 DD 16"
                                                     <<"68 2F 00 43 04 00 00 00 00 E2 88 88 88 99 99 04 24 00 00 00 00 00 F1 01 00 02 00 10 00 68 24 00 00 00 00 00 68 11 04 33 33 34 33 D6 16 E1 16"
                                                     <<"68 2F 00 43 04 00 00 00 00 E3 88 88 88 99 99 04 25 00 00 00 00 00 F1 01 00 02 00 10 00 68 25 00 00 00 00 00 68 11 04 33 33 34 33 D7 16 E5 16"
                                                     <<"68 2F 00 43 04 00 00 00 00 E4 88 88 88 99 99 04 26 00 00 00 00 00 F1 01 00 02 00 10 00 68 26 00 00 00 00 00 68 11 04 33 33 34 33 D8 16 E9 16"
                                                     <<"68 2F 00 43 04 00 00 00 00 E5 88 88 88 99 99 04 27 00 00 00 00 00 F1 01 00 02 00 10 00 68 27 00 00 00 00 00 68 11 04 33 33 34 33 D9 16 ED 16";
                        for(auto i:msgs)
                            sendSrcMsg(dvcType,dvcId,QByteArray::fromHex(i.replace(" ","").toLatin1()));
                        p_timer->start(90*1000);
                        msgSeq=0xCB;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("并发抄表-Err0x6D 开始"));
                    }
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_F1F1_0x6D_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x02&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp&&dynamic_pointer_cast<Afn00F2>(p_Frame3762Base)->error_code_==0x6D)
                {
                    p_timer->stop();
                    uchar count=uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq)-msgSeq;
                    if(count<20)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("最大并发数小于20条"));
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("宽带增补内容测试流程:流程测试异常"));
                        return;
                    }
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("并发抄表最大并发数为:%1").arg(count));
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("并发抄表-Err0x6D 结束"));
                    emScriptRunState=Wait_RecoverRouter_Finish;
                    QString msg=QString("68 0F 00 43 00 00 28 32 00 E6 12 04 00 99 16").replace(" ","");
                    sendSrcMsg(dvcType,dvcId,QByteArray::fromHex(msg.toLatin1()));
                    msgSeq=0xE6;
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("向路由发送恢复命令AFN=12H-F3 开始"));
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_RecoverRouter_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp&&uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq)==msgSeq)
                {
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("向路由发送恢复命令AFN=12H-F3 结束"));
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("宽带增补内容测试流程:流程测试成功"));
                    p_AbstractScriptHost->updateProgress(ProcessState_Success, QString("宽带增补内容测试流程:流程测试成功"));
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

void Script_HplcSupplementFlow::processMsgFromMeter0x00(DvcType dvcType, int dvcId, QByteArray msg)
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
    switch(emScriptRunState)
    {
        case Wait_F1F1_Finish:
        {
            if(msg.contains(QByteArray::fromHex(QString("ABCD").toLatin1())))
            {
                QByteArray tmpSendMsg=QByteArray::fromHex(QString("FE FE 04 33 00 00 00 00 03 15 11 19 53 19 11 15 03 23 20 FE").replace(" ","").toLatin1());
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        default:
            break;
    }
}

void Script_HplcSupplementFlow::processMsgFromMeter0x01(DvcType dvcType, int dvcId, QByteArray msg)
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
    switch(emScriptRunState)
    {
        case Wait_F1F1_Finish:
        {
            if(msg.contains(QByteArray::fromHex(QString("680433").toLatin1())))
            {
                QByteArray tmpSendMsg=QByteArray::fromHex(QString("68 04 33 00 00 00 00 68 81 06 43 C3 A4 89 67 45 6D 16").replace(" ","").toLatin1());
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        default:
            break;
    }
}

void Script_HplcSupplementFlow::processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID)
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

void Script_HplcSupplementFlow::processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID)
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
            case Wait_F1F1_Finish:
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

void Script_HplcSupplementFlow::sendMsg(DvcType , int , int , shared_ptr<void> )
{

}

void Script_HplcSupplementFlow::timer_timeout()
{
    switch(emScriptRunState)
    {
        default:
        {
            p_timer->stop();
            p_AbstractScriptHost->updateProgress(ProcessState_Error, QString("timeout p_timer!!!"));
            break;
        }
    }
}

void Script_HplcSupplementFlow::maxAllowTimer_timeout()
{
    p_maxAllowTimer->stop();
    p_AbstractScriptHost->updateProgress(ProcessState_Error, QString("timeout p_maxAllowTimer!!!"));
}

void Script_HplcSupplementFlow::delayTimer_timeout()
{

}
