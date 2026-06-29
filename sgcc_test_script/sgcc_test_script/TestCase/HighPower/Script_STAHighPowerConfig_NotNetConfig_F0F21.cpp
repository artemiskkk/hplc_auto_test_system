#include "Script_STAHighPowerConfig_NotNetConfig_F0F21.h"

Script_STAHighPowerConfig_NotNetConfig_F0F21::Script_STAHighPowerConfig_NotNetConfig_F0F21(QObject *parent) : QObject(parent)
{
    emScriptRunState=TestInit;
    resultFlag=false;
    sendMsgOct.clear();

    p_CtrInfoList=make_shared<QList<shared_ptr<CtrInfo>>>();
    p_BuildNetwork_GW=new BuildNetwork_GW();

    p_Frame3762Helper=make_shared<Frame3762Helper>();
    p_HardReset_01F1=make_shared<Afn01F1>();
    p_AddSlaveNode_11F1=make_shared<Afn11F1>();
    p_DeleteSlaveNode_11F2=make_shared<Afn11F2>();

    p_QueryNetScale_10F9=make_shared<Afn10F9>();
    p_TransparentTransmit_02F1=make_shared<Afn02F1>();
    p_ConfigSuperpowerPara_F0F21=make_shared<AfnF0F21>();

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
Script_STAHighPowerConfig_NotNetConfig_F0F21::~Script_STAHighPowerConfig_NotNetConfig_F0F21()
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

void Script_STAHighPowerConfig_NotNetConfig_F0F21::execute()
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

void Script_STAHighPowerConfig_NotNetConfig_F0F21::stop()
{
    if(p_AbstractScriptHost==nullptr)
        return;
    p_AbstractScriptHost->updateProgress(ProcessState_Error, "Test stop!");
    if(needPowerOff==true)
        powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost);
}

void Script_STAHighPowerConfig_NotNetConfig_F0F21::addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq)
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


void Script_STAHighPowerConfig_NotNetConfig_F0F21::setHost(AbstractScriptHost *host)
{
    p_AbstractScriptHost=host;
    p_BuildNetwork_GW->setHost(host);
}
bool Script_STAHighPowerConfig_NotNetConfig_F0F21::config(const QMap<QString,QString> *paraDic)
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

void Script_STAHighPowerConfig_NotNetConfig_F0F21::processMsg(DvcType dvcType,int id,uchar* data,int datalen)
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

void Script_STAHighPowerConfig_NotNetConfig_F0F21::processCtrlDvcRes(DvcType dvcType,QList<int> idList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params)
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
        case Wait_DeleteSlaveNode_11F2_Finish:
        {
            break;
        }
        case Wait_HardResetTest_Finish:
        {
            break;
        }
        case Wait_SetSuperpowerPara_0C0C_Finish:
        {
            break;
        }
        case Wait_AddSlaveNode_11F1_Finish:
        {
            break;
        }
        case Wait_QueryNetScale_10F9_2_Finish:
        {
            break;
        }
        case Wait_QuerySuperpowerPara_Finish:
        {
            break;
        }
        case ScriptSuccess:
        {
            break;
        }
    }
}
void Script_STAHighPowerConfig_NotNetConfig_F0F21::sendSrcMsg(DvcType dvcType, int dvcId, QByteArray msg)
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

void Script_STAHighPowerConfig_NotNetConfig_F0F21::processMsgFromCCO(DvcType dvcType, int dvcId)
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

//                                pullPinReset_STA(p_CtrInfoList,p_AbstractScriptHost);

                                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_DeleteSlaveNode_11F2);
                                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "删除从节点档案，等待--确认");
                                emScriptRunState=Wait_DeleteSlaveNode_11F2_Finish;
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
        case Wait_DeleteSlaveNode_11F2_Finish:
        {
            if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();//接收到一条完整报文
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "删除从节点回复确认！");
                //删除节点地址后，给相应的模块拉复位
                pullPinReset_STA(p_CtrInfoList,p_AbstractScriptHost);

                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_HardReset_01F1);
                emScriptRunState=Wait_HardResetTest_Finish;
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--路由硬件复位（01F1），等待--确认");
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
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
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到确认");
            }
            else if(p_Frame3762Base->afn_ == 0x03&&p_Frame3762Base->dt1_==0x02&&p_Frame3762Base->dt2_==0x01&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到03F10上报");

                index=0;
//                em02F1_State=Set_02F1;
                emConfigSuperpowerParaState = Wait_Config_0C0C_Finish;
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_ConfigSuperpowerPara_F0F21);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "F0F21配置STA大功率参数_0C0C，等待--确认");
                emScriptRunState=Wait_SetSuperpowerPara_0C0C_Finish;
                p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
            case Wait_SetSuperpowerPara_0C0C_Finish:
            {
                if(p_Frame3762Base->afn_ == char(0x00)&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "给不在档案内电表，F0F21配置大功率参数0C0C完成，回复确认");
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "等待400s");
                    p_delayTimer->start(400*1000);
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_AddSlaveNode_11F1_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();//接收到一条完整报文
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("添加新从节点收到确认！"));
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryNetScale_10F9);
                    emScriptRunState=Wait_QueryNetScale_10F9_2_Finish;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "组网完成，发送--查询网络规模（10F9），等待--回复");
                    p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
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
                    if(p_QueryNetScale_10F9_Up->network_scale_<p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size()+1)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,"等待10s后查询网络规模");
                        p_delayTimer->start(10*1000);
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,"组网完成！！！");

                        index=0;
                        em02F1_State=Query_02F1;
                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_TransparentTransmit_02F1);
                        emScriptRunState=Wait_QuerySuperpowerPara_Finish;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--透传查询从节点大功率参数（02F1），等待--回复");
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
            case Wait_QuerySuperpowerPara_Finish:
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
//                        index++;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("透传回复数据:%1").arg(QString::fromLatin1(p_TransparentTransmit_02F1_Up->frame_content_.toHex().toUpper())));

                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,"测试成功【GW-STA-F021-0004-V01】大功率配置-不入网配置（F0F21）");

                        nodeInfoList.clear();
                        nodeParaList.clear();
                        emScriptRunState = ScriptSuccess;
                        resultFlag=true;
                        p_AbstractScriptHost->updateProgress(ProcessState_Success,QString("脚本执行成功，%1").arg(test_name_));
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
//                            index++;
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

void Script_STAHighPowerConfig_NotNetConfig_F0F21::processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID)
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

void Script_STAHighPowerConfig_NotNetConfig_F0F21::processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID)
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

void Script_STAHighPowerConfig_NotNetConfig_F0F21::sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame)
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
    else if(frame==p_DeleteSlaveNode_11F2)
    {
        p_DeleteSlaveNode_11F2->ctrl_field_.dir=kDirDown;
        p_DeleteSlaveNode_11F2->ctrl_field_.prm=kActive;
        p_DeleteSlaveNode_11F2->ctrl_field_.comn_type=kHplc;

        p_DeleteSlaveNode_11F2->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_DeleteSlaveNode_11F2->info_field_.info_field_down.comu_rate=0;
        p_DeleteSlaveNode_11F2->info_field_.info_field_down.comu_module_ident=0;
        p_DeleteSlaveNode_11F2->node_address_list_.clear();
        Address nodeAddr;
        memcpy(nodeAddr.addr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(0)->mtrAddr,6);
        p_DeleteSlaveNode_11F2->node_address_list_.append(nodeAddr);
        p_DeleteSlaveNode_11F2->node_num_=uchar(p_DeleteSlaveNode_11F2->node_address_list_.size());
        sendMsgOct=p_DeleteSlaveNode_11F2->EncodeFrame();
        sendMsgLog=QString("》》删除从节点11F2：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_AddSlaveNode_11F1)
    {
        p_AddSlaveNode_11F1->ctrl_field_.dir=kDirDown;
        p_AddSlaveNode_11F1->ctrl_field_.prm=kActive;
        p_AddSlaveNode_11F1->ctrl_field_.comn_type=kHplc;

        p_AddSlaveNode_11F1->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_AddSlaveNode_11F1->info_field_.info_field_down.comu_rate=0;
        p_AddSlaveNode_11F1->info_field_.info_field_down.comu_module_ident=0;

        p_AddSlaveNode_11F1->node_parameter_list_.clear();

        NodeParameter nodePara;
        memcpy(nodePara.node_address_.addr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(0)->mtrAddr,6);
        nodePara.protocol_type_=char(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(0)->prtcl);
        p_AddSlaveNode_11F1->node_parameter_list_.append(nodePara);

//        if(emScriptRunState==Wait_AddNode_Building_Finish)
//        {
//            if(index+num<p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().size())
//            {
//                for(int i=0;i<num;i++)
//                {
//                    NodeParameter nodePara;
//                    memcpy(nodePara.node_address_.addr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(index+i)->mtrAddr,6);
//                    nodePara.protocol_type_=char(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(index+i)->prtcl);
//                    p_AddSlaveNode_11F1->node_parameter_list_.append(nodePara);
//                }
//            }
//            else
//            {
//                for(int i=0;i<p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().size()-index;i++)
//                {
//                    NodeParameter nodePara;
//                    memcpy(nodePara.node_address_.addr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(index+i)->mtrAddr,6);
//                    nodePara.protocol_type_=char(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(index+i)->prtcl);
//                    p_AddSlaveNode_11F1->node_parameter_list_.append(nodePara);
//                }
//            }
//        }
//        else
//        {
//            NodeParameter nodePara;
//            memcpy(nodePara.node_address_.addr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(0)->mtrAddr,6);
//            nodePara.protocol_type_=char(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(0)->prtcl);
//            p_AddSlaveNode_11F1->node_parameter_list_.append(nodePara);
//        }
        p_AddSlaveNode_11F1->node_num_=uchar(p_AddSlaveNode_11F1->node_parameter_list_.size());
        sendMsgOct=p_AddSlaveNode_11F1->EncodeFrame();
        sendMsgLog=QString("》》添加从节点11F1：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_ConfigSuperpowerPara_F0F21)
    {
        p_ConfigSuperpowerPara_F0F21->ctrl_field_.dir=kDirDown;
        p_ConfigSuperpowerPara_F0F21->ctrl_field_.prm=kActive;
        p_ConfigSuperpowerPara_F0F21->ctrl_field_.comn_type=kHplc;

        p_ConfigSuperpowerPara_F0F21->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_ConfigSuperpowerPara_F0F21->info_field_.info_field_down.comu_rate=0;
        p_ConfigSuperpowerPara_F0F21->info_field_.info_field_down.comu_module_ident=0;

        switch (emConfigSuperpowerParaState)
        {
            case Wait_Config_0C0C_Finish:
            {
                p_ConfigSuperpowerPara_F0F21->dac_voltage_=0x0C;
                p_ConfigSuperpowerPara_F0F21->peak_clipping_factor_=0x0C;
                break;
            }
            case Wait_Config_0505_Finish:
            {
                p_ConfigSuperpowerPara_F0F21->dac_voltage_=0x05;
                p_ConfigSuperpowerPara_F0F21->peak_clipping_factor_=0x05;
                break;
            }
        }
        sendMsgOct=p_ConfigSuperpowerPara_F0F21->EncodeFrame();
        sendMsgLog=QString("》》查询大功率参数：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
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

void Script_STAHighPowerConfig_NotNetConfig_F0F21::timer_timeoutProc()
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

void Script_STAHighPowerConfig_NotNetConfig_F0F21::delayTimer_timeoutProc()
{
    p_delayTimer->stop();

    switch(emScriptRunState)
    {
    case Wait_SetSuperpowerPara_0C0C_Finish:
    {
        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_AddSlaveNode_11F1);
        emScriptRunState=Wait_AddSlaveNode_11F1_Finish;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "添加不在档案的从节点，等待--回复");
        p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
        break;
    }
    case Wait_QueryNetScale_10F9_2_Finish:
    {
        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryNetScale_10F9);
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询网络规模（10F9），等待--回复");
        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
        break;
    }
    default:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==p_timer");
        emScriptRunState=ScriptSuccess;
        break;
    }
    }
}
QByteArray Script_STAHighPowerConfig_NotNetConfig_F0F21::getQueryFrame(ParaType type)
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
QByteArray Script_STAHighPowerConfig_NotNetConfig_F0F21::getSetFrame(ParaType type)
{
    QByteArray msg;
    shared_ptr<SetParam_Down> p_SetParam_Down=make_shared<SetParam_Down>();
    QList<ParamInfo> setParamInfoList;
    if(type==DAC)
    {
        switch (emConfigSuperpowerParaState)
        {
            case Wait_Config_0505_Finish:
            {
                vol=char(0x05);
                factor=char(0x05);
                break;
            }
            case Wait_Config_0C0C_Finish:
            {
                vol=char(0x0C);
                factor=char(0x0C);
                break;
            }
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
void Script_STAHighPowerConfig_NotNetConfig_F0F21::maxAllowTimer_timeoutProc()
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
        case Wait_DeleteSlaveNode_11F2_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_DeleteSlaveNode_11F2_Finish maxAllow timeout!!!");
            break;
        }
        case Wait_HardResetTest_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_HardResetTest_Finish maxAllow timeout!!!");
            break;
        }
        case Wait_SetSuperpowerPara_0C0C_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_SetSuperpowerPara_0C0C_Finish maxAllow timeout!!!");
            break;
        }
        case Wait_AddSlaveNode_11F1_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_AddSlaveNode_11F1_Finish maxAllow timeout!!!");
            break;
        }
        case Wait_QueryNetScale_10F9_2_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_QueryNetScale_10F9_2_Finish maxAllow timeout!!!");
            break;
        }
        case Wait_QuerySuperpowerPara_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_QuerySuperpowerPara_Finish maxAllow timeout!!!");
            break;
        }
        default:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==p_maxAllowTimer");
            break;
        }
    }
}
