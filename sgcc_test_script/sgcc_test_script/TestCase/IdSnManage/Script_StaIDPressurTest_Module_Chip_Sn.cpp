#include "Script_StaIDPressurTest_Module_Chip_Sn.h"

Script_StaIDPressurTest_Module_Chip_Sn::Script_StaIDPressurTest_Module_Chip_Sn(QObject *parent) : QObject(parent)
{
    emScriptRunState=ScriptInit;
    resultFlag=false;
    sendMsgOct.clear();


    p_CtrInfoList=make_shared<QList<shared_ptr<CtrInfo>>>();
    p_BuildNetwork_GW=new BuildNetwork_GW();

    p_Frame3762Helper=make_shared<Frame3762Helper>();
    p_HardReset_01F1=make_shared<Afn01F1>();
    p_QueryNetScale_10F9=make_shared<Afn10F9>();
    p_QueryChipID_10F40=make_shared<Afn10F40>();
    p_QueryStaID_10F40=make_shared<Afn10F40>();

    p_SetChipID_F0F39=make_shared<AfnF0F39>();
    p_QueryChipID_F0F40=make_shared<AfnF0F40>();
    p_SetStaID_F0F39=make_shared<AfnF0F39>();
    p_QueryStaID_F0F40=make_shared<AfnF0F40>();
    p_SetSn_F0F42=make_shared<AfnF0F42>();
    p_QuerySn_F0F41=make_shared<AfnF0F41>();

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

    header = QDateTime::currentDateTime().toString("yyMMddhhmmss");
    chipId = header + "070809101112131415161718192021222324";
    staId = header + "0708091011121314151617181920212223242526272829303132333435363738394041424344454647484950";
    sn = header + "0708090A0B0C0D0E0F16171819";
}

Script_StaIDPressurTest_Module_Chip_Sn::~Script_StaIDPressurTest_Module_Chip_Sn()
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
void Script_StaIDPressurTest_Module_Chip_Sn::execute()
{
    p_AbstractScriptHost->updateProgress(ProcessState_Start,"脚本执行流程描述打印：-----whc(2024年1月20)\r\n"
                                                             "1.通用组网流程，组网场景3(需要组网)，Wait_BuildNetFinish_Whole；\r\n"
                                                             "2.F0F39设置芯片ID(24字节)(单/三相)，F0F40查询芯片ID(24字节)(单/三相)，SetAndQueryChipID；\r\n"
                                                             "3.F0F39设置模块ID(50字节(单/三相))，F0F40查询模块ID(50字节)(单/三相)，SetAndQueryStaID；\r\n"
                                                             "4.F0F42设置SN(19字节)(单/三相)，F0F41查询SN(19字节)(单/三相)，SetAndQuerySN；\r\n"
                                                             "5.以上设置查询重复20遍\r\n");
    p_AbstractScriptHost->updateProgress(ProcessState_Start, "Test start!"+test_name_);
    emScriptRunState=ScriptInit;
    resultFlag=false;
    if(needBuildNet==true)
    {
        p_BuildNetwork_GW->execute();
        emScriptRunState=Wait_BuildNetFinish_Whole;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "等待全网组网完成");
        p_timer->start((timerForReachThresld+timerAfterReachThresld)*1000);
    }
    p_maxAllowTimer->start((timerForReachThresld+timerAfterReachThresld)*1000);
}
void Script_StaIDPressurTest_Module_Chip_Sn::stop()
{
    if(p_AbstractScriptHost==nullptr)
        return;
    p_AbstractScriptHost->updateProgress(ProcessState_Error, "Test stop!"+test_name_);
    if(needPowerOff==true)
        powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost);
}
void Script_StaIDPressurTest_Module_Chip_Sn::addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq)
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
void Script_StaIDPressurTest_Module_Chip_Sn::setHost(AbstractScriptHost *host)
{
    p_AbstractScriptHost=host;
    p_BuildNetwork_GW->setHost(host);
}
bool Script_StaIDPressurTest_Module_Chip_Sn::config(const QMap<QString,QString> *paraDic)
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
void Script_StaIDPressurTest_Module_Chip_Sn::processMsg(DvcType dvcType, int id, uchar *data, int datalen)
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
            {
                emScriptRunState = SetAndQueryChipID;
                if (execute_num_ == 1)
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString(">>>>压力测试进行第%1轮，\r\n").arg(QString ::number(execute_num_)));
                tryTimes =0;
                is_query_chipid_ =false;
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--串口设置单相STA芯片ID");
                sendMsg(SingleSTA,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_SetChipID_F0F39);
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
                    if(emScriptRunState==SetAndQueryChipID || emScriptRunState==SetAndQueryStaID || emScriptRunState==SetAndQuerySN)
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
void Script_StaIDPressurTest_Module_Chip_Sn::processCtrlDvcRes(DvcType dvcType, QList<int> snList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params)
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

void Script_StaIDPressurTest_Module_Chip_Sn::processMsgFromCCO(DvcType dvcType, int dvcId)
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
        case SetAndQueryChipID:
        {
            if(p_Frame3762Base->afn_==0 && dtValue3762==1 && p_Frame3762Base->ctrl_field_.dir==kDirUp)
            {
                if (index == 0)
                {
                    tryTimes=0;
                    index = index + 1;
                    is_query_chipid_ =false;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到确认，单相STA芯片ID设置成功。");
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--设置三相24字节芯片ID，等待--确认\r\n");
                    sendMsg(ThreeSTA,p_CtrInfoList->at(0)->ctrlID,index,p_SetChipID_F0F39);
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                }
                else if (index == 1)
                {
                    p_timer->stop();
                    tryTimes=0;
                    index = 0;
                    is_query_chipid_ =true;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到确认，三相STA芯片ID设置成功。");
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--F0F40查询单相芯片ID，等待--确认\r\n");
                    sendMsg(SingleSTA,p_CtrInfoList->at(0)->ctrlID,index,p_QueryChipID_F0F40);
                    p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                }

            }
            else if(p_Frame3762Base->afn_==char(0xf0) && dtValue3762==40 && p_Frame3762Base->ctrl_field_.dir==kDirUp)
            {
                p_timer->stop();
                shared_ptr<AfnF0F40> p_QueryChipID_F0F40_Up = dynamic_pointer_cast<AfnF0F40>(p_Frame3762Base);
                if(memcmp(p_QueryChipID_F0F40_Up->read_chip_id_unit_up_.mac_address_.addr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(0)->mtrAddr,6)==0
                    && p_QueryChipID_F0F40_Up->read_chip_id_unit_up_.id_type_==0x01
                    && p_QueryChipID_F0F40_Up->read_chip_id_unit_up_.id_format_==0x02)
                {
                    QString receive_chipId_single = QString(p_QueryChipID_F0F40_Up->read_chip_id_unit_up_.id_content_.toHex());
                    if (receive_chipId_single == chipId)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString(">>>>期望单相STA芯片ID为：%1，实际单相STA芯片ID为：%2，符合要求。\r\n").arg(chipId).arg(receive_chipId_single));
                        tryTimes=0;
                        index = index +1;
                        is_query_chipid_ =true;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询三相STA芯片ID，等待--回复\r\n");
                        sendMsg(ThreeSTA,p_CtrInfoList->at(0)->ctrlID,index,p_QueryChipID_F0F40);
                        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("SetAndQueryChipID，回复芯片ID与期望芯片ID不一致测试失败，期望单相STA芯片ID为：%1，实际单相STA芯片ID为：%2，%3\r\n").arg(chipId).arg(receive_chipId_single).arg(test_name_));
                }
                else if (memcmp(p_QueryChipID_F0F40_Up->read_chip_id_unit_up_.mac_address_.addr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(1)->mtrAddr,6)==0
                         && p_QueryChipID_F0F40_Up->read_chip_id_unit_up_.id_type_==0x01
                         && p_QueryChipID_F0F40_Up->read_chip_id_unit_up_.id_format_==0x02)
                {
                    QString receive_chipId_three = QString(p_QueryChipID_F0F40_Up->read_chip_id_unit_up_.id_content_.toHex());
                    if (receive_chipId_three == chipId)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString(">>>>期望三相STA芯片ID为：%1，实际三相STA芯片ID为：%2，符合要求。\r\n").arg(chipId).arg(receive_chipId_three));
                        tryTimes=0;
                        index = 0;
                        emScriptRunState = SetAndQueryStaID;
                        is_query_staid_ =false;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--设置单相STA19字节模块ID，等待--确认\r\n");
                        sendMsg(SingleSTA,p_CtrInfoList->at(0)->ctrlID,index,p_SetStaID_F0F39);
                        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("SetAndQueryChipID，回复芯片ID与期望芯片ID不一致测试失败，期望三相STA芯片ID为：%1，期望三相STA芯片ID为：%2，%3\r\n").arg(chipId).arg(receive_chipId_three).arg(test_name_));
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("SetAndQueryChipID，单通/三通的MAC地址或ID类型或ID格式有误测试失败，%1\r\n").arg(test_name_));
                }
            }
            else
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到其他376.2报文，等待--回复");
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case SetAndQueryStaID:
        {
            if(p_Frame3762Base->afn_==0 && dtValue3762==1 && p_Frame3762Base->ctrl_field_.dir==kDirUp)
            {
                if (index == 0)
                {
                    tryTimes=0;
                    index = index + 1;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到确认，单相STA模块ID设置成功。");
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--设置三相50字节模块ID，等待--确认\r\n");
                    is_query_staid_ =false;
                    sendMsg(ThreeSTA,p_CtrInfoList->at(0)->ctrlID,index,p_SetStaID_F0F39);
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                }
                else if (index == 1)
                {
                    p_timer->stop();
                    tryTimes=0;
                    index = 0;
                    is_query_staid_ =true;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到确认，三相STA模块ID设置成功。");
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--F0F40查询单相模块ID，等待--确认\r\n");
                    sendMsg(SingleSTA,p_CtrInfoList->at(0)->ctrlID,index,p_QueryStaID_F0F40);
                    p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                }
                else
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "SetAndQueryStaID，回复表地址与期望不符测试失败，"+test_name_);

            }
            else if(p_Frame3762Base->afn_==char(0xf0) && dtValue3762==40 && p_Frame3762Base->ctrl_field_.dir==kDirUp)
            {
                p_timer->stop();
                shared_ptr<AfnF0F40> p_QueryChipID_F0F40_Up = dynamic_pointer_cast<AfnF0F40>(p_Frame3762Base);
                if(memcmp(p_QueryChipID_F0F40_Up->read_chip_id_unit_up_.mac_address_.addr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(0)->mtrAddr,6)==0
                    && p_QueryChipID_F0F40_Up->read_chip_id_unit_up_.id_type_==0x02
                    && p_QueryChipID_F0F40_Up->read_chip_id_unit_up_.id_format_==0x02)
                {
                    QString receive_moduleId_single = QString(p_QueryChipID_F0F40_Up->read_chip_id_unit_up_.id_content_.toHex());
                    if (receive_moduleId_single == staId)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString(">>>>期望单相STA模块ID为：%1，实际单相STA模块ID为：%2，符合要求。\r\n").arg(staId).arg(receive_moduleId_single));
                        tryTimes=0;
                        index = index +1;
                        is_query_staid_ =true;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询三相STA模块ID，等待--回复\r\n");
                        sendMsg(ThreeSTA,p_CtrInfoList->at(0)->ctrlID,index,p_QueryStaID_F0F40);
                        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("SetAndQueryStaID，回复模块ID与期望模块ID不一致测试失败，期望单相STA模块ID为：%1，实际单相STA模块ID为：%2，%3\r\n").arg(staId).arg(receive_moduleId_single).arg(test_name_));
                }
                else if (memcmp(p_QueryChipID_F0F40_Up->read_chip_id_unit_up_.mac_address_.addr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(1)->mtrAddr,6)==0
                         && p_QueryChipID_F0F40_Up->read_chip_id_unit_up_.id_type_==0x02
                         && p_QueryChipID_F0F40_Up->read_chip_id_unit_up_.id_format_==0x02)
                {
                    QString receive_moduleId_three = QString(p_QueryChipID_F0F40_Up->read_chip_id_unit_up_.id_content_.toHex());
                    if (receive_moduleId_three == staId)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString(">>>>期望三相STA模块ID为：%1，实际三相STA模块ID为：%2，符合要求。\r\n").arg(staId).arg(receive_moduleId_three));
                        tryTimes=0;
                        index = 0;
                        is_query_sn_ =false;
                        emScriptRunState = SetAndQuerySN;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--设置单相STA的SN，等待--确认\r\n");
                        sendMsg(SingleSTA,p_CtrInfoList->at(0)->ctrlID,index,p_SetSn_F0F42);
                        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("SetAndQueryStaID，回复模块ID与期望模块ID不一致测试失败，期望三相STA模块ID为：%1，期望三相STA模块ID为：%2，%3\r\n").arg(staId).arg(receive_moduleId_three).arg(test_name_));
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("SetAndQueryStaID，单通/三通的MAC地址或ID类型或ID格式有误测试失败，%1\r\n").arg(test_name_));
                }
            }
            else
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到其他376.2报文，等待--回复");
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case SetAndQuerySN:
        {
            if(p_Frame3762Base->afn_==0 && dtValue3762==1 && p_Frame3762Base->ctrl_field_.dir==kDirUp)
            {
                if (index == 0)
                {
                    tryTimes=0;
                    index = index + 1;
                    is_query_sn_ =false;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到确认，单相STA的SN设置成功。");
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--设置三相19字节的SN，等待--确认\r\n");
                    sendMsg(ThreeSTA,p_CtrInfoList->at(0)->ctrlID,index,p_SetSn_F0F42);
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                }
                else if (index == 1)
                {
                    p_timer->stop();
                    tryTimes=0;
                    index = 0;
                    is_query_sn_ =true;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到确认，三相STA的SN设置成功。");
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--F0F41查询单相的SN，等待--确认\r\n");
                    sendMsg(SingleSTA,p_CtrInfoList->at(0)->ctrlID,index,p_QuerySn_F0F41);
                    p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                }
            }
            else if(p_Frame3762Base->afn_==char(0xf0) && dtValue3762==41 && p_Frame3762Base->ctrl_field_.dir==kDirUp)
            {
                p_timer->stop();
                shared_ptr<AfnF0F41> p_QuerySn_F0F41_Up = dynamic_pointer_cast<AfnF0F41>(p_Frame3762Base);
                if(memcmp(p_QuerySn_F0F41_Up->mac_address_.addr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(0)->mtrAddr,6)==0
                    && p_QuerySn_F0F41_Up->device_type_==0x03)
                {
                    QString receive_sn_single = QString(p_QuerySn_F0F41_Up->sn_content_.toHex()).toUpper();
                    if (receive_sn_single == sn)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString(">>>>期望单相STA的SN为：%1，实际单相STA的SN为：%2，符合要求。\r\n").arg(sn).arg(receive_sn_single));
                        tryTimes=0;
                        index = index +1;
                        is_query_sn_ =true;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询三相STA的SN，等待--回复\r\n");
                        sendMsg(ThreeSTA,p_CtrInfoList->at(0)->ctrlID,index,p_QuerySn_F0F41);
                        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("SetAndQuerySN，回复的SN与期望的SN不一致测试失败，期望单相STA的SN为：%1，实际单相STA的SN为：%2，%3\r\n").arg(sn).arg(receive_sn_single).arg(test_name_));
                }
                else if(memcmp(p_QuerySn_F0F41_Up->mac_address_.addr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(1)->mtrAddr,6)==0
                         && p_QuerySn_F0F41_Up->device_type_==0x03)
                {
                    QString receive_sn_three = QString(p_QuerySn_F0F41_Up->sn_content_.toHex()).toUpper();
                    if (receive_sn_three == sn)
                    {
                        if (execute_num_ < 20)
                        {
                            is_query_sn_ =false;
                            is_query_chipid_ =false;
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString(">>>>期望三相STA的SN为：%1，实际三相STA的SN为：%2，符合要求。\r\n").arg(sn).arg(receive_sn_three));
                            execute_num_ = execute_num_+1;
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString(">>>>压力测试进行第%1轮，\r\n").arg(QString ::number(execute_num_)));
                            tryTimes=0;
                            index = 0;
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--设置单相STA的24字节的芯片ID，等待--确认\r\n");
                            emScriptRunState = SetAndQueryChipID;
                            sendMsg(SingleSTA,p_CtrInfoList->at(0)->ctrlID,index,p_SetChipID_F0F39);
                            p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                        }
                        else
                        {
                            emScriptRunState=ScriptSuccess;
                            p_AbstractScriptHost->updateProgress(ProcessState_Success,  "脚本执行成功，"+test_name_+"\r\n");
                        }
                    }
                    else
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("SetAndQuerySN，回复的SN与期望的SN不一致测试失败，期望三相STA的SN为：%1，期望三相STA的SN为：%2，%3\r\n").arg(sn).arg(receive_sn_three).arg(test_name_));
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("SetAndQuerySN，单通/三通的MAC地址或ID类型或ID格式有误测试失败，%1\r\n").arg(test_name_));
                }
            }
            else
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到其他376.2报文，等待--回复");
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

void Script_StaIDPressurTest_Module_Chip_Sn::processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID)
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
void Script_StaIDPressurTest_Module_Chip_Sn::processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID)
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
void Script_StaIDPressurTest_Module_Chip_Sn::sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame)
{
    if(frame==p_HardReset_01F1)
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
    else if(frame==p_QueryChipID_10F40)
    {
        p_QueryChipID_10F40->ctrl_field_.dir=kDirDown;
        p_QueryChipID_10F40->ctrl_field_.prm=kActive;
        p_QueryChipID_10F40->ctrl_field_.comn_type=kHplc;

        p_QueryChipID_10F40->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_QueryChipID_10F40->info_field_.info_field_down.comu_rate=0;
        p_QueryChipID_10F40->info_field_.info_field_down.comu_module_ident=0;

        p_QueryChipID_10F40->read_chip_id_unit_down_.device_type_=0x03;
        memcpy(p_QueryChipID_10F40->read_chip_id_unit_down_.mac_address_.addr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(meterID)->mtrAddr,6);
        p_QueryChipID_10F40->read_chip_id_unit_down_.id_type_=0x01;

        sendMsgOct=p_QueryChipID_10F40->EncodeFrame();
        sendMsgLog=QString("》》查询STA芯片ID 10F40：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_QueryStaID_10F40)
    {
        p_QueryChipID_10F40->ctrl_field_.dir=kDirDown;
        p_QueryChipID_10F40->ctrl_field_.prm=kActive;
        p_QueryChipID_10F40->ctrl_field_.comn_type=kHplc;

        p_QueryChipID_10F40->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_QueryChipID_10F40->info_field_.info_field_down.comu_rate=0;
        p_QueryChipID_10F40->info_field_.info_field_down.comu_module_ident=0;

        p_QueryChipID_10F40->read_chip_id_unit_down_.device_type_=0x03;
        memcpy(p_QueryChipID_10F40->read_chip_id_unit_down_.mac_address_.addr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(meterID)->mtrAddr,6);
        p_QueryChipID_10F40->read_chip_id_unit_down_.id_type_=0x02;

        sendMsgOct=p_QueryChipID_10F40->EncodeFrame();
        sendMsgLog=QString("》》查询STA模块ID 10F40：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_SetChipID_F0F39)
    {
        p_SetChipID_F0F39->ctrl_field_.dir=kDirDown;
        p_SetChipID_F0F39->ctrl_field_.prm=kActive;
        p_SetChipID_F0F39->ctrl_field_.comn_type=kHplc;

        p_SetChipID_F0F39->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_SetChipID_F0F39->info_field_.info_field_down.comu_rate=0;
        p_SetChipID_F0F39->info_field_.info_field_down.comu_module_ident=0;

        memcpy(p_SetChipID_F0F39->config_chip_id_unit_.mac_address_.addr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(meterID)->mtrAddr,6);
        p_SetChipID_F0F39->config_chip_id_unit_.id_type_=0x01;
        p_SetChipID_F0F39->config_chip_id_unit_.id_format_=0x02;
        p_SetChipID_F0F39->config_chip_id_unit_.id_content_=QByteArray::fromHex(chipId.toLatin1());
        p_SetChipID_F0F39->config_chip_id_unit_.id_length_=uchar(p_SetChipID_F0F39->config_chip_id_unit_.id_content_.size());

        sendMsgOct=p_SetChipID_F0F39->EncodeFrame();
        sendMsgLog=QString("》》设置STA芯片ID F0F39：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_SetStaID_F0F39)
    {
        p_SetStaID_F0F39->ctrl_field_.dir=kDirDown;
        p_SetStaID_F0F39->ctrl_field_.prm=kActive;
        p_SetStaID_F0F39->ctrl_field_.comn_type=kHplc;

        p_SetStaID_F0F39->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_SetStaID_F0F39->info_field_.info_field_down.comu_rate=0;
        p_SetStaID_F0F39->info_field_.info_field_down.comu_module_ident=0;

        memcpy(p_SetStaID_F0F39->config_chip_id_unit_.mac_address_.addr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(meterID)->mtrAddr,6);
        p_SetStaID_F0F39->config_chip_id_unit_.id_type_=0x02;
        p_SetStaID_F0F39->config_chip_id_unit_.id_format_=0x02;
        p_SetStaID_F0F39->config_chip_id_unit_.id_content_=QByteArray::fromHex(staId.toLatin1());
        p_SetStaID_F0F39->config_chip_id_unit_.id_length_=uchar(p_SetStaID_F0F39->config_chip_id_unit_.id_content_.size());

        sendMsgOct=p_SetStaID_F0F39->EncodeFrame();
        sendMsgLog=QString("》》设置STA模块ID F0F39：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_QueryChipID_F0F40)
    {
        p_QueryChipID_F0F40->ctrl_field_.dir=kDirDown;
        p_QueryChipID_F0F40->ctrl_field_.prm=kActive;
        p_QueryChipID_F0F40->ctrl_field_.comn_type=kHplc;

        p_QueryChipID_F0F40->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_QueryChipID_F0F40->info_field_.info_field_down.comu_rate=0;
        p_QueryChipID_F0F40->info_field_.info_field_down.comu_module_ident=0;

        memcpy(p_QueryChipID_F0F40->read_chip_id_unit_down_.mac_address_.addr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(meterID)->mtrAddr,6);
        p_QueryChipID_F0F40->read_chip_id_unit_down_.id_type_=0x01;
        p_QueryChipID_F0F40->read_chip_id_unit_down_.id_format_=0x02;
        p_QueryChipID_F0F40->read_chip_id_unit_down_.id_length_=0x18;

        sendMsgOct=p_QueryChipID_F0F40->EncodeFrame();
        sendMsgLog=QString("》》查询STA芯片ID F0F40：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_QueryStaID_F0F40)
    {
        p_QueryStaID_F0F40->ctrl_field_.dir=kDirDown;
        p_QueryStaID_F0F40->ctrl_field_.prm=kActive;
        p_QueryStaID_F0F40->ctrl_field_.comn_type=kHplc;

        p_QueryStaID_F0F40->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_QueryStaID_F0F40->info_field_.info_field_down.comu_rate=0;
        p_QueryStaID_F0F40->info_field_.info_field_down.comu_module_ident=0;

        memcpy(p_QueryStaID_F0F40->read_chip_id_unit_down_.mac_address_.addr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(meterID)->mtrAddr,6);
        p_QueryStaID_F0F40->read_chip_id_unit_down_.id_type_=0x02;
        p_QueryStaID_F0F40->read_chip_id_unit_down_.id_format_=0x02;
        p_QueryStaID_F0F40->read_chip_id_unit_down_.id_length_=0x32;

        sendMsgOct=p_QueryStaID_F0F40->EncodeFrame();
        sendMsgLog=QString("》》查询STA模块ID F0F40：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_SetSn_F0F42)
    {
        p_SetSn_F0F42->ctrl_field_.dir=kDirDown;
        p_SetSn_F0F42->ctrl_field_.prm=kActive;
        p_SetSn_F0F42->ctrl_field_.comn_type=kHplc;

        p_SetSn_F0F42->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_SetSn_F0F42->info_field_.info_field_down.comu_rate=0;
        p_SetSn_F0F42->info_field_.info_field_down.comu_module_ident=0;

        p_SetSn_F0F42->device_type_=0x03;
        memcpy(p_SetSn_F0F42->mac_address_.addr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(meterID)->mtrAddr,6);
        p_SetSn_F0F42->sn_content_=QByteArray::fromHex(sn.toLatin1());

        sendMsgOct=p_SetSn_F0F42->EncodeFrame();
        sendMsgLog=QString("》》设置STA生产序列号Sn F0F42：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_QuerySn_F0F41)
    {
        p_QuerySn_F0F41->ctrl_field_.dir=kDirDown;
        p_QuerySn_F0F41->ctrl_field_.prm=kActive;
        p_QuerySn_F0F41->ctrl_field_.comn_type=kHplc;

        p_QuerySn_F0F41->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_QuerySn_F0F41->info_field_.info_field_down.comu_rate=0;
        p_QuerySn_F0F41->info_field_.info_field_down.comu_module_ident=0;

        p_QuerySn_F0F41->device_type_=0x03;
        memcpy(p_QuerySn_F0F41->mac_address_.addr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(meterID)->mtrAddr,6);

        sendMsgOct=p_QuerySn_F0F41->EncodeFrame();
        sendMsgLog=QString("》》查询STA生产序列号Sn F0F41：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
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
void Script_StaIDPressurTest_Module_Chip_Sn::sendSrcMsg(DvcType dvcType, int dvcId, QByteArray msg)
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

void Script_StaIDPressurTest_Module_Chip_Sn::timer_timeoutProc()
{
    if(emScriptRunState==SetAndQueryChipID)
    {
        if (is_query_chipid_ ==false)//设置芯片ID超时
        {
            if(++tryTimes<3)
            {
                if (index ==0)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--设置单相24字节芯片ID，等待--确认\r\n");
                    sendMsg(SingleSTA,p_CtrInfoList->at(0)->ctrlID,index,p_SetChipID_F0F39);
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                }
                else if (index ==1)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--设置三相24字节芯片ID，等待--确认\r\n");
                    sendMsg(ThreeSTA,p_CtrInfoList->at(0)->ctrlID,index,p_SetChipID_F0F39);
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                }
            }
            else
            {
                tryTimes=0;
                p_AbstractScriptHost->updateProgress(ProcessState_Error, "SetAndQueryChipID：timeout!!! "+test_name_);
            }
        }
        else if (is_query_chipid_ ==true)//查询芯片ID超时
        {
            if(++tryTimes<3)
            {
                if (index ==0)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--F0F40查询单相芯片ID，等待--确认\r\n");
                    sendMsg(SingleSTA,p_CtrInfoList->at(0)->ctrlID,index,p_QueryChipID_F0F40);
                    p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                }
                else if (index ==1)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--F0F40查询三相芯片ID，等待--确认\r\n");
                    sendMsg(ThreeSTA,p_CtrInfoList->at(0)->ctrlID,index,p_QueryChipID_F0F40);
                    p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                }
            }
            else
            {
                tryTimes=0;
                p_AbstractScriptHost->updateProgress(ProcessState_Error, "SetAndQueryChipID：timeout!!! "+test_name_);
            }
        }
    }
    else if(emScriptRunState==SetAndQueryStaID)
    {
        if(is_query_staid_ == false)//设置模块ID超时
        {
            if(++tryTimes<3)
            {
                if (index ==0)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--设置单相50字节模块ID，等待--确认\r\n");
                    sendMsg(SingleSTA,p_CtrInfoList->at(0)->ctrlID,index,p_SetStaID_F0F39);
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                }
                else if (index ==1)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--设置三相50字节模块ID，等待--确认\r\n");
                    sendMsg(ThreeSTA,p_CtrInfoList->at(0)->ctrlID,index,p_SetStaID_F0F39);
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                }
            }
            else
            {
                tryTimes=0;
                p_AbstractScriptHost->updateProgress(ProcessState_Error, "SetAndQueryStaID：timeout!!! "+test_name_);
            }
        }
        else if (is_query_staid_ ==true)//查询模块ID超时
        {
            if(++tryTimes<3)
            {
                if (index ==0)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询单相STA模块ID，等待--回复\r\n");
                    sendMsg(SingleSTA,p_CtrInfoList->at(0)->ctrlID,index,p_QueryStaID_F0F40);
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                }
                else if (index ==1)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询三相STA模块ID，等待--回复\r\n");
                    sendMsg(ThreeSTA,p_CtrInfoList->at(0)->ctrlID,index,p_QueryStaID_F0F40);
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                }
            }
            else
            {
                tryTimes=0;
                p_AbstractScriptHost->updateProgress(ProcessState_Error, "SetAndQueryStaID：timeout!!! "+test_name_);
            }
        }
    }
    else if(emScriptRunState==SetAndQuerySN)
    {
        if (is_query_sn_ ==false)//设置SN超时
        {
            if(++tryTimes<3)
            {
                if (index ==0)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--设置单相19字节的SN，等待--确认\r\n");
                    sendMsg(SingleSTA,p_CtrInfoList->at(0)->ctrlID,index,p_SetSn_F0F42);
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                }
                else if (index ==1)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--设置三相19字节的SN，等待--确认\r\n");
                    sendMsg(ThreeSTA,p_CtrInfoList->at(0)->ctrlID,index,p_SetSn_F0F42);
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                }
            }
            else
            {
                tryTimes=0;
                p_AbstractScriptHost->updateProgress(ProcessState_Error, "SetAndQuerySN：timeout!!! "+test_name_);
            }
        }
        else if (is_query_sn_ == true)//查询SN超时
        {
            if(++tryTimes<3)
            {
                if (index ==0)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--F0F41查询单相的SN，等待--确认\r\n");
                    sendMsg(SingleSTA,p_CtrInfoList->at(0)->ctrlID,index,p_QuerySn_F0F41);
                    p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                }
                else if (index ==1)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询三相STA的SN，等待--回复\r\n");
                    sendMsg(ThreeSTA,p_CtrInfoList->at(0)->ctrlID,index,p_QuerySn_F0F41);
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                }
            }
            else
            {
                tryTimes=0;
                p_AbstractScriptHost->updateProgress(ProcessState_Error, "SetAndQuerySN：timeout!!! "+test_name_);
            }
        }
    }
    else
    {
        p_timer->stop();
        p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==p_Timer"+test_name_);
    }

}

void Script_StaIDPressurTest_Module_Chip_Sn::maxAllowTimer_timeoutProc()
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
void Script_StaIDPressurTest_Module_Chip_Sn::delayTimer_timeoutProc()
{
    p_delayTimer->stop();
    if(flagBuildNetOver == false)
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询网络规模（10F9），等待--回复");
        tryTimes =0;
        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryNetScale_10F9);
        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
    }
    else
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询STA芯片ID（10F40），等待--回复");
        tryTimes =0;
        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryChipID_10F40);
        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
    }
}

QByteArray Script_StaIDPressurTest_Module_Chip_Sn::getQueryFrame()
{
    QByteArray msg;
    shared_ptr<ChkParam_Down> p_ChkParam_Down=make_shared<ChkParam_Down>();
    QList<ushort> chkParamIdList;
    chkParamIdList.append(26);

    p_ChkParam_Down->msgSeq=msgSeq;
    memcpy(p_ChkParam_Down->srcAddr,p_CtrInfoList->at(0)->ccoAddr,6);
    memcpy(p_ChkParam_Down->dstAddr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(0)->mtrAddr,6);
    p_ChkParam_Down->idList=chkParamIdList;
    p_ChkParam_Down->idCnt=ushort(chkParamIdList.size());

    msg=ChkParam_Down::encode_ChkParam_Down(p_ChkParam_Down);
    return msg;
}
