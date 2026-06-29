#include "Script_ChipIdManage_CCO_V3Key.h"

Script_ChipIdManage_CCO_V3Key::Script_ChipIdManage_CCO_V3Key(QObject *parent) : QObject(parent)
{
    emScriptRunState=TestInit;
    resultFlag=false;
    sendMsgOct.clear();

    p_CtrInfoList=make_shared<QList<shared_ptr<CtrInfo>>>();
    p_BuildNetwork_GW=new BuildNetwork_GW();

    p_Frame3762Helper=make_shared<Frame3762Helper>();
    p_ParaInit_01F2=make_shared<Afn01F2>();
    p_QueryRouterChipID_F0F40=make_shared<AfnF0F40>();
    p_QueryRouterChipID_10F112=make_shared<Afn10F112>();
    p_QueryRouterChipID_10F40=make_shared<Afn10F40>();
    p_SetRouterChipIDKey_F0F43=make_shared<AfnF0F43>();
    p_QueryRouterChipIDKey_F0F44=make_shared<AfnF0F44>();
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

 //   chipIdHeader=QDateTime::currentDateTime().toString("yyMMddhhmmss");
    chipIdKeyList<<chipId_0+key_0<<chipId_1+key_1;//<<chipId_2
}
Script_ChipIdManage_CCO_V3Key::~Script_ChipIdManage_CCO_V3Key()
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
void Script_ChipIdManage_CCO_V3Key::execute()
{
    p_AbstractScriptHost->updateProgress(ProcessState_Start, "Test start!");

    emScriptRunState=TestInit;
    resultFlag=false;
    addrList.clear();

    if(needBuildNet==true)
    {
        p_BuildNetwork_GW->execute();
        emScriptRunState=Wait_BuildNetFinish_Whole;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "等待全网组网完成");
        p_timer->start((timerForReachThresld+timerAfterReachThresld)*1000);
    }
    else
    {
//        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,chipIdIndex,p_SetRouterChipID_F0F39);
//        emScriptRunState=Wait_SetRouterChipID_Finish;
//        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--设置CCO芯片ID，等待--确认");
//        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
    }
    p_maxAllowTimer->start((timerForReachThresld+timerAfterReachThresld)*1000);
}
void Script_ChipIdManage_CCO_V3Key::stop()
{
    if(p_AbstractScriptHost==nullptr)
        return;
    p_AbstractScriptHost->updateProgress(ProcessState_Error, "Test stop!");
    if(needPowerOff==true)
        powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost);
}
void Script_ChipIdManage_CCO_V3Key::addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq)
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
void Script_ChipIdManage_CCO_V3Key::setHost(AbstractScriptHost *host)
{
    p_AbstractScriptHost=host;
    p_BuildNetwork_GW->setHost(host);
}
bool Script_ChipIdManage_CCO_V3Key::config(const QMap<QString,QString> *paraDic)
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
void Script_ChipIdManage_CCO_V3Key::processMsg(DvcType dvcType, int id, uchar *data, int datalen)
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

            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询CCO芯片ID+密钥");
            emScriptRunState=SetRouterChipIDKey_Before;
            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryRouterChipID_10F40);
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
void Script_ChipIdManage_CCO_V3Key::processCtrlDvcRes(DvcType dvcType, QList<int> chipIdList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params)
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
            p_BuildNetwork_GW->processCtrlDvcRes(dvcType,chipIdList,ctrlCmdType,isSucs,params);
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

void Script_ChipIdManage_CCO_V3Key::processMsgFromCCO(DvcType dvcType, int dvcId)
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
        uchar dtValue3762=get3762Dt(p_Frame3762Base->dt1_,p_Frame3762Base->dt2_);
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
        case SetRouterChipIDKey_Before:
        {
            if(p_Frame3762Base->afn_==char(0xf0) && dtValue3762==44 && p_Frame3762Base->ctrl_field_.dir==kDirUp)
            {
                p_timer->stop();
                shared_ptr<AfnF0F44> p_QueryRouterChipIDKey_F0F44_Up = dynamic_pointer_cast<AfnF0F44>(p_Frame3762Base);
                QByteArray chipIDKey;
                for(int i=p_QueryRouterChipIDKey_F0F44_Up->read_chip_id_unit_up_.id_length_-1; i>=0; i--)
                {
                    chipIDKey.append(p_QueryRouterChipIDKey_F0F44_Up->read_chip_id_unit_up_.id_content_.at(i));
                }
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "CCO当前芯片ID+密钥："+chipIDKey.toHex());
                chipIdKeyIndex = 0;
                dstChipId = chipId_0;
                if(chipIDKey.toHex() == chipIdKeyList.at(chipIdKeyIndex))
                {
                    chipIdKeyIndex = 1;
                    dstChipId = chipId_1;
                }
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "设置CCO芯片ID+密钥，等待--确认");
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,chipIdKeyIndex,p_SetRouterChipIDKey_F0F43);
                p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
            }
            else if(p_Frame3762Base->afn_==0 && dtValue3762==1 && p_Frame3762Base->ctrl_field_.dir==kDirUp)
            {
                p_timer->stop();
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到确认，查询CCO设置后的芯片ID+密钥");
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryRouterChipIDKey_F0F44);
                emScriptRunState = SetRouterChipIDKey_After;
                p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case SetRouterChipIDKey_After:
        {
            if(p_Frame3762Base->afn_==char(0xf0) && dtValue3762==44 && p_Frame3762Base->ctrl_field_.dir==kDirUp)
            {
                p_timer->stop();
                shared_ptr<AfnF0F44> p_QueryRouterChipIDKey_F0F44_Up = dynamic_pointer_cast<AfnF0F44>(p_Frame3762Base);
                QByteArray chipIDKey;
                for(int i=p_QueryRouterChipIDKey_F0F44_Up->read_chip_id_unit_up_.id_length_-1; i>=0; i--)
                {
                    chipIDKey.append(p_QueryRouterChipIDKey_F0F44_Up->read_chip_id_unit_up_.id_content_.at(i));
                }
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "CCO当前芯片ID+密钥："+chipIDKey.toHex());
                if(memcmp(p_QueryRouterChipIDKey_F0F44_Up->read_chip_id_unit_up_.mac_address_.addr,p_CtrInfoList->at(0)->ccoAddr,6)==0
                        && p_QueryRouterChipIDKey_F0F44_Up->read_chip_id_unit_up_.id_type_==0x03
                        && p_QueryRouterChipIDKey_F0F44_Up->read_chip_id_unit_up_.id_format_==0x02
                        && p_QueryRouterChipIDKey_F0F44_Up->read_chip_id_unit_up_.id_length_==346
                        && chipIDKey.toHex()==chipIdKeyList.at(chipIdKeyIndex))
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "CCO芯片ID+密钥与设置一致，参数初始化，等待--确认");
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_ParaInit_01F2);
                    p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, "CCO芯片ID+密钥与设置不一致");
                }
            }
            else if(p_Frame3762Base->afn_==0 && dtValue3762==1 && p_Frame3762Base->ctrl_field_.dir==kDirUp)
            {
                p_timer->stop();
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到确认，查询CCO设置后的芯片ID+密钥");
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryRouterChipIDKey_F0F44);
                emScriptRunState = ReadChipIDKeyAfterParaInit;
                p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case ReadChipIDKeyAfterParaInit:
        {
            if(p_Frame3762Base->afn_==char(0xf0) && dtValue3762==44 && p_Frame3762Base->ctrl_field_.dir==kDirUp)
            {
                p_timer->stop();
                shared_ptr<AfnF0F44> p_QueryRouterChipIDKey_F0F44_Up = dynamic_pointer_cast<AfnF0F44>(p_Frame3762Base);
                QByteArray chipIDKey;
                for(int i=p_QueryRouterChipIDKey_F0F44_Up->read_chip_id_unit_up_.id_length_-1; i>=0; i--)
                {
                    chipIDKey.append(p_QueryRouterChipIDKey_F0F44_Up->read_chip_id_unit_up_.id_content_.at(i));
                }
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "路由当前芯片ID+密钥："+chipIDKey.toHex());
                if(memcmp(p_QueryRouterChipIDKey_F0F44_Up->read_chip_id_unit_up_.mac_address_.addr,p_CtrInfoList->at(0)->ccoAddr,6)==0
                        && p_QueryRouterChipIDKey_F0F44_Up->read_chip_id_unit_up_.id_type_==0x03
                        && p_QueryRouterChipIDKey_F0F44_Up->read_chip_id_unit_up_.id_format_==0x02
                        && p_QueryRouterChipIDKey_F0F44_Up->read_chip_id_unit_up_.id_length_==346
                        && chipIDKey.toHex()==chipIdKeyList.at(chipIdKeyIndex))
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "参数初始化后CCO芯片ID+密钥未改变，发送--查询CCO芯片ID（F0F40），等待--回复");
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryRouterChipID_F0F40);
                    p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, "参数初始化后CCO芯片ID+密钥发生改变");
                }
            }
            else if(p_Frame3762Base->afn_==char(0xf0) && dtValue3762==40 && p_Frame3762Base->ctrl_field_.dir==kDirUp)
            {
                p_timer->stop();
                shared_ptr<AfnF0F40> p_QueryRouterChipID_F0F40_Up = dynamic_pointer_cast<AfnF0F40>(p_Frame3762Base);
                if(memcmp(p_QueryRouterChipID_F0F40_Up->read_chip_id_unit_up_.mac_address_.addr,p_CtrInfoList->at(0)->ccoAddr,6)==0
                        && p_QueryRouterChipID_F0F40_Up->read_chip_id_unit_up_.id_type_==0x01)
                    //    && p_QueryRouterChipID_F0F40_Up->read_chip_id_unit_up_.id_format_==0x02
                {
                    QByteArray chipID;
                    for(int i=p_QueryRouterChipID_F0F40_Up->read_chip_id_unit_up_.id_length_-1; i>=0; i--)
                    {
                        chipID.append(p_QueryRouterChipID_F0F40_Up->read_chip_id_unit_up_.id_content_.at(i));
                    }
                    if(dstChipId == QString(chipID.toHex()))
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("CCO芯片ID为%1，符合要求").arg(QString(chipID.toHex())));
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询CCO芯片ID（10F40），等待--回复");
                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryRouterChipID_10F40);
                        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("CCO芯片ID为%1，不符合要求").arg(QString(chipID.toHex())));
                    }
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("F0F40回复MAC地址或ID类型或ID格式不符合要求"));
                }
            }
            else if(p_Frame3762Base->afn_==0x10 && dtValue3762==40 && p_Frame3762Base->ctrl_field_.dir==kDirUp)
            {
                p_timer->stop();
                shared_ptr<Afn10F40> p_QueryRouterChipID_10F40_Up = dynamic_pointer_cast<Afn10F40>(p_Frame3762Base);
                if(memcmp(p_QueryRouterChipID_10F40_Up->read_chip_id_unit_up_.mac_address_.addr,p_CtrInfoList->at(0)->ccoAddr,6)==0
                        && p_QueryRouterChipID_10F40_Up->read_chip_id_unit_up_.id_type_==0x01
                        && p_QueryRouterChipID_10F40_Up->read_chip_id_unit_up_.device_type_==0x02)
                {
                    QByteArray chipID;
                    for(int i=p_QueryRouterChipID_10F40_Up->read_chip_id_unit_up_.id_length_-1; i>=0; i--)
                    {
                        chipID.append(p_QueryRouterChipID_10F40_Up->read_chip_id_unit_up_.id_content_.at(i));
                    }
                    if(dstChipId == QString(chipID.toHex()))
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("CCO芯片ID为%1，符合要求").arg(QString(chipID.toHex())));
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询CCO芯片ID（10F112），等待--回复");
                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryRouterChipID_10F112);
                        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("CCO芯片ID为%1，不符合要求").arg(QString(chipID.toHex())));
                    }
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("10F40回复MAC地址或ID类型或设备类型不符合要求"));
                }
            }
            else if(p_Frame3762Base->afn_==0x10 && dtValue3762==112 && p_Frame3762Base->ctrl_field_.dir==kDirUp)
            {
                p_timer->stop();
                shared_ptr<Afn10F112> p_QueryRouterChipID_10F112_Up = dynamic_pointer_cast<Afn10F112>(p_Frame3762Base);
                if(memcmp(p_QueryRouterChipID_10F112_Up->node_chip_info_unit_.node_chip_info_list_.at(0).node_address_.addr,p_CtrInfoList->at(0)->ccoAddr,6)==0
                        &&p_QueryRouterChipID_10F112_Up->node_chip_info_unit_.node_chip_info_list_.at(0).node_device_type_==0x02)
                {
                    QByteArray chipID;
                    for(int i=23; i>=0; i--)
                    {
                        chipID.append(p_QueryRouterChipID_10F112_Up->node_chip_info_unit_.node_chip_info_list_.at(0).node_chip_id_.at(i));
                    }
                    if(dstChipId == QString(chipID.toHex()))
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("CCO芯片ID为%1，符合要求").arg(QString(chipID.toHex())));
                        emScriptRunState=ScriptSuccess;
                        p_AbstractScriptHost->updateProgress(ProcessState_Success, QString("CCO芯片ID+密钥测试成功"));
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("CCO芯片ID为%1，不符合要求").arg(QString(chipID.toHex())));
                    }
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("10F112回复节点地址或设备类型不符合要求"));
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
void Script_ChipIdManage_CCO_V3Key::processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID)
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
void Script_ChipIdManage_CCO_V3Key::processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID)
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
void Script_ChipIdManage_CCO_V3Key::sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame)
{
    if(frame==p_ParaInit_01F2)
    {
        p_ParaInit_01F2->ctrl_field_.dir=kDirDown;
        p_ParaInit_01F2->ctrl_field_.prm=kActive;
        p_ParaInit_01F2->ctrl_field_.comn_type=kHplc;

        p_ParaInit_01F2->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_ParaInit_01F2->info_field_.info_field_down.comu_rate=0;
        p_ParaInit_01F2->info_field_.info_field_down.comu_module_ident=0;

        sendMsgOct=p_ParaInit_01F2->EncodeFrame();
        sendMsgLog=QString("》》路由参数初始化01F2：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_QueryRouterChipID_10F40)
    {
        p_QueryRouterChipID_10F40->ctrl_field_.dir=kDirDown;
        p_QueryRouterChipID_10F40->ctrl_field_.prm=kActive;
        p_QueryRouterChipID_10F40->ctrl_field_.comn_type=kHplc;

        p_QueryRouterChipID_10F40->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_QueryRouterChipID_10F40->info_field_.info_field_down.comu_rate=0;
        p_QueryRouterChipID_10F40->info_field_.info_field_down.comu_module_ident=0;

        p_QueryRouterChipID_10F40->read_chip_id_unit_down_.device_type_=0x02;
        memcpy(p_QueryRouterChipID_10F40->read_chip_id_unit_down_.mac_address_.addr,p_CtrInfoList->at(0)->ccoAddr,6);
        p_QueryRouterChipID_10F40->read_chip_id_unit_down_.id_type_=0x01;

        sendMsgOct=p_QueryRouterChipID_10F40->EncodeFrame();
        sendMsgLog=QString("》》查询CCO芯片ID 10F40：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_QueryRouterChipID_F0F40)
    {
        p_QueryRouterChipID_F0F40->ctrl_field_.dir=kDirDown;
        p_QueryRouterChipID_F0F40->ctrl_field_.prm=kActive;
        p_QueryRouterChipID_F0F40->ctrl_field_.comn_type=kHplc;

        p_QueryRouterChipID_F0F40->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_QueryRouterChipID_F0F40->info_field_.info_field_down.comu_rate=0;
        p_QueryRouterChipID_F0F40->info_field_.info_field_down.comu_module_ident=0;

        memcpy(p_QueryRouterChipID_F0F40->read_chip_id_unit_down_.mac_address_.addr,p_CtrInfoList->at(0)->ccoAddr,6);
        p_QueryRouterChipID_F0F40->read_chip_id_unit_down_.id_type_=0x01;
        p_QueryRouterChipID_F0F40->read_chip_id_unit_down_.id_format_=0x00;
        p_QueryRouterChipID_F0F40->read_chip_id_unit_down_.id_length_=0x18;

        sendMsgOct=p_QueryRouterChipID_F0F40->EncodeFrame();
        sendMsgLog=QString("》》查询CCO芯片ID F0F40：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_QueryRouterChipID_10F112)
    {
        p_QueryRouterChipID_10F112->ctrl_field_.dir=kDirDown;
        p_QueryRouterChipID_10F112->ctrl_field_.prm=kActive;
        p_QueryRouterChipID_10F112->ctrl_field_.comn_type=kHplc;

        p_QueryRouterChipID_10F112->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_QueryRouterChipID_10F112->info_field_.info_field_down.comu_rate=0;
        p_QueryRouterChipID_10F112->info_field_.info_field_down.comu_module_ident=0;

        p_QueryRouterChipID_10F112->node_num_=0x01;
        p_QueryRouterChipID_10F112->node_start_no_=0x01;

        sendMsgOct=p_QueryRouterChipID_10F112->EncodeFrame();
        sendMsgLog=QString("》》查询CCO芯片ID 10F112：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_SetRouterChipIDKey_F0F43)
    {
        p_SetRouterChipIDKey_F0F43->ctrl_field_.dir=kDirDown;
        p_SetRouterChipIDKey_F0F43->ctrl_field_.prm=kActive;
        p_SetRouterChipIDKey_F0F43->ctrl_field_.comn_type=kHplc;

        p_SetRouterChipIDKey_F0F43->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_SetRouterChipIDKey_F0F43->info_field_.info_field_down.comu_rate=0;
        p_SetRouterChipIDKey_F0F43->info_field_.info_field_down.comu_module_ident=0;

        memcpy(p_SetRouterChipIDKey_F0F43->mac_address_.addr,p_CtrInfoList->at(0)->ccoAddr,6);
        p_SetRouterChipIDKey_F0F43->id_type_=0x03;
        p_SetRouterChipIDKey_F0F43->id_format_=0x02;

        QByteArray chipIdKey=QByteArray::fromHex(chipIdKeyList.at(meterID).toLatin1());
        for(int i=chipIdKey.size()-1; i>=0; i--)
        {
            p_SetRouterChipIDKey_F0F43->id_content_.append(chipIdKey.at(i));
        }
        p_SetRouterChipIDKey_F0F43->id_length_=ushort(p_SetRouterChipIDKey_F0F43->id_content_.size());

        sendMsgOct=p_SetRouterChipIDKey_F0F43->EncodeFrame();
        sendMsgLog=QString("》》设置CCO芯片ID和密钥 F0F43：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_QueryRouterChipIDKey_F0F44)
    {
        p_QueryRouterChipIDKey_F0F44->ctrl_field_.dir=kDirDown;
        p_QueryRouterChipIDKey_F0F44->ctrl_field_.prm=kActive;
        p_QueryRouterChipIDKey_F0F44->ctrl_field_.comn_type=kHplc;

        p_QueryRouterChipIDKey_F0F44->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_QueryRouterChipIDKey_F0F44->info_field_.info_field_down.comu_rate=0;
        p_QueryRouterChipIDKey_F0F44->info_field_.info_field_down.comu_module_ident=0;

        memcpy(p_QueryRouterChipIDKey_F0F44->read_chip_id_unit_down_.mac_address_.addr,p_CtrInfoList->at(0)->ccoAddr,6);
        p_QueryRouterChipIDKey_F0F44->read_chip_id_unit_down_.id_type_=0x03;
        p_QueryRouterChipIDKey_F0F44->read_chip_id_unit_down_.id_format_=0x02;
        p_QueryRouterChipIDKey_F0F44->read_chip_id_unit_down_.id_length_=346;

        sendMsgOct=p_QueryRouterChipIDKey_F0F44->EncodeFrame();
        sendMsgLog=QString("》》查询CCO芯片ID和密钥 F0F44：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
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
void Script_ChipIdManage_CCO_V3Key::sendSrcMsg(DvcType dvcType, int dvcId, QByteArray msg)
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

void Script_ChipIdManage_CCO_V3Key::timer_timeoutProc()
{
    p_timer->stop();
    switch(emScriptRunState)
    {
        case Wait_BuildNetFinish_Whole:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_BuildNetFinish_Whole timeout!!!");
            break;
        }
        default:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "p_timer timeout");
            break;
        }
    }
}
void Script_ChipIdManage_CCO_V3Key::maxAllowTimer_timeoutProc()
{
    p_maxAllowTimer->stop();
    switch(emScriptRunState)
    {
        default:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==p_maxAllowTimer");
            break;
        }
    }
}
void Script_ChipIdManage_CCO_V3Key::delayTimer_timeoutProc()
{
    p_delayTimer->stop();
 //   index=0;
   // p_maxAllowTimer->stop();
}

