#include "Script_StaModuleID.h"

Script_StaModuleID::Script_StaModuleID(QObject *parent) : QObject(parent)
{
    emScriptRunState=ScriptInit;
    resultFlag=false;
    sendMsgOct.clear();

    p_CtrInfoList=make_shared<QList<shared_ptr<CtrInfo>>>();
    p_BuildNetwork_GW=new BuildNetwork_GW();

    p_Frame3762Helper=make_shared<Frame3762Helper>();
    p_HardReset_01F1=make_shared<Afn01F1>();
    p_ParaInit_01F2=make_shared<Afn01F2>();
    p_QueryNetScale_10F9=make_shared<Afn10F9>();

    p_QueryStaModuleID_10F7=make_shared<Afn10F7>();
    p_QueryStaModuleID_10F40=make_shared<Afn10F40>();
    p_TransmitData_02F1=make_shared<Afn02F1>();
    p_SetModuleID_F0F39=make_shared<AfnF0F39>();
    p_QueryModuleID_F0F40=make_shared<AfnF0F40>();


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

    QString chipIdHeader=QDateTime::currentDateTime().toString("yyMMddhhmmss");
    moduleId_50Bytes_ = "0102030405060708091011121314151617181920212223242526272829303132333435363738394041424344" +  chipIdHeader;
    moduleId_51Bytes_ = "0203040506070809101112131415161718192021222324252627282930313233343536373839404142434445" + chipIdHeader;
    moduleId_49Bytes_ = "01020304050607080910111213141516171819202122232425262728293031323334353637383940414243" + chipIdHeader;
    moduleId_02F1Send_ = chipIdHeader + "0102030405060708091011121314151617181920212223242526272829303132333435363738394041424344";
}

Script_StaModuleID::~Script_StaModuleID()
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
void Script_StaModuleID::execute()
{
    p_AbstractScriptHost->updateProgress(ProcessState_Start,"脚本执行流程描述打印：-----whc(2024年1月20)\r\n"
                                                             "1.通用组网流程，组网场景3(需要组网)，Wait_BuildNetFinish_Whole；\r\n"
                                                             "2.F0F40初始查询模块ID(单/三相)，Wait_QueryModuleIDInit_ F0F40_Finish；\r\n"
                                                             "3.F0F39设置模块ID(50字节(单/三相))，Wait_SetModuleID_50Bytes _ F0F39_Finish；\r\n"
                                                             "4.F0F40查询模块ID(50字节)(单/三相)，Wait_QueryModuleID_50Bytes_ F0F40_Finish；\r\n"
                                                             "5.F0F39设置模块ID(51字节)(单/三相)，Wait_SetModuleID_51Bytes _ F0F39_Finish；\r\n"
                                                             "6.F0F40查询模块ID(51字节)(单/三相)，Wait_QueryModuleID_51Bytes_ F0F40_Finish；\r\n"
                                                             "7.F0F39设置模块ID(49字节)(单/三相)，Wait_SetModuleID_49Bytes _ F0F39_Finish；\r\n"
                                                             "8.F0F40查询模块ID(49字节)(单/三相)，Wait_QueryModuleID_49Bytes_ F0F40_Finish；\r\n"
                                                             "9.F0F39设置模块ID(0字节)(单/三相)，Wait_SetModuleID_0Bytes _ F0F39_Finish；\r\n"
                                                             "10.F0F40查询模块ID(0字节)(单/三相)，Wait_QueryModuleID_0Bytes_ F0F40_Finish；\r\n"
                                                             "11.02F1透传初始查询模块ID(单/三相)，Wait_QueryModuleIDInit_02F1_Finish；\r\n"
                                                             "12.02F1透传设置模块ID(单/三相)，Wait_SetModuleID_02F1_Finish；\r\n"
                                                             "13.01F1硬件复位，10F9查询网络规模，Wait_HardReset_01F1_Finish；\r\n"
                                                             "14.02F1透传查询模块ID(单/三相)，Wait_QueryModuleID_10F7_Finish；\r\n");
    p_AbstractScriptHost->updateProgress(ProcessState_Start, "Test start!"+test_name_);
    emScriptRunState=ScriptInit;
    resultFlag=false;
    addrList.clear();

    if(needBuildNet == true)
    {
        p_BuildNetwork_GW->execute();
        emScriptRunState = Wait_BuildNetFinish_Whole;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "等待全网组网完成");
        p_timer->start((timerForReachThresld+timerAfterReachThresld)*1000);
    }
    else
    {
        index = 0;
        emScriptRunState=Wait_QueryModuleIDInit_F0F40_Finish;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--初始查询单相STA模块ID(F0F40)，等待--回复\r\n");
        sendMsg(SingleSTA,p_CtrInfoList->at(0)->ctrlID,index,p_QueryModuleID_F0F40);
        p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
    }
    p_maxAllowTimer->start((timerForReachThresld+timerAfterReachThresld)*1000);

}
void Script_StaModuleID::stop()
{
    if(p_AbstractScriptHost==nullptr)
        return;
    p_AbstractScriptHost->updateProgress(ProcessState_Error, "Test stop!"+test_name_);
    if(needPowerOff==true)
        powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost);
}
void Script_StaModuleID::addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq)
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
void Script_StaModuleID::setHost(AbstractScriptHost *host)
{
    p_AbstractScriptHost=host;
    p_BuildNetwork_GW->setHost(host);
}
bool Script_StaModuleID::config(const QMap<QString,QString> *paraDic)
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
void Script_StaModuleID::processMsg(DvcType dvcType, int id, uchar *data, int datalen)
{
    if(p_CtrInfoList->size()==0)
        return;
    if(emScriptRunState == Wait_BuildNetFinish_Whole)
    {
        if(!p_BuildNetwork_GW->buildNetworkResultFlag)
            p_BuildNetwork_GW->processMsg(dvcType,id,data,datalen);
        else
        {
            if(p_BuildNetwork_GW->emScriptRunState == BuildNetFinish && p_BuildNetwork_GW->buildNetworkResultFlag==true)
            {
                tryTimes=0;
                index=0;
                emScriptRunState = Wait_QueryModuleIDInit_F0F40_Finish;
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--初始查询单相STA模块ID，等待--确认\r\n");
                sendMsg(SingleSTA,p_CtrInfoList->at(0)->ctrlID,index,p_QueryModuleID_F0F40);
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
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
        else if(dvcType==ReadCtrlDvc)
        {
            p_CtrInfoList->at(0)->bufReadCtrlDvc.append(recvTempData);
            processMsgFromReadCtrlDvc(dvcType,id);
        }
        else if(dvcType==SingleSTA || dvcType==ThreeSTA)
        {
            for(int i=0; i<p_CtrInfoList->at(0)->keyList.size(); i++)
            {
                if(dvcType == p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->value(p_CtrInfoList->at(0)->keyList.at(i))->slotPosition
                    && id == p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->value(p_CtrInfoList->at(0)->keyList.at(i))->dvcId)
                {
                    if(emScriptRunState==Wait_QueryModuleIDInit_F0F40_Finish || emScriptRunState==Wait_SetModuleID_50Bytes_F0F39_Finish || emScriptRunState==Wait_QueryModuleID_50Bytes_F0F40_Finish
                        || emScriptRunState==Wait_SetModuleID_51Bytes_F0F39_Finish|| emScriptRunState==Wait_QueryModuleID_51Bytes_F0F40_Finish|| emScriptRunState==Wait_SetModuleID_49Bytes_F0F39_Finish
                        || emScriptRunState==Wait_QueryModuleID_49Bytes_F0F40_Finish|| emScriptRunState==Wait_SetModuleID_0Bytes_F0F39_Finish|| emScriptRunState==Wait_QueryModuleID_0Bytes_F0F40_Finish)
                    {
                        p_CtrInfoList->at(0)->buf.append(recvTempData);
                        processMsgFromCCO(dvcType,id);
                    }
                    else
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
void Script_StaModuleID::processCtrlDvcRes(DvcType dvcType, QList<int> snList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params)
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
        p_BuildNetwork_GW->processCtrlDvcRes(dvcType,snList,ctrlCmdType,isSucs,params);
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

void Script_StaModuleID::processMsgFromCCO(DvcType dvcType, int dvcId)
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
        case ScriptInit:
        {
            break;
        }
        case Wait_BuildNetFinish_Whole:
        {
            break;
        }
        case Wait_QueryModuleIDInit_F0F40_Finish:
        {
            if(p_Frame3762Base->afn_==char(0xf0) && dtValue3762==40 && p_Frame3762Base->ctrl_field_.dir==kDirUp)
            {
                p_timer->stop();
                shared_ptr<AfnF0F40> p_QueryChipID_F0F40_Up = dynamic_pointer_cast<AfnF0F40>(p_Frame3762Base);
                if(memcmp(p_QueryChipID_F0F40_Up->read_chip_id_unit_up_.mac_address_.addr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(0)->mtrAddr,6)==0
                    && p_QueryChipID_F0F40_Up->read_chip_id_unit_up_.id_type_==0x02)
                    //&& p_QueryChipID_F0F40_Up->read_chip_id_unit_up_.id_format_==0x02)
                {
                    QString receive_moduleId_single = QString(p_QueryChipID_F0F40_Up->read_chip_id_unit_up_.id_content_.toHex());
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString(">>>>初始查询单相STA模块ID为：%1。\r\n").arg(receive_moduleId_single));
                    tryTimes=0;
                    index = index +1;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--初始查询三相STA模块ID，等待--回复\r\n");
                    sendMsg(ThreeSTA,p_CtrInfoList->at(0)->ctrlID,index,p_QueryModuleID_F0F40);
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                 }
                else if (memcmp(p_QueryChipID_F0F40_Up->read_chip_id_unit_up_.mac_address_.addr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(1)->mtrAddr,6)==0
                         && p_QueryChipID_F0F40_Up->read_chip_id_unit_up_.id_type_==0x02)
                         //&& p_QueryChipID_F0F40_Up->read_chip_id_unit_up_.id_format_==0x02)
                {
                    QString receive_moduleId_three = QString(p_QueryChipID_F0F40_Up->read_chip_id_unit_up_.id_content_.toHex());
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString(">>>>初始查询三相STA模块ID为：%1。\r\n").arg(receive_moduleId_three));
                    tryTimes=0;
                    index = 0;
                    emScriptRunState = Wait_SetModuleID_50Bytes_F0F39_Finish;
                    id_length =len_50Bytes;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--设置F0F39单相50字节模块ID，等待--确认\r\n");
                    sendMsg(SingleSTA,p_CtrInfoList->at(0)->ctrlID,index,p_SetModuleID_F0F39);
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                 }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("Wait_QueryModuleIDInit_F0F40_Finish，单通/三通的MAC地址或ID类型或ID格式有误，%1\r\n").arg(test_name_));
                }
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case Wait_SetModuleID_50Bytes_F0F39_Finish:
        {
            if(p_Frame3762Base->afn_== char(0x00) && dtValue3762==1 && p_Frame3762Base->ctrl_field_.dir==kDirUp)
            {
                if (index == 0)
                {
                    tryTimes=0;
                    index = index + 1;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到确认，单相STA模块ID设置成功。");
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--设置三相50字节模块ID，等待--确认\r\n");
                    sendMsg(ThreeSTA,p_CtrInfoList->at(0)->ctrlID,index,p_SetModuleID_F0F39);
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                }
                else if (index == 1)
                {
                    p_timer->stop();
                    emScriptRunState = Wait_QueryModuleID_50Bytes_F0F40_Finish;
                    tryTimes=0;
                    index = 0;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到确认，三相STA模块ID设置成功。");
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--F0F40查询单相模块ID，等待--确认\r\n");
                    sendMsg(SingleSTA,p_CtrInfoList->at(0)->ctrlID,index,p_QueryModuleID_F0F40);
                    p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                }
                else
                   p_AbstractScriptHost->updateProgress(ProcessState_Processing, "Wait_SetModuleID_50Bytes_F0F39_Finish，回复表地址与期望不符测试失败，"+test_name_);

            }
            else
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到其他376.2报文，等待--回复");
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case Wait_QueryModuleID_50Bytes_F0F40_Finish:
        {
            if(p_Frame3762Base->afn_==char(0xf0) && dtValue3762==40 && p_Frame3762Base->ctrl_field_.dir==kDirUp)
            {
                p_timer->stop();
                shared_ptr<AfnF0F40> p_QueryChipID_F0F40_Up = dynamic_pointer_cast<AfnF0F40>(p_Frame3762Base);
                if(memcmp(p_QueryChipID_F0F40_Up->read_chip_id_unit_up_.mac_address_.addr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(0)->mtrAddr,6)==0
                    && p_QueryChipID_F0F40_Up->read_chip_id_unit_up_.id_type_==0x02
                    && p_QueryChipID_F0F40_Up->read_chip_id_unit_up_.id_format_==0x02)
                {
                    QString receive_moduleId_single = QString(p_QueryChipID_F0F40_Up->read_chip_id_unit_up_.id_content_.toHex());
                    if (receive_moduleId_single == moduleId_50Bytes_)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString(">>>>期望单相STA模块ID为：%1，实际单相STA模块ID为：%2，符合要求。\r\n").arg(moduleId_50Bytes_).arg(receive_moduleId_single));
                        tryTimes=0;
                        index = index +1;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询三相STA模块ID，等待--回复\r\n");
                        sendMsg(ThreeSTA,p_CtrInfoList->at(0)->ctrlID,index,p_QueryModuleID_F0F40);
                        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("Wait_QueryModuleID_50Bytes_F0F40_Finish，回复模块ID与期望模块ID不一致测试失败，期望单相STA模块ID为：%1，实际单相STA模块ID为：%2，%3\r\n").arg(moduleId_50Bytes_).arg(receive_moduleId_single).arg(test_name_));
                }
                else if (memcmp(p_QueryChipID_F0F40_Up->read_chip_id_unit_up_.mac_address_.addr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(1)->mtrAddr,6)==0
                         && p_QueryChipID_F0F40_Up->read_chip_id_unit_up_.id_type_==0x02
                         && p_QueryChipID_F0F40_Up->read_chip_id_unit_up_.id_format_==0x02)
                {
                    QString receive_moduleId_three = QString(p_QueryChipID_F0F40_Up->read_chip_id_unit_up_.id_content_.toHex());
                    if (receive_moduleId_three == moduleId_50Bytes_)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString(">>>>期望三相STA模块ID为：%1，实际三相STA模块ID为：%2，符合要求。\r\n").arg(moduleId_50Bytes_).arg(receive_moduleId_three));
                        tryTimes=0;
                        index = 0;
                        id_length =len_51Bytes;
                        emScriptRunState = Wait_SetModuleID_51Bytes_F0F39_Finish;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--设置单相STA51字节模块ID，等待--否认\r\n");
                        sendMsg(SingleSTA,p_CtrInfoList->at(0)->ctrlID,index,p_SetModuleID_F0F39);
                        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("Wait_QueryModuleID_50Bytes_F0F40_Finish，回复模块ID与期望模块ID不一致测试失败，期望三相STA模块ID为：%1，期望三相STA模块ID为：%2，%3\r\n").arg(moduleId_50Bytes_).arg(receive_moduleId_three).arg(test_name_));
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("Wait_QueryModuleID_50Bytes_F0F40_Finish，单通/三通的MAC地址或ID类型或ID格式有误测试失败，%1\r\n").arg(test_name_));
                }
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case Wait_SetModuleID_51Bytes_F0F39_Finish:
        {
            if(p_Frame3762Base->afn_== char(0x00) && dtValue3762==2 && p_Frame3762Base->ctrl_field_.dir==kDirUp)
            {
                if (index == 0)
                {
                    tryTimes=0;
                    index = index + 1;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到否认，单相STA模块ID未设置成功，符合要求。");
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--设置三相STA51字节模块ID，等待--否认\r\n");
                    sendMsg(ThreeSTA,p_CtrInfoList->at(0)->ctrlID,index,p_SetModuleID_F0F39);
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                }
                else if (index == 1)
                {
                    p_timer->stop();
                    emScriptRunState = Wait_QueryModuleID_51Bytes_F0F40_Finish;
                    tryTimes=0;
                    index = 0;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到否认，三相STA模块ID未设置成功，符合要求。\r\n");
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--F0F40查询单相STA模块ID，等待--回复\r\n");
                    sendMsg(SingleSTA,p_CtrInfoList->at(0)->ctrlID,index,p_QueryModuleID_F0F40);
                    p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                }
            }
            else
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到其他376.2报文，等待--回复\r\n");
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case Wait_QueryModuleID_51Bytes_F0F40_Finish:
        {
            if(p_Frame3762Base->afn_==char(0xf0) && dtValue3762==40 && p_Frame3762Base->ctrl_field_.dir==kDirUp)
            {
                p_timer->stop();
                shared_ptr<AfnF0F40> p_QueryChipID_F0F40_Up = dynamic_pointer_cast<AfnF0F40>(p_Frame3762Base);
                if(memcmp(p_QueryChipID_F0F40_Up->read_chip_id_unit_up_.mac_address_.addr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(0)->mtrAddr,6)==0
                    && p_QueryChipID_F0F40_Up->read_chip_id_unit_up_.id_type_==0x02
                    && p_QueryChipID_F0F40_Up->read_chip_id_unit_up_.id_format_==0x02)
                {
                    QString receive_moduleId_single = QString(p_QueryChipID_F0F40_Up->read_chip_id_unit_up_.id_content_.toHex());
                    if (receive_moduleId_single == moduleId_50Bytes_)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString(">>>>期望单相STA模块ID为：%1，实际单相模块ID为：%2，符合要求。\r\n").arg(moduleId_50Bytes_).arg(receive_moduleId_single));
                        tryTimes=0;
                        index = index +1;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--查询三相STA模块ID，等待--回复\r\n");
                        sendMsg(ThreeSTA,p_CtrInfoList->at(0)->ctrlID,index,p_QueryModuleID_F0F40);
                        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("Wait_QueryModuleID_51Bytes_F0F40_Finish，回复模块ID与期望模块ID不一致测试失败，期望单相STA模块ID为：%1，实际单相STA模块ID为：%2，%3\r\n").arg(moduleId_50Bytes_).arg(receive_moduleId_single).arg(test_name_));
                }
                else if (memcmp(p_QueryChipID_F0F40_Up->read_chip_id_unit_up_.mac_address_.addr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(1)->mtrAddr,6)==0
                         && p_QueryChipID_F0F40_Up->read_chip_id_unit_up_.id_type_==0x02
                         && p_QueryChipID_F0F40_Up->read_chip_id_unit_up_.id_format_==0x02)
                {
                    QString receive_chipId_threee = QString(p_QueryChipID_F0F40_Up->read_chip_id_unit_up_.id_content_.toHex());
                    if (receive_chipId_threee == moduleId_50Bytes_)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString(">>>>期望三相STA模块ID为：%1，实际三相STA模块ID为：%2，符合要求。\r\n").arg(moduleId_50Bytes_).arg(receive_chipId_threee));
                        tryTimes=0;
                        index = 0;
                        id_length =len_49Bytes;
                        emScriptRunState = Wait_SetModuleID_49Bytes_F0F39_Finish;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--设置单相49字节模块ID，等待--确认\r\n");
                        sendMsg(SingleSTA,p_CtrInfoList->at(0)->ctrlID,index,p_SetModuleID_F0F39);
                        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("Wait_QueryModuleID_51Bytes_F0F40_Finish，回复模块ID与期望模块ID不一致测试失败，期望三相STA模块ID为：%1，实际三相STA模块ID为：%2，%3\r\n").arg(moduleId_50Bytes_).arg(receive_chipId_threee).arg(test_name_));
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("Wait_QueryModuleID_51Bytes_F0F40_Finish，单通/三通的MAC地址或ID类型或ID格式有误测试失败，%1\r\n").arg(test_name_));
                }
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case Wait_SetModuleID_49Bytes_F0F39_Finish:
        {
            if(p_Frame3762Base->afn_== char(0x00) && dtValue3762==1 && p_Frame3762Base->ctrl_field_.dir==kDirUp)
            {
                if (index == 0)
                {
                    tryTimes=0;
                    index = index + 1;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到确认，单相STA模块ID设置成功，符合要求。");
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--设置三相49字节模块ID，等待--确认\r\n");
                    sendMsg(ThreeSTA,p_CtrInfoList->at(0)->ctrlID,index,p_SetModuleID_F0F39);
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                }
                else if (index == 1)
                {
                    p_timer->stop();

                    emScriptRunState = Wait_QueryModuleID_49Bytes_F0F40_Finish;
                    tryTimes=0;
                    index = 0;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到确认，三相STA模块ID设置成功，符合要求。");
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--F0F40查询单相模块ID，等待--回复\r\n");
                    sendMsg(SingleSTA,p_CtrInfoList->at(0)->ctrlID,index,p_QueryModuleID_F0F40);
                    p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                }

            }
            else
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到其他376.2报文，等待--回复\r\n");
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case Wait_QueryModuleID_49Bytes_F0F40_Finish:
        {
            if(p_Frame3762Base->afn_==char(0xf0) && dtValue3762==40 && p_Frame3762Base->ctrl_field_.dir==kDirUp)
            {
                p_timer->stop();
                shared_ptr<AfnF0F40> p_QueryChipID_F0F40_Up = dynamic_pointer_cast<AfnF0F40>(p_Frame3762Base);
                if(memcmp(p_QueryChipID_F0F40_Up->read_chip_id_unit_up_.mac_address_.addr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(0)->mtrAddr,6)==0
                    && p_QueryChipID_F0F40_Up->read_chip_id_unit_up_.id_type_==0x02
                    && p_QueryChipID_F0F40_Up->read_chip_id_unit_up_.id_format_==0x02)
                {
                    QString receive_moduleId_single = QString(p_QueryChipID_F0F40_Up->read_chip_id_unit_up_.id_content_.toHex());
                    if (receive_moduleId_single == moduleId_49Bytes_)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString(">>>>期望单相STA模块ID为：%1，实际单相STA模块ID为：%2，符合要求。\r\n").arg(moduleId_49Bytes_).arg(receive_moduleId_single));
                        tryTimes=0;
                        index = index +1;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--查询三相模块ID，等待--回复\r\n");
                        sendMsg(ThreeSTA,p_CtrInfoList->at(0)->ctrlID,index,p_QueryModuleID_F0F40);
                        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("Wait_QueryModuleID_49Bytes_F0F40_Finish，回复模块ID与期望模块ID不一致测试失败，期望单相模块ID为：%1，实际单相模块ID为：%2，%3\r\n").arg(moduleId_49Bytes_).arg(receive_moduleId_single).arg(test_name_));
                }
                else if (memcmp(p_QueryChipID_F0F40_Up->read_chip_id_unit_up_.mac_address_.addr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(1)->mtrAddr,6)==0
                         && p_QueryChipID_F0F40_Up->read_chip_id_unit_up_.id_type_==0x02
                         && p_QueryChipID_F0F40_Up->read_chip_id_unit_up_.id_format_==0x02)
                {
                    QString receive_moduleId_three = QString(p_QueryChipID_F0F40_Up->read_chip_id_unit_up_.id_content_.toHex());
                    if (receive_moduleId_three == moduleId_49Bytes_)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString(">>>>期望三相STA模块ID为：%1，实际三相STA模块ID为：%2，符合要求。\r\n").arg(moduleId_49Bytes_).arg(receive_moduleId_three));
                        emScriptRunState = Wait_SetModuleID_0Bytes_F0F39_Finish;
                        tryTimes=0;
                        index = 0;
                        id_length =len_0Bytes;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--设置单相0字节模块ID，等待--确认\r\n");
                        sendMsg(SingleSTA,p_CtrInfoList->at(0)->ctrlID,index,p_SetModuleID_F0F39);
                        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("Wait_QueryModuleID_49Bytes_F0F40_Finish，回复模块ID与期望模块ID不一致测试失败，期望三相STA模块ID为：%1，实际三相STA模块ID为：%2，%3\r\n").arg(moduleId_49Bytes_).arg(receive_moduleId_three).arg(test_name_));
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("Wait_QueryModuleID_49Bytes_F0F40_Finish，单通/三通的MAC地址或ID类型或ID格式有误测试失败，%1\r\n").arg(test_name_));
                }
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case Wait_SetModuleID_0Bytes_F0F39_Finish:
        {
            if(p_Frame3762Base->afn_== char(0x00) && dtValue3762==1 && p_Frame3762Base->ctrl_field_.dir==kDirUp)
            {
                if (index == 0)
                {
                    tryTimes=0;
                    index = index + 1;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到确认，单相STA模块ID设置成功，符合要求。");
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--设置三相0字节模块ID，等待--确认\r\n");
                    sendMsg(ThreeSTA,p_CtrInfoList->at(0)->ctrlID,index,p_SetModuleID_F0F39);
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                }
                else if (index == 1)
                {
                    p_timer->stop();
                    emScriptRunState = Wait_QueryModuleID_0Bytes_F0F40_Finish;
                    tryTimes=0;
                    index = 0;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到确认，三相STA模块ID设置成功，符合要求。");
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--F0F40查询单相模块ID，等待--回复\r\n");
                    sendMsg(SingleSTA,p_CtrInfoList->at(0)->ctrlID,index,p_QueryModuleID_F0F40);
                    p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                }

            }
            else
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到其他376.2报文，等待--回复\r\n");
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case Wait_QueryModuleID_0Bytes_F0F40_Finish:
        {
            if(p_Frame3762Base->afn_==char(0xf0) && dtValue3762==40 && p_Frame3762Base->ctrl_field_.dir==kDirUp)
            {
                p_timer->stop();
                shared_ptr<AfnF0F40> p_QueryChipID_F0F40_Up = dynamic_pointer_cast<AfnF0F40>(p_Frame3762Base);
                if(memcmp(p_QueryChipID_F0F40_Up->read_chip_id_unit_up_.mac_address_.addr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(0)->mtrAddr,6)==0
                    && p_QueryChipID_F0F40_Up->read_chip_id_unit_up_.id_type_==0x02
                    && p_QueryChipID_F0F40_Up->read_chip_id_unit_up_.id_format_==0x02)
                {
                    QString receive_moduleId_single = QString(p_QueryChipID_F0F40_Up->read_chip_id_unit_up_.id_content_.toHex());
                    if (receive_moduleId_single == moduleId_0Bytes_)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString(">>>>期望单相STA模块ID为：%1，实际单相STA模块ID为：%2，符合要求。\r\n").arg(moduleId_0Bytes_).arg(receive_moduleId_single));
                        tryTimes=0;
                        index = index +1;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--查询三相模块ID，等待--回复\r\n");
                        sendMsg(ThreeSTA,p_CtrInfoList->at(0)->ctrlID,index,p_QueryModuleID_F0F40);
                        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("Wait_QueryModuleID_0Bytes_F0F40_Finish，回复模块ID与期望模块ID不一致测试失败，期望单相模块ID为：%1，实际单相模块ID为：%2，%3\r\n").arg(moduleId_0Bytes_).arg(receive_moduleId_single).arg(test_name_));
                }
                else if (memcmp(p_QueryChipID_F0F40_Up->read_chip_id_unit_up_.mac_address_.addr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(1)->mtrAddr,6)==0
                         && p_QueryChipID_F0F40_Up->read_chip_id_unit_up_.id_type_==0x02
                         && p_QueryChipID_F0F40_Up->read_chip_id_unit_up_.id_format_==0x02)
                {
                    QString receive_moduleId_three = QString(p_QueryChipID_F0F40_Up->read_chip_id_unit_up_.id_content_.toHex());
                    if (receive_moduleId_three == moduleId_0Bytes_)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString(">>>>期望三相STA模块ID为：%1，实际三相STA模块ID为：%2，符合要求。\r\n").arg(moduleId_0Bytes_).arg(receive_moduleId_three));
                        emScriptRunState = Wait_QueryModuleIDInit_02F1_Finish;
                        tryTimes=0;
                        index = 0;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--透传初始查询单相0字节模块ID，等待--回复\r\n");
                        sendMsg(ReadCtrlDvc,p_CtrInfoList->at(0)->ctrlID,index,p_TransmitData_02F1);
                        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("Wait_QueryModuleID_0Bytes_F0F40_Finish，回复模块ID与期望模块ID不一致测试失败，期望三相STA模块ID为：%1，实际三相STA模块ID为：%2，%3\r\n").arg(moduleId_0Bytes_).arg(receive_moduleId_three).arg(test_name_));
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("Wait_QueryModuleID_0Bytes_F0F40_Finish，单通/三通的MAC地址或ID类型或ID格式有误测试失败，%1\r\n").arg(test_name_));
                }
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case Wait_HardReset_01F1_Finish:
        {
            if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                is_hardReset_01F1_ =false;
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>硬件初始化，回复确认！\r\n");
            }
            else if(p_Frame3762Base->afn_ == 0x03&&p_Frame3762Base->dt1_==0x02&&p_Frame3762Base->dt2_==0x01&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                index=0;
                flagBuildNetOver=false;
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryNetScale_10F9);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--查询网络规模（10F9），等待--回复\r\n");
                p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
            }
            else if(p_Frame3762Base->afn_ == 0x10&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x01&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                shared_ptr<Afn10F9> p_QueryNetScale_10F9_Up=std::dynamic_pointer_cast<Afn10F9>(p_Frame3762Base);
                if(p_QueryNetScale_10F9_Up->network_scale_==p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size()+1)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>网络规模与档案一致，组网完成，30s后查10F40\r\n");
                    flagBuildNetOver=true;
                    emScriptRunState = Wait_QueryModuleID_10F40_Finish;
                    p_delayTimer->start(30*1000);
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString(">>>>查询网络规模（%1轮）！\r\n").arg(++index));
                    p_delayTimer->start(5*1000);
                }

            }
            break;
        }
        case Wait_QueryModuleID_10F40_Finish:
        {
            if(p_Frame3762Base->afn_==char(0x10) && dtValue3762==40 && p_Frame3762Base->ctrl_field_.dir==kDirUp)
            {
                p_timer->stop();
                shared_ptr<Afn10F40> p_QueryStaID_10F40_Up=dynamic_pointer_cast<Afn10F40>(p_Frame3762Base);
                if(memcmp(p_QueryStaID_10F40_Up->read_chip_id_unit_up_.mac_address_.addr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(0)->mtrAddr,6)==0
                    &&p_QueryStaID_10F40_Up->read_chip_id_unit_up_.id_type_==0x02
                    &&p_QueryStaID_10F40_Up->read_chip_id_unit_up_.device_type_==0x03)
                {
                    QString receive_moduleId_single = QString(p_QueryStaID_10F40_Up->read_chip_id_unit_up_.id_content_.toHex());
                    if(moduleId_02F1Send_ == receive_moduleId_single)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString(">>>>期望单相STA的模块ID：%1，实际单相STA的模块ID：%2。符合要求\r\n").arg(moduleId_02F1Send_).arg(receive_moduleId_single));
                        tryTimes = 0;
                        index=index +1;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--查询三相STA模块ID（10F40），等待--回复\r\n");
                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,index,p_QueryStaModuleID_10F40);
                        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("Wait_QueryModuleID_10F40_Finish，回复模块ID与期望模块ID不一致测试失败，期望单相STA模块ID为：%1，实际单相STA模块ID为：%2，%3\r\n").arg(moduleId_02F1Send_).arg(receive_moduleId_single).arg(test_name_));
                }
                else if(memcmp(p_QueryStaID_10F40_Up->read_chip_id_unit_up_.mac_address_.addr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(1)->mtrAddr,6)==0
                         &&p_QueryStaID_10F40_Up->read_chip_id_unit_up_.id_type_==0x02
                         &&p_QueryStaID_10F40_Up->read_chip_id_unit_up_.device_type_==0x03)
                {
                    QString receive_moduleId_three =  QString(p_QueryStaID_10F40_Up->read_chip_id_unit_up_.id_content_.toHex());
                    if(moduleId_02F1Send_ == receive_moduleId_three)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString(">>>>期望三相STA的模块ID：%1，实际三相STA的模块ID：%2。符合要求\r\n").arg(moduleId_02F1Send_).arg(receive_moduleId_three));
                        tryTimes = 0;
                        index=0;
                        is_moduleid_10F40_ =false;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--查询单相STA模块ID（10F7），等待--回复\r\n");
                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,index,p_QueryStaModuleID_10F7);
                        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("Wait_QueryModuleID_10F40_Finish，回复模块ID与期望模块ID不一致测试失败，期望三相STA模块ID为：%1，期望三相STA模块ID为：%2，%3\r\n").arg(moduleId_02F1Send_).arg(receive_moduleId_three).arg(test_name_));
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("Wait_QueryModuleID_10F40_Finish，单通/三通的MAC地址或ID类型或ID格式有误测试失败，%1\r\n").arg(test_name_));
                }
            }
            else if (p_Frame3762Base->afn_==char(0x10) && dtValue3762==7 && p_Frame3762Base->ctrl_field_.dir==kDirUp)
            {
                p_timer->stop();
                shared_ptr<Afn10F7> p_QueryStaID_10F7_Up=dynamic_pointer_cast<Afn10F7>(p_Frame3762Base);

                for(int i=0;i<p_QueryStaID_10F7_Up->node_id_info_unit_.node_id_info_list_.size();i++)
                {
                    if(memcmp(p_QueryStaID_10F7_Up->node_id_info_unit_.node_id_info_list_.at(i).node_address_.addr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(0)->mtrAddr,6)==0 && index ==0)
                    {
                        flagFind=true;
                        QString  receive_moduleId_single = QString(p_QueryStaID_10F7_Up->node_id_info_unit_.node_id_info_list_.at(i).node_id_.toHex());
                        if(moduleId_02F1Send_==receive_moduleId_single)
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString(">>>>期望单相STA模块ID为：%1，实际单相STA模块ID为：%2，符合要求。\r\n").arg(moduleId_02F1Send_).arg(receive_moduleId_single));
                            tryTimes=0;
                            index =index+1;
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--查询三相STA模块ID（10F7），等待--回复\r\n");
                            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,index,p_QueryStaModuleID_10F7);
                            p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
                        }
                        else
                            p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("Wait_QueryModuleID_10F40_Finish，回复模块ID与期望模块ID不一致测试失败，期望单相STA模块ID为：%1，实际单相STA模块ID为：%2，%3\r\n").arg(moduleId_02F1Send_).arg(receive_moduleId_single).arg(test_name_));
                    }
                    else if(memcmp(p_QueryStaID_10F7_Up->node_id_info_unit_.node_id_info_list_.at(i).node_address_.addr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(0)->mtrAddr,6)==0 && index ==1)
                    {
                        flagFind=true;
                        index=0;
                        QString  receive_moduleId_three =  QString(p_QueryStaID_10F7_Up->node_id_info_unit_.node_id_info_list_.at(i).node_id_.toHex());
                        if(moduleId_02F1Send_==receive_moduleId_three)
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString(">>>>期望三相STA模块ID为：%1，实际三相STA模块ID为：%2，符合要求\r\n").arg(moduleId_02F1Send_).arg(receive_moduleId_three));
                            emScriptRunState= ScriptSuccess;
                            resultFlag=true;
                            p_AbstractScriptHost->updateProgress(ProcessState_Success, "脚本执行成功，"+test_name_+"\r\n");
                        }
                        else
                            p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("Wait_QueryModuleID_10F40_Finish，回复模块ID与期望模块ID不一致测试失败，期望三相STA模块ID为：%1，实际三相STA模块ID为：%2，%3\r\n").arg(moduleId_02F1Send_).arg(receive_moduleId_three).arg(test_name_));
                    }
                }
                if(flagFind==false)
                {
                    index+=topoCntPerTime;
                    if(index<=p_QueryStaID_10F7_Up->node_id_info_unit_.node_total_num_)
                    {
                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,index,p_QueryStaModuleID_10F7);
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--查询STA模块ID（10F7），等待--回复\r\n");
                        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed,">>>>没有目标节点\r\n");
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
        default:
            break;
        }
    }
}

void Script_StaModuleID::processMsgFromReadCtrlDvc(DvcType dvcType, int dvcId)
{
    if(dvcId!=p_CtrInfoList->at(0)->ctrlID)
        return;
    bool haveCompleteMsg=true;
    while(haveCompleteMsg)
    {
        qInfo()<<QString("CCO-解析前 buf3762=%1").arg(QString((p_CtrInfoList->at(0)->bufReadCtrlDvc.toHex())));
        shared_ptr<Frame3762Base> p_Frame3762Base = p_Frame3762Helper->DecodeLocalMsg(&(p_CtrInfoList->at(0)->bufReadCtrlDvc),haveCompleteMsg);
        qInfo()<<QString("CCO-解析后 buf3762=%1").arg(QString((p_CtrInfoList->at(0)->bufReadCtrlDvc.toHex())));
        if(p_Frame3762Base==nullptr)
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
        case Wait_QueryModuleIDInit_02F1_Finish:
        {
            if(p_Frame3762Base->afn_ == 0x02&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                shared_ptr<Afn02F1> p_TransmitData_02F1_Up=dynamic_pointer_cast<Afn02F1>(p_Frame3762Base);
                if(p_TransmitData_02F1_Up->frame_content_.size()!=0)
                {
                    shared_ptr<ChkParam_Up> p_ChkParam_Up=ChkParam_Up::decode_ChkParam_Up(&p_TransmitData_02F1_Up->frame_content_);
                    QString receive_moduleId_single = QString(p_ChkParam_Up->paramInfoList.at(0).idCntnt.toHex()).toUpper().remove(0,2);
                    QString receive_moduleId_three =  QString(p_ChkParam_Up->paramInfoList.at(0).idCntnt.toHex()).toUpper().remove(0,2);

                    if(p_ChkParam_Up->paramInfoList.at(0).id == 28 && receive_moduleId_single == moduleId_0Bytes_)
                    {
                        if (index ==0)
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString(">>>>期望单相STA模块ID为：%1，实际单相STA模块ID为：%2，符合要求。\r\n").arg(moduleId_0Bytes_).arg(receive_moduleId_single));
                            tryTimes=0;
                            index = index+1;
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--透传查询三相STA模块ID，等待--回复\r\n");
                            sendMsg(dvcType,p_CtrInfoList->at(0)->ctrlID,index,p_TransmitData_02F1);
                            p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
                        }
                        else if (index ==1)
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString(">>>>期望三相STA模块ID为：%1，实际三相STA模块ID为：%2，符合要求。\r\n").arg(moduleId_0Bytes_).arg(receive_moduleId_single));
                            tryTimes=0;
                            index = 0;
                            emScriptRunState = Wait_SetModuleID_02F1_Finish;
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--透传设置单相STA模块ID，等待--确认\r\n");
                            sendMsg(ReadCtrlDvc,p_CtrInfoList->at(0)->ctrlID,index,p_TransmitData_02F1);
                            p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
                        }
                        else
                            p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("Wait_QueryModuleIDInit_02F1_Finish，回复表地址不是工装表地址测试失败，%1\r\n").arg(test_name_));
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("Wait_QueryModuleIDInit_02F1_Finish，回复模块ID有误，%1\r\n").arg(test_name_));

                    }
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("Wait_QueryModuleIDInit_02F1_Finish，透传数据内容长度为0,进行透传重发。\r\n"));
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--透传查询模块ID，等待--确认\r\n");
                    p_timer->start(500);
                }
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case Wait_SetModuleID_02F1_Finish:
        {
            if(p_Frame3762Base->afn_ == 0x02&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                shared_ptr<Afn02F1> p_TransmitData_02F1_Up=dynamic_pointer_cast<Afn02F1>(p_Frame3762Base);
                if(p_TransmitData_02F1_Up->frame_content_.size()!=0)
                {
                    shared_ptr<SetParam_Up> p_SetParam_Up=SetParam_Up::decode_SetParam_Up(&p_TransmitData_02F1_Up->frame_content_);
                    if(p_SetParam_Up->paramSetRlstInfoList.at(0).setRes==0x00&&p_SetParam_Up->paramSetRlstInfoList.at(0).id == 28)
                    {
                        if(index == 0)
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>收到确认，O2F1设置单相STA模块ID成功。\r\n");
                            tryTimes = 0;
                            index = index +1 ;
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--02F1设置三相STA模块ID，等待--回复\r\n");
                            sendMsg(ReadCtrlDvc,p_CtrInfoList->at(0)->ctrlID,index,p_TransmitData_02F1);
                            p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                        }
                        else if (index ==1)
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>收到确认，O2F1设置三相STA模块ID成功。\r\n");
                            tryTimes = 0;
                            index = 0 ;
                            emScriptRunState = Wait_HardReset_01F1_Finish;
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--硬件复位（01F1），等待--确认\r\n");
                            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_HardReset_01F1);
                            p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                        }
                        else
                            p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("Wait_SetModuleID_02F1_Finish，02F1透传设置模块ID回复表地址有误测试失败，%1\r\n").arg(test_name_));

                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("Wait_SetModuleID_02F1_Finish，02F1透传设置模块ID回复有误，%1\r\n").arg(test_name_));
                    }
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("Wait_SetModuleID_02F1_Finish，透传数据内容长度为0,进行透传重发。\r\n"));
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--透传设置模块ID，等待--确认\r\n");
                    p_timer->start(500);
                }
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        default:
            break;
        case ScriptSuccess:
        {
            break;
        }
        }
    }
}
void Script_StaModuleID::processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID)
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
void Script_StaModuleID::processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID)
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
void Script_StaModuleID::sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame)
{
    if(frame==p_QueryModuleID_F0F40)
    {
        p_QueryModuleID_F0F40->ctrl_field_.dir=kDirDown;
        p_QueryModuleID_F0F40->ctrl_field_.prm=kActive;
        p_QueryModuleID_F0F40->ctrl_field_.comn_type=kHplc;

        p_QueryModuleID_F0F40->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_QueryModuleID_F0F40->info_field_.info_field_down.comu_rate=0;
        p_QueryModuleID_F0F40->info_field_.info_field_down.comu_module_ident=0;

        memcpy(p_QueryModuleID_F0F40->read_chip_id_unit_down_.mac_address_.addr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(meterID)->mtrAddr,6);
        p_QueryModuleID_F0F40->read_chip_id_unit_down_.id_type_=0x02;
        p_QueryModuleID_F0F40->read_chip_id_unit_down_.id_format_=0x02;
        p_QueryModuleID_F0F40->read_chip_id_unit_down_.id_length_=0x32;

        sendMsgOct=p_QueryModuleID_F0F40->EncodeFrame();
        sendMsgLog=QString("》》查询STA模块ID F0F40：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_SetModuleID_F0F39)
    {
        p_SetModuleID_F0F39->ctrl_field_.dir=kDirDown;
        p_SetModuleID_F0F39->ctrl_field_.prm=kActive;
        p_SetModuleID_F0F39->ctrl_field_.comn_type=kHplc;

        p_SetModuleID_F0F39->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_SetModuleID_F0F39->info_field_.info_field_down.comu_rate=0;
        p_SetModuleID_F0F39->info_field_.info_field_down.comu_module_ident=0;

        memcpy(p_SetModuleID_F0F39->config_chip_id_unit_.mac_address_.addr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(meterID)->mtrAddr,6);
        p_SetModuleID_F0F39->config_chip_id_unit_.id_type_=0x02;
        p_SetModuleID_F0F39->config_chip_id_unit_.id_format_=0x02;

        QString moduleId_51Bytes =moduleId_51Bytes_+"01";
        if (id_length == len_50Bytes)
            p_SetModuleID_F0F39->config_chip_id_unit_.id_content_=QByteArray::fromHex(moduleId_50Bytes_.toLatin1());
        else if (id_length ==len_51Bytes)
            p_SetModuleID_F0F39->config_chip_id_unit_.id_content_=QByteArray::fromHex(moduleId_51Bytes.toLatin1());
        else if (id_length ==len_49Bytes)
            p_SetModuleID_F0F39->config_chip_id_unit_.id_content_=QByteArray::fromHex(moduleId_49Bytes_.toLatin1());
        else if (id_length ==len_0Bytes)
            p_SetModuleID_F0F39->config_chip_id_unit_.id_content_=QByteArray::fromHex(moduleId_0Bytes_.toLatin1());

        p_SetModuleID_F0F39->config_chip_id_unit_.id_length_=uchar(p_SetModuleID_F0F39->config_chip_id_unit_.id_content_.size());

        sendMsgOct=p_SetModuleID_F0F39->EncodeFrame();
        sendMsgLog=QString("》》设置STA模块ID F0F39：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_TransmitData_02F1)
    {
        p_TransmitData_02F1->ctrl_field_.dir=kDirDown;
        p_TransmitData_02F1->ctrl_field_.prm=kActive;
        p_TransmitData_02F1->ctrl_field_.comn_type=kHplc;

        p_TransmitData_02F1->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_TransmitData_02F1->info_field_.info_field_down.comu_rate=0;
        p_TransmitData_02F1->info_field_.info_field_down.comu_module_ident=1;

        memcpy(p_TransmitData_02F1->address_field_.src_addr,p_CtrInfoList->at(0)->ccoAddr,6);
        memcpy(p_TransmitData_02F1->address_field_.dst_addr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(meterID)->mtrAddr,6);

        p_TransmitData_02F1->protocol_type_=0x04;

        if(emScriptRunState == Wait_QueryModuleIDInit_02F1_Finish)
            p_TransmitData_02F1->frame_content_=getQueryFrame();
        else if(emScriptRunState== Wait_SetModuleID_02F1_Finish)
            p_TransmitData_02F1->frame_content_=getSetFrame();

        p_TransmitData_02F1->frame_length_=uchar(p_TransmitData_02F1->frame_content_.size());
        sendMsgOct=p_TransmitData_02F1->EncodeFrame();

        sendMsgLog=QString("》》数据透传02F1：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_QueryStaModuleID_10F40)
    {
        p_QueryStaModuleID_10F40->ctrl_field_.dir=kDirDown;
        p_QueryStaModuleID_10F40->ctrl_field_.prm=kActive;
        p_QueryStaModuleID_10F40->ctrl_field_.comn_type=kHplc;

        p_QueryStaModuleID_10F40->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_QueryStaModuleID_10F40->info_field_.info_field_down.comu_rate=0;
        p_QueryStaModuleID_10F40->info_field_.info_field_down.comu_module_ident=0;

        p_QueryStaModuleID_10F40->read_chip_id_unit_down_.device_type_=0x03;
        memcpy(p_QueryStaModuleID_10F40->read_chip_id_unit_down_.mac_address_.addr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(meterID)->mtrAddr,6);
        p_QueryStaModuleID_10F40->read_chip_id_unit_down_.id_type_=0x02;

        sendMsgOct=p_QueryStaModuleID_10F40->EncodeFrame();
        sendMsgLog=QString("》》查询STA模块ID 10F40：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_QueryStaModuleID_10F7)
    {
        p_QueryStaModuleID_10F7->ctrl_field_.dir=kDirDown;
        p_QueryStaModuleID_10F7->ctrl_field_.prm=kActive;
        p_QueryStaModuleID_10F7->ctrl_field_.comn_type=kHplc;

        p_QueryStaModuleID_10F7->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_QueryStaModuleID_10F7->info_field_.info_field_down.comu_rate=0;
        p_QueryStaModuleID_10F7->info_field_.info_field_down.comu_module_ident=0;

        p_QueryStaModuleID_10F7->node_start_no_=index;
        p_QueryStaModuleID_10F7->node_num_=topoCntPerTime;

        sendMsgOct=p_QueryStaModuleID_10F7->EncodeFrame();
        sendMsgLog=QString("》》查询STA模块ID 10F7：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
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
        sendMsgLog=QString("》》硬件初始化01F1：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
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
    else if(frame==p_ParaInit_01F2)
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
void Script_StaModuleID::sendSrcMsg(DvcType dvcType, int dvcId, QByteArray msg)
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

void Script_StaModuleID::timer_timeoutProc()
{
    if(emScriptRunState==Wait_QueryModuleIDInit_F0F40_Finish)
    {
        if(++tryTimes<3)
        {
            if (index ==0)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--初始查询单相STA模块ID，等待--确认\r\n");
                sendMsg(SingleSTA,p_CtrInfoList->at(0)->ctrlID,index,p_QueryModuleID_F0F40);
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            }
            else if (index ==1)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--初始查询三相STA模块ID，等待--确认\r\n");
                sendMsg(ThreeSTA,p_CtrInfoList->at(0)->ctrlID,index,p_QueryModuleID_F0F40);
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            }
        }
        else
        {
            tryTimes=0;
            p_AbstractScriptHost->updateProgress(ProcessState_Error, "Wait_QueryModuleIDInit_F0F40_Finish：timeout!!! "+test_name_);
        }
    }
    else if (emScriptRunState==Wait_SetModuleID_50Bytes_F0F39_Finish)
    {
        if(++tryTimes<3)
        {
            if (index ==0)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--设置单相STA模块ID(50字节)，等待--确认\r\n");
                sendMsg(SingleSTA,p_CtrInfoList->at(0)->ctrlID,index,p_SetModuleID_F0F39);
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            }
            else if (index ==1)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--设置三相STA模块ID(50字节)，等待--确认\r\n");
                sendMsg(ThreeSTA,p_CtrInfoList->at(0)->ctrlID,index,p_SetModuleID_F0F39);
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            }
        }
        else
        {
            tryTimes=0;
            p_AbstractScriptHost->updateProgress(ProcessState_Error, "Wait_SetModuleID_50Bytes_F0F39_Finish：timeout!!! "+test_name_);
        }
    }
    else if (emScriptRunState==Wait_QueryModuleID_50Bytes_F0F40_Finish)
    {
        if(++tryTimes<3)
        {
            if (index ==0)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--查询单相STA模块ID(50字节)，等待--确认\r\n");
                sendMsg(SingleSTA,p_CtrInfoList->at(0)->ctrlID,index,p_QueryModuleID_F0F40);
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            }
            else if (index ==1)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--查询三相STA模块ID(50字节)，等待--确认\r\n");
                sendMsg(ThreeSTA,p_CtrInfoList->at(0)->ctrlID,index,p_QueryModuleID_F0F40);
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            }
        }
        else
        {
            tryTimes=0;
            p_AbstractScriptHost->updateProgress(ProcessState_Error, "Wait_QueryModuleID_50Bytes_F0F40_Finish：timeout!!! "+test_name_);
        }
    }
    else if (emScriptRunState==Wait_SetModuleID_51Bytes_F0F39_Finish)
    {
        if(++tryTimes<3)
        {
            if (index ==0)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--设置单相STA模块ID(51字节)，等待--否认\r\n");
                sendMsg(SingleSTA,p_CtrInfoList->at(0)->ctrlID,index,p_SetModuleID_F0F39);
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            }
            else if (index ==1)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--设置三相STA模块ID(51字节)，等待--否认\r\n");
                sendMsg(ThreeSTA,p_CtrInfoList->at(0)->ctrlID,index,p_SetModuleID_F0F39);
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            }
        }
        else
        {
            tryTimes=0;
            p_AbstractScriptHost->updateProgress(ProcessState_Error, "Wait_SetModuleID_51Bytes_F0F39_Finish：timeout!!! "+test_name_);
        }
    }
    else if (emScriptRunState==Wait_QueryModuleID_51Bytes_F0F40_Finish)
    {
        if(++tryTimes<3)
        {
            if (index ==0)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--查询单相STA模块ID(51字节)，等待--确认\r\n");
                sendMsg(SingleSTA,p_CtrInfoList->at(0)->ctrlID,index,p_QueryModuleID_F0F40);
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            }
            else if (index ==1)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询三相STA模块ID(51字节)，等待--确认\r\n");
                sendMsg(ThreeSTA,p_CtrInfoList->at(0)->ctrlID,index,p_QueryModuleID_F0F40);
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            }
        }
        else
        {
            tryTimes=0;
            p_AbstractScriptHost->updateProgress(ProcessState_Error, "Wait_QueryModuleID_51Bytes_F0F40_Finish：timeout!!! "+test_name_);
        }
    }
    else if (emScriptRunState==Wait_SetModuleID_49Bytes_F0F39_Finish)
    {
        if(++tryTimes<3)
        {
            if (index ==0)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--设置单相STA模块ID(49字节)，等待--确认\r\n");
                sendMsg(SingleSTA,p_CtrInfoList->at(0)->ctrlID,index,p_SetModuleID_F0F39);
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            }
            else if (index ==1)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--设置三相STA模块ID(49字节)，等待--确认\r\n");
                sendMsg(ThreeSTA,p_CtrInfoList->at(0)->ctrlID,index,p_SetModuleID_F0F39);
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            }
        }
        else
        {
            tryTimes=0;
            p_AbstractScriptHost->updateProgress(ProcessState_Error, "Wait_SetModuleID_49Bytes_F0F39_Finish：timeout!!! "+test_name_);
        }
    }
    else if (emScriptRunState==Wait_QueryModuleID_49Bytes_F0F40_Finish)
    {
        if(++tryTimes<3)
        {
            if (index ==0)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--查询单相STA模块ID(49字节)，等待--确认\r\n");
                sendMsg(SingleSTA,p_CtrInfoList->at(0)->ctrlID,index,p_QueryModuleID_F0F40);
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            }
            else if (index ==1)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--查询三相STA模块ID(49字节)，等待--确认\r\n");
                sendMsg(ThreeSTA,p_CtrInfoList->at(0)->ctrlID,index,p_QueryModuleID_F0F40);
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            }
        }
        else
        {
            tryTimes=0;
            p_AbstractScriptHost->updateProgress(ProcessState_Error, "Wait_QueryModuleID_49Bytes_F0F40_Finish：timeout!!! "+test_name_);
        }
    }
    else if (emScriptRunState==Wait_SetModuleID_0Bytes_F0F39_Finish)
    {
        if(++tryTimes<3)
        {
            if (index ==0)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--设置单相STA模块ID(0字节)，等待--确认\r\n");
                sendMsg(SingleSTA,p_CtrInfoList->at(0)->ctrlID,index,p_SetModuleID_F0F39);
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            }
            else if (index ==1)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--设置三相STA模块ID(0字节)，等待--确认\r\n");
                sendMsg(ThreeSTA,p_CtrInfoList->at(0)->ctrlID,index,p_SetModuleID_F0F39);
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            }
        }
        else
        {
            tryTimes=0;
            p_AbstractScriptHost->updateProgress(ProcessState_Error, "Wait_SetModuleID_0Bytes_F0F39_Finish：timeout!!! "+test_name_);
        }
    }
    else if (emScriptRunState==Wait_QueryModuleID_0Bytes_F0F40_Finish)
    {
        if(++tryTimes<3)
        {
            if (index ==0)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--查询单相STA模块ID(0字节)，等待--确认\r\n");
                sendMsg(SingleSTA,p_CtrInfoList->at(0)->ctrlID,index,p_QueryModuleID_F0F40);
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            }
            else if (index ==1)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--查询三相STA模块ID(0字节)，等待--确认\r\n");
                sendMsg(ThreeSTA,p_CtrInfoList->at(0)->ctrlID,index,p_QueryModuleID_F0F40);
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            }
        }
        else
        {
            tryTimes=0;
            p_AbstractScriptHost->updateProgress(ProcessState_Error, "Wait_QueryModuleID_0Bytes_F0F40_Finish：timeout!!! "+test_name_);
        }
    }
    else if (emScriptRunState==Wait_QueryModuleIDInit_02F1_Finish)
    {
        if(++tryTimes<3)
        {
            if (index ==0)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--02F1初始查询单相STA模块ID，等待--回复\r\n");
                sendMsg(ReadCtrlDvc,p_CtrInfoList->at(0)->ctrlID,index, p_TransmitData_02F1);
                p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
            }
            else if (index ==1)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--02F1初始查询三相STA模块ID，等待--回复\r\n");
                sendMsg(ReadCtrlDvc,p_CtrInfoList->at(0)->ctrlID,index,p_TransmitData_02F1);
                p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
            }
        }
        else
        {
            tryTimes=0;
            p_AbstractScriptHost->updateProgress(ProcessState_Error, "Wait_QueryModuleIDInit_02F1_Finish：timeout!!! "+test_name_);
        }
    }
    else if (emScriptRunState==Wait_SetModuleID_02F1_Finish)
    {
        if(++tryTimes<3)
        {
            if (index ==0)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--02F1透传设置单相STA模块ID，等待--回复\r\n");
                sendMsg(ReadCtrlDvc,p_CtrInfoList->at(0)->ctrlID,index, p_TransmitData_02F1);
                p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
            }
            else if (index ==1)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--02F1透传设置三相STA模块ID，等待--回复\r\n");
                sendMsg(ReadCtrlDvc,p_CtrInfoList->at(0)->ctrlID,index,p_TransmitData_02F1);
                p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
            }
        }
        else
        {
            tryTimes=0;
            p_AbstractScriptHost->updateProgress(ProcessState_Error, "Wait_SetModuleID_02F1_Finish：timeout!!! "+test_name_);
        }
    }
    else if (emScriptRunState == Wait_HardReset_01F1_Finish)
    {
        if (is_hardReset_01F1_ ==true)
        {
            if(++tryTimes<3)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>发送--硬件复位（01F1），等待--确认\r\n");
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_HardReset_01F1);
                p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
            }
            else
            {
                tryTimes=0;
                p_AbstractScriptHost->updateProgress(ProcessState_Error, "Wait_HardReset_01F1_Finish：timeout!!! "+test_name_);
            }
        }
        else if (is_hardReset_01F1_ ==false)
        {
            if(++tryTimes<3)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--查询网络规模（10F9），等待--回复\r\n");
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryNetScale_10F9);
                p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
            }
            else
            {
                tryTimes=0;
                p_AbstractScriptHost->updateProgress(ProcessState_Error, "Wait_HardReset_01F1_Finish：timeout：timeout!!! "+test_name_);
            }
        }
    }
    else if (emScriptRunState == Wait_QueryModuleID_10F40_Finish)
    {
        if (is_moduleid_10F40_ ==true)
        {
            if(++tryTimes<3)
            {
                if (index ==0)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--查询单相STA模块ID(10F40)，等待--回复\r\n");
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,index, p_QueryStaModuleID_10F40);
                    p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                }
                else if (index ==1)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--查询三相STA模块ID(10F40)，等待--回复\r\n");
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,index,p_QueryStaModuleID_10F40);
                    p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                }
            }
            else
            {
                tryTimes=0;
                p_AbstractScriptHost->updateProgress(ProcessState_Error, "Wait_QueryModuleID_10F40_Finish：timeout!!! "+test_name_);
            }
        }
        else if (is_moduleid_10F40_ ==false)
        {
            if(++tryTimes<3)
            {
                if (index ==0)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--查询单相STA模块ID(10F112)，等待--回复\r\n");
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,index, p_QueryStaModuleID_10F7);
                    p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                }
                else if (index ==1)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--查询三相STA模块ID(10F112)，等待--回复\r\n");
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,index,p_QueryStaModuleID_10F7);
                    p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                }
            }
            else
            {
                tryTimes=0;
                p_AbstractScriptHost->updateProgress(ProcessState_Error, "Wait_QueryModuleID_10F40_Finish：timeout!!! "+test_name_);
            }
        }
    }
    else
    {
        tryTimes=0;
        p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==p_Timer"+test_name_);
    }
}
void Script_StaModuleID::maxAllowTimer_timeoutProc()
{
    switch(emScriptRunState)
    {

    default:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==p_maxAllowTimer"+test_name_);
        break;
    }
    }
}
void Script_StaModuleID::delayTimer_timeoutProc()
{
    p_delayTimer->stop();
    if(flagBuildNetOver==false)
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--查询网络规模（10F9），等待--回复");
        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryNetScale_10F9);
        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
    }
    else
    {
        tryTimes = 0;
        index = 0 ;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--10F40查询单相STA模块ID，等待--回复");
        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,index,p_QueryStaModuleID_10F40);
        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
    }
}

QByteArray Script_StaModuleID::getSetFrame()
{
    QByteArray msg;
    shared_ptr<SetParam_Down> p_SetParam_Down=make_shared<SetParam_Down>();
    QList<ParamInfo> setParamInfoList;
    ParamInfo tmpParamInfo;
    tmpParamInfo.id=28;//模块ID
    QString id_type = "02";
    tmpParamInfo.idCntnt=QByteArray::fromHex((moduleId_02F1Send_+id_type).toLatin1());
    tmpParamInfo.idLen=ushort(tmpParamInfo.idCntnt.size());
    setParamInfoList.append(tmpParamInfo);

    p_SetParam_Down->msgSeq=msgSeq;
    memcpy(p_SetParam_Down->srcAddr,p_CtrInfoList->at(0)->ccoAddr,6);
    memcpy(p_SetParam_Down->dstAddr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(index)->mtrAddr,6);
    p_SetParam_Down->paramInfoList=setParamInfoList;
    p_SetParam_Down->idCnt=ushort(setParamInfoList.size());

    msg=SetParam_Down::encode_SetParam_Down(p_SetParam_Down);
    return msg;
}

QByteArray Script_StaModuleID::getQueryFrame()
{
    QByteArray msg;
    shared_ptr<ChkParam_Down> p_ChkParam_Down=make_shared<ChkParam_Down>();
    QList<ushort> chkParamIdList;
    chkParamIdList.append(28);

    p_ChkParam_Down->msgSeq=msgSeq;
    memcpy(p_ChkParam_Down->srcAddr,p_CtrInfoList->at(0)->ccoAddr,6);
    memcpy(p_ChkParam_Down->dstAddr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(index)->mtrAddr,6);
    p_ChkParam_Down->idList=chkParamIdList;
    p_ChkParam_Down->idCnt=ushort(chkParamIdList.size());

    msg=ChkParam_Down::encode_ChkParam_Down(p_ChkParam_Down);
    return msg;
}
