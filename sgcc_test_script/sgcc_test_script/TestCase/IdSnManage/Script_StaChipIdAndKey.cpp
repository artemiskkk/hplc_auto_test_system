#include "Script_StaChipIdAndKey.h"
Script_StaChipIdAndKey::Script_StaChipIdAndKey(QObject *parent) : QObject(parent)
{
    emScriptRunState = ScriptInit;
    resultFlag=false;
    sendMsgOct.clear();

    p_CtrInfoList=make_shared<QList<shared_ptr<CtrInfo>>>();
    p_BuildNetwork_GW=new BuildNetwork_GW();

    p_Frame3762Helper=make_shared<Frame3762Helper>();
    p_QueryStaChipID_10F112=make_shared<Afn10F112>();
    p_QueryStaChipID_10F40=make_shared<Afn10F40>();
    p_TransmitData_02F1=make_shared<Afn02F1>();
    p_QueryNetScale_10F9=make_shared<Afn10F9>();
    p_HardReset_01F1=make_shared<Afn01F1>();
    p_SetStaChipIDKey_F0F43=make_shared<AfnF0F43>();
    p_QueryStaChipIDKey_F0F44=make_shared<AfnF0F44>();
    p_SetChipID_F0F39=make_shared<AfnF0F39>();
    p_QueryStaChipID_F0F40=make_shared<AfnF0F40>();

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

    QString chipIdHeader;
    chipIdHeader=QDateTime::currentDateTime().toString("yyMMddhhmmss");
    chip_id_ = "010203040506070809101112131415161718" +  chipIdHeader;
}
Script_StaChipIdAndKey::~Script_StaChipIdAndKey()
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



void Script_StaChipIdAndKey::execute()
{
    p_AbstractScriptHost->updateProgress(ProcessState_Start,"脚本执行流程描述打印：-----whc(2024年1月20)\r\n"
                                                             "1.通用组网流程，组网场景3(需要组网)，Wait_BuildNetFinish_Whole；\r\n"
                                                             "2.F0F44初始查询芯片ID和密钥(单/三相)，Wait_QueryChipIDAndKeyInit_F0F44_Finish；\r\n"
                                                             "3.F0F43设置芯片ID和密钥(单/三相)，F0F44查询芯片ID和密钥(单/三相)，Wait_SetChipIDAndKey_F0F43_Finish；\r\n"
                                                             "4.F0F43设置异常芯片ID和密钥(单/三相)，F0F44查询异常芯片ID和密钥(单/三相)，Wait_SetNormalChipIDAndKey_F0F43_Finish；\r\n"
                                                             "5.F0F39设置芯片ID(单/三相)，F0F40查询芯片ID(单/三相)，Wait_SetChipID_F0F39_Finish；\r\n"
                                                             "6.F0F44查询芯片ID和密钥(单/三相)，Wait_QueryChipIDAndKey_F0F44_Finish；\r\n");
    p_AbstractScriptHost->updateProgress(ProcessState_Start, "Test start!"+test_name_);
    emScriptRunState=ScriptInit;
    resultFlag=false;
    //addrList.clear();

    if(needBuildNet==true)
    {
        p_BuildNetwork_GW->execute();
        emScriptRunState=Wait_BuildNetFinish_Whole;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "等待全网组网完成");
        p_timer->start((timerForReachThresld+timerAfterReachThresld)*1000);
    }
    else
    {
        //        index=0;
        //        emScriptRunState=Wait_SetStaChipId_Finish;
        //        sendMsg(ReadCtrlDvc,p_CtrInfoList->at(0)->ctrlID,index,p_TransmitData_02F1);
        //        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--透传设置STA芯片ID，等待--确认");
        //        p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
    }
    p_maxAllowTimer->start((timerForReachThresld+timerAfterReachThresld)*1000);
}

void Script_StaChipIdAndKey::stop()
{
    if(p_AbstractScriptHost==nullptr)
        return;
    p_AbstractScriptHost->updateProgress(ProcessState_Error, "Test stop!"+test_name_);
    if(needPowerOff==true)
        powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost);
}
void Script_StaChipIdAndKey::addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq)
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
void Script_StaChipIdAndKey::setHost(AbstractScriptHost *host)
{
    p_AbstractScriptHost=host;
    p_BuildNetwork_GW->setHost(host);
}
bool Script_StaChipIdAndKey::config(const QMap<QString,QString> *paraDic)
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
void Script_StaChipIdAndKey::processMsg(DvcType dvcType, int id, uchar *data, int datalen)
{
    if(p_CtrInfoList->size()==0)
        return;
    if(emScriptRunState == Wait_BuildNetFinish_Whole)
    {
        if(!p_BuildNetwork_GW->buildNetworkResultFlag)
            p_BuildNetwork_GW->processMsg(dvcType,id,data,datalen);
        else
        {
            //p_BuildNetwork_GW->initBuildNetWork();
            if(p_BuildNetwork_GW->emScriptRunState == BuildNetFinish && p_BuildNetwork_GW->buildNetworkResultFlag==true)
            {
                tryTimes = 0;
                index = 0;
                emScriptRunState = Wait_QueryChipIDAndKeyInit_F0F44_Finish;
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--串口初始查询STA芯片ID+密钥（F0F44）");
                sendMsg(SingleSTA,p_CtrInfoList->at(0)->ctrlID,index,p_QueryStaChipID_10F40);
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
                    if(emScriptRunState == Wait_QueryChipIDAndKeyInit_F0F44_Finish ||emScriptRunState == Wait_SetChipIDAndKey_F0F43_Finish||emScriptRunState == Wait_SetNormalChipIDAndKey_F0F43_Finish
                        ||emScriptRunState == Wait_SetChipID_F0F39_Finish||emScriptRunState == Wait_QueryChipIDAndKey_F0F44_Finish)
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
void Script_StaChipIdAndKey::processCtrlDvcRes(DvcType dvcType, QList<int> snList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params)
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

void Script_StaChipIdAndKey::processMsgFromReadCtrlDvc(DvcType dvcType, int dvcId)
{
    if(dvcId!=p_CtrInfoList->at(0)->ctrlID)
        return;
    bool haveCompleteMsg=true;
    while(haveCompleteMsg)
    {
        qInfo()<<QString("抄控器-解析前 buf3762=%1").arg(QString((p_CtrInfoList->at(0)->bufReadCtrlDvc.toHex())));
        shared_ptr<Frame3762Base> p_Frame3762Base = p_Frame3762Helper->DecodeLocalMsg(&(p_CtrInfoList->at(0)->bufReadCtrlDvc),haveCompleteMsg);
        qInfo()<<QString("抄控器-解析后 buf3762=%1").arg(QString((p_CtrInfoList->at(0)->bufReadCtrlDvc.toHex())));
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
            if (dvcType ==SingleSTA)
            {
                //
            }
            break;
        }
        default:
            break;
        }
    }
}

void Script_StaChipIdAndKey::processMsgFromCCO(DvcType dvcType, int dvcId)
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
        case Wait_QueryChipIDAndKeyInit_F0F44_Finish:
        {
            if(p_Frame3762Base->afn_==char(0xf0) && dtValue3762==44 && p_Frame3762Base->ctrl_field_.dir==kDirUp)
            {
                p_timer->stop();
                shared_ptr<AfnF0F44> p_QueryStaChipIDKey_F0F44_Up = dynamic_pointer_cast<AfnF0F44>(p_Frame3762Base);
                QByteArray chipIDKey;
                for(int i=p_QueryStaChipIDKey_F0F44_Up->read_chip_id_unit_up_.id_length_-1; i>=0; i--)
                {
                    chipIDKey.append(p_QueryStaChipIDKey_F0F44_Up->read_chip_id_unit_up_.id_content_.at(i));
                }
                QString receive_chipIDKey = QString(chipIDKey.toHex());

                if (memcmp(p_QueryStaChipIDKey_F0F44_Up->read_chip_id_unit_up_.mac_address_.addr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(0)->mtrAddr,6)==0
                    && p_QueryStaChipIDKey_F0F44_Up->read_chip_id_unit_up_.id_type_==0x03
                    && p_QueryStaChipIDKey_F0F44_Up->read_chip_id_unit_up_.id_format_==0x02
                    && p_QueryStaChipIDKey_F0F44_Up->read_chip_id_unit_up_.id_length_==346)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString(">>>>初始查询单相STA的芯片ID和密钥为：%1，芯片ID和密钥长度符合要求(固定346字节)。\r\n").arg(receive_chipIDKey));
                    tryTimes=0;
                    index = index + 1;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString(">>>>发送--初始查询三相STA的芯片ID和密钥。\r\n"));
                    sendMsg(ThreeSTA,p_CtrInfoList->at(0)->ctrlID,index,p_QueryStaChipIDKey_F0F44);
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                }
                else if (memcmp(p_QueryStaChipIDKey_F0F44_Up->read_chip_id_unit_up_.mac_address_.addr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(1)->mtrAddr,6)==0
                         && p_QueryStaChipIDKey_F0F44_Up->read_chip_id_unit_up_.id_type_==0x03
                         && p_QueryStaChipIDKey_F0F44_Up->read_chip_id_unit_up_.id_format_==0x02
                         && p_QueryStaChipIDKey_F0F44_Up->read_chip_id_unit_up_.id_length_==346)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString(">>>>初始查询三相STA的芯片ID和密钥为：%1，芯片ID和密钥长度符合要求(固定346字节)。\r\n").arg(receive_chipIDKey));
                    tryTimes=0;
                    index = 0;
                    is_query_normal_id_and_key_= false;
                    emScriptRunState = Wait_SetChipIDAndKey_F0F43_Finish;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString(">>>>发送--设置单相STA的芯片ID和密钥(正常值)，等待--确认。\r\n"));
                    sendMsg(SingleSTA,p_CtrInfoList->at(0)->ctrlID,index,p_SetStaChipIDKey_F0F43);
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                }
                else
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("Wait_QueryChipIDAndKeyInit_F0F44_Finish，回复的期望地址/ID类型/ID格式/长度有误测试失败。%1\r\n").arg(test_name_));
            }
            else
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("Wait_QueryChipIDAndKeyInit_F0F44_Finish，收到其他376.2报文，进行处理。\r\n"));
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }

            break;
        }
        case Wait_SetChipIDAndKey_F0F43_Finish:
        {
            if(p_Frame3762Base->afn_==0 && dtValue3762==1 && p_Frame3762Base->ctrl_field_.dir==kDirUp)
            {
                if (index ==0)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到确认，串口设置单相STA的芯片ID+密钥成功，符合要求。\r\n");
                    p_timer->stop();
                    tryTimes =0;
                    index =index + 1;
                    is_query_normal_id_and_key_= false;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--设置三相STA的芯片ID和密钥(正常值)，等待--确认。\r\n");
                    sendMsg(ThreeSTA,p_CtrInfoList->at(0)->ctrlID,index,p_SetStaChipIDKey_F0F43);
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                }
                else if (index == 1)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,  "收到确认，串口设置三相STA的芯片ID+密钥成功，符合要求。\r\n");
                    p_timer->stop();
                    tryTimes =0;
                    index =0;
                    is_query_normal_id_and_key_= true;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--查询单相STA的芯片ID和密钥(正常值)。\r\n");
                    sendMsg(SingleSTA,p_CtrInfoList->at(0)->ctrlID,index,p_QueryStaChipIDKey_F0F44);
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                }
                else
                p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("Wait_SetChipIDAndKey_F0F43_Finish，回复确认帧的期望序号（0单相，1三相）有误测试失败。%1\r\n").arg(test_name_));

            }
            else if(p_Frame3762Base->afn_==char(0xf0) && dtValue3762==44 && p_Frame3762Base->ctrl_field_.dir==kDirUp)
            {
                p_timer->stop();
                shared_ptr<AfnF0F44> p_QueryStaChipIDKey_F0F44_Up = dynamic_pointer_cast<AfnF0F44>(p_Frame3762Base);
                QByteArray chipIDKey;
                for(int i=p_QueryStaChipIDKey_F0F44_Up->read_chip_id_unit_up_.id_length_-1; i>=0; i--)
                {
                    chipIDKey.append(p_QueryStaChipIDKey_F0F44_Up->read_chip_id_unit_up_.id_content_.at(i));
                }
                QString receive_chipIDKey = QString(chipIDKey.toHex());

                if (memcmp(p_QueryStaChipIDKey_F0F44_Up->read_chip_id_unit_up_.mac_address_.addr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(0)->mtrAddr,6)==0
                    && p_QueryStaChipIDKey_F0F44_Up->read_chip_id_unit_up_.id_type_==0x03
                    && p_QueryStaChipIDKey_F0F44_Up->read_chip_id_unit_up_.id_format_==0x02
                    && p_QueryStaChipIDKey_F0F44_Up->read_chip_id_unit_up_.id_length_==346
                    && receive_chipIDKey == normal_id_and_key_)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString(">>>>期望单相STA的芯片ID和密钥(正常值)为：%1，实际回复单相STA的芯片ID和密钥为%2。\r\n").arg(normal_id_and_key_).arg(receive_chipIDKey));
                    tryTimes=0;
                    index = index + 1;
                    is_query_normal_id_and_key_= true;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString(">>>>发送--查询三相STA的芯片ID和密钥(正常值)。\r\n"));
                    sendMsg(ThreeSTA,p_CtrInfoList->at(0)->ctrlID,index,p_QueryStaChipIDKey_F0F44);
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                }
                else if (memcmp(p_QueryStaChipIDKey_F0F44_Up->read_chip_id_unit_up_.mac_address_.addr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(1)->mtrAddr,6)==0
                         && p_QueryStaChipIDKey_F0F44_Up->read_chip_id_unit_up_.id_type_==0x03
                         && p_QueryStaChipIDKey_F0F44_Up->read_chip_id_unit_up_.id_format_==0x02
                         && p_QueryStaChipIDKey_F0F44_Up->read_chip_id_unit_up_.id_length_==346
                         && receive_chipIDKey == normal_id_and_key_)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString(">>>>期望三相STA的芯片ID和密钥(正常值)为：%1，实际回复三相STA的芯片ID和密钥为%2。\r\n").arg(normal_id_and_key_).arg(receive_chipIDKey));
                    tryTimes=0;
                    index = 0;
                    is_query_normal_id_and_key_= false;
                    is_query_abnormal_id_and_key_= false;
                    emScriptRunState = Wait_SetNormalChipIDAndKey_F0F43_Finish;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString(">>>>发送--设置单相STA的芯片ID和密钥(异常值)。\r\n"));
                    sendMsg(SingleSTA,p_CtrInfoList->at(0)->ctrlID,index,p_SetStaChipIDKey_F0F43);
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                }
                else
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("Wait_SetChipIDAndKey_F0F43_Finish，回复的期望地址/ID类型/ID格式/长度/芯片ID和密钥的内容有误测试失败。%1\r\n").arg(test_name_));
            }
            else
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("Wait_SetChipIDAndKey_F0F43_Finish，收到其他376.2报文，进行处理。\r\n"));
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }

            break;
        }
        case Wait_SetNormalChipIDAndKey_F0F43_Finish:
        {
            if(p_Frame3762Base->afn_==0 && dtValue3762==1 && p_Frame3762Base->ctrl_field_.dir==kDirUp)
            {
                if (index ==0)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到确认，串口设置单相STA的芯片ID+密钥成功(异常值)，符合要求。\r\n");
                    p_timer->stop();
                    tryTimes =0;
                    index =index + 1;
                    is_query_abnormal_id_and_key_= false;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--设置三相STA的芯片ID和密钥(异常值)，等待--确认。\r\n");
                    sendMsg(ThreeSTA,p_CtrInfoList->at(0)->ctrlID,index,p_SetStaChipIDKey_F0F43);
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                }
                else if (index == 1)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,  "收到确认，串口设置三相STA的芯片ID+密钥成功(异常值)，符合要求。\r\n");
                    p_timer->stop();
                    tryTimes =0;
                    index =0;
                    is_query_abnormal_id_and_key_= true;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--查询单相STA的芯片ID和密钥(异常值)。\r\n");
                    sendMsg(SingleSTA,p_CtrInfoList->at(0)->ctrlID,index,p_QueryStaChipIDKey_F0F44);
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                }
                else
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("Wait_SetNormalChipIDAndKey_F0F43_Finish，回复确认帧的期望序号（0单相，1三相）有误测试失败。%1\r\n").arg(test_name_));

            }
            else if(p_Frame3762Base->afn_==char(0xf0) && dtValue3762==44 && p_Frame3762Base->ctrl_field_.dir==kDirUp)
            {
                p_timer->stop();
                shared_ptr<AfnF0F44> p_QueryStaChipIDKey_F0F44_Up = dynamic_pointer_cast<AfnF0F44>(p_Frame3762Base);
                QByteArray chipIDKey;
                for(int i=p_QueryStaChipIDKey_F0F44_Up->read_chip_id_unit_up_.id_length_-1; i>=0; i--)
                {
                    chipIDKey.append(p_QueryStaChipIDKey_F0F44_Up->read_chip_id_unit_up_.id_content_.at(i));
                }
                QString receive_chipIDKey = QString(chipIDKey.toHex());

                if (memcmp(p_QueryStaChipIDKey_F0F44_Up->read_chip_id_unit_up_.mac_address_.addr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(0)->mtrAddr,6)==0
                    && p_QueryStaChipIDKey_F0F44_Up->read_chip_id_unit_up_.id_type_==0x03
                    && p_QueryStaChipIDKey_F0F44_Up->read_chip_id_unit_up_.id_format_==0x02
                    && p_QueryStaChipIDKey_F0F44_Up->read_chip_id_unit_up_.id_length_==346
                    && receive_chipIDKey == normal_id_and_key_)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString(">>>>查询单相STA的芯片ID和密钥(异常值)为：%1，芯片ID和密钥长度符合要求(固定346字节)。\r\n").arg(receive_chipIDKey));
                    tryTimes=0;
                    index = index + 1;
                    is_query_abnormal_id_and_key_= true;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString(">>>>发送--查询三相STA的芯片ID和密钥(异常值)。\r\n"));
                    sendMsg(ThreeSTA,p_CtrInfoList->at(0)->ctrlID,index,p_QueryStaChipIDKey_F0F44);
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                }
                else if (memcmp(p_QueryStaChipIDKey_F0F44_Up->read_chip_id_unit_up_.mac_address_.addr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(1)->mtrAddr,6)==0
                         && p_QueryStaChipIDKey_F0F44_Up->read_chip_id_unit_up_.id_type_==0x03
                         && p_QueryStaChipIDKey_F0F44_Up->read_chip_id_unit_up_.id_format_==0x02
                         && p_QueryStaChipIDKey_F0F44_Up->read_chip_id_unit_up_.id_length_==346
                         && receive_chipIDKey == normal_id_and_key_)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString(">>>>查询三相STA的芯片ID和密钥(异常值)为：%1，芯片ID和密钥长度符合要求(固定346字节)。\r\n").arg(receive_chipIDKey));
                    tryTimes=0;
                    index = 0;
                    is_query_chip_id_= false;
                    emScriptRunState = Wait_SetChipID_F0F39_Finish;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString(">>>>发送--设置单相STA的芯片ID(F0F39)。\r\n"));
                    sendMsg(SingleSTA,p_CtrInfoList->at(0)->ctrlID,index,p_SetChipID_F0F39);
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                }
                else
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("Wait_SetChipIDAndKey_F0F43_Finish，回复的期望地址/ID类型/ID格式/长度/芯片ID和密钥的内容有误测试失败。%1\r\n").arg(test_name_));
            }
            else
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("Wait_SetChipIDAndKey_F0F43_Finish，收到其他376.2报文，进行处理。\r\n"));
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }

            break;
        }
        case Wait_SetChipID_F0F39_Finish:
        {
            if(p_Frame3762Base->afn_== char(0x00) && dtValue3762==1 && p_Frame3762Base->ctrl_field_.dir==kDirUp)
            {
                if (index == 0)
                {
                    tryTimes=0;
                    index = index + 1;
                    is_query_chip_id_= false;
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
                    is_query_chip_id_= true;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到确认，三相STA芯片ID设置成功。");
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--F0F40查询单相芯片ID，等待--确认\r\n");
                    sendMsg(SingleSTA,p_CtrInfoList->at(0)->ctrlID,index,p_QueryStaChipID_F0F40);
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                }
                else
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("Wait_SetChipID_F0F39_Finish，回复确认帧的序号（0单相，1三相）有误测试失败。%1\r\n").arg(test_name_));

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
                    if (receive_chipId_single == chip_id_)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString(">>>>期望单相STA芯片ID为：%1，实际单相STA芯片ID为：%2，符合要求。\r\n").arg(chip_id_).arg(receive_chipId_single));
                        tryTimes=0;
                        index = index +1;
                        is_query_chip_id_= true;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询三相STA芯片ID，等待--回复\r\n");
                        sendMsg(ThreeSTA,p_CtrInfoList->at(0)->ctrlID,index,p_QueryStaChipID_F0F40);
                        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("Wait_SetChipID_F0F39_Finish，回复芯片ID与期望芯片ID不一致测试失败，期望单相STA芯片ID为：%1，实际单相STA芯片ID为：%2，%3\r\n").arg(chip_id_).arg(receive_chipId_single).arg(test_name_));
                }
                else if (memcmp(p_QueryChipID_F0F40_Up->read_chip_id_unit_up_.mac_address_.addr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(1)->mtrAddr,6)==0
                         && p_QueryChipID_F0F40_Up->read_chip_id_unit_up_.id_type_==0x01
                         && p_QueryChipID_F0F40_Up->read_chip_id_unit_up_.id_format_==0x02)
                {
                    QString receive_chipId_three = QString(p_QueryChipID_F0F40_Up->read_chip_id_unit_up_.id_content_.toHex());
                    if (receive_chipId_three == chip_id_)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString(">>>>期望三相STA芯片ID为：%1，实际三相STA芯片ID为：%2，符合要求。\r\n").arg(chip_id_).arg(receive_chipId_three));
                        tryTimes=0;
                        index = 0;
                        is_query_chip_id_= false;
                        emScriptRunState = Wait_QueryChipIDAndKey_F0F44_Finish;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--查询单相STA的芯片ID和密钥，等待--回复\r\n");
                        sendMsg(SingleSTA,p_CtrInfoList->at(0)->ctrlID,index,p_QueryStaChipIDKey_F0F44);
                        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("Wait_SetChipID_F0F39_Finish，回复芯片ID与期望芯片ID不一致测试失败，期望三相STA芯片ID为：%1，期望三相STA芯片ID为：%2，%3\r\n").arg(chip_id_).arg(receive_chipId_three).arg(test_name_));
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("Wait_SetChipID_F0F39_Finish，单通/三通的MAC地址或ID类型或ID格式有误测试失败，%1\r\n").arg(test_name_));
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
        case Wait_QueryChipIDAndKey_F0F44_Finish:
        {
            if(p_Frame3762Base->afn_==char(0xf0) && dtValue3762==44 && p_Frame3762Base->ctrl_field_.dir==kDirUp)
            {
                p_timer->stop();
                shared_ptr<AfnF0F44> p_QueryStaChipIDKey_F0F44_Up = dynamic_pointer_cast<AfnF0F44>(p_Frame3762Base);
                QByteArray chipIDKey;
                for(int i=p_QueryStaChipIDKey_F0F44_Up->read_chip_id_unit_up_.id_length_-1; i>=0; i--)
                {
                    chipIDKey.append(p_QueryStaChipIDKey_F0F44_Up->read_chip_id_unit_up_.id_content_.at(i));
                }
                QString receive_chipIDKey = QString(chipIDKey.toHex());

                if (memcmp(p_QueryStaChipIDKey_F0F44_Up->read_chip_id_unit_up_.mac_address_.addr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(0)->mtrAddr,6)==0
                    && p_QueryStaChipIDKey_F0F44_Up->read_chip_id_unit_up_.id_type_==0x03
                    && p_QueryStaChipIDKey_F0F44_Up->read_chip_id_unit_up_.id_format_==0x02
                    && p_QueryStaChipIDKey_F0F44_Up->read_chip_id_unit_up_.id_length_==346
                    && receive_chipIDKey == normal_id_and_key_)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString(">>>>期望单相STA的芯片ID和密钥(正常值)为：%1，实际回复单相STA的芯片ID和密钥为%2。\r\n").arg(normal_id_and_key_).arg(receive_chipIDKey));
                    tryTimes=0;
                    index = index + 1;
                    is_query_normal_id_and_key_= true;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString(">>>>发送--查询三相STA的芯片ID和密钥(正常值)。\r\n"));
                    sendMsg(ThreeSTA,p_CtrInfoList->at(0)->ctrlID,index,p_QueryStaChipIDKey_F0F44);
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                }
                else if (memcmp(p_QueryStaChipIDKey_F0F44_Up->read_chip_id_unit_up_.mac_address_.addr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(1)->mtrAddr,6)==0
                         && p_QueryStaChipIDKey_F0F44_Up->read_chip_id_unit_up_.id_type_==0x03
                         && p_QueryStaChipIDKey_F0F44_Up->read_chip_id_unit_up_.id_format_==0x02
                         && p_QueryStaChipIDKey_F0F44_Up->read_chip_id_unit_up_.id_length_==346
                         && receive_chipIDKey == normal_id_and_key_)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString(">>>>期望三相STA的芯片ID和密钥(正常值)为：%1，实际回复三相STA的芯片ID和密钥为%2。\r\n").arg(normal_id_and_key_).arg(receive_chipIDKey));
                    tryTimes=0;
                    index = 0;
                    is_query_normal_id_and_key_= false;
                    is_query_abnormal_id_and_key_= false;
                    emScriptRunState= ScriptSuccess;
                    resultFlag=true;
                    p_AbstractScriptHost->updateProgress(ProcessState_Success, "脚本执行成功，"+test_name_+"\r\n");
                }
                else
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("Wait_SetChipIDAndKey_F0F43_Finish，回复的期望地址/ID类型/ID格式/长度/芯片ID和密钥的内容有误测试失败。%1\r\n").arg(test_name_));
            }
            else
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("Wait_SetChipIDAndKey_F0F43_Finish，收到其他376.2报文，进行处理。\r\n"));
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

void Script_StaChipIdAndKey::processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID)
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
void Script_StaChipIdAndKey::processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID)
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
void Script_StaChipIdAndKey::sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame)
{
    if(frame==p_TransmitData_02F1)
    {
        p_TransmitData_02F1->ctrl_field_.dir=kDirDown;
        p_TransmitData_02F1->ctrl_field_.prm=kActive;
        p_TransmitData_02F1->ctrl_field_.comn_type=kHplc;

        p_TransmitData_02F1->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_TransmitData_02F1->info_field_.info_field_down.comu_rate=0;
        p_TransmitData_02F1->info_field_.info_field_down.comu_module_ident=1;

        memcpy(p_TransmitData_02F1->address_field_.src_addr,p_CtrInfoList->at(0)->ccoAddr,6);
        memcpy(p_TransmitData_02F1->address_field_.dst_addr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(0)->mtrAddr,6);

        p_TransmitData_02F1->protocol_type_=0x04;
        p_TransmitData_02F1->frame_content_=getQueryFrame();
        p_TransmitData_02F1->frame_length_=uchar(p_TransmitData_02F1->frame_content_.size());

        sendMsgOct=p_TransmitData_02F1->EncodeFrame();
        sendMsgLog=QString("》》数据透传02F1：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
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
    else if(frame==p_QueryStaChipID_10F40)
    {
        p_QueryStaChipID_10F40->ctrl_field_.dir=kDirDown;
        p_QueryStaChipID_10F40->ctrl_field_.prm=kActive;
        p_QueryStaChipID_10F40->ctrl_field_.comn_type=kHplc;

        p_QueryStaChipID_10F40->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_QueryStaChipID_10F40->info_field_.info_field_down.comu_rate=0;
        p_QueryStaChipID_10F40->info_field_.info_field_down.comu_module_ident=0;

        p_QueryStaChipID_10F40->read_chip_id_unit_down_.device_type_=0x03;
        memcpy(p_QueryStaChipID_10F40->read_chip_id_unit_down_.mac_address_.addr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(0)->mtrAddr,6);
        p_QueryStaChipID_10F40->read_chip_id_unit_down_.id_type_=0x01;

        sendMsgOct=p_QueryStaChipID_10F40->EncodeFrame();
        sendMsgLog=QString("》》查询STA芯片ID 10F40：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_QueryStaChipID_10F112)
    {
        p_QueryStaChipID_10F112->ctrl_field_.dir=kDirDown;
        p_QueryStaChipID_10F112->ctrl_field_.prm=kActive;
        p_QueryStaChipID_10F112->ctrl_field_.comn_type=kHplc;

        p_QueryStaChipID_10F112->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_QueryStaChipID_10F112->info_field_.info_field_down.comu_rate=0;
        p_QueryStaChipID_10F112->info_field_.info_field_down.comu_module_ident=0;

        p_QueryStaChipID_10F112->node_start_no_=2;
        p_QueryStaChipID_10F112->node_num_=1;

        sendMsgOct=p_QueryStaChipID_10F112->EncodeFrame();
        sendMsgLog=QString("》》查询STA芯片ID 10F112：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_SetStaChipIDKey_F0F43)
    {
        p_SetStaChipIDKey_F0F43->ctrl_field_.dir=kDirDown;
        p_SetStaChipIDKey_F0F43->ctrl_field_.prm=kActive;
        //p_SetStaChipIDKey_F0F43->ctrl_field_.comn_type=kHplc;
        p_SetStaChipIDKey_F0F43->ctrl_field_.comn_type=kCenterRoute;

        p_SetStaChipIDKey_F0F43->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_SetStaChipIDKey_F0F43->info_field_.info_field_down.comu_rate=0;
        p_SetStaChipIDKey_F0F43->info_field_.info_field_down.comu_module_ident=0;

        memcpy(p_SetStaChipIDKey_F0F43->mac_address_.addr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(meterID)->mtrAddr,6);
        p_SetStaChipIDKey_F0F43->id_type_=0x03;
        p_SetStaChipIDKey_F0F43->id_format_=0x02;

        QByteArray chipIdKey;
        if (emScriptRunState ==Wait_SetChipIDAndKey_F0F43_Finish )
            chipIdKey = QByteArray::fromHex(normal_id_and_key_.toLatin1());
        else if (emScriptRunState == Wait_SetNormalChipIDAndKey_F0F43_Finish)
            chipIdKey = QByteArray::fromHex(abnormal_id_and_key_.toLatin1());

        for(int i=chipIdKey.size()-1; i>=0; i--)
        {
            p_SetStaChipIDKey_F0F43->id_content_.append(chipIdKey.at(i));
        }

        p_SetStaChipIDKey_F0F43->id_length_=ushort(p_SetStaChipIDKey_F0F43->id_content_.size());

        sendMsgOct=p_SetStaChipIDKey_F0F43->EncodeFrame();
        sendMsgLog=QString("》》设置STA芯片ID和密钥 F0F43：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_QueryStaChipIDKey_F0F44)
    {
        p_QueryStaChipIDKey_F0F44->ctrl_field_.dir=kDirDown;
        p_QueryStaChipIDKey_F0F44->ctrl_field_.prm=kActive;
        p_QueryStaChipIDKey_F0F44->ctrl_field_.comn_type=kHplc;

        p_QueryStaChipIDKey_F0F44->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_QueryStaChipIDKey_F0F44->info_field_.info_field_down.comu_rate=0;
        p_QueryStaChipIDKey_F0F44->info_field_.info_field_down.comu_module_ident=0;

        memcpy(p_QueryStaChipIDKey_F0F44->read_chip_id_unit_down_.mac_address_.addr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(meterID)->mtrAddr,6);
        p_QueryStaChipIDKey_F0F44->read_chip_id_unit_down_.id_type_=0x03;
        p_QueryStaChipIDKey_F0F44->read_chip_id_unit_down_.id_format_=0x02;
        p_QueryStaChipIDKey_F0F44->read_chip_id_unit_down_.id_length_=346;

        sendMsgOct=p_QueryStaChipIDKey_F0F44->EncodeFrame();
        sendMsgLog=QString("》》查询STA芯片ID和密钥 F0F44：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
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
        p_SetChipID_F0F39->config_chip_id_unit_.id_content_=QByteArray::fromHex(chip_id_.toLatin1());

        p_SetChipID_F0F39->config_chip_id_unit_.id_length_=uchar(p_SetChipID_F0F39->config_chip_id_unit_.id_content_.size());

        sendMsgOct=p_SetChipID_F0F39->EncodeFrame();
        sendMsgLog=QString("》》设置STA芯片ID F0F39：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_QueryStaChipID_F0F40)
    {
        p_QueryStaChipID_F0F40->ctrl_field_.dir=kDirDown;
        p_QueryStaChipID_F0F40->ctrl_field_.prm=kActive;
        p_QueryStaChipID_F0F40->ctrl_field_.comn_type=kHplc;

        p_QueryStaChipID_F0F40->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_QueryStaChipID_F0F40->info_field_.info_field_down.comu_rate=0;
        p_QueryStaChipID_F0F40->info_field_.info_field_down.comu_module_ident=0;

        memcpy(p_QueryStaChipID_F0F40->read_chip_id_unit_down_.mac_address_.addr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(meterID)->mtrAddr,6);
        p_QueryStaChipID_F0F40->read_chip_id_unit_down_.id_type_=0x01;
        p_QueryStaChipID_F0F40->read_chip_id_unit_down_.id_format_=0x02;
        p_QueryStaChipID_F0F40->read_chip_id_unit_down_.id_length_=0x18;

        sendMsgOct=p_QueryStaChipID_F0F40->EncodeFrame();
        sendMsgLog=QString("》》查询STA芯片ID F0F40：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
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
void Script_StaChipIdAndKey::sendSrcMsg(DvcType dvcType, int dvcId, QByteArray msg)
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

void Script_StaChipIdAndKey::timer_timeoutProc()
{
    if(emScriptRunState==Wait_QueryChipIDAndKeyInit_F0F44_Finish)
    {
        if(++tryTimes<3)
        {
            if (index ==0)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString(">>>>发送--初始查询单相STA的芯片ID和密钥。\r\n"));
                sendMsg(SingleSTA,p_CtrInfoList->at(0)->ctrlID,index,p_QueryStaChipIDKey_F0F44);
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            }
            else if (index ==1)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString(">>>>发送--初始查询三相STA的芯片ID和密钥。\r\n"));
                sendMsg(ThreeSTA,p_CtrInfoList->at(0)->ctrlID,index,p_QueryStaChipIDKey_F0F44);
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            }
        }
        else
        {
            tryTimes=0;
            p_AbstractScriptHost->updateProgress(ProcessState_Error, "Wait_QueryChipIDAndKeyInit_F0F44_Finish：timeout!!! "+test_name_);
        }
    }
    else if (emScriptRunState==Wait_SetChipIDAndKey_F0F43_Finish)
    {
        if(is_query_normal_id_and_key_ == false)
        {
            if(++tryTimes<3)
            {
                if (index ==0)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString(">>>>发送--设置单相STA的芯片ID和密钥(正常值)，等待--确认。\r\n"));
                    sendMsg(SingleSTA,p_CtrInfoList->at(0)->ctrlID,index,p_SetStaChipIDKey_F0F43);
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                }
                else if (index ==1)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--设置三相STA的芯片ID和密钥(正常值)，等待--确认。\r\n");
                    sendMsg(ThreeSTA,p_CtrInfoList->at(0)->ctrlID,index,p_SetStaChipIDKey_F0F43);
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                }
            }
            else
            {
                tryTimes=0;
                p_AbstractScriptHost->updateProgress(ProcessState_Error, "Wait_SetChipIDAndKey_F0F43_Finish：timeout!!! "+test_name_);
            }
        }
        else
        {
            if(++tryTimes<3)
            {
                if (index ==0)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--查询单相STA的芯片ID和密钥(正常值)。\r\n");
                    sendMsg(SingleSTA,p_CtrInfoList->at(0)->ctrlID,index,p_QueryStaChipIDKey_F0F44);
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                }
                else if (index ==1)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString(">>>>发送--查询三相STA的芯片ID和密钥(正常值)。\r\n"));
                    sendMsg(ThreeSTA,p_CtrInfoList->at(0)->ctrlID,index,p_QueryStaChipIDKey_F0F44);
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                }
            }
            else
            {
                tryTimes=0;
                p_AbstractScriptHost->updateProgress(ProcessState_Error, "Wait_SetNormalChipIDAndKey_F0F43_Finish：timeout!!! "+test_name_);
            }
        }
    }
    else if (emScriptRunState==Wait_SetNormalChipIDAndKey_F0F43_Finish)
    {
        if(is_query_abnormal_id_and_key_ == false)
        {
            if(++tryTimes<3)
            {
                if (index ==0)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString(">>>>发送--设置单相STA的芯片ID和密钥(异常值)。\r\n"));
                    sendMsg(SingleSTA,p_CtrInfoList->at(0)->ctrlID,index,p_SetStaChipIDKey_F0F43);
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                }
                else if (index ==1)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--设置三相STA的芯片ID和密钥(异常值)，等待--确认。\r\n");
                    sendMsg(ThreeSTA,p_CtrInfoList->at(0)->ctrlID,index,p_SetStaChipIDKey_F0F43);
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                }
            }
            else
            {
                tryTimes=0;
                p_AbstractScriptHost->updateProgress(ProcessState_Error, "Wait_SetNormalChipIDAndKey_F0F43_Finish：timeout!!! "+test_name_);
            }
        }
        else
        {
            if(++tryTimes<3)
            {
                if (index ==0)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--查询单相STA的芯片ID和密钥(异常值)。\r\n");
                    sendMsg(SingleSTA,p_CtrInfoList->at(0)->ctrlID,index,p_QueryStaChipIDKey_F0F44);
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                }
                else if (index ==1)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString(">>>>发送--查询三相STA的芯片ID和密钥(异常值)。\r\n"));
                    sendMsg(ThreeSTA,p_CtrInfoList->at(0)->ctrlID,index,p_QueryStaChipIDKey_F0F44);
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                }
            }
            else
            {
                tryTimes=0;
                p_AbstractScriptHost->updateProgress(ProcessState_Error, "Wait_SetNormalChipIDAndKey_F0F43_Finish：timeout!!! "+test_name_);
            }
        }
    }
    else if (emScriptRunState==Wait_SetChipID_F0F39_Finish)
    {
        if(is_query_chip_id_ == false)
        {
            if(++tryTimes<3)
            {
                if (index ==0)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString(">>>>发送--设置单相STA的芯片ID(F0F39)。\r\n"));
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
                p_AbstractScriptHost->updateProgress(ProcessState_Error, "Wait_SetChipID_F0F39_Finish：timeout!!! "+test_name_);
            }
        }
        else
        {
            if(++tryTimes<3)
            {
                if (index ==0)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--F0F40查询单相芯片ID，等待--确认\r\n");
                    sendMsg(SingleSTA,p_CtrInfoList->at(0)->ctrlID,index,p_QueryStaChipID_F0F40);
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                }
                else if (index ==1)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询三相STA芯片ID，等待--回复\r\n");
                    sendMsg(ThreeSTA,p_CtrInfoList->at(0)->ctrlID,index,p_QueryStaChipID_F0F40);
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                }
            }
            else
            {
                tryTimes=0;
                p_AbstractScriptHost->updateProgress(ProcessState_Error, "Wait_SetChipID_F0F39_Finish：timeout!!! "+test_name_);
            }
        }
    }
    else if (emScriptRunState==Wait_QueryChipIDAndKey_F0F44_Finish)
    {
        if(++tryTimes<3)
        {
            if (index ==0)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--查询单相STA的芯片ID和密钥，等待--回复\r\n");
                sendMsg(SingleSTA,p_CtrInfoList->at(0)->ctrlID,index,p_QueryStaChipIDKey_F0F44);
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            }
            else if (index ==1)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString(">>>>发送--查询三相STA的芯片ID和密钥(正常值)。\r\n"));
                sendMsg(ThreeSTA,p_CtrInfoList->at(0)->ctrlID,index,p_QueryStaChipIDKey_F0F44);
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            }
        }
        else
        {
            tryTimes=0;
            p_AbstractScriptHost->updateProgress(ProcessState_Error, "Wait_QueryChipIDAndKey_F0F44_Finish：timeout!!! "+test_name_);
        }
    }
}

void Script_StaChipIdAndKey::maxAllowTimer_timeoutProc()
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
void Script_StaChipIdAndKey::delayTimer_timeoutProc()
{
    p_delayTimer->stop();
    if(flagBuildNetOver == false)
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询网络规模（10F9），等待--回复");
        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryNetScale_10F9);
        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
    }
    else
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询STA芯片ID（10F40），等待--回复");
        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryStaChipID_10F40);
        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
    }
}

QByteArray Script_StaChipIdAndKey::getQueryFrame()
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
