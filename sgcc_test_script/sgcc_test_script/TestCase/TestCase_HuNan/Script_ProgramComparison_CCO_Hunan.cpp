#include "Script_ProgramComparison_CCO_Hunan.h"

Script_ProgramComparison_CCO_Hunan::Script_ProgramComparison_CCO_Hunan(QObject *parent) : QObject(parent)
{
    emScriptRunState=ProgramComparisonInit;
    resultFlag=false;
    sendMsgOct.clear();

    p_CtrInfoList=make_shared<QList<shared_ptr<CtrInfo>>>();
    p_BuildNetwork_GW=new BuildNetwork_GW();

    p_Frame3762Helper=make_shared<Frame3762Helper>();
    p_QueryCCOVersionInfo_03F130=make_shared<Afn03F130>();
    p_QueryCCOData_03F131=make_shared<Afn03F131>();

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
Script_ProgramComparison_CCO_Hunan::~Script_ProgramComparison_CCO_Hunan()
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
void Script_ProgramComparison_CCO_Hunan::execute()
{
    p_AbstractScriptHost->updateProgress(ProcessState_Start, "Test start!");

    emScriptRunState=ProgramComparisonInit;
    resultFlag=false;
    SoftwareInfo();//初始化03F130的数据单元
    if(needBuildNet==true)
    {
        p_BuildNetwork_GW->execute();
        emScriptRunState=Wait_BuildNetFinish_Whole;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "等待全网组网完成");
    }
    else
    {
        //meterInfoInit();
        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryCCOVersionInfo_03F130);
        emScriptRunState=Wait_QueryCCOVersionInfo_03F130;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询CCO版本信息（03F131），等待--回复");
        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
    }
    p_maxAllowTimer->start((timerForReachThresld+timerAfterReachThresld)*1000);
}
void Script_ProgramComparison_CCO_Hunan::stop()
{
    if(p_AbstractScriptHost==nullptr)
        return;
    p_AbstractScriptHost->updateProgress(ProcessState_Error, "Test stop!");
    if(needPowerOff==true)
        powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost);
}
void Script_ProgramComparison_CCO_Hunan::addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq)
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
void Script_ProgramComparison_CCO_Hunan::setHost(AbstractScriptHost *host)
{
    p_AbstractScriptHost=host;
    p_BuildNetwork_GW->setHost(host);
}
bool Script_ProgramComparison_CCO_Hunan::config(const QMap<QString,QString> *paraDic)
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
void Script_ProgramComparison_CCO_Hunan::processMsg(DvcType dvcType, int id, uchar *data, int datalen)
{
    if(p_CtrInfoList->size()==0)
        return;
    if(emScriptRunState==Wait_BuildNetFinish_Whole)
    {
        if(!p_BuildNetwork_GW->buildNetworkResultFlag)
            p_BuildNetwork_GW->processMsg(dvcType,id,data,datalen);
        else
        {
            if(p_BuildNetwork_GW->emScriptRunState==BuildNetFinish&&p_BuildNetwork_GW->buildNetworkResultFlag==true)
            {
                tryTimes=0;

                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryCCOVersionInfo_03F130);
                emScriptRunState=Wait_QueryCCOVersionInfo_03F130;
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询CCO版本信息（03F130），等待--回复");
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
void Script_ProgramComparison_CCO_Hunan::processCtrlDvcRes(DvcType dvcType, QList<int> idList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params)
{
    if(isSucs==false)
        return;
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到控制命令回复，参数个数=%1").arg(QString::number(params.size())));
    //    if(emScriptRunState!=Wait_BuildNetFinish_Whole)
    //        Refresh_CtrInfo_Result_for_CtrlCmdRes(p_CtrInfoList->at(0), dvcType, idList.at(0), ctrlCmdType);
    QList<int> sendParams;
    switch(emScriptRunState)
    {
        case ProgramComparisonInit:
        {
            break;
        }
        case Wait_BuildNetFinish_Whole:
        {
            p_BuildNetwork_GW->processCtrlDvcRes(dvcType,idList,ctrlCmdType,isSucs,params);
            break;
        }
        case Wait_QueryCCOVersionInfo_03F130:
        {
            break;
        }
        case Wait_QueryCCOData_03F131_Finish:
        {
            break;
        }
        case Wait_QueryCCOData_03F131_2_Finish:
        {
            break;
        }
        case Wait_QueryCCOData_03F131_3_Finish:
        {
            break;
        }
        case ScriptSuccess:
        {
            break;
        }
    }
}

void Script_ProgramComparison_CCO_Hunan::processMsgFromCCO(DvcType dvcType, int dvcId)
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
            case ProgramComparisonInit:
            {
                break;
            }
            case Wait_BuildNetFinish_Whole:
            {
                break;
            }
            case Wait_QueryCCOVersionInfo_03F130:
            {
                if(p_Frame3762Base->afn_ == 0x03&&p_Frame3762Base->dt1_==0x02&&p_Frame3762Base->dt2_==0x10&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();//接收到一条完整报文
                    shared_ptr<Afn03F130> p_QueryCCOVersionInfo_03F130_Up=dynamic_pointer_cast<Afn03F130>(p_Frame3762Base);
//                    tryTimes=0;
//                    index=0;

//                    if(memcmp(p_QueryCCOVersionInfo_03F130_Up->data_unit_up_.vendor_code_,softwareInfoData.vendor_code_,2)==0&&
//                       memcmp(p_QueryCCOVersionInfo_03F130_Up->data_unit_up_.version_date_,softwareInfoData.version_date_,3)==0&&
//                       memcmp(p_QueryCCOVersionInfo_03F130_Up->data_unit_up_.software_version_,softwareInfoData.software_version_,2)==0&&
//                       p_QueryCCOVersionInfo_03F130_Up->data_unit_up_.mcu_type_==softwareInfoData.mcu_type_)
//                    if(
//                            p_QueryCCOVersionInfo_03F130_Up->data_unit_up_.vendor_code_==softwareInfoData.vendor_code_
//                            &&p_QueryCCOVersionInfo_03F130_Up->data_unit_up_.version_date_==softwareInfoData.version_date_
//                            &&p_QueryCCOVersionInfo_03F130_Up->data_unit_up_.software_version_==softwareInfoData.software_version_
//                            &&p_QueryCCOVersionInfo_03F130_Up->data_unit_up_.mcu_type_==softwareInfoData.mcu_type_)
                    if(p_QueryCCOVersionInfo_03F130_Up->data_unit_up_==softwareInfoData)
                    {
//                        nodeInfoList.clear();
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,"03F130查询CCO版本信息完成");

                        /****载入CCO程序包*****/
                        LoadUpdateFile();
                        /****起始地址和单次对比长度****/
                        softwareInfoInit(0,SEG_LEN_512);

                        //开始比较程序段
                        times=ushort(programCompareList.size());
                        index=0;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询CCO程序比对（03F131），等待--回复");
                        emScriptRunState=Wait_QueryCCOData_03F131_Finish;
                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryCCOData_03F131);
                        p_timer->start(30*1000);
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询CCO程序比对（03F131），等待--回复");
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("03F130查询CCO版本信息"
                                                                                          "vendor_code_=%0;"
                                                                                          "version_date_=%1;"
                                                                                          "software_version_=%2;"
                                                                                          "mcu_type_=%3;"
                                                                                          "测试失败").
                                                             arg(QString(QByteArray(p_QueryCCOVersionInfo_03F130_Up->data_unit_up_.vendor_code_))).
                                                             arg(QString(QByteArray(p_QueryCCOVersionInfo_03F130_Up->data_unit_up_.version_date_))).
                                                             arg(QString(QByteArray(p_QueryCCOVersionInfo_03F130_Up->data_unit_up_.software_version_))).
                                                             arg(QString(p_QueryCCOVersionInfo_03F130_Up->data_unit_up_.mcu_type_)));
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("03F130设置CCO版本信息"
                                                                                          "vendor_code_=%0;"
                                                                                          "version_date_=%1;"
                                                                                          "software_version_=%2;"
                                                                                          "mcu_type_=%3;"
                                                                                          "测试失败").
                                                             arg(QString(QByteArray(softwareInfoData.vendor_code_))).
                                                             arg(QString(QByteArray(softwareInfoData.version_date_))).
                                                             arg(QString(QByteArray(softwareInfoData.software_version_))).
                                                             arg(QString(softwareInfoData.mcu_type_)));
                    }

                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
        case Wait_QueryCCOData_03F131_Finish:
        {
            if(p_Frame3762Base->afn_ == 0x03&&p_Frame3762Base->dt1_==0x04&&p_Frame3762Base->dt2_==0x10&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                tryTimes=0;
                shared_ptr<Afn03F131> p_QueryCCOData_03F131_Up=dynamic_pointer_cast<Afn03F131>(p_Frame3762Base);
                if(p_QueryCCOData_03F131_Up->compare_data_==programCompareList.at(index).compareData)//判断条件
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("第%1段程序比较一致").arg(++index));
                    //开始比对下一段
                    if(index<times)
                    {
                        emScriptRunState=Wait_QueryCCOData_03F131_Finish;
                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryCCOData_03F131);
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询CCO程序比对（03F131），等待--回复");
                        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                    {
                        /****起始地址和单次对比长度****/
                        softwareInfoInit(65536,SEG_LEN_512);

                        //开始比较程序段
                        times=ushort(programCompareList.size());
                        index=0;
                        emScriptRunState=Wait_QueryCCOData_03F131_2_Finish;
                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryCCOData_03F131);
                        p_timer->start(30*1000);
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询CCO程序比对（03F131）_2，等待--回复");

                    }
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("第%1段程序比较不一致").arg(index+1));
                }
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }

        case Wait_QueryCCOData_03F131_2_Finish:
        {
            if(p_Frame3762Base->afn_ == 0x03&&p_Frame3762Base->dt1_==0x04&&p_Frame3762Base->dt2_==0x10&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                tryTimes=0;
                shared_ptr<Afn03F131> p_QueryCCOData_03F131_Up=dynamic_pointer_cast<Afn03F131>(p_Frame3762Base);
                if(p_QueryCCOData_03F131_Up->compare_data_==programCompareList.at(index).compareData)//判断条件
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("第%1段程序比较一致").arg(++index));
                    //开始比对下一段
                    if(index<times)
                    {
                        emScriptRunState=Wait_QueryCCOData_03F131_2_Finish;
                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryCCOData_03F131);
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询CCO程序比对（03F131）_2，等待--回复");
                        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                    {
                        /****起始地址和单次对比长度****/
                        softwareInfoInit(65536,SEG_LEN_2048);

                        //开始比较程序段
                        times=ushort(programCompareList.size());
                        index=0;
                        emScriptRunState=Wait_QueryCCOData_03F131_3_Finish;
                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryCCOData_03F131);
                        p_timer->start(30*1000);
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询CCO程序比对（03F131）_3，等待--回复");
//                        emScriptRunState=ScriptSuccess;
//                        p_AbstractScriptHost->updateProgress(ProcessState_Success, QString("软件备案测试成功"));
                    }
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("第%1段程序比较不一致").arg(index+1));
                }
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case Wait_QueryCCOData_03F131_3_Finish:
        {
            if(p_Frame3762Base->afn_ == 0x03&&p_Frame3762Base->dt1_==0x04&&p_Frame3762Base->dt2_==0x10&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                tryTimes=0;
                shared_ptr<Afn03F131> p_QueryCCOData_03F131_Up=dynamic_pointer_cast<Afn03F131>(p_Frame3762Base);
                if(p_QueryCCOData_03F131_Up->compare_data_==programCompareList.at(index).compareData)//判断条件
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("第%1段程序比较一致").arg(++index));
                    //开始比对下一段
                    if(index<times)
                    {
                        emScriptRunState=Wait_QueryCCOData_03F131_3_Finish;
                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryCCOData_03F131);
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询CCO程序比对（03F131）_3，等待--回复");
                        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                    {
                        emScriptRunState=ScriptSuccess;
                        p_AbstractScriptHost->updateProgress(ProcessState_Success, QString("程序对比测试成功"));
                    }
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("第%1段程序比较不一致").arg(index+1));
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
void Script_ProgramComparison_CCO_Hunan::processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID)
{
    bool haveCompleteMsg=true;
    while(haveCompleteMsg)
    {
        qInfo()<<QString("645-解析前 buf645=%1").arg(QString(((*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[mtrlID]->buf645).toHex()));
        shared_ptr<Frame645Base> MsgBase_645_ptr = Frame645Helper::DecodeLocalMsg(&((*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[mtrlID]->buf645),haveCompleteMsg);
        if(MsgBase_645_ptr==nullptr)
        {
            break;
        }
        switch(emScriptRunState)
        {
            case ProgramComparisonInit:
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
//                    uchar di[4]={0x00};
//                    if(MsgBase_645_ptr->ctrlCode_==BROADCAST)
//                    {
//                        Address address;
//                        memcpy(address.addr,(*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[mtrlID]->mtrAddr,6);
//                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("645电表%1，收到0x08广播校时报文").arg(QString(QByteArray(address.addr,6).toHex())));
//                        //p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("当前广播校时成功率：%1%;").arg(calSuccessRate()*100));
////                        if(calSuccessRate()>=1.0)
////                        {
////                            emScriptRunState=ScriptSuccess;
////                            p_AbstractScriptHost->updateProgress(ProcessState_Success, QString("广播校时成功率：%1%;").arg(calSuccessRate()*100));

////                        }
//                    }
//                    else
//                    {
//                        QByteArray tmpSendMsg=prcsOther645Msg(MsgBase_645_ptr->ctrlCode_,MsgBase_645_ptr->dataLen_,di,MsgBase_645_ptr->addr_,(*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[mtrlID]->mtrAddr,p_CtrInfoList);
//                        sendSrcMsg(dvcType,dvcId,tmpSendMsg);
//                    }

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
void Script_ProgramComparison_CCO_Hunan::processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID)
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
            case ProgramComparisonInit:
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
void Script_ProgramComparison_CCO_Hunan::sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame)
{
    if(frame==p_QueryCCOData_03F131)
    {
        p_QueryCCOData_03F131->ctrl_field_.dir=kDirDown;
        p_QueryCCOData_03F131->ctrl_field_.prm=kActive;
        p_QueryCCOData_03F131->ctrl_field_.comn_type=kHplc;

        p_QueryCCOData_03F131->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_QueryCCOData_03F131->info_field_.info_field_down.comu_rate=0;
        p_QueryCCOData_03F131->info_field_.info_field_down.comu_module_ident=0;

        p_QueryCCOData_03F131->data_unit_down_.cpu_no_=softwareInfoData.cpu_no_;
        memcpy(p_QueryCCOData_03F131->data_unit_down_.compare_start_address_,programCompareList.at(index).startAddress,4);
        p_QueryCCOData_03F131->data_unit_down_.compare_data_len_=ushort(programCompareList.at(index).programSegment.size());

        sendMsgOct=p_QueryCCOData_03F131->EncodeFrame();
        sendMsgLog=QString("》》程序比对03F131：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_QueryCCOVersionInfo_03F130)
    {
        p_QueryCCOVersionInfo_03F130->ctrl_field_.dir=kDirDown;
        p_QueryCCOVersionInfo_03F130->ctrl_field_.prm=kActive;
        p_QueryCCOVersionInfo_03F130->ctrl_field_.comn_type=kHplc;

        p_QueryCCOVersionInfo_03F130->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_QueryCCOVersionInfo_03F130->info_field_.info_field_down.comu_rate=0;
        p_QueryCCOVersionInfo_03F130->info_field_.info_field_down.comu_module_ident=0;

        sendMsgOct=p_QueryCCOVersionInfo_03F130->EncodeFrame();
        sendMsgLog=QString("》》查询CCO版本信息：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
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
void Script_ProgramComparison_CCO_Hunan::sendSrcMsg(DvcType dvcType, int dvcId, QByteArray msg)
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

void Script_ProgramComparison_CCO_Hunan::timer_timeoutProc()
{
    switch(emScriptRunState)
    {
        case Wait_BuildNetFinish_Whole:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait build whole net finish timeout!!!");
            break;
        }

    case Wait_QueryCCOData_03F131_Finish:
    {
        if(++tryTimes<=3)
        {
            emScriptRunState=Wait_QueryCCOData_03F131_Finish;
            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryCCOData_03F131);
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询CCO程序比对（03F131），等待--回复");
            p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
        }
        else
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "达到最大尝试次数");
        }
        break;
    }
    case Wait_QueryCCOData_03F131_2_Finish:
    {
        if(++tryTimes<=3)
        {
            emScriptRunState=Wait_QueryCCOData_03F131_2_Finish;
            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryCCOData_03F131);
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询CCO程序比对（03F131），等待--回复");
            p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
        }
        else
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "达到最大尝试次数");
        }
        break;
    }
    case Wait_QueryCCOData_03F131_3_Finish:
    {
        if(++tryTimes<=3)
        {
            emScriptRunState=Wait_QueryCCOData_03F131_3_Finish;
            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryCCOData_03F131);
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询CCO程序比对（03F131），等待--回复");
            p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
        }
        else
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "达到最大尝试次数");
        }
        break;
    }
//        case Wait_Finish_Broadcast:
//        {
//            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "Wait_Finish_Broadcast timeout!!! ");
//            p_AbstractScriptHost->updateProgress(ProcessState_Processing, getFailMeterNo());
//            p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("广播校时成功率：%1%;").arg(calSuccessRate()*100));
//            break;
//        }
        default:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==p_timer");
            break;
        }
    }
}
void Script_ProgramComparison_CCO_Hunan::maxAllowTimer_timeoutProc()
{
    p_maxAllowTimer->stop();
    switch(emScriptRunState)
    {
//        case Wait_Finish_Broadcast:
//        {
//            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "Wait_Finish_Broadcast timeout!!!"+QString("广播校时成功率：%1%").arg(calSuccessRate()*100));
//            p_AbstractScriptHost->updateProgress(ProcessState_Processing,getFailMeterNo());
//            p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==p_maxAllowTimer");
//            break;
//        }
        default:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==p_maxAllowTimer");
            break;
        }
    }
}
void Script_ProgramComparison_CCO_Hunan::delayTimer_timeoutProc()
{
    p_delayTimer->stop();
//    index=0;
//    p_AbstractScriptHost->updateProgress(ProcessState_Processing, getFailMeterNo());


//    if(meterInfoList.size()==p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size())
//    {
//        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("湖南精确校时测试成功"));
//        emScriptRunState=ScriptSuccess;
//        p_AbstractScriptHost->updateProgress(ProcessState_Success, QString("湖南精确校时测试成功"));
//    }
//    else
//    {
//        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("应报%1，实际上报%2").arg(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size()).arg(meterInfoList.size()));
//        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("%1只电表未上报时钟偏差事件").arg(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size()-meterInfoList.size()));
//    }
}

//void Script_ProgramComparison_CCO_Hunan::meterInfoInit()
//{
//    meterInfoList.clear();
//    for(int i=0;i<p_CtrInfoList->size();i++)
//    {
//        QList<MeterInfoForSingleNet*> meterList=p_CtrInfoList->at(i)->p_MeterInfoForSingleNetList->values();
//        for(int j=0;j<meterList.size();j++)
//        {
//            if(meterList.at(j)->slotPosition==SingleSTA||meterList.at(j)->slotPosition==ThreeSTA)
//            {
//                MeterInfoBroadcast_Struct meterInfo_ST;
//                memcpy(meterInfo_ST.meterNo.addr,meterList.at(j)->mtrAddr,6);
//                meterInfo_ST.protocolType=meterList.at(j)->prtcl;
//                meterInfo_ST.readFlag=false;

//                meterInfoList.append(meterInfo_ST);
//            }
//        }
//    }
//}
//bool Script_ProgramComparison_CCO_Hunan::isMeterExist(Address address)
//{
//    for(int i=0;i<meterInfoList.size();i++)
//    {
//        if(address==meterInfoList.at(i).meterNo)
//        {
//            return true;
//        }
//    }
//    return false;
//}
//int Script_ProgramComparison_CCO_Hunan::getMeterInfo(Address address)
//{
//    for(int i=0;i<meterInfoList.size();i++)
//    {
//        if(address==meterInfoList.at(i).meterNo)
//        {
//            return i;
//        }
//    }
//    return -1;
//}
//double Script_ProgramComparison_CCO_Hunan::calSuccessRate()
//{
//    double successCount=0.0;
//    for(int i=0;i<meterInfoList.size();i++)
//    {
//        if(meterInfoList.at(i).readFlag==true)
//            successCount++;
//    }
//    return successCount/double(meterInfoList.size());
//}
//QString Script_ProgramComparison_CCO_Hunan::getFailMeterNo()
//{
//    QString failMeterNo;
//    for(int i=0;i<meterInfoList.size();i++)
//    {
//        if(meterInfoList.at(i).readFlag==false)
//            failMeterNo+=QString(QByteArray(meterInfoList.at(i).meterNo.addr,6).toHex())+";";
//    }
//    return failMeterNo;
//}

//void Script_ProgramComparison_CCO_Hunan::LoadUpdateFile()
//{
//    QStringList fileNameFilters;
//    QStringList fileList;
//    QString path;


//    if(1==dstUpgradeDvc)
//    {
//        path=tr("DataBase\\Upgrade\\路由程序(旧-新)");
//    }
//    else if(2==dstUpgradeDvc)
//    {
//        path=tr("DataBase\\Upgrade\\路由程序(新-新)");
//    }
//    else if(3==dstUpgradeDvc)
//    {
//        path=tr("DataBase\\Upgrade\\路由程序(文件类型错误)");
//    }
//    else if(4==dstUpgradeDvc)
//    {
//        path=tr("DataBase\\Upgrade\\路由程序(大于512K)");
//    }
//    else if(5==dstUpgradeDvc)
//    {
//        path=tr("DataBase\\Upgrade\\路由程序(重发100包和末包)");
//    }
//    else if(6==dstUpgradeDvc)
//    {
//        path=tr("DataBase\\Upgrade\\路由程序(100包长度256)");
//    }
//    else if(7==dstUpgradeDvc)
//    {
//        path=tr("DataBase\\Upgrade\\路由程序(先发101包后发100包)");
//    }
//    else
//    {
//        p_AbstractScriptHost->updateProgress(ProcessState_Error, "dstUpgradeDvc配置有误");
//        return;
//    }

//    QDir *updateFileDir=new QDir(path);
//    fileNameFilters << "*" ;
//    QList<QFileInfo> *fileInfo=new QList<QFileInfo>(updateFileDir->entryInfoList(fileNameFilters,QDir::Files,QDir::NoSort));

//    qDebug()<<QString("获取到的文件数：%1").arg(fileInfo->count());

//    QString str_file_name;
//    for (int i=0;i<fileInfo->count();i++)
//    {
//        str_file_name += fileInfo->at(i).fileName() + "---";
//    }
//    qDebug()<<str_file_name;

//    if(fileInfo->count() != 1)
//    {
//        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "没有CCO升级文件或CCO升级文件多于1个!!!");
//        return;
//    }

//    QString filePath=path+"\\"+fileInfo->at(0).fileName();

//    delete fileInfo;
//    delete updateFileDir;


//    char upgradeFileBuf[1024*1024];
//    QFile file;
//    file.setFileName(filePath);
//    if(!file.open(QIODevice::ReadOnly))
//    {
//        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "打开CCO升级文件失败!!!");
//        return;
//    }


//    QDataStream in(&file);
//    dataLen=in.readRawData(upgradeFileBuf,1024*1024);
//    file.close();

//    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("升级包大小=(%1);").arg(dataLen));
//    rawUpdateFile=QByteArray::fromRawData(upgradeFileBuf,dataLen);

//    totalSegs_512 = ushort((dataLen%SEG_LEN_512==0)?dataLen/SEG_LEN_512:(dataLen/SEG_LEN_512+1));
//    totalSegs_2048 = ushort((dataLen%SEG_LEN_2048==0)?dataLen/SEG_LEN_2048:(dataLen/SEG_LEN_2048+1));

////    p_FileTransfer_15F1_Down->file_transfer_unit_.total_num_=totalSegs_512;
////    for(int i=0; i<p_FileTransfer_15F1_Down->file_transfer_unit_.total_num_; i++)
////    {
////        transResList.append(false);
////    }
//}

void Script_ProgramComparison_CCO_Hunan::LoadUpdateFile()
{
    //加载程序文件
    QStringList fileNameFilters;
    QStringList fileList;
    QString path=tr("DataBase\\Upgrade\\湖南HPLC路由");
//    QString path=tr("DataBase\\Upgrade\\国网HPLC路由(旧-新)");
//    QString path=tr("D:\\Topscomm\\Topscomm\\TopscommScript\\GW_Script\\SG_TestSystem\\DataBase\\Upgrade\\国网HPLC路由(旧-新)");

    QDir *updateFileDir=new QDir(path);
    fileNameFilters << "*" ;
    QList<QFileInfo> *fileInfo=new QList<QFileInfo>(updateFileDir->entryInfoList(fileNameFilters,QDir::Files,QDir::NoSort));

    if(fileInfo->count() != 1)
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "没有CCO升级文件或CCO升级文件多于1个!!!");
    }
    QString filePath=path+"\\"+fileInfo->at(0).fileName();

    delete fileInfo;
    delete updateFileDir;

    QFile file(filePath);
    if(!file.open(QIODevice::ReadOnly))
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "打开CCO升级文件失败!!!");
        return;
    }
    //
    originProgram=file.readAll();
}
void Script_ProgramComparison_CCO_Hunan::SoftwareInfo()
{
    //获取软件信息,根据程序名
    softwareInfoData.cpu_no_=0x00;
    softwareInfoData.vendor_code_[1]='T';
    softwareInfoData.vendor_code_[0]='C';
    char date[3]={0x18,0x0C,0x15};//241221
    memcpy(softwareInfoData.version_date_,date,3);
    softwareInfoData.hardware_version_[0]=0x01;
    softwareInfoData.hardware_version_[1]=0x02;
    softwareInfoData.software_version_[0]=0x02;
    softwareInfoData.software_version_[1]=0x43;
    softwareInfoData.mcu_type_len_=2;
    softwareInfoData.mcu_type_.append('5');
    softwareInfoData.mcu_type_.append('R');
}
void Script_ProgramComparison_CCO_Hunan::softwareInfoInit(int currentIndex,int segmentLen)
{
//    //加载程序文件
//    QStringList fileNameFilters;
//    QStringList fileList;
//    QString path=tr("DataBase\\Upgrade\\湖南HPLC路由");

//    QDir *updateFileDir=new QDir(path);
//    fileNameFilters << "*" ;
//    QList<QFileInfo> *fileInfo=new QList<QFileInfo>(updateFileDir->entryInfoList(fileNameFilters,QDir::Files,QDir::NoSort));

//    if(fileInfo->count() != 1)
//    {
//        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "没有CCO升级文件或CCO升级文件多于1个!!!");
//    }
//    QString filePath=path+"\\"+fileInfo->at(0).fileName();

//    delete fileInfo;
//    delete updateFileDir;

//    QFile file(filePath);
//    if(!file.open(QIODevice::ReadOnly))
//    {
//        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "打开CCO升级文件失败!!!");
//        return;
//    }
//    //
//    originProgram=file.readAll();

//    //获取软件信息,根据程序名
//    softwareInfoData.cpu_no_=0x00;
//    softwareInfoData.vendor_code_[1]='T';
//    softwareInfoData.vendor_code_[0]='C';
//    char date[3]={0x1D,0x07,0x14};//290720
//    memcpy(softwareInfoData.version_date_,date,3);
//    softwareInfoData.hardware_version_[0]=0x01;
//    softwareInfoData.hardware_version_[1]=0x02;
//    softwareInfoData.software_version_[0]=0x01;
//    softwareInfoData.software_version_[1]=0x43;
//    softwareInfoData.mcu_type_len_=2;
//    softwareInfoData.mcu_type_.append('5');
//    softwareInfoData.mcu_type_.append('R');
    //获取compare信息

//    int currentIndex=0;
    programCompareList.clear();

    while (currentIndex<originProgram.size())
    {
        ProgramCompareStruct programCompareST;
        programCompareST.startAddress[0]=char(currentIndex);
        programCompareST.startAddress[1]=char(currentIndex>>8);
        programCompareST.startAddress[2]=char(currentIndex>>16);
        programCompareST.startAddress[3]=char(currentIndex>>24);
        if(currentIndex+segmentLen<=originProgram.size())
            programCompareST.programSegment=originProgram.mid(currentIndex,segmentLen);
        else
            programCompareST.programSegment=originProgram.mid(currentIndex);
        programCompareST.compareData=QCryptographicHash::hash(programCompareST.programSegment, QCryptographicHash::Sha1);
        programCompareList.append(programCompareST);
        currentIndex+=segmentLen;
    }
}
