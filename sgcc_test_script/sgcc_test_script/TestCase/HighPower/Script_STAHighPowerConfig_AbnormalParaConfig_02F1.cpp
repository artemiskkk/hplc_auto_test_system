#include "Script_STAHighPowerConfig_AbnormalParaConfig_02F1.h"

Script_STAHighPowerConfig_AbnormalParaConfig_02F1::Script_STAHighPowerConfig_AbnormalParaConfig_02F1(QObject *parent) : QObject(parent)
{
    emScriptRunState=TestInit;
    resultFlag=false;
    sendMsgOct.clear();

    p_CtrInfoList=make_shared<QList<shared_ptr<CtrInfo>>>();
    p_BuildNetwork_GW=new BuildNetwork_GW();

    p_Frame3762Helper=make_shared<Frame3762Helper>();

    p_QueryNetScale_10F9=make_shared<Afn10F9>();
    p_TransparentTransmit_02F1=make_shared<Afn02F1>();

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
Script_STAHighPowerConfig_AbnormalParaConfig_02F1::~Script_STAHighPowerConfig_AbnormalParaConfig_02F1()
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

void Script_STAHighPowerConfig_AbnormalParaConfig_02F1::execute()
{
    p_AbstractScriptHost->updateProgress(ProcessState_Start, "Test start!");

    resultFlag=false;

    if(needBuildNet==true)
    {
        p_BuildNetwork_GW->execute();
        emScriptRunState=Wait_BuildNetFinish_Whole;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "等待全网组网完成");
        p_timer->start((timerForReachThresld+timerAfterReachThresld)*1000);
    }
    else
    {
        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryNetScale_10F9);
        emScriptRunState=Wait_QueryNetScale_10F9_Finish;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询网络规模（10F9），等待--回复");
        p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
    }
    p_maxAllowTimer->start((timerForReachThresld+timerAfterReachThresld)*1000);
}

void Script_STAHighPowerConfig_AbnormalParaConfig_02F1::stop()
{
    if(p_AbstractScriptHost==nullptr)
        return;
    p_AbstractScriptHost->updateProgress(ProcessState_Error, "Test stop!");
    if(needPowerOff==true)
        powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost);
}

void Script_STAHighPowerConfig_AbnormalParaConfig_02F1::addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq)
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


void Script_STAHighPowerConfig_AbnormalParaConfig_02F1::setHost(AbstractScriptHost *host)
{
    p_AbstractScriptHost=host;
    p_BuildNetwork_GW->setHost(host);
}
bool Script_STAHighPowerConfig_AbnormalParaConfig_02F1::config(const QMap<QString,QString> *paraDic)
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

void Script_STAHighPowerConfig_AbnormalParaConfig_02F1::processMsg(DvcType dvcType,int id,uchar* data,int datalen)
{
    if(p_CtrInfoList->size()==0)
        return;
    if(emScriptRunState==Wait_BuildNetFinish_Whole)
    {
        if(!p_BuildNetwork_GW->buildNetworkResultFlag)
            p_BuildNetwork_GW->processMsg(dvcType,id,data,datalen);
        else
        {
            p_timer->stop();
            if(p_BuildNetwork_GW->emScriptRunState==BuildNetFinish&&p_BuildNetwork_GW->buildNetworkResultFlag==true)
            {
                tryTimes=0;
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryNetScale_10F9);
                emScriptRunState=Wait_QueryNetScale_10F9_Finish;
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询网络规模（10F9），等待--回复");
                p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
                p_maxAllowTimer->start(timerForReachThresld*1000);
            }
        }
    }
    else
    {
        QByteArray tmpRecvTempData=QByteArray::fromRawData(reinterpret_cast<char*>(data),datalen);
        QByteArray recvTempData;
        recvTempData.append(tmpRecvTempData);
        delete[] data;

        if(p_CtrInfoList->size()==0)
            return;

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
                if(dvcType==SingleSTA)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到单通报文：%1").arg(QString(recvTempData.toHex())));
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到三通报文：%1").arg(QString(recvTempData.toHex())));
                }
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

void Script_STAHighPowerConfig_AbnormalParaConfig_02F1::processCtrlDvcRes(DvcType dvcType,QList<int> idList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params)
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
        case Wait_QueryNetScale_10F9_Finish:
        {
            break;
        }
        case Wait_TransparentTransmit_02F1_Finish:
        {
            break;
        }
        case Wait_SetSuperpowerPara_FFFF_Finish:
        {
            break;
        }
        case Wait_QuerySuperpowerPara_FFFF_Finish:
        {
            break;
        }
        case Wait_SetSuperpowerPara_0A1A_Finish:
        {
            break;
        }
        case Wait_QuerySuperpowerPara_0A1A_Finish:
        {
             break;
        }
        case Wait_SetSuperpowerPara_0309_Finish:
        {
            break;
        }
        case Wait_QuerySuperpowerPara_0309_Finish:
        {
            break;
        }
        case Wait_SetSuperpowerPara_151A_Finish:
        {
             break;
        }
        case Wait_QuerySuperpowerPara_151A_Finish:
        {
            break;
        }
        case ScriptSuccess:
        {
            break;
        }
    }
}
void Script_STAHighPowerConfig_AbnormalParaConfig_02F1::sendSrcMsg(DvcType dvcType, int dvcId, QByteArray msg)
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

void Script_STAHighPowerConfig_AbnormalParaConfig_02F1::processMsgFromCCO(DvcType dvcType, int dvcId)
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
            case Wait_QueryNetScale_10F9_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x10&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x01&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();//接收到一条完整报文
                    shared_ptr<Afn10F9> p_QueryNetScale_10F9_Up=dynamic_pointer_cast<Afn10F9>(p_Frame3762Base);
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("当前网络规模为%1").arg(p_QueryNetScale_10F9_Up->network_scale_));
                    if(p_QueryNetScale_10F9_Up->network_scale_==p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size()+1)
                    {
                        em02F1_State=Query_02F1;
                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_TransparentTransmit_02F1);
                        emScriptRunState=Wait_TransparentTransmit_02F1_Finish;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--透传查询从节点大功率参数（02F1），等待--回复");
                        p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("当前网络规模为%1，不符合要求").arg(p_QueryNetScale_10F9_Up->network_scale_));
                    }
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_TransparentTransmit_02F1_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x02&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    //存储上报的DAC和削峰因子，
                    p_timer->stop();
                    shared_ptr<Afn02F1> p_TransparentTransmit_02F1_Up=dynamic_pointer_cast<Afn02F1>(p_Frame3762Base);
//                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("上报数据长度为%1").arg(p_TransparentTransmit_02F1_Up->frame_length_));
                    if(p_TransparentTransmit_02F1_Up->frame_length_==0x22
                            &&((p_TransparentTransmit_02F1_Up->frame_content_.at(28)>=4&&p_TransparentTransmit_02F1_Up->frame_content_.at(28)<=20))
                            &&((p_TransparentTransmit_02F1_Up->frame_content_.at(33)>=5&&p_TransparentTransmit_02F1_Up->frame_content_.at(33)<=25)||p_TransparentTransmit_02F1_Up->frame_content_.at(33)==0x00))
                    {
                        NodeInfo_Struct nodePara;
                        nodePara.dac_voltage=p_TransparentTransmit_02F1_Up->frame_content_.at(28);
                        nodePara.peak_clipping_factor=p_TransparentTransmit_02F1_Up->frame_content_.at(33);
                        nodeParaList.append(nodePara);

                        index++;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("透传回复数据:%1").arg(QString::fromLatin1(p_TransparentTransmit_02F1_Up->frame_content_.toHex().toUpper())));
                        if(index==p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size())
                        {
                            if((1-(nodeInfoList.size()/p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size()))>=0.9)
                            {
                                nodeInfoList.clear();
                                p_AbstractScriptHost->updateProgress(ProcessState_Processing,"02F1查询STA大功率参数-测试成功");

                                index=0;
                                em02F1_State=Set_02F1;
                                emConfigSuperpowerParaState = Wait_Config_FFFF_Finish;
                                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_TransparentTransmit_02F1);
                                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "02F1配置STA大功率参数_FFFF，等待--确认");
                                emScriptRunState=Wait_SetSuperpowerPara_FFFF_Finish;
                                p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
                            }
                            else
                            {
                                p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("透传成功率为：%0，测试失败").arg(QString(1-(nodeInfoList.size()/p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size()))));
                            }
                        }
                        else
                        {
                            tryTimes=0;
                            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_TransparentTransmit_02F1);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "透传STA功率参数（02F1），等待--回复");
                            p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
                        }
                    }
                    else if(p_TransparentTransmit_02F1_Up->frame_length_==0)
                    {
                        if(++tryTimes>=3)
                        {
                            NodeInfo_Struct eventST;
                            eventST.deviceType=p_TransparentTransmit_02F1->protocol_type_;
                            memcpy(eventST.nodeAddress.addr,p_TransparentTransmit_02F1->address_field_.dst_addr,6);
                            nodeInfoList.append(eventST);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("查询STA大功率参数失败的节点号为：%1").arg(QString(QByteArray(eventST.nodeAddress.addr,6).toHex())));
                            index++;
                            p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("脚本执行失败，%1").arg(test_name_));
                        }
                        else
                        {
                            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_TransparentTransmit_02F1);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "透传回复数据为空，重发02F1--");
                            p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
                        }
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("透传回复数据异常，数据:%1").arg(QString::fromLatin1(p_TransparentTransmit_02F1_Up->frame_content_.toHex().toUpper())));
                    }
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_SetSuperpowerPara_FFFF_Finish:
            {
                if(p_Frame3762Base->afn_ == char(0x02)&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    shared_ptr<Afn02F1> p_Config_02F1_Up=dynamic_pointer_cast<Afn02F1>(p_Frame3762Base);
                    index++;
                    if(p_Config_02F1_Up->frame_length_==char(0x1e)
                            &&p_Config_02F1_Up->frame_content_.at(26)==03
                            &&p_Config_02F1_Up->frame_content_.at(29)==03)
                    {
                        if(index==p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size())
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "02F1配置STA大功率参数_FFFF完成");
                            index=0;

                            em02F1_State=Query_02F1;
                            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_TransparentTransmit_02F1);
                            emScriptRunState=Wait_QuerySuperpowerPara_FFFF_Finish;
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--透传查询从节点大功率参数（02F1），等待--回复");
                            p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
                        }
                        else
                        {
                            em02F1_State=Set_02F1;
                            emConfigSuperpowerParaState = Wait_Config_FFFF_Finish;
                            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_TransparentTransmit_02F1);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "02F1配置STA大功率参数_FFFF，等待--确认");
                            emScriptRunState=Wait_SetSuperpowerPara_FFFF_Finish;
                            p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
                        }

                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("透传回复数据异常，数据:%1").arg(QString::fromLatin1(p_Config_02F1_Up->frame_content_.toHex().toUpper())));
                    }
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_QuerySuperpowerPara_FFFF_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x02&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    //存储上报的DAC和削峰因子，
                    p_timer->stop();
                    shared_ptr<Afn02F1> p_TransparentTransmit_02F1_Up=dynamic_pointer_cast<Afn02F1>(p_Frame3762Base);
                    if(p_TransparentTransmit_02F1_Up->frame_length_==0x22
                            &&(p_TransparentTransmit_02F1_Up->frame_content_.at(28)==nodeParaList.at(index).dac_voltage)
                            &&(p_TransparentTransmit_02F1_Up->frame_content_.at(33)==nodeParaList.at(index).peak_clipping_factor))
                    {
                        index++;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("透传回复数据:%1").arg(QString::fromLatin1(p_TransparentTransmit_02F1_Up->frame_content_.toHex().toUpper())));
                        if(index==p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size())
                        {
                            if((1-(nodeInfoList.size()/p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size()))>=0.9)
                            {
                                nodeInfoList.clear();
                                p_AbstractScriptHost->updateProgress(ProcessState_Processing,"02F1查询STA大功率参数-测试成功");

                                index=0;
                                em02F1_State=Set_02F1;
                                emConfigSuperpowerParaState = Wait_Config_0A1A_Finish;
                                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_TransparentTransmit_02F1);
                                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "02F1配置STA大功率参数_0A1A，等待--确认");
                                emScriptRunState=Wait_SetSuperpowerPara_0A1A_Finish;
                                p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
                            }
                            else
                            {
                                p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("透传成功率为：%0，测试失败").arg(QString(1-(nodeInfoList.size()/p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size()))));
                            }
                        }
                        else
                        {
                            tryTimes=0;
                            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_TransparentTransmit_02F1);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "透传STA功率参数（02F1），等待--回复");
                            p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
                        }
                    }
                    else if(p_TransparentTransmit_02F1_Up->frame_length_==0)
                    {
                        if(++tryTimes>=3)
                        {
                            NodeInfo_Struct eventST;
                            eventST.deviceType=p_TransparentTransmit_02F1->protocol_type_;
                            memcpy(eventST.nodeAddress.addr,p_TransparentTransmit_02F1->address_field_.dst_addr,6);
                            nodeInfoList.append(eventST);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("查询STA大功率参数失败的节点号为：%1").arg(QString(QByteArray(eventST.nodeAddress.addr,6).toHex())));
                            index++;
                            p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("脚本执行失败，%1").arg(test_name_));
                        }
                        else
                        {
                            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_TransparentTransmit_02F1);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "透传回复数据为空，重发02F1--");
                            p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
                        }
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("透传回复数据异常，数据:%1").arg(QString::fromLatin1(p_TransparentTransmit_02F1_Up->frame_content_.toHex().toUpper())));
                    }
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_SetSuperpowerPara_0A1A_Finish:
            {
                if(p_Frame3762Base->afn_ == char(0x02)&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    shared_ptr<Afn02F1> p_Config_02F1_Up=dynamic_pointer_cast<Afn02F1>(p_Frame3762Base);
                    index++;
                    if(p_Config_02F1_Up->frame_length_==char(0x1e)
                            &&p_Config_02F1_Up->frame_content_.at(26)==00
                            &&p_Config_02F1_Up->frame_content_.at(29)==03)
                    {
                        if(index==p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size())
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "02F1配置STA大功率参数_OA1A完成");
                            index=0;

                            em02F1_State=Query_02F1;
                            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_TransparentTransmit_02F1);
                            emScriptRunState=Wait_QuerySuperpowerPara_0A1A_Finish;
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--透传查询从节点大功率参数（02F1），等待--回复");
                            p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
                        }
                        else
                        {
                            em02F1_State=Set_02F1;
                            emConfigSuperpowerParaState = Wait_Config_0A1A_Finish;
                            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_TransparentTransmit_02F1);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "02F1配置STA大功率参数_0A1A，等待--确认");
                            emScriptRunState=Wait_SetSuperpowerPara_0A1A_Finish;
                            p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
                        }

                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("透传回复数据异常，数据:%1").arg(QString::fromLatin1(p_Config_02F1_Up->frame_content_.toHex().toUpper())));
                    }
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_QuerySuperpowerPara_0A1A_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x02&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    //存储上报的DAC和削峰因子，
                    p_timer->stop();
                    shared_ptr<Afn02F1> p_TransparentTransmit_02F1_Up=dynamic_pointer_cast<Afn02F1>(p_Frame3762Base);
                    if(p_TransparentTransmit_02F1_Up->frame_length_==0x22
                            &&(p_TransparentTransmit_02F1_Up->frame_content_.at(28)==char(0x0A))
                            &&(p_TransparentTransmit_02F1_Up->frame_content_.at(33)==nodeParaList.at(index).peak_clipping_factor))
                    {
                        index++;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("透传回复数据:%1").arg(QString::fromLatin1(p_TransparentTransmit_02F1_Up->frame_content_.toHex().toUpper())));
                        if(index==p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size())
                        {
                            if((1-(nodeInfoList.size()/p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size()))>=0.9)
                            {
                                nodeInfoList.clear();
                                p_AbstractScriptHost->updateProgress(ProcessState_Processing,"02F1查询STA大功率参数0A1A-测试完成");

                                index=0;
                                em02F1_State=Set_02F1;
                                emConfigSuperpowerParaState = Wait_Config_0309_Finish;
                                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_TransparentTransmit_02F1);
                                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "02F1配置STA大功率参数_0309，等待--确认");
                                emScriptRunState=Wait_SetSuperpowerPara_0309_Finish;
                                p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
                            }
                            else
                            {
                                p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("透传成功率为：%0，测试失败").arg(QString(1-(nodeInfoList.size()/p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size()))));
                            }
                        }
                        else
                        {
                            tryTimes=0;
                            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_TransparentTransmit_02F1);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "透传STA功率参数（02F1），等待--回复");
                            p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
                        }
                    }
                    else if(p_TransparentTransmit_02F1_Up->frame_length_==0)
                    {
                        if(++tryTimes>=3)
                        {
                            NodeInfo_Struct eventST;
                            eventST.deviceType=p_TransparentTransmit_02F1->protocol_type_;
                            memcpy(eventST.nodeAddress.addr,p_TransparentTransmit_02F1->address_field_.dst_addr,6);
                            nodeInfoList.append(eventST);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("查询STA大功率参数失败的节点号为：%1").arg(QString(QByteArray(eventST.nodeAddress.addr,6).toHex())));
                            index++;
                            p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("脚本执行失败，%1").arg(test_name_));
                        }
                        else
                        {
                            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_TransparentTransmit_02F1);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "透传回复数据为空，重发02F1--");
                            p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
                        }
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("透传回复数据异常，数据:%1").arg(QString::fromLatin1(p_TransparentTransmit_02F1_Up->frame_content_.toHex().toUpper())));
                    }
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_SetSuperpowerPara_0309_Finish:
            {
                if(p_Frame3762Base->afn_ == char(0x02)&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    shared_ptr<Afn02F1> p_Config_02F1_Up=dynamic_pointer_cast<Afn02F1>(p_Frame3762Base);
                    index++;
                    if(p_Config_02F1_Up->frame_length_==char(0x1e)
                            &&p_Config_02F1_Up->frame_content_.at(26)==03
                            &&p_Config_02F1_Up->frame_content_.at(29)==00)
                    {
                        if(index==p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size())
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "02F1配置STA大功率参数_0309完成");
                            index=0;

                            em02F1_State=Query_02F1;
                            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_TransparentTransmit_02F1);
                            emScriptRunState=Wait_QuerySuperpowerPara_0309_Finish;
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--透传查询从节点大功率参数（02F1），等待--回复");
                            p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
                        }
                        else
                        {
                            em02F1_State=Set_02F1;
                            emConfigSuperpowerParaState = Wait_Config_0309_Finish;
                            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_TransparentTransmit_02F1);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "02F1配置STA大功率参数_0A1A，等待--确认");
                            emScriptRunState=Wait_SetSuperpowerPara_0309_Finish;
                            p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
                        }

                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("透传回复数据异常，数据:%1").arg(QString::fromLatin1(p_Config_02F1_Up->frame_content_.toHex().toUpper())));
                    }
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_QuerySuperpowerPara_0309_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x02&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    shared_ptr<Afn02F1> p_TransparentTransmit_02F1_Up=dynamic_pointer_cast<Afn02F1>(p_Frame3762Base);
                    if(p_TransparentTransmit_02F1_Up->frame_length_==0x22
                            &&(p_TransparentTransmit_02F1_Up->frame_content_.at(28)==char(0x0A))
                            &&(p_TransparentTransmit_02F1_Up->frame_content_.at(33)==char(0x09)))
                    {
                        index++;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("透传回复数据:%1").arg(QString::fromLatin1(p_TransparentTransmit_02F1_Up->frame_content_.toHex().toUpper())));
                        if(index==p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size())
                        {
                            if((1-(nodeInfoList.size()/p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size()))>=0.9)
                            {
                                nodeInfoList.clear();
                                p_AbstractScriptHost->updateProgress(ProcessState_Processing,"02F1查询STA大功率参数0A1A-测试完成");

                                index=0;
                                em02F1_State=Set_02F1;
                                emConfigSuperpowerParaState = Wait_Config_151A_Finish;
                                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_TransparentTransmit_02F1);
                                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "02F1配置STA大功率参数_151A，等待--确认");
                                emScriptRunState=Wait_SetSuperpowerPara_151A_Finish;
                                p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
                            }
                            else
                            {
                                p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("透传成功率为：%0，测试失败").arg(QString(1-(nodeInfoList.size()/p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size()))));
                            }
                        }
                        else
                        {
                            tryTimes=0;
                            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_TransparentTransmit_02F1);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "透传STA功率参数（02F1），等待--回复");
                            p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
                        }
                    }
                    else if(p_TransparentTransmit_02F1_Up->frame_length_==0)
                    {
                        if(++tryTimes>=3)
                        {
                            NodeInfo_Struct eventST;
                            eventST.deviceType=p_TransparentTransmit_02F1->protocol_type_;
                            memcpy(eventST.nodeAddress.addr,p_TransparentTransmit_02F1->address_field_.dst_addr,6);
                            nodeInfoList.append(eventST);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("查询STA大功率参数失败的节点号为：%1").arg(QString(QByteArray(eventST.nodeAddress.addr,6).toHex())));
                            index++;
                            p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("脚本执行失败，%1").arg(test_name_));
                        }
                        else
                        {
                            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_TransparentTransmit_02F1);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "透传回复数据为空，重发02F1--");
                            p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
                        }
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("透传回复数据异常，数据:%1").arg(QString::fromLatin1(p_TransparentTransmit_02F1_Up->frame_content_.toHex().toUpper())));
                    }
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_SetSuperpowerPara_151A_Finish:
            {
                if(p_Frame3762Base->afn_ == char(0x02)&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    shared_ptr<Afn02F1> p_Config_02F1_Up=dynamic_pointer_cast<Afn02F1>(p_Frame3762Base);
                    index++;
                    if(p_Config_02F1_Up->frame_length_==char(0x1e)
                            &&p_Config_02F1_Up->frame_content_.at(26)==03
                            &&p_Config_02F1_Up->frame_content_.at(29)==03)
                    {
                        if(index==p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size())
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "02F1配置STA大功率参数_151A完成");
                            index=0;

                            em02F1_State=Query_02F1;
                            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_TransparentTransmit_02F1);
                            emScriptRunState=Wait_QuerySuperpowerPara_151A_Finish;
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--透传查询从节点大功率参数（02F1），等待--回复");
                            p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
                        }
                        else
                        {
                            em02F1_State=Set_02F1;
                            emConfigSuperpowerParaState = Wait_Config_151A_Finish;
                            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_TransparentTransmit_02F1);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "02F1配置STA大功率参数_0A1A，等待--确认");
                            emScriptRunState=Wait_SetSuperpowerPara_151A_Finish;
                            p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
                        }

                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("透传回复数据异常，数据:%1").arg(QString::fromLatin1(p_Config_02F1_Up->frame_content_.toHex().toUpper())));
                    }
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_QuerySuperpowerPara_151A_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x02&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    shared_ptr<Afn02F1> p_TransparentTransmit_02F1_Up=dynamic_pointer_cast<Afn02F1>(p_Frame3762Base);
                    if(p_TransparentTransmit_02F1_Up->frame_length_==0x22
                            &&(p_TransparentTransmit_02F1_Up->frame_content_.at(28)==char(0x0A))
                            &&(p_TransparentTransmit_02F1_Up->frame_content_.at(33)==char(0x09)))
                    {
                        index++;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("透传回复数据:%1").arg(QString::fromLatin1(p_TransparentTransmit_02F1_Up->frame_content_.toHex().toUpper())));
                        if(index==p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size())
                        {
                            if((1-(nodeInfoList.size()/p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size()))>=0.9)
                            {
                                p_AbstractScriptHost->updateProgress(ProcessState_Processing,"测试成功【GW-STA-F021-0010-V01】大功率配置-异常参数配置（02F1）");

                                nodeInfoList.clear();
                                nodeParaList.clear();
                                emScriptRunState = ScriptSuccess;
                                resultFlag=true;
                                p_AbstractScriptHost->updateProgress(ProcessState_Success,QString("脚本执行成功，%1").arg(test_name_));
                            }
                            else
                            {
                                p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("透传成功率为：%0，测试失败").arg(QString(1-(nodeInfoList.size()/p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size()))));
                            }
                        }
                        else
                        {
                            tryTimes=0;
                            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_TransparentTransmit_02F1);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "透传STA功率参数（02F1），等待--回复");
                            p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
                        }
                    }
                    else if(p_TransparentTransmit_02F1_Up->frame_length_==0)
                    {
                        if(++tryTimes>=3)
                        {
                            NodeInfo_Struct eventST;
                            eventST.deviceType=p_TransparentTransmit_02F1->protocol_type_;
                            memcpy(eventST.nodeAddress.addr,p_TransparentTransmit_02F1->address_field_.dst_addr,6);
                            nodeInfoList.append(eventST);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("查询STA大功率参数失败的节点号为：%1").arg(QString(QByteArray(eventST.nodeAddress.addr,6).toHex())));
                            index++;
                            p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("脚本执行失败，%1").arg(test_name_));
                        }
                        else
                        {
                            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_TransparentTransmit_02F1);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "透传回复数据为空，重发02F1--");
                            p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
                        }
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("透传回复数据异常，数据:%1").arg(QString::fromLatin1(p_TransparentTransmit_02F1_Up->frame_content_.toHex().toUpper())));
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

void Script_STAHighPowerConfig_AbnormalParaConfig_02F1::processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID)
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

void Script_STAHighPowerConfig_AbnormalParaConfig_02F1::processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID)
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

void Script_STAHighPowerConfig_AbnormalParaConfig_02F1::sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame)
{
    sendMsgOct.clear();
    if(frame==p_TransparentTransmit_02F1)
    {
        p_TransparentTransmit_02F1->ctrl_field_.dir=kDirDown;
        p_TransparentTransmit_02F1->ctrl_field_.prm=kActive;
        p_TransparentTransmit_02F1->ctrl_field_.comn_type=kHplc;

        p_TransparentTransmit_02F1->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_TransparentTransmit_02F1->info_field_.info_field_down.comu_rate=0;
        p_TransparentTransmit_02F1->info_field_.info_field_down.comu_module_ident=1;

        uchar tmpAddr[6];
        memcpy(tmpAddr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(index)->mtrAddr,6);

        memcpy(&p_TransparentTransmit_02F1->address_field_.src_addr,p_CtrInfoList->at(0)->ccoAddr,6);
        memcpy(&p_TransparentTransmit_02F1->address_field_.dst_addr,tmpAddr,6);

        p_TransparentTransmit_02F1->protocol_type_=0x04;
//        p_TransparentTransmit_02F1->frame_content_=getQueryFrame(DAC);
        switch(em02F1_State)
        {
            case Set_02F1:
            {
                p_TransparentTransmit_02F1->frame_content_=getSetFrame(DAC);
                break;
            }
            case Query_02F1:
            {
                p_TransparentTransmit_02F1->frame_content_=getQueryFrame(DAC);
                break;
            }
        }

        p_TransparentTransmit_02F1->frame_length_=uchar(p_TransparentTransmit_02F1->frame_content_.size());
        sendMsgOct=p_TransparentTransmit_02F1->EncodeFrame();
        sendMsgLog=QString("》》透传STA功率参数02F1：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
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
    else
    {
        return;
    }
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, sendMsgLog);

    uchar *sendMsg=new uchar[uint(sendMsgOct.size())];
    memcpy(sendMsg,reinterpret_cast<uchar*>(sendMsgOct.data()),uint(sendMsgOct.size()));
    p_AbstractScriptHost->sendMsg2Dvc(dvcType,dvcId,sendMsg,sendMsgOct.size());
}

void Script_STAHighPowerConfig_AbnormalParaConfig_02F1::timer_timeoutProc()
{
    p_timer->stop();
    switch(emScriptRunState)
    {
        case Wait_BuildNetFinish_Whole:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_BuildNetFinish_Whole timeout!!!");
            break;
        }
        case Wait_QueryNetScale_10F9_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_QueryNetScale_10F9_Finish timeout!!!");
            break;
        }
        case Wait_TransparentTransmit_02F1_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_TransparentTransmit_02F1_Finish timeout!!!");
            break;
        }
        default:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==p_timer");
            break;
        }
    }
}

void Script_STAHighPowerConfig_AbnormalParaConfig_02F1::delayTimer_timeoutProc()
{
    p_delayTimer->stop();

    switch(emScriptRunState)
    {
    default:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==p_timer");
        emScriptRunState=ScriptSuccess;
        break;
    }
    }
}
QByteArray Script_STAHighPowerConfig_AbnormalParaConfig_02F1::getQueryFrame(ParaType type)
{
    QByteArray msg;
    shared_ptr<ChkParam_Down> p_ChkParam_Down=make_shared<ChkParam_Down>();
    QList<ushort> chkParamIdList;
    if(type==DAC)
    {
        chkParamIdList.append(39);
        chkParamIdList.append(40);
    }
    else if(type==FrameDetect)
    {
        chkParamIdList.append(41);
        chkParamIdList.append(42);
    }
    else if(type==Suppress)
    {
        chkParamIdList.append(43);
    }
    else if(type==RelateThreshold)
    {
        chkParamIdList.append(44);
    }

    uchar tmpCcoAddr[6];
    uchar tmpStaAddr[6];
    memcpy(tmpCcoAddr,p_CtrInfoList->at(0)->ccoAddr,6);
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("index = %1").arg(index));
    memcpy(tmpStaAddr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(index)->mtrAddr,6);//index表示对应目的地址，随着发送的目的地址而改变

    p_ChkParam_Down->msgSeq=msgSeq;
    memcpy(p_ChkParam_Down->srcAddr,tmpCcoAddr,6);
    memcpy(p_ChkParam_Down->dstAddr,tmpStaAddr,6);
    p_ChkParam_Down->idList=chkParamIdList;
    p_ChkParam_Down->idCnt=ushort(chkParamIdList.size());

    msg=ChkParam_Down::encode_ChkParam_Down(p_ChkParam_Down);
    return msg;
}
QByteArray Script_STAHighPowerConfig_AbnormalParaConfig_02F1::getSetFrame(ParaType type)
{
    QByteArray msg;
    shared_ptr<SetParam_Down> p_SetParam_Down=make_shared<SetParam_Down>();
    QList<ParamInfo> setParamInfoList;
    if(type==DAC)
    {
        switch (emConfigSuperpowerParaState)
        {
        case Wait_Config_FFFF_Finish:
        {
            vol=char(0xFF);
            factor=char(0xFF);
            break;
        }
        case Wait_Config_0A1A_Finish:
        {
            vol=char(0x0A);
            factor=char(0x1A);
            break;
        }
        case Wait_Config_0309_Finish:
        {
            vol=char(0x03);
            factor=char(0x09);
            break;
        }
        case Wait_Config_151A_Finish:
        {
            vol=char(0x15);
            factor=char(0x1A);
            break;
        }
//        default:
//        {
//            p_AbstractScriptHost->updateProgress(ProcessState_Error, "emConfigSuperpowerParaState State machine run error!!!");
//            break;
//        }
        }
        ParamInfo tmpParamInfo;
        tmpParamInfo.id=39;
        tmpParamInfo.idLen=1;
        tmpParamInfo.idCntnt.append(vol);
        setParamInfoList.append(tmpParamInfo);
        ParamInfo tmpParamInfo1;
        tmpParamInfo1.id=40;
        tmpParamInfo1.idLen=1;
        tmpParamInfo1.idCntnt.append(factor);
        setParamInfoList.append(tmpParamInfo1);
    }
    else if(type==FrameDetect)
    {
        ParamInfo tmpParamInfo;
        tmpParamInfo.id=41;
        tmpParamInfo.idLen=4;
        tmpParamInfo.idCntnt=QByteArray::fromHex(multi.toLatin1());
        setParamInfoList.append(tmpParamInfo);

        tmpParamInfo.id=42;
        tmpParamInfo.idLen=4;
        tmpParamInfo.idCntnt=QByteArray::fromHex(threshold.toLatin1());
        setParamInfoList.append(tmpParamInfo);
    }
    else if(type==Suppress)
    {
        ParamInfo tmpParamInfo;
        tmpParamInfo.id=43;
        tmpParamInfo.idLen=4;
        tmpParamInfo.idCntnt=QByteArray::fromHex(suppress.toLatin1());
        setParamInfoList.append(tmpParamInfo);
    }
    else if(type==RelateThreshold)
    {
        ParamInfo tmpParamInfo;
        tmpParamInfo.id=44;
        tmpParamInfo.idLen=1;
        tmpParamInfo.idCntnt.append(char(relateThreshold));
        setParamInfoList.append(tmpParamInfo);
    }

    uchar tmpCcoAddr[6];
    uchar tmpStaAddr[6];
    memcpy(tmpCcoAddr,p_CtrInfoList->at(0)->ccoAddr,6);
    memcpy(tmpStaAddr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(index)->mtrAddr,6);
    p_SetParam_Down->msgSeq=msgSeq;
    memcpy(p_SetParam_Down->srcAddr,tmpCcoAddr,6);
    memcpy(p_SetParam_Down->dstAddr,tmpStaAddr,6);
    p_SetParam_Down->paramInfoList=setParamInfoList;
    p_SetParam_Down->idCnt=ushort(setParamInfoList.size());

    msg=SetParam_Down::encode_SetParam_Down(p_SetParam_Down);
    return msg;
}
void Script_STAHighPowerConfig_AbnormalParaConfig_02F1::maxAllowTimer_timeoutProc()
{
    p_maxAllowTimer->stop();
    switch(emScriptRunState)
    {
        case Wait_BuildNetFinish_Whole:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_BuildNetFinish_Whole maxAllow timeout!!!");
            break;
        }
        case Wait_QueryNetScale_10F9_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_QueryNetScale_10F9_Finish maxAllow timeout!!!");
            break;
        }
        case Wait_TransparentTransmit_02F1_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_TransparentTransmit_02F1_Finish maxAllow timeout!!!");
            break;
        }
        case Wait_SetSuperpowerPara_FFFF_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_SetSuperpowerPara_FFFF_Finish maxAllow timeout!!!");
            break;
        }
        case Wait_QuerySuperpowerPara_FFFF_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_QuerySuperpowerPara_FFFF_Finish maxAllow timeout!!!");
            break;
        }
        case Wait_SetSuperpowerPara_0A1A_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_SetSuperpowerPara_0A1A_Finish maxAllow timeout!!!");
            break;
        }
        case Wait_QuerySuperpowerPara_0A1A_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_QuerySuperpowerPara_0A1A_Finish maxAllow timeout!!!");
            break;
        }
        case Wait_SetSuperpowerPara_0309_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_SetSuperpowerPara_0309_Finish maxAllow timeout!!!");
            break;
        }
        case Wait_QuerySuperpowerPara_0309_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_QuerySuperpowerPara_0309_Finish maxAllow timeout!!!");
            break;
        }
        case Wait_SetSuperpowerPara_151A_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_SetSuperpowerPara_151A_Finish maxAllow timeout!!!");
            break;
        }
        case Wait_QuerySuperpowerPara_151A_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_QuerySuperpowerPara_151A_Finish maxAllow timeout!!!");
            break;
        }
        default:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==p_maxAllowTimer");
            break;
        }
    }
}
