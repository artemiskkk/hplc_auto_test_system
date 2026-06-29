#include "Script_CurveRead_F1F100_Hunan.h"

Script_CurveRead_F1F100_Hunan::Script_CurveRead_F1F100_Hunan(QObject *parent) : QObject(parent)
{
    emScriptRunState=ReadMeterInit;
    resultFlag=false;
    sendMsgOct.clear();

    p_CtrInfoList=make_shared<QList<shared_ptr<CtrInfo>>>();
    p_BuildNetwork_GW=new BuildNetwork_GW();

    p_Frame3762Helper=make_shared<Frame3762Helper>();
    p_RouterPause_12F2=make_shared<Afn12F2>();
    p_RouterRecover_12F3=make_shared<Afn12F3>();
    p_CurveReadMeter_F1F100=make_shared<AfnF1F100>();

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
Script_CurveRead_F1F100_Hunan::~Script_CurveRead_F1F100_Hunan()
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
void Script_CurveRead_F1F100_Hunan::execute()
{
    p_AbstractScriptHost->updateProgress(ProcessState_Start, "Test start!");

    emScriptRunState=ReadMeterInit;
    resultFlag=false;
    addrList.clear();
    readInfoInit();
    readFrameListInit();
    if(needBuildNet==true)
    {
        p_BuildNetwork_GW->execute();
        emScriptRunState=Wait_BuildNetFinish_Whole;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "等待全网组网完成");
        p_timer->start((timerForReachThresld+timerAfterReachThresld)*1000);
    }
    else
    {
        //readInfoInit();
        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_RouterPause_12F2);
        emScriptRunState=Wait_00F1_for_12F2_Pause;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--路由暂停（12F2），等待--确认");
        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
    }
    p_maxAllowTimer->start((timerForReachThresld+timerAfterReachThresld)*1000);

}
void Script_CurveRead_F1F100_Hunan::stop()
{
    if(p_AbstractScriptHost==nullptr)
        return;
    p_AbstractScriptHost->updateProgress(ProcessState_Error, "Test stop!");
    if(needPowerOff==true)
        powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost);
}
void Script_CurveRead_F1F100_Hunan::addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq)
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
void Script_CurveRead_F1F100_Hunan::setHost(AbstractScriptHost *host)
{
    p_AbstractScriptHost=host;
    p_BuildNetwork_GW->setHost(host);
}
bool Script_CurveRead_F1F100_Hunan::config(const QMap<QString,QString> *paraDic)
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
        if(paraDic->keys().contains("maxFrameNum"))
        {
            this->maxFrameNum = (*paraDic)["maxFrameNum"].toUShort();
        }
        if(paraDic->keys().contains("maxParallelNum"))
        {
            this->maxParallelNum = (*paraDic)["maxParallelNum"].toUShort();
        }
        if(paraDic->keys().contains("readPeriod"))
        {
            this->readPeriod = (*paraDic)["readPeriod"].toUShort();
        }
//        if(paraDic->keys().contains("maxReadRound"))
//        {
//            this->maxReadRound = (*paraDic)["maxReadRound"].toUShort();
//        }
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
void Script_CurveRead_F1F100_Hunan::processMsg(DvcType dvcType, int id, uchar *data, int datalen)
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
                //readInfoInit();
                tryTimes=0;
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,id,p_RouterPause_12F2);
                emScriptRunState=Wait_00F1_for_12F2_Pause;
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--路由暂停（12F2），等待--确认");
                p_timer->start(10*1000);
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
void Script_CurveRead_F1F100_Hunan::processCtrlDvcRes(DvcType dvcType, QList<int> idList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params)
{
    if(isSucs==false)
        return;
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到控制命令回复，参数个数=%1").arg(QString::number(params.size())));
    //    if(emScriptRunState!=Wait_BuildNetFinish_Whole)
    //        Refresh_CtrInfo_Result_for_CtrlCmdRes(p_CtrInfoList->at(0), dvcType, idList.at(0), ctrlCmdType);
    QList<int> sendParams;
    switch(emScriptRunState)
    {
        case ReadMeterInit:
        {
            break;
        }
        case Wait_BuildNetFinish_Whole:
        {
            p_BuildNetwork_GW->processCtrlDvcRes(dvcType,idList,ctrlCmdType,isSucs,params);
            break;
        }
        case Wait_00F1_for_12F2_Pause:
        {
            break;
        }
        case Wait_Finish_Curve:
        {
            break;
        }
        case Wait_00F1_for_12F3_Recover:
        {
            break;
        }
        case ScriptSuccess:
        {
            break;
        }
    }
}

void Script_CurveRead_F1F100_Hunan::processMsgFromCCO(DvcType dvcType, int dvcId)
{
    if(dvcId!=p_CtrInfoList->at(0)->ctrlID)
        return;
    bool haveCompleteMsg=true;
    while(haveCompleteMsg)
    {
        qInfo()<<QString("CCO-解析前 buf3762=%1").arg(QString((p_CtrInfoList->at(0)->buf.toHex())));
        shared_ptr<Frame3762Base> p_Frame3762Base = p_Frame3762Helper->DecodeLocalMsg(&(p_CtrInfoList->at(0)->buf),haveCompleteMsg,Hunan);
        qInfo()<<QString("CCO-解析后 buf3762=%1").arg(QString((p_CtrInfoList->at(0)->buf.toHex())));
        if(p_Frame3762Base==nullptr)
        {
            continue;
        }
        switch(emScriptRunState)
        {
            case ReadMeterInit:
            {
                break;
            }
            case Wait_BuildNetFinish_Whole:
            {
                break;
            }
            case Wait_00F1_for_12F2_Pause:
            {
                if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();//接收到一条完整报文
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("路由暂停完成，等待%1min后开始曲线抄读！").arg(readPeriod*totalReadNum));
                    p_delayTimer->start(60*1000);//1分钟

                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_Finish_Curve:
            {
                if(p_Frame3762Base->afn_==char(0xF1)&&p_Frame3762Base->dt1_==0x08&&p_Frame3762Base->dt2_==0x0C&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    shared_ptr<AfnF1F100> p_CurveReadMeter_F1F100_Up=dynamic_pointer_cast<AfnF1F100>(p_Frame3762Base);
                    Address srcAddr;
                    memcpy(srcAddr.addr,p_CurveReadMeter_F1F100_Up->address_field_.src_addr,6);
                    if(emReadMode==MeterNo)
                    {
                        int currentMeterIndex=getMeterNoIndex(srcAddr);
                        if(currentMeterIndex!=-1)
                        {
                            if(p_CurveReadMeter_F1F100_Up->unit_up_.frame_length_==0)
                                continue;
                            QByteArray msgBuf=p_CurveReadMeter_F1F100_Up->unit_up_.frame_content_;
                            if(p_CurveReadMeter_F1F100_Up->unit_up_.protocol_type_==DLT645_2007)
                            {
                                while(msgBuf.size()>0)
                                {
                                    bool res=true;
                                    shared_ptr<Frame645Base> MsgBase_645_ptr = Frame645Helper::DecodeLocalMsg(&msgBuf,res);
                                    if(MsgBase_645_ptr==nullptr)
                                        continue;
                                    if(MsgBase_645_ptr->ctrlCode_==NORMAL_RESP)
                                    {
                                        shared_ptr<RspsNormal_ReadData_0x91> p_ReadData_0x91=dynamic_pointer_cast<RspsNormal_ReadData_0x91>(MsgBase_645_ptr);
                                        if(memcmp(p_ReadData_0x91->addr_,srcAddr.addr,6)!=0)
                                            continue;
                                        for(int i=0;i<readInfoList.at(currentMeterIndex).dataUnitList.size();i++)
                                        {
                                            if(memcmp(readInfoList.at(currentMeterIndex).dataUnitList.at(i).dataIdLen.data_id_,p_ReadData_0x91->di,4)==0)
                                            {
                                                QByteArray dateTime=p_ReadData_0x91->data.mid(0,5);
                                                uchar readNum=uchar(p_ReadData_0x91->data.at(6));
                                                QList<QByteArray> dataList;
                                                for(int j=7;p_ReadData_0x91->data.size();j+=readInfoList.at(currentMeterIndex).dataUnitList.at(i).dataIdLen.reply_data_len_)
                                                {
                                                    QByteArray data=p_ReadData_0x91->data.mid(j,readInfoList.at(currentMeterIndex).dataUnitList.at(i).dataIdLen.reply_data_len_);
                                                    dataList.append(data);
                                                }
                                                for(int j=0;j<readInfoList.at(currentMeterIndex).dataUnitList.at(i).roundList.size();j++)
                                                {
                                                    if(memcmp(readInfoList.at(currentMeterIndex).dataUnitList.at(i).roundList.at(j).dateTime,dateTime,5)==0)
                                                    {
                                                        for(int n=0;n<readNum;n++)
                                                        {
                                                            readInfoList[currentMeterIndex].dataUnitList[i].roundList[j+n].replyData=dataList.at(n);
                                                            if(isNotReplyEmpty(dataList.at(n))==true)
                                                                readInfoList[currentMeterIndex].dataUnitList[i].roundList[j+n].readFlag=true;
                                                        }
                                                        break;
                                                    }
                                                }
                                                break;
                                            }
                                            else
                                                continue;
                                        }
                                    }
                                    else if(MsgBase_645_ptr->ctrlCode_==AbNORMAL_RESP_ReadData)
                                    {
                                        shared_ptr<RspsAbNormal_ReadData_0xD1> p_RspsAbNormal_0xD1=dynamic_pointer_cast<RspsAbNormal_ReadData_0xD1>(MsgBase_645_ptr);
                                        if(memcmp(p_RspsAbNormal_0xD1->addr_,srcAddr.addr,6)!=0)
                                            continue;
                                    }
                                    else
                                        continue;

                                }
                            }
                            else if(p_CurveReadMeter_F1F100_Up->unit_up_.protocol_type_==OOP)
                            {
    //                            bool res=true;
    //                            shared_ptr<FrameOOPBase> MsgBase_OOP_ptr = FrameOOPHelper::DecodeMsg(&msgBuf,res);
    //                            if(MsgBase_OOP_ptr==nullptr)
    //                                continue;
    //                            if(0!=memcmp(QByteArray::fromHex(MsgBase_OOP_ptr->address_field_.sa.address.toLatin1()),readInfoList.at(currentMeterIndex).meterNo.addr,6))
    //                                continue;
    //                            if(MsgBase_OOP_ptr->service_type_!=GET_RESPONSE_SERVER && MsgBase_OOP_ptr->service_sub_type_!=uchar(GetResponseType::kGetResponseNormal))
    //                                continue;
    //                            shared_ptr<GetResponseNormal> p_GetResponseNormal=dynamic_pointer_cast<GetResponseNormal>(MsgBase_OOP_ptr);
    //                            //待修改
    //                            OAD oad;
    //                            oad.OI=PosActEne_OI;
    //                            oad.attribute.feature=0;
    //                            oad.attribute.seq=2;
    //                            oad.element_index=0;
    //                            for(int i=0;i<readInfoList.at(currentMeterIndex).dataUnitList.size();i++)
    //                            {
    //                                if(p_GetResponseNormal->a_result_normal_.oad==oad)
    //                                {
    //                                    readInfoList[currentMeterIndex].dataUnitList[i].successRound++;
    //                                    break;
    //                                }
    //                                else
    //                                    continue;
    //                            }
                            }
                            parallelCount--;
                            while(parallelCount<maxParallelNum)
                            {
                                if(meterIndex>=meterNoModeFrameList.size())
                                    break;
                                if(index>=meterNoModeFrameList.at(meterIndex).frameList.size())
                                {
                                    meterIndex++;
                                    index=0;
                                }
                                sendMsg(CCO_GW,dvcId,meterIndex,p_CurveReadMeter_F1F100);
                                parallelCount++;
                                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--曲线抄读（F1F100），等待--抄表完成");
                            }
                            if(parallelCount==0)//说明没有并发的
                            {
                               //统计按表号抄表的抽读情况
                                QString display=QString("最初抄读数据如下\n");
                                for(int i=0;i<readInfoList.size();i++)
                                {
                                    display.append(QString("表号：%1 ").arg(QString(QByteArray(readInfoList.at(i).meterNo.addr,6).toHex())));
                                    for(int j=0;j<readInfoList.at(i).dataUnitList.size();j++)
                                    {
                                        if(readInfoList.at(i).protocolType==DLT645_2007)
                                            display.append(QString("数据项%0：%1 %2 ").arg(j+1).arg(QString(QByteArray(readInfoList.at(i).dataUnitList.at(j).dataIdLen.data_id_,4).toHex())).arg(QString(readInfoList.at(i).dataUnitList.at(j).roundList.at(0).replyData.toHex())));
                                        else
                                            display.append(QString("OAD%0：%1 %2 ").arg(j+1).arg(QString(QByteArray(readInfoList.at(i).dataUnitList.at(j).dataIdLen.data_id_,4).toHex())).arg(QString(readInfoList.at(i).dataUnitList.at(j).roundList.at(0).replyData.toHex())));
                                    }
                                    display.append("\n");
                                }
                                p_AbstractScriptHost->updateProgress(ProcessState_Processing, display);
                               //展示结果
                                //开始按轮次抄读
                                readInfoClear();
                                emReadMode=RoundIndex;
                                index=0;
                                roundIndex=0;//96*表数
                                while(parallelCount<maxParallelNum)
                                {
                                    if(roundIndex>=roundIndexModeFrameList.size())
                                        break;
                                    if(index>=roundIndexModeFrameList.at(roundIndex).frameList.size())
                                    {
                                        roundIndex++;
                                        index=0;
                                    }
                                    sendMsg(CCO_GW,dvcId,roundIndex,p_CurveReadMeter_F1F100);
                                    parallelCount++;
                                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--曲线抄读（F1F100），等待--抄表完成");
                               }
                            }
                        }

                    }
                    else if(emReadMode==RoundIndex)
                    {
                        int currentMeterIndex=getMeterNoIndex(srcAddr);
                        if(currentMeterIndex!=-1)
                        {
                            if(p_CurveReadMeter_F1F100_Up->unit_up_.frame_length_==0)
                                continue;
                            QByteArray msgBuf=p_CurveReadMeter_F1F100_Up->unit_up_.frame_content_;
                            if(p_CurveReadMeter_F1F100_Up->unit_up_.protocol_type_==DLT645_2007)
                            {
                                while(msgBuf.size()>0)
                                {
                                    bool res=true;
                                    shared_ptr<Frame645Base> MsgBase_645_ptr = Frame645Helper::DecodeLocalMsg(&msgBuf,res);
                                    if(MsgBase_645_ptr==nullptr)
                                        continue;
                                    if(MsgBase_645_ptr->ctrlCode_==NORMAL_RESP)
                                    {
                                        shared_ptr<RspsNormal_ReadData_0x91> p_ReadData_0x91=dynamic_pointer_cast<RspsNormal_ReadData_0x91>(MsgBase_645_ptr);
                                        if(memcmp(p_ReadData_0x91->addr_,srcAddr.addr,6)!=0)
                                            continue;
                                        for(int i=0;i<readInfoList.at(currentMeterIndex).dataUnitList.size();i++)
                                        {
                                            if(memcmp(readInfoList.at(currentMeterIndex).dataUnitList.at(i).dataIdLen.data_id_,p_ReadData_0x91->di,4)==0)
                                            {
                                                QByteArray dateTime=p_ReadData_0x91->data.mid(0,5);
                                                uchar readNum=uchar(p_ReadData_0x91->data.at(6));
                                                QList<QByteArray> dataList;
                                                for(int j=7;p_ReadData_0x91->data.size();j+=readInfoList.at(currentMeterIndex).dataUnitList.at(i).dataIdLen.reply_data_len_)
                                                {
                                                    QByteArray data=p_ReadData_0x91->data.mid(j,readInfoList.at(currentMeterIndex).dataUnitList.at(i).dataIdLen.reply_data_len_);
                                                    dataList.append(data);
                                                }
                                                for(int j=0;j<readInfoList.at(currentMeterIndex).dataUnitList.at(i).roundList.size();j++)
                                                {
                                                    if(memcmp(readInfoList.at(currentMeterIndex).dataUnitList.at(i).roundList.at(j).dateTime,dateTime,5)==0)
                                                    {
                                                        for(int n=0;n<readNum;n++)
                                                        {
                                                            readInfoList[currentMeterIndex].dataUnitList[i].roundList[j+n].replyData=dataList.at(n);
                                                            if(isNotReplyEmpty(dataList.at(n))==true)
                                                                readInfoList[currentMeterIndex].dataUnitList[i].roundList[j+n].readFlag=true;
                                                        }
                                                        break;
                                                    }
                                                }
                                                break;
                                            }
                                            else
                                                continue;
                                        }
                                    }
                                    else if(MsgBase_645_ptr->ctrlCode_==AbNORMAL_RESP_ReadData)
                                    {
                                        shared_ptr<RspsAbNormal_ReadData_0xD1> p_RspsAbNormal_0xD1=dynamic_pointer_cast<RspsAbNormal_ReadData_0xD1>(MsgBase_645_ptr);
                                        if(memcmp(p_RspsAbNormal_0xD1->addr_,srcAddr.addr,6)!=0)
                                            continue;
                                    }
                                    else
                                        continue;
                                }
                            }
                            else if(p_CurveReadMeter_F1F100_Up->unit_up_.protocol_type_==OOP)
                            {
    //                            bool res=true;
    //                            shared_ptr<FrameOOPBase> MsgBase_OOP_ptr = FrameOOPHelper::DecodeMsg(&msgBuf,res);
    //                            if(MsgBase_OOP_ptr==nullptr)
    //                                continue;
    //                            if(0!=memcmp(QByteArray::fromHex(MsgBase_OOP_ptr->address_field_.sa.address.toLatin1()),readInfoList.at(currentMeterIndex).meterNo.addr,6))
    //                                continue;
    //                            if(MsgBase_OOP_ptr->service_type_!=GET_RESPONSE_SERVER && MsgBase_OOP_ptr->service_sub_type_!=uchar(GetResponseType::kGetResponseNormal))
    //                                continue;
    //                            shared_ptr<GetResponseNormal> p_GetResponseNormal=dynamic_pointer_cast<GetResponseNormal>(MsgBase_OOP_ptr);
    //                            //待修改
    //                            OAD oad;
    //                            oad.OI=PosActEne_OI;
    //                            oad.attribute.feature=0;
    //                            oad.attribute.seq=2;
    //                            oad.element_index=0;
    //                            for(int i=0;i<readInfoList.at(currentMeterIndex).dataUnitList.size();i++)
    //                            {
    //                                if(p_GetResponseNormal->a_result_normal_.oad==oad)
    //                                {
    //                                    readInfoList[currentMeterIndex].dataUnitList[i].successRound++;
    //                                    break;
    //                                }
    //                                else
    //                                    continue;
    //                            }
                            }
                            parallelCount--;
                            while(parallelCount<maxParallelNum)
                            {
                                if(meterIndex>=meterNoModeFrameList.size())
                                    break;
                                if(index>=meterNoModeFrameList.at(meterIndex).frameList.size())
                                {
                                    meterIndex++;
                                    index=0;
                                }
                                sendMsg(CCO_GW,dvcId,meterIndex,p_CurveReadMeter_F1F100);
                                parallelCount++;
                                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--曲线抄读（F1F100），等待--抄表完成");
                            }
                            if(parallelCount==0)//说明没有并发的
                            {
                                //统计按表号抄表的抽读情况
                                QString display=QString("最初抄读数据如下\n");
                                for(int i=0;i<readInfoList.size();i++)
                                {
                                    display.append(QString("表号：%1 ").arg(QString(QByteArray(readInfoList.at(i).meterNo.addr,6).toHex())));
                                    for(int j=0;j<readInfoList.at(i).dataUnitList.size();j++)
                                    {
                                        if(readInfoList.at(i).protocolType==DLT645_2007)
                                            display.append(QString("数据项%0：%1 %2 ").arg(j+1).arg(QString(QByteArray(readInfoList.at(i).dataUnitList.at(j).dataIdLen.data_id_,4).toHex())).arg(QString(readInfoList.at(i).dataUnitList.at(j).roundList.at(0).replyData.toHex())));
                                        else
                                            display.append(QString("OAD%0：%1 %2 ").arg(j+1).arg(QString(QByteArray(readInfoList.at(i).dataUnitList.at(j).dataIdLen.data_id_,4).toHex())).arg(QString(readInfoList.at(i).dataUnitList.at(j).roundList.at(0).replyData.toHex())));
                                    }
                                    display.append("\n");
                                }
                                p_AbstractScriptHost->updateProgress(ProcessState_Processing, display);
                                emScriptRunState=ScriptSuccess;
                                p_AbstractScriptHost->updateProgress(ProcessState_Success, "曲线抄读（F1F100）测试成功");
                            }
                        }

                    }

                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_00F1_for_12F3_Recover:
            {
                if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    index=0;
                    emScriptRunState=ScriptSuccess;
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
void Script_CurveRead_F1F100_Hunan::processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID)
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
            case ReadMeterInit:
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
void Script_CurveRead_F1F100_Hunan::processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID)
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
            case ReadMeterInit:
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
void Script_CurveRead_F1F100_Hunan::sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame)
{
    if(frame==p_RouterPause_12F2)
    {
        p_RouterPause_12F2->ctrl_field_.dir=kDirDown;
        p_RouterPause_12F2->ctrl_field_.prm=kActive;
        p_RouterPause_12F2->ctrl_field_.comn_type=kHplc;

        p_RouterPause_12F2->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_RouterPause_12F2->info_field_.info_field_down.comu_rate=0;
        p_RouterPause_12F2->info_field_.info_field_down.comu_module_ident=0;

        sendMsgOct=p_RouterPause_12F2->EncodeFrame();
        sendMsgLog=QString("》》路由暂停12F2：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_RouterRecover_12F3)
    {
        p_RouterRecover_12F3->ctrl_field_.dir=kDirDown;
        p_RouterRecover_12F3->ctrl_field_.prm=kActive;
        p_RouterRecover_12F3->ctrl_field_.comn_type=kHplc;

        p_RouterRecover_12F3->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_RouterRecover_12F3->info_field_.info_field_down.comu_rate=0;
        p_RouterRecover_12F3->info_field_.info_field_down.comu_module_ident=0;

        sendMsgOct=p_RouterRecover_12F3->EncodeFrame();
        sendMsgLog=QString("》》路由回复12F3：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_CurveReadMeter_F1F100)
    {
        p_CurveReadMeter_F1F100->ctrl_field_.dir=kDirDown;
        p_CurveReadMeter_F1F100->ctrl_field_.prm=kActive;
        p_CurveReadMeter_F1F100->ctrl_field_.comn_type=kHplc;

        p_CurveReadMeter_F1F100->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_CurveReadMeter_F1F100->info_field_.info_field_down.comu_rate=0;
        p_CurveReadMeter_F1F100->info_field_.info_field_down.comu_module_ident=1;

        uchar tmpAddr[6];
        memcpy(tmpAddr,readInfoList.at(meterID).meterNo.addr,6);
        uchar tmpCcoAddr[6];
        memcpy(tmpCcoAddr,p_CtrInfoList->at(0)->ccoAddr,6);
        memcpy(p_CurveReadMeter_F1F100->address_field_.src_addr,tmpCcoAddr,6);
        memcpy(p_CurveReadMeter_F1F100->address_field_.dst_addr,tmpAddr,6);

        if(emReadMode==MeterNo)
        {
            if(meterNoModeFrameList.at(meterID).protocolType==DLT645_2007)
            {
                QByteArray msg645;
                //2
                int frameCount=0;//单帧报文数
                for(int i=index;i<meterNoModeFrameList.at(meterID).frameList.size();i++)
                {
                    if(frameCount<maxFrameNum)
                    {
                        msg645.append(meterNoModeFrameList.at(meterID).frameList.at(i));
                        frameCount++;
                    }
                    else
                        break;
                }
                index+=frameCount;

                p_CurveReadMeter_F1F100->unit_down_.protocol_type_=0x02;
                p_CurveReadMeter_F1F100->unit_down_.preserve_=0x00;
                p_CurveReadMeter_F1F100->unit_down_.frame_content_=msg645;
                p_CurveReadMeter_F1F100->unit_down_.frame_length_=uchar(msg645.size());

                sendMsgOct=p_CurveReadMeter_F1F100->EncodeFrame();
                sendMsgLog=QString("》》曲线抄读F1F100,抄读645电表%0：%1\n").arg(QString((QByteArray(reinterpret_cast<char*>(tmpAddr),6).toHex()))).arg(QString(sendMsgOct.toHex()));
            }
            else if(readInfoList.at(meterID).protocolType==OOP)
            {
                shared_ptr<GetRequestNormal> p_GetRequestNormal_ReadData=make_shared<GetRequestNormal>();
                p_GetRequestNormal_ReadData->ctrl_field_.dir = 0;
                p_GetRequestNormal_ReadData->ctrl_field_.prm = 1;
                p_GetRequestNormal_ReadData->ctrl_field_.fra = 0;
                p_GetRequestNormal_ReadData->ctrl_field_.res = 0;
                p_GetRequestNormal_ReadData->ctrl_field_.sc = 0;
                p_GetRequestNormal_ReadData->ctrl_field_.func = 3;

                p_GetRequestNormal_ReadData->address_field_.sa.addr_type = 0;
                p_GetRequestNormal_ReadData->address_field_.sa.logic_addr = 0;
                p_GetRequestNormal_ReadData->address_field_.sa.addr_len = 5;
                p_GetRequestNormal_ReadData->address_field_.sa.address = QString((QByteArray(reinterpret_cast<char*>(tmpAddr),6).toHex()));
                p_GetRequestNormal_ReadData->address_field_.ca.address = 0x00;

                p_GetRequestNormal_ReadData->oad_.OI = PosActEne_OI;
                p_GetRequestNormal_ReadData->oad_.attribute.feature = 0;
                p_GetRequestNormal_ReadData->oad_.attribute.seq = 2;
                p_GetRequestNormal_ReadData->oad_.element_index = 0;

                p_GetRequestNormal_ReadData->piid_.reserve = 0;
                p_GetRequestNormal_ReadData->piid_.serve_priority = 0;
                p_GetRequestNormal_ReadData->piid_.serve_seq = 1;

                p_GetRequestNormal_ReadData->time_tag_field_.optional_ = 0;

                QByteArray msg698=p_GetRequestNormal_ReadData->EncodeFrame();
                //readInfoList[meterID].dataUnitList[0].readRound++;

                p_CurveReadMeter_F1F100->unit_down_.preserve_=0x00;
                p_CurveReadMeter_F1F100->unit_down_.protocol_type_=0x03;
                p_CurveReadMeter_F1F100->unit_down_.frame_content_=msg698;
                p_CurveReadMeter_F1F100->unit_down_.frame_length_=uchar(msg698.size());

                sendMsgOct=p_CurveReadMeter_F1F100->EncodeFrame();
                sendMsgLog=QString("》》曲线抄读F1F100,抄读OOP电表%0：%1\n").arg(QString((QByteArray(reinterpret_cast<char*>(tmpAddr),6).toHex()))).arg(QString(sendMsgOct.toHex()));

                startTime=QDateTime::currentDateTime().toString(CURRENT_TIME_FORMAT);
            }
            else
                return;
        }
        else if(emReadMode==RoundIndex)
        {
            if(roundIndexModeFrameList.at(meterID).protocolType==DLT645_2007)
            {
                QByteArray msg645;
                //2
                int frameCount=0;//单帧报文数
                for(int i=index;i<roundIndexModeFrameList.at(meterID).frameList.size();i++)
                {
                    if(frameCount<maxFrameNum)
                    {
                        msg645.append(roundIndexModeFrameList.at(meterID).frameList.at(i));
                        frameCount++;
                    }
                    else
                        break;
                }
                index+=frameCount;

                p_CurveReadMeter_F1F100->unit_down_.protocol_type_=0x02;
                p_CurveReadMeter_F1F100->unit_down_.preserve_=0x00;
                p_CurveReadMeter_F1F100->unit_down_.frame_content_=msg645;
                p_CurveReadMeter_F1F100->unit_down_.frame_length_=uchar(msg645.size());

                sendMsgOct=p_CurveReadMeter_F1F100->EncodeFrame();
                sendMsgLog=QString("》》曲线抄读F1F1,抄读645电表%0：%1\n").arg(QString((QByteArray(reinterpret_cast<char*>(tmpAddr),6).toHex()))).arg(QString(sendMsgOct.toHex()));
            }
            else if(readInfoList.at(meterID).protocolType==OOP)
            {
            }
        }

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
void Script_CurveRead_F1F100_Hunan::sendSrcMsg(DvcType dvcType, int dvcId, QByteArray msg)
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

void Script_CurveRead_F1F100_Hunan::timer_timeoutProc()
{
    switch(emScriptRunState)
    {
        case Wait_BuildNetFinish_Whole:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait build whole net finish timeout!!!");
            break;
        }
        case Wait_00F1_for_12F2_Pause:
        {
            if(++tryTimes>=3)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_00F1_for_12F2_Pause1st timeout!!!");
            }
            else
            {
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_RouterPause_12F2);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--路由暂停命令（12F2），等待--确认");
            }
            break;
        }
        case Wait_Finish_Curve:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "Wait_F1F1_UP timeout!!! Read Next Meter");

            break;
        }
        case Wait_00F1_for_12F3_Recover:
        {
            if(++tryTimes>=3)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_00F1_for_12F1_Restart timeout!!!");
            }
            else
            {
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_RouterRecover_12F3);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--路由恢复命令（12F3），等待--确认");
            }
            break;
        }
        default:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==p_timer");
            break;
        }
    }
}
void Script_CurveRead_F1F100_Hunan::maxAllowTimer_timeoutProc()
{
    switch(emScriptRunState)
    {
        case Wait_Finish_Curve:
        {
            p_maxAllowTimer->stop();
            p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==p_maxAllowTimer");
            break;
        }
        default:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==p_maxAllowTimer");
            break;
        }
    }
}
void Script_CurveRead_F1F100_Hunan::delayTimer_timeoutProc()
{
    p_delayTimer->stop();
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("已等待%1min，总共需要等待%2min，开始抄读曲线数据").arg(++waitedTime).arg(readPeriod*totalReadNum));
    if(waitedTime>readPeriod*totalReadNum)//已等待足够时间
    {
        meterIndex=0;
        parallelCount=0;//并行数

        emScriptRunState=Wait_Finish_Curve;
        emReadMode=MeterNo;
        if(emReadMode==MeterNo)
        {
            while(parallelCount<maxParallelNum)
            {
                if(meterIndex>=meterNoModeFrameList.size())
                    break;
                if(index>=meterNoModeFrameList.at(meterIndex).frameList.size())
                {
                    meterIndex++;
                    index=0;
                }
                sendMsg(CCO_GW,INSIGNIFICANCE,meterIndex,p_CurveReadMeter_F1F100);
                parallelCount++;
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--曲线抄读（F1F100），等待--抄表完成");
           }
        }
    }
    else
    {
        p_delayTimer->start(60*1000);//1分钟
    }
}

QList<DataIdLen> Script_CurveRead_F1F100_Hunan::dataIdInit(QString dataId)
{
    QList<DataIdLen> dataIdUnitList;
    QStringList dataIdList=dataId.split(',');
    for(int i=0;i<dataIdList.size();i++)
    {
        QStringList dataIdAndLen=dataIdList.at(i).split(' ');
        DataIdLen dataIdST;
        memcpy(dataIdST.data_id_,QByteArray::fromHex(dataIdAndLen.at(0).toLatin1()),4);
        dataIdST.reply_data_len_=uchar(dataIdAndLen.at(1).toUInt(nullptr,16));
        dataIdUnitList.append(dataIdST);
    }
    return dataIdUnitList;
}

void Script_CurveRead_F1F100_Hunan::readInfoInit()
{
    QDateTime startDateTime=QDateTime::currentDateTime().addSecs(-60*readPeriod*totalReadNum);
    QList<RoundMark> roundList;
    for(int i=0;i<=totalReadNum;i++)
    {
        RoundMark roundMark;
        roundMark.dateTime=QByteArray::fromHex(startDateTime.addSecs(60*readPeriod*i).toString("mmhhddMMyy").toLatin1());
        roundList.append(roundMark);
    }
    //645数据项
    singleDataId_645_List=dataIdInit(single645);
    triDataId_645_List=dataIdInit(tri645);
    //OOP OAD
    singleDataId_OOP_List=dataIdInit(singleOOP);
    triDataId_OOP_List=dataIdInit(triOOP);

    readInfoList.clear();
    for(int i=0;i<p_CtrInfoList->size();i++)
    {
        QList<MeterInfoForSingleNet*> meterInfoList=p_CtrInfoList->at(i)->p_MeterInfoForSingleNetList->values();
        //////////////
        /////////////
        /// /////////////
        /// /////////////
        for(int j=2;j<meterInfoList.size();j++)
        {
            ReadInfo_Curve readInfo_ST;
            memcpy(readInfo_ST.meterNo.addr,meterInfoList.at(j)->mtrAddr,6);
            readInfo_ST.protocolType=meterInfoList.at(j)->prtcl;
            if(readInfo_ST.protocolType==0x02)
            {
                if(meterInfoList.at(j)->realPhase==0x01||meterInfoList.at(j)->realPhase==0x02||meterInfoList.at(j)->realPhase==0x04)
                {
                    //singleDataId_645_List.size()
                    for(int n=0;n<2;n++)//与数据项个数对应
                    {
                        ReadDataUnit readData;
                        readData.dataIdLen=singleDataId_645_List.at(n);
                        readData.roundList.append(roundList);
                        readInfo_ST.dataUnitList.append(readData);
                    }
                }
                else if(meterInfoList.at(j)->realPhase==0x07)
                {
                    //triDataId_645_List.size()
                    for(int n=0;n<2;n++)//与数据项个数对应
                    {
                        ReadDataUnit readData;
                        readData.dataIdLen=triDataId_645_List.at(n);
                        readData.roundList.append(roundList);
                        readInfo_ST.dataUnitList.append(readData);
                    }
                }
            }
            else if(readInfo_ST.protocolType==0x03)
            {
                if(meterInfoList.at(j)->realPhase==0x01||meterInfoList.at(j)->realPhase==0x02||meterInfoList.at(j)->realPhase==0x04)
                {
                    //singleDataId_OOP_List.size()
                    for(int n=0;n<2;n++)//与数据项个数对应
                    {
                        ReadDataUnit readData;
                        readData.dataIdLen=singleDataId_OOP_List.at(n);
                        readData.roundList.append(roundList);
                        readInfo_ST.dataUnitList.append(readData);
                    }
                }
                else if(meterInfoList.at(j)->realPhase==0x07)
                {
                    //triDataId_OOP_List.size()
                    for(int n=0;n<2;n++)//与数据项个数对应
                    {
                        ReadDataUnit readData;
                        readData.dataIdLen=triDataId_OOP_List.at(n);
                        readData.roundList.append(roundList);
                        readInfo_ST.dataUnitList.append(readData);
                    }
                }
            }
            readInfoList.append(readInfo_ST);
        }
    }
}

void Script_CurveRead_F1F100_Hunan::readInfoClear()
{
    for(int i=0;i<readInfoList.size();i++)
    {
        for(int j=0;j<readInfoList.at(i).dataUnitList.size();j++)
        {
            for(int n=0;n<readInfoList.at(i).dataUnitList.at(j).roundList.size();n++)
            {
                readInfoList[n].dataUnitList[j].roundList[n].readFlag=false;
                readInfoList[n].dataUnitList[j].roundList[n].replyData.clear();
            }
        }
    }
}

void Script_CurveRead_F1F100_Hunan::readFrameListInit()
{
    for(int i=0;i<readInfoList.size();i++)
    {
        if(readInfoList.at(i).protocolType==DLT645_2007)
        {
            ReadModeFrame readModeFrame;
            readModeFrame.protocolType=DLT645_2007;
            memcpy(readModeFrame.meterNo.addr,readInfoList.at(i).meterNo.addr,6);
            shared_ptr<Rqst_ReadData_0x11> p_ReadData_0x11=make_shared<Rqst_ReadData_0x11>(addr,4,0);
            memcpy(p_ReadData_0x11->addr_,readInfoList.at(i).meterNo.addr,6);
            for(int j=0;j<readInfoList.at(i).dataUnitList.size();j++)
            {
                memcpy(p_ReadData_0x11->di,readInfoList.at(i).dataUnitList.at(j).dataIdLen.data_id_,4);
                for(int n=0;n<readInfoList.at(i).dataUnitList.at(j).roundList.size();n+=singleReadNum)//根据点数
                {
                    memcpy(p_ReadData_0x11->dateTime,readInfoList.at(i).dataUnitList.at(j).roundList.at(n).dateTime,5);
                    //待改
                    if(n+singleReadNum<readInfoList.at(i).dataUnitList.at(j).roundList.size())
                        p_ReadData_0x11->blockOfRcrd=singleReadNum;
                    else
                        p_ReadData_0x11->blockOfRcrd=uchar(readInfoList.at(i).dataUnitList.at(j).roundList.size()-n);
                    readModeFrame.frameList.append(p_ReadData_0x11->EncodeFrame());
                }
            }
            meterNoModeFrameList.append(readModeFrame);

        }
        else if(readInfoList.at(i).protocolType==OOP)
        {
            //待添加
        }
    }
    for(int j=0;j<totalReadNum;j++)
    {
        for(int i=0;i<readInfoList.size();i++)
        {
            if(readInfoList.at(i).protocolType==DLT645_2007)
            {
                ReadModeFrame readModeFrame;
                memcpy(readModeFrame.meterNo.addr,readInfoList.at(i).meterNo.addr,6);
                shared_ptr<Rqst_ReadData_0x11> p_ReadData_0x11=make_shared<Rqst_ReadData_0x11>(addr,4,0);
                memcpy(p_ReadData_0x11->addr_,readInfoList.at(i).meterNo.addr,6);

                for(int n=0;n<readInfoList.at(i).dataUnitList.size();n++)
                {
                    memcpy(p_ReadData_0x11->di,readInfoList.at(i).dataUnitList.at(n).dataIdLen.data_id_,4);
                    memcpy(p_ReadData_0x11->dateTime,readInfoList.at(i).dataUnitList.at(n).roundList.at(j).dateTime,5);
                    p_ReadData_0x11->blockOfRcrd=1;
                    readModeFrame.frameList.append(p_ReadData_0x11->EncodeFrame());
                }
                roundIndexModeFrameList.append(readModeFrame);
            }
            else if(readInfoList.at(i).protocolType==OOP)
            {
                //待添加
            }
        }
    }
}

int Script_CurveRead_F1F100_Hunan::getMeterNoIndex(Address address)
{
    for(int i=0;i<readInfoList.size();i++)
    {
        if(address==readInfoList.at(i).meterNo)
        {
            return i;
        }
    }
    return -1;
}

bool Script_CurveRead_F1F100_Hunan::isNotReplyEmpty(QByteArray data)
{
    int count=0;
    for(int i=0;i<data.size();i++)
    {
        if(data.at(i)==char(0xff))
            count++;
    }
    if(count==data.size()||data.isEmpty())
        return false;
    else
        return true;
}


