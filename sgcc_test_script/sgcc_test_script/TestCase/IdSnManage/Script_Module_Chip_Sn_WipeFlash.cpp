#include "Script_Module_Chip_Sn_WipeFlash.h"
Script_Module_Chip_Sn_WipeFlash::Script_Module_Chip_Sn_WipeFlash(QObject *parent) : QObject(parent)
{
    emScriptRunState=ScriptInit;
    resultFlag=false;
    sendMsgOct.clear();


    p_CtrInfoList=make_shared<QList<shared_ptr<CtrInfo>>>();
    p_BuildNetwork_GW=new BuildNetwork_GW();

    p_Frame3762Helper=make_shared<Frame3762Helper>();
    p_EraseFlash_F0F12=make_shared<AfnF0F12>();
    p_SetChipID_F0F39=make_shared<AfnF0F39>();
    p_QueryChipID_F0F40=make_shared<AfnF0F40>();
    p_SetRouterID_F0F39=make_shared<AfnF0F39>();
    p_QueryRouterID_F0F40=make_shared<AfnF0F40>();
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

Script_Module_Chip_Sn_WipeFlash::~Script_Module_Chip_Sn_WipeFlash()
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
void Script_Module_Chip_Sn_WipeFlash::execute()
{
    p_AbstractScriptHost->updateProgress(ProcessState_Start,"脚本执行流程描述打印：-----whc(2024年04月03)\r\n"
                                                             "1.通用组网流程，组网场景1(不需要组网)，Wait_BuildNetFinish_Whole；\r\n"
                                                             "2.F0F40查询芯片ID(24字节)，Wait_InitialQueryChipID_Finish；\r\n"
                                                             "3.F0F40查询模块ID(50字节)，Wait_InitialQueryRouterID_Finish；\r\n"
                                                             "4.F0F41查询SN(19字节)，Wait_InitialQueryChipSN_Finish；\r\n"
                                                             "5.F0F39设置芯片ID(24字节)，Wait_SetChipID_Finish；\r\n"
                                                             "6.F0F39设置模块ID(50字节)，Wait_SetRouterID_Finish；\r\n"
                                                             "7.F0F42设置SN(19字节)，Wait_SetSN_Finish；\r\n"
                                                             "8.F0F12擦除FLASH，Wait_WipeFlash_Finish；\r\n"
                                                             "9.F0F40查询芯片ID(24字节)，Wait_QueryChipID_Finish；\r\n"
                                                             "10.F0F40查询模块ID(50字节)，Wait_QueryRouterID_Finish；\r\n"
                                                             "12.F0F41查询SN(19字节)，Wait_QueryQuerySN_Finish；\r\n");
    p_AbstractScriptHost->updateProgress(ProcessState_Start, "Test start!"+test_name_);
    emScriptRunState = ScriptInit;
    resultFlag=false;
    if(needBuildNet==true)
    {
    }
    else
    {
        p_BuildNetwork_GW->execute();
        emScriptRunState=Wait_BuildNetFinish_Whole;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "等待全网组网完成");
        p_timer->start((timerForReachThresld+timerAfterReachThresld)*1000);
    }
    p_maxAllowTimer->start((timerForReachThresld+timerAfterReachThresld)*1000);
}
void Script_Module_Chip_Sn_WipeFlash::stop()
{
    if(p_AbstractScriptHost==nullptr)
        return;
    p_AbstractScriptHost->updateProgress(ProcessState_Error, "Test stop!"+test_name_);
    if(needPowerOff==true)
        powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost);
}
void Script_Module_Chip_Sn_WipeFlash::addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq)
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
void Script_Module_Chip_Sn_WipeFlash::setHost(AbstractScriptHost *host)
{
    p_AbstractScriptHost=host;
    p_BuildNetwork_GW->setHost(host);
}
bool Script_Module_Chip_Sn_WipeFlash::config(const QMap<QString,QString> *paraDic)
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
void Script_Module_Chip_Sn_WipeFlash::processMsg(DvcType dvcType, int id, uchar *data, int datalen)
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
                p_maxAllowTimer->start(2*60*1000);//开启脚本最大超时定时器
                tryTimes =0;
                emScriptRunState = Wait_InitialQueryChipID_Finish;
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--初始查询CCO芯片ID");
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryChipID_F0F40);
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
void Script_Module_Chip_Sn_WipeFlash::processCtrlDvcRes(DvcType dvcType, QList<int> snList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params)
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

void Script_Module_Chip_Sn_WipeFlash::processMsgFromCCO(DvcType dvcType, int dvcId)
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
        case Wait_InitialQueryChipID_Finish:
        {
            if(p_Frame3762Base->afn_==char(0xf0) && dtValue3762==40 && p_Frame3762Base->ctrl_field_.dir==kDirUp)
            {
                p_timer->stop();
                shared_ptr<AfnF0F40> p_QueryChipID_F0F40_Up = dynamic_pointer_cast<AfnF0F40>(p_Frame3762Base);
                if(memcmp(p_QueryChipID_F0F40_Up->read_chip_id_unit_up_.mac_address_.addr,p_CtrInfoList->at(0)->ccoAddr,6)==0
                    &&p_QueryChipID_F0F40_Up->read_chip_id_unit_up_.id_type_==0x01)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString(">>>>初始查询CCO的芯片ID为：%1。\r\n").arg(QString(p_QueryChipID_F0F40_Up->read_chip_id_unit_up_.id_content_.toHex())));
                    if(p_QueryChipID_F0F40_Up->read_chip_id_unit_up_.id_content_.size() == 24)
                    {
                        tryTimes=0;
                        emScriptRunState = Wait_InitialQueryRouterID_Finish;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询CCO模块ID，等待--回复\r\n");
                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryRouterID_F0F40);
                        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("Wait_InitialQueryChipID_Finish，初始查询CCO芯片ID时长度不为24字节测试失败，%1\r\n").arg(test_name_));
                }
                else
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("Wait_InitialQueryChipID_Finish，初始查询CCO芯片ID时地址有误，或者ID类型有误测试失败，%1\r\n").arg(test_name_));
            }
            else
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到其他376.2报文，等待--回复");
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case Wait_InitialQueryRouterID_Finish:
        {
            if(p_Frame3762Base->afn_==char(0xf0) && dtValue3762==40 && p_Frame3762Base->ctrl_field_.dir==kDirUp)
            {
                p_timer->stop();
                shared_ptr<AfnF0F40> p_QueryRouterID_F0F40_Up = dynamic_pointer_cast<AfnF0F40>(p_Frame3762Base);
                if(memcmp(p_QueryRouterID_F0F40_Up->read_chip_id_unit_up_.mac_address_.addr,p_CtrInfoList->at(0)->ccoAddr,6)==0
                    &&p_QueryRouterID_F0F40_Up->read_chip_id_unit_up_.id_type_==0x02)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString(">>>>初始查询CCO的路由ID为：%1。\r\n").arg(QString(p_QueryRouterID_F0F40_Up->read_chip_id_unit_up_.id_content_.toHex())));
                    tryTimes=0;
                    emScriptRunState = Wait_InitialQueryChipSN_Finish;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询CCO的SN，等待--回复\r\n");
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QuerySn_F0F41);
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                }
                else
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("Wait_InitialQueryRouterID_Finish，初始查询CCO的路由ID时地址，或ID类型有误测试失败，%1\r\n").arg(test_name_));
            }
            else
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到其他376.2报文，等待--回复");
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case Wait_InitialQueryChipSN_Finish:
        {
            if(p_Frame3762Base->afn_==char(0xf0) && dtValue3762==41 && p_Frame3762Base->ctrl_field_.dir==kDirUp)
            {
                p_timer->stop();
                shared_ptr<AfnF0F41> p_QueryRouterSn_F0F41_Up = dynamic_pointer_cast<AfnF0F41>(p_Frame3762Base);
                if(memcmp(p_QueryRouterSn_F0F41_Up->mac_address_.addr,p_CtrInfoList->at(0)->ccoAddr,6)==0)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString(">>>>初始查询CCO的SN为：%1。\r\n").arg(QString(p_QueryRouterSn_F0F41_Up->sn_content_.toHex())));
                    if(p_QueryRouterSn_F0F41_Up->sn_content_.size() == 19)
                    {
                        tryTimes=0;
                        emScriptRunState = Wait_SetChipID_Finish;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--设置CCO芯片ID，等待--回复确认\r\n");
                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_SetChipID_F0F39);
                        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("Wait_InitialQueryChipSN_Finish，初始查询CCOSN时长度不为19字节测试失败，%1\r\n").arg(test_name_));
                }
                else
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("Wait_InitialQueryChipSN_Finish，初始查询CCOSN时地址有误测试失败，%1\r\n").arg(test_name_));
            }
            else
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到其他376.2报文，等待--回复");
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case Wait_SetChipID_Finish:
        {
            if(p_Frame3762Base->afn_==0 && dtValue3762==1 && p_Frame3762Base->ctrl_field_.dir==kDirUp)
            {
                p_timer->stop();
                tryTimes=0;
                emScriptRunState = Wait_SetRouterID_Finish;
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到确认，CCO芯片ID设置成功。");
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--设置CCO路由ID，等待--确认\r\n");
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_SetRouterID_F0F39);
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            }
            else
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到其他376.2报文，等待--回复");
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case Wait_SetRouterID_Finish:
        {
            if(p_Frame3762Base->afn_==0 && dtValue3762==1 && p_Frame3762Base->ctrl_field_.dir==kDirUp)
            {
                p_timer->stop();
                tryTimes=0;
                emScriptRunState = Wait_SetSN_Finish;
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到确认，CCO路由ID设置成功。");
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--设置CCOSN，等待--确认\r\n");
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_SetSn_F0F42);
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            }
            else
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到其他376.2报文，等待--回复");
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case Wait_SetSN_Finish:
        {
            if(p_Frame3762Base->afn_==0 && dtValue3762==1 && p_Frame3762Base->ctrl_field_.dir==kDirUp)
            {
                p_timer->stop();
                tryTimes=0;
                emScriptRunState = Wait_WipeFlash_Finish;
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到确认，CCOSN设置成功。");
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--F0F12擦除Flash，等待--确认\r\n");
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_EraseFlash_F0F12);
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            }
            else
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到其他376.2报文，等待--回复");
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case Wait_WipeFlash_Finish:
        {
            if(p_Frame3762Base->afn_==0 && dtValue3762==1 && p_Frame3762Base->ctrl_field_.dir==kDirUp)
            {
                p_timer->stop();
                emScriptRunState = Wait_QueryChipID_Finish;
                tryTimes =0;
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--查询CCO芯片ID");
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryChipID_F0F40);
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            }
            else
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到其他376.2报文，等待--回复");
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case Wait_QueryChipID_Finish:
        {
            if(p_Frame3762Base->afn_==char(0xf0) && dtValue3762==40 && p_Frame3762Base->ctrl_field_.dir==kDirUp)
            {
                p_timer->stop();
                shared_ptr<AfnF0F40> p_QueryChipID_F0F40_Up = dynamic_pointer_cast<AfnF0F40>(p_Frame3762Base);
                if(memcmp(p_QueryChipID_F0F40_Up->read_chip_id_unit_up_.mac_address_.addr,p_CtrInfoList->at(0)->ccoAddr,6)==0
                    &&p_QueryChipID_F0F40_Up->read_chip_id_unit_up_.id_type_==0x01)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString(">>>>查询CCO的芯片ID为：%1。期望芯片ID为:%2。\r\n").arg(QString(p_QueryChipID_F0F40_Up->read_chip_id_unit_up_.id_content_.toHex().toUpper())).arg(chipId));

                    if(QString(p_QueryChipID_F0F40_Up->read_chip_id_unit_up_.id_content_.toHex().toUpper()) == chipId)
                    {
                        tryTimes=0;
                        emScriptRunState = Wait_QueryRouterID_Finish;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询CCO模块ID，等待--回复\r\n");
                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryRouterID_F0F40);
                        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("Wait_QueryChipID_Finish，查询CCO芯片ID时与擦除Flash前设置ID不一致测试失败，%1\r\n").arg(test_name_));
                }
                else
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("Wait_QueryChipID_Finish，查询CCO芯片ID时地址有误、或者ID类型错误测试失败，%1\r\n").arg(test_name_));
            }
            else
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到其他376.2报文，等待--回复");
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case Wait_QueryRouterID_Finish:
        {
            if(p_Frame3762Base->afn_==char(0xf0) && dtValue3762==40 && p_Frame3762Base->ctrl_field_.dir==kDirUp)
            {
                p_timer->stop();
                shared_ptr<AfnF0F40> p_QueryRouterID_F0F40_Up = dynamic_pointer_cast<AfnF0F40>(p_Frame3762Base);
                if(memcmp(p_QueryRouterID_F0F40_Up->read_chip_id_unit_up_.mac_address_.addr,p_CtrInfoList->at(0)->ccoAddr,6)==0
                    &&p_QueryRouterID_F0F40_Up->read_chip_id_unit_up_.id_type_==0x02)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString(">>>>查询CCO的路由ID为：%1。期望路由ID为:%2。\r\n").arg(QString(p_QueryRouterID_F0F40_Up->read_chip_id_unit_up_.id_content_.toHex().toUpper())).arg(staId));
                    if(QString(p_QueryRouterID_F0F40_Up->read_chip_id_unit_up_.id_content_.toHex().toUpper()) == staId)
                    {
                        tryTimes=0;
                        emScriptRunState = Wait_QueryQuerySN_Finish;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询CCO的SN，等待--回复\r\n");
                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QuerySn_F0F41);
                        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("Wait_QueryRouterID_Finish，查询CCO路由ID时与擦除Flash前设置ID不一致测试失败，%1\r\n").arg(test_name_));
                }
                else
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("Wait_QueryRouterID_Finish，查询CCO芯片ID时地址有误、或者ID类型错误测试失败，%1\r\n").arg(test_name_));
            }
            else
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到其他376.2报文，等待--回复");
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case Wait_QueryQuerySN_Finish:
        {
            if(p_Frame3762Base->afn_==char(0xf0) && dtValue3762==41 && p_Frame3762Base->ctrl_field_.dir==kDirUp)
            {
                p_timer->stop();
                shared_ptr<AfnF0F41> p_QueryRouterSn_F0F41_Up=dynamic_pointer_cast<AfnF0F41>(p_Frame3762Base);
                if(memcmp(p_QueryRouterSn_F0F41_Up->mac_address_.addr,p_CtrInfoList->at(0)->ccoAddr,6)==0
                    &&p_QueryRouterSn_F0F41_Up->device_type_==0x02)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString(">>>>查询CCO的SN为：%1。期望SN为:%2。\r\n").arg(QString(p_QueryRouterSn_F0F41_Up->sn_content_.toHex().toUpper())).arg(sn));
                    if(QString(p_QueryRouterSn_F0F41_Up->sn_content_.toHex().toUpper()) == sn)
                    {
                        p_maxAllowTimer->stop();
                        emScriptRunState=ScriptSuccess;
                        p_AbstractScriptHost->updateProgress(ProcessState_Success,  "脚本执行成功，"+test_name_+"\r\n");
                    }
                    else
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("Wait_QueryQuerySN_Finish，查询CCOSN时与擦除Flash前设置ID不一致测试失败，%1\r\n").arg(test_name_));
                }
                else
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("Wait_QueryQuerySN_Finish，查询CCOSN时地址有误、或者设备类型错误测试失败，%1\r\n").arg(test_name_));
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

void Script_Module_Chip_Sn_WipeFlash::processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID)
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
void Script_Module_Chip_Sn_WipeFlash::processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID)
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
void Script_Module_Chip_Sn_WipeFlash::sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame)
{
    if(frame==p_SetChipID_F0F39)
    {
        p_SetChipID_F0F39->ctrl_field_.dir=kDirDown;
        p_SetChipID_F0F39->ctrl_field_.prm=kActive;
        p_SetChipID_F0F39->ctrl_field_.comn_type=kHplc;

        p_SetChipID_F0F39->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_SetChipID_F0F39->info_field_.info_field_down.comu_rate=0;
        p_SetChipID_F0F39->info_field_.info_field_down.comu_module_ident=0;

        memcpy(p_SetChipID_F0F39->config_chip_id_unit_.mac_address_.addr,p_CtrInfoList->at(0)->ccoAddr,6);
        p_SetChipID_F0F39->config_chip_id_unit_.id_type_=0x01;
        p_SetChipID_F0F39->config_chip_id_unit_.id_format_=0x02;
        p_SetChipID_F0F39->config_chip_id_unit_.id_content_=QByteArray::fromHex(chipId.toLatin1());
        p_SetChipID_F0F39->config_chip_id_unit_.id_length_=uchar(p_SetChipID_F0F39->config_chip_id_unit_.id_content_.size());

        sendMsgOct=p_SetChipID_F0F39->EncodeFrame();
        sendMsgLog=QString("》》设置CCO芯片ID F0F39：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_SetRouterID_F0F39)
    {
        p_SetRouterID_F0F39->ctrl_field_.dir=kDirDown;
        p_SetRouterID_F0F39->ctrl_field_.prm=kActive;
        p_SetRouterID_F0F39->ctrl_field_.comn_type=kHplc;

        p_SetRouterID_F0F39->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_SetRouterID_F0F39->info_field_.info_field_down.comu_rate=0;
        p_SetRouterID_F0F39->info_field_.info_field_down.comu_module_ident=0;

        memcpy(p_SetRouterID_F0F39->config_chip_id_unit_.mac_address_.addr,p_CtrInfoList->at(0)->ccoAddr,6);
        p_SetRouterID_F0F39->config_chip_id_unit_.id_type_=0x02;
        p_SetRouterID_F0F39->config_chip_id_unit_.id_format_=0x02;
        p_SetRouterID_F0F39->config_chip_id_unit_.id_content_=QByteArray::fromHex(staId.toLatin1());
        p_SetRouterID_F0F39->config_chip_id_unit_.id_length_=uchar(p_SetRouterID_F0F39->config_chip_id_unit_.id_content_.size());

        sendMsgOct=p_SetRouterID_F0F39->EncodeFrame();
        sendMsgLog=QString("》》设置CCO路由ID F0F39：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_SetSn_F0F42)
    {
        p_SetSn_F0F42->ctrl_field_.dir=kDirDown;
        p_SetSn_F0F42->ctrl_field_.prm=kActive;
        p_SetSn_F0F42->ctrl_field_.comn_type=kHplc;

        p_SetSn_F0F42->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_SetSn_F0F42->info_field_.info_field_down.comu_rate=0;
        p_SetSn_F0F42->info_field_.info_field_down.comu_module_ident=0;

        p_SetSn_F0F42->device_type_=0x02;
        memcpy(p_SetSn_F0F42->mac_address_.addr,p_CtrInfoList->at(0)->ccoAddr,6);
        p_SetSn_F0F42->sn_content_=QByteArray::fromHex(sn.toLatin1());

        sendMsgOct=p_SetSn_F0F42->EncodeFrame();
        sendMsgLog=QString("》》设置CCO生产序列号Sn F0F42：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_QueryChipID_F0F40)
    {
        p_QueryChipID_F0F40->ctrl_field_.dir=kDirDown;
        p_QueryChipID_F0F40->ctrl_field_.prm=kActive;
        p_QueryChipID_F0F40->ctrl_field_.comn_type=kHplc;

        p_QueryChipID_F0F40->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_QueryChipID_F0F40->info_field_.info_field_down.comu_rate=0;
        p_QueryChipID_F0F40->info_field_.info_field_down.comu_module_ident=0;

        memcpy(p_QueryChipID_F0F40->read_chip_id_unit_down_.mac_address_.addr,p_CtrInfoList->at(0)->ccoAddr,6);
        p_QueryChipID_F0F40->read_chip_id_unit_down_.id_type_=0x01;
        p_QueryChipID_F0F40->read_chip_id_unit_down_.id_format_=0x02;
        p_QueryChipID_F0F40->read_chip_id_unit_down_.id_length_=0x18;

        sendMsgOct=p_QueryChipID_F0F40->EncodeFrame();
        sendMsgLog=QString("》》查询CCO芯片ID F0F40：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_QueryRouterID_F0F40)
    {
        p_QueryRouterID_F0F40->ctrl_field_.dir=kDirDown;
        p_QueryRouterID_F0F40->ctrl_field_.prm=kActive;
        p_QueryRouterID_F0F40->ctrl_field_.comn_type=kHplc;

        p_QueryRouterID_F0F40->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_QueryRouterID_F0F40->info_field_.info_field_down.comu_rate=0;
        p_QueryRouterID_F0F40->info_field_.info_field_down.comu_module_ident=0;

        memcpy(p_QueryRouterID_F0F40->read_chip_id_unit_down_.mac_address_.addr,p_CtrInfoList->at(0)->ccoAddr,6);
        p_QueryRouterID_F0F40->read_chip_id_unit_down_.id_type_=0x02;
        p_QueryRouterID_F0F40->read_chip_id_unit_down_.id_format_=0x02;
        p_QueryRouterID_F0F40->read_chip_id_unit_down_.id_length_=0x32;

        sendMsgOct=p_QueryRouterID_F0F40->EncodeFrame();
        sendMsgLog=QString("》》查询CCO路由ID F0F40：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_QuerySn_F0F41)
    {
        p_QuerySn_F0F41->ctrl_field_.dir=kDirDown;
        p_QuerySn_F0F41->ctrl_field_.prm=kActive;
        p_QuerySn_F0F41->ctrl_field_.comn_type=kHplc;

        p_QuerySn_F0F41->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_QuerySn_F0F41->info_field_.info_field_down.comu_rate=0;
        p_QuerySn_F0F41->info_field_.info_field_down.comu_module_ident=0;

        p_QuerySn_F0F41->device_type_=0x02;
        memcpy(p_QuerySn_F0F41->mac_address_.addr,p_CtrInfoList->at(0)->ccoAddr,6);

        sendMsgOct=p_QuerySn_F0F41->EncodeFrame();
        sendMsgLog=QString("》》查询CCO生产序列号Sn F0F41：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_EraseFlash_F0F12)
    {
        p_EraseFlash_F0F12->ctrl_field_.dir=kDirDown;
        p_EraseFlash_F0F12->ctrl_field_.prm=kActive;
        p_EraseFlash_F0F12->ctrl_field_.comn_type=kHplc;

        p_EraseFlash_F0F12->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_EraseFlash_F0F12->info_field_.info_field_down.comu_rate=0;
        p_EraseFlash_F0F12->info_field_.info_field_down.comu_module_ident=0;

        sendMsgOct=p_EraseFlash_F0F12->EncodeFrame();
        sendMsgLog=QString("》》擦除Flash F0F12：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
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
void Script_Module_Chip_Sn_WipeFlash::sendSrcMsg(DvcType dvcType, int dvcId, QByteArray msg)
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

void Script_Module_Chip_Sn_WipeFlash::timer_timeoutProc()
{
    if(emScriptRunState == Wait_InitialQueryChipID_Finish)
    {
        if(++tryTimes<3)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--初始查询CCO芯片ID");
            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryChipID_F0F40);
            p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
        }
        else
        {
            tryTimes=0;
            p_AbstractScriptHost->updateProgress(ProcessState_Error, "Wait_InitialQueryChipID_Finish：timeout!!! "+test_name_);
        }
    }
    else if(emScriptRunState == Wait_InitialQueryRouterID_Finish)
    {
        if(++tryTimes<3)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询CCO模块ID，等待--回复\r\n");
            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryRouterID_F0F40);
            p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
        }
        else
        {
            tryTimes=0;
            p_AbstractScriptHost->updateProgress(ProcessState_Error, "Wait_InitialQueryRouterID_Finish：timeout!!! "+test_name_);
        }
    }
    else if(emScriptRunState == Wait_InitialQueryChipSN_Finish)
    {
        if(++tryTimes<3)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询CCO的SN，等待--回复\r\n");
            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QuerySn_F0F41);
            p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
        }
        else
        {
            tryTimes=0;
            p_AbstractScriptHost->updateProgress(ProcessState_Error, "Wait_InitialQueryChipSN_Finish：timeout!!! "+test_name_);
        }
    }
    else if(emScriptRunState == Wait_SetChipID_Finish)
    {
        if(++tryTimes<3)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--设置CCO芯片ID，等待--回复确认\r\n");
            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_SetChipID_F0F39);
            p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
        }
        else
        {
            tryTimes=0;
            p_AbstractScriptHost->updateProgress(ProcessState_Error, "Wait_SetChipID_Finish：timeout!!! "+test_name_);
        }
    }
    else if(emScriptRunState == Wait_SetRouterID_Finish)
    {
        if(++tryTimes<3)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--设置CCO路由ID，等待--确认\r\n");
            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_SetRouterID_F0F39);
            p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
        }
        else
        {
            tryTimes=0;
            p_AbstractScriptHost->updateProgress(ProcessState_Error, "Wait_SetRouterID_Finish：timeout!!! "+test_name_);
        }
    }
    else if(emScriptRunState == Wait_SetSN_Finish)
    {
        if(++tryTimes<3)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--设置CCOSN，等待--确认\r\n");
            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_SetSn_F0F42);
            p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
        }
        else
        {
            tryTimes=0;
            p_AbstractScriptHost->updateProgress(ProcessState_Error, "Wait_SetSN_Finish：timeout!!! "+test_name_);
        }
    }
    else if(emScriptRunState == Wait_WipeFlash_Finish)
    {
        if(++tryTimes<3)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--F0F12擦除Flash，等待--确认\r\n");
            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_EraseFlash_F0F12);
            p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
        }
        else
        {
            tryTimes=0;
            p_AbstractScriptHost->updateProgress(ProcessState_Error, "Wait_WipeFlash_Finish：timeout!!! "+test_name_);
        }
    }
    else if(emScriptRunState == Wait_QueryChipID_Finish)
    {
        if(++tryTimes<3)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--查询CCO芯片ID");
            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryChipID_F0F40);
            p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
        }
        else
        {
            tryTimes=0;
            p_AbstractScriptHost->updateProgress(ProcessState_Error, "Wait_QueryChipID_Finish：timeout!!! "+test_name_);
        }
    }
    else if(emScriptRunState == Wait_QueryRouterID_Finish)
    {
        if(++tryTimes<3)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询CCO模块ID，等待--回复\r\n");
            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryRouterID_F0F40);
            p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
        }
        else
        {
            tryTimes=0;
            p_AbstractScriptHost->updateProgress(ProcessState_Error, "Wait_QueryRouterID_Finish：timeout!!! "+test_name_);
        }
    }
    else if(emScriptRunState == Wait_QueryQuerySN_Finish)
    {
        if(++tryTimes<3)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询CCO的SN，等待--回复\r\n");
            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QuerySn_F0F41);
            p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
        }
        else
        {
            tryTimes=0;
            p_AbstractScriptHost->updateProgress(ProcessState_Error, "Wait_QueryQuerySN_Finish：timeout!!! "+test_name_);
        }
    }
    else
    {
        p_timer->stop();
        p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==p_Timer"+test_name_);
    }

}

void Script_Module_Chip_Sn_WipeFlash::maxAllowTimer_timeoutProc()
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
void Script_Module_Chip_Sn_WipeFlash::delayTimer_timeoutProc()
{
    p_delayTimer->stop();
    if(flagBuildNetOver == false)
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Error, "delayTimer_timeoutProc timeout!!!==p_delayTimer");
    }
}

QByteArray Script_Module_Chip_Sn_WipeFlash::getQueryFrame()
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
