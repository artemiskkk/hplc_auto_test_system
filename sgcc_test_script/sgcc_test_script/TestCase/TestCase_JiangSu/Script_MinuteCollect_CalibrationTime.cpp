#include "Script_MinuteCollect_CalibrationTime.h"
#include <QDataStream>
#include <QDir>

Script_MinuteCollect_CalibrationTime::Script_MinuteCollect_CalibrationTime(QObject *parent) : QObject(parent)
{
    emScriptRunState=ReadMeterInit;
    resultFlag=false;
    sendMsgOct.clear();

    p_CtrInfoList=make_shared<QList<shared_ptr<CtrInfo>>>();
    p_BuildNetwork_GW=new BuildNetwork_GW();

    p_Frame3762Helper=make_shared<Frame3762Helper>();
    p_RouterPause_12F2=make_shared<Afn12F2>();
    p_QueryRouterState_10F4=make_shared<Afn10F4>();
    p_RouterRecover_12F3=make_shared<Afn12F3>();
    p_ParallelReadMeter_F1F1=make_shared<AfnF1F1>();
    p_ReqCtrTime_14F2=make_shared<Afn14F2>();//**请求集中器时间

    p_Frame645Helper=make_shared<Frame645Helper>();
    p_MeterAddrResp_93=make_shared<RspsNormal_ReadAddr_0x93>(addr,6);
    p_FrameOOPHelper=make_shared<FrameOOPHelper>();
    p_GetResponseNormal_ReadAddr=make_shared<GetResponseNormal>();

    p_ActionRequest_RollCallReport=make_shared<ActionRequest>();
    p_ReportNotificationRecordList=make_shared<ReportNotificationRecordList>();

    p_timer=new QTimer();
    p_maxAllowTimer=new QTimer();
    p_delayTimer=new QTimer();
    connect(p_timer,SIGNAL(timeout()),this,SLOT(timer_timeoutProc()));
    connect(p_maxAllowTimer,SIGNAL(timeout()),this,SLOT(maxAllowTimer_timeoutProc()));
    connect(p_delayTimer,SIGNAL(timeout()),this,SLOT(delayTimer_timeoutProc()));
}

Script_MinuteCollect_CalibrationTime::~Script_MinuteCollect_CalibrationTime()
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

void Script_MinuteCollect_CalibrationTime::execute()
{
    p_AbstractScriptHost->updateProgress(ProcessState_Start, "Test start!");

    emScriptRunState=ReadMeterInit;
    resultFlag=false;
    addrList.clear();
    readInfoInit();
    if(needBuildNet==true)
    {
        p_BuildNetwork_GW->execute();
        emScriptRunState=Wait_BuildNetFinish_Whole;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "等待全网组网完成");
        p_timer->start((timerForReachThresld+timerAfterReachThresld)*1000);
    }
    else
    {
        //**等待路由请求集中器时钟完成
        emScriptRunState=Wait_RouterReqTime_14F2_Finish;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "等待路由请求集中器时钟-14F2完成");
        //**直接等待上报，无需定时
        //**p_delayTimer->start(waitTime*60*1000);
    }
    p_maxAllowTimer->start(timerMaxReachThresld*60*1000);

}

void Script_MinuteCollect_CalibrationTime::stop()
{
    if(p_AbstractScriptHost==nullptr)
        return;
    p_AbstractScriptHost->updateProgress(ProcessState_Error, "Test stop!");
    if(needPowerOff==true)
        powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost);
}

void Script_MinuteCollect_CalibrationTime::addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq)
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

void Script_MinuteCollect_CalibrationTime::setHost(AbstractScriptHost *host)
{
    p_AbstractScriptHost=host;
    p_BuildNetwork_GW->setHost(host);
}

bool Script_MinuteCollect_CalibrationTime::config(const QMap<QString, QString> *paraDic)
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
        if(paraDic->keys().contains("maxFrameNum"))
        {
            this->maxFrameNum = (*paraDic)["maxFrameNum"].toUShort();
        }
        if(paraDic->keys().contains("maxParallelNum"))
        {
            this->maxParallelNum = (*paraDic)["maxParallelNum"].toUShort();
        }
        if(paraDic->keys().contains("notInParameterCjqNum"))
        {
            p_CtrInfoList->at(0)->notInParameterCjqNum = (*paraDic)["notInParameterCjqNum"].toUShort();
        }
        if(paraDic->keys().contains("cjqAddressAccessNetFlag"))
        {
            if((*paraDic)["cjqAddressAccessNetFlag"].toLower()=="false")
            {
                p_CtrInfoList->at(0)->cjqAddressAccessNetFlag=false;
            }
            else
            {
                p_CtrInfoList->at(0)->cjqAddressAccessNetFlag=true;
            }
        }
        if(paraDic->keys().contains("dataIdIndexList"))
        {
            QStringList list=(*paraDic)["dataIdIndexList"].split(",");

            for(int i=0;i<list.size();i++)
            {
                QStringList list1=list.at(i).split("-");
                if(list1.size()==2)
                {
                    int j=list1.at(0).toInt();
                    while(j<=list1.at(1).toInt())
                    {
                        this->dataIdIndexList.append(j);
                        j++;
                    }
                }
                else
                {
                    this->dataIdIndexList.append(list1.at(0).toInt());
                }
            }
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

void Script_MinuteCollect_CalibrationTime::processMsg(DvcType dvcType, int id, uchar *data, int datalen)
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
                //**等待路由请求集中器时钟完成
//                emScriptRunState=Wait_RouterReqTime_14F2_Finish;
//                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "等待路由请求集中器时钟-14F2完成");
                //**直接等待上报，无需定时
                //**p_delayTimer->start(waitTime*60*1000);
                emScriptRunState=Wait_00F1_for_12F2_Pause;
                curTime=QTime::currentTime();
                QString cTime = curTime.toString("hh:mm:ss");
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("【curTime】当前时间为：%1").arg(cTime));
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("等待20分钟后开始点名上传冻结数据！"));
                p_delayTimer->start(waitTime*60*1000);
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
        else
        {
            return;
        }
    }
}

void Script_MinuteCollect_CalibrationTime::processCtrlDvcRes(DvcType dvcType, QList<int> idList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params)
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
        case Wait_RouterReqTime_14F2_Finish:
        {
            break;
        }
        case Wait_00F1_for_12F2_Pause:
        {
            break;
        }
        case Wait_Finish_F1F1:
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
        default:
            break;
    }
}


void Script_MinuteCollect_CalibrationTime::processMsgFromCCO(DvcType dvcType, int dvcId)
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
            case ReadMeterInit:
            {
                break;
            }
            case Wait_BuildNetFinish_Whole:
            {
                break;
            }
            case Wait_RouterReqTime_14F2_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x14&&p_Frame3762Base->dt1_==0x02&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
//                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到--路由请求集中器时间(14F2)"));
//                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_ReqCtrTime_14F2);
//                    emScriptRunState=Wait_00F1_for_12F2_Pause;
//                    curTime=QTime::currentTime();
//                    QString cTime = curTime.toString("hh:mm:ss");
//                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("【curTime】当前时间为：%1").arg(cTime));
//                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("等待20分钟后开始点名上传冻结数据！"));
//                    p_delayTimer->start(waitTime*60*1000);

                }
                break;
            }
            case Wait_00F1_for_12F2_Pause:
            {
                //**收到暂停路由确认上报之后，查询路由运行状态
                if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();//接收到一条完整报文
                    emScriptRunState=Wait_QueryRouterState_Finish;
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryRouterState_10F4);
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询路由运行状态（10F4），等待--回复");
                    p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_QueryRouterState_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x10&&p_Frame3762Base->dt1_==0x08&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();//接收到一条完整报文
                    shared_ptr<Afn10F4> p_QueryRouterState_10F4_Up=dynamic_pointer_cast<Afn10F4>(p_Frame3762Base);
                    if(p_QueryRouterState_10F4_Up->router_operate_state_unit_.operate_state_word_.router_complete_flag_==0x01)
                    {
                        tryTimes=0;
                        index=0;
                        meterIndex=0;
                        times=ushort(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size());

                        emScriptRunState=Wait_Finish_F1F1;
                        //1
                        while(parallelCount<maxParallelNum)
                        {
                            if(meterIndex>=readInfoList.size())
                                break;
                            if(readInfoList.at(meterIndex).readFlag!=ReadSuccess)
                            {
                                emSendOopProtype=pro_ActionRequest_RollCallRepor;
                                dvcNo = dvcId;
                                sendMsg(CCO_GW,dvcId,meterIndex,p_ParallelReadMeter_F1F1);
                                parallelCount++;
                                meterIndex++;
                                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--并发抄表（F1F1），等待--抄表完成");
                            }
                       }
                        //开始计时
                        p_timer->start((maxMonitorTime*2)*1000);
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("等待30s，查询路由运行状态"));
                        p_delayTimer->start(30*1000);

                    }

                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_Finish_F1F1:
            {
                if(p_Frame3762Base->afn_==char(0xF1)&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    shared_ptr<AfnF1F1> p_ParallelReadMeter_F1F1_Up=dynamic_pointer_cast<AfnF1F1>(p_Frame3762Base);

                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("debug<<收到F1F1上报"));

                    Address srcAddr;
                    memcpy(srcAddr.addr,p_ParallelReadMeter_F1F1_Up->address_field_.src_addr,6);
                    int currentMeterIndex=getReadInfo(srcAddr);
                    //**直接进行判断
                    if(p_ParallelReadMeter_F1F1_Up->unit_up_.frame_length_!=0)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("debug<<收到F1F1上报_报文长度不为0"));
                        QByteArray msgBuf=p_ParallelReadMeter_F1F1_Up->unit_up_.frame_content_;
                        QString str_frame_content = QString(msgBuf.toHex());
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("上报报文内容msgBuf=%1").arg(str_frame_content));
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("上报报文内容str_frame_content=%1, %2").arg(str_frame_content[28]).arg(str_frame_content[29]));

                        char temp_88 = 0x88, temp_02 = 0x02,
                                temp_60 = 0x60, temp_12 = 0x12, temp_03 = 0x03, temp_00 = 0x00,
                                temp_50 = 0x50, temp_06 = 0x06,
                                temp_04 = 0x04, temp_87 = 0x87,
                                temp_01 = 0x01, temp_43 = 0x43,
                                temp_7f = 0x7f, temp_c8 = 0xc8;
                        //**简单粗暴直接判断具体位的具体值
                        //**判断具体服务器
                        if(temp_88==msgBuf[14]&&temp_02==msgBuf[15])
                        {
                              p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("debug<<收到日冻结-复合数据上报"));
                              //**判断AResultRecord：OAD
                              if(temp_60==msgBuf[18]&&temp_12==msgBuf[19]&&temp_03==msgBuf[20]&&temp_00==msgBuf[21])
                              {
                                  if(temp_50==msgBuf[24]&&temp_06==msgBuf[25]&&temp_02==msgBuf[26]&&temp_00==msgBuf[27])
                                  {
                                      p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到月冻结-复合数据上报"));
                                      emSendOopProtype=pro_reportResponseRecordList;
                                      monFlay=false;
                                      sendMsg(CCO_GW,dvcId,meterIndex,p_ParallelReadMeter_F1F1);
                                      p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("发送--集中器响应上报报文--F1F1"));
                                      p_timer->start((maxMonitorTime*2)*1000);
                                  }
                                  else if(temp_50==msgBuf[24]&&temp_04==msgBuf[25]&&temp_02==msgBuf[26]&&temp_00==msgBuf[27])
                                  {
                                      p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到日冻结-复合数据上报"));
                                      emSendOopProtype=pro_reportResponseRecordList;
                                      dayFlay=false;
                                      sendMsg(CCO_GW,dvcId,meterIndex,p_ParallelReadMeter_F1F1);
                                      p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("发送--集中器响应上报报文--F1F1"));
                                      p_timer->start((maxMonitorTime*2)*1000);
                                  }
                                  else if(temp_50==msgBuf[24]&&temp_02==msgBuf[25]&&temp_02==msgBuf[26]&&temp_00==msgBuf[27])
                                  {
                                      p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到分钟冻结-复合数据上报"));
                                      minFlag=false;
                                      emSendOopProtype=pro_minuteFroRecordList;
                                      sendMsg(CCO_GW,dvcId,meterIndex,p_ParallelReadMeter_F1F1);
                                      p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("发送--集中器响应上报报文--F1F1"));
                                      p_timer->start((maxMonitorTime*2)*1000);
                                  }
                                  else if(temp_c8 == msgBuf[msgBuf.size()-6]&&minFlag==false&&dayFlay==false&&monFlay==false)
                                  {
                                      p_AbstractScriptHost->updateProgress(ProcessState_Success, QString("收到--采集器上报分钟冻结之后无后续数据！测试成功！"));
                                      emScriptRunState=ScriptSuccess;
                                  }
                                  else
                                  {
                                      p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("等待5min后重新进行点名上报"));
                                      p_delayTimer->start(5*60*1000);
                                  }
                              }
                        }
                        //**88 06分钟采集上报
                        else if(temp_88==msgBuf[14]&&temp_06==msgBuf[15])
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("debug<<收到分钟冻结-复合数据上报!!"));
                           if(temp_60==msgBuf[17]&&temp_12==msgBuf[18]&&temp_03==msgBuf[19]&&temp_00==msgBuf[20])
                            {
                                p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("debug<<收到分钟冻结-复合数据上报!!@@"));
                               //**取上报电表数
                               char temp_gn = msgBuf[40];
                               temp_gn =40 + temp_gn * 8 + 1;
                               char temp_gn_1 = temp_gn + 1;

                               if(temp_50==msgBuf[temp_gn]&&temp_02==msgBuf[temp_gn_1])
                                 {
                                     p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到分钟冻结-复合数据上报"));
                                     minFlag=false;
                                     emSendOopProtype=pro_minuteFroRecordList;
                                     sendMsg(CCO_GW,dvcId,meterIndex,p_ParallelReadMeter_F1F1);
                                     p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("发送--集中器响应上报报文--F1F1"));
                                     p_timer->start((maxMonitorTime*2)*1000);
                                 }
                               else if(temp_06==msgBuf[msgBuf.size()-6]&&minFlag==false&&dayFlay==false&&monFlay==false)
                               {
                                   p_AbstractScriptHost->updateProgress(ProcessState_Success, QString("收到--采集器正常上报否认报文！测试成功！"));
                                   emScriptRunState=ScriptSuccess;
                               }
                               else
                               {
                                   p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("等待5min后重新进行点名上报"));
                                   p_delayTimer->start(5*60*1000);
                               }
                            }
                        }

                        else if(temp_87==msgBuf[14]&&temp_01==msgBuf[15])
                        {
                            if(temp_60==msgBuf[17]&&temp_43==msgBuf[18]&&temp_7f==msgBuf[19]&&temp_00==msgBuf[20])
                            {
                                if(temp_06==msgBuf[21]&&minFlag==false)
                                {
                                    p_AbstractScriptHost->updateProgress(ProcessState_Success, QString("收到--采集器正常上报否认报文！测试成功！"));
                                    emScriptRunState=ScriptSuccess;
                                }
                            }
                        }




                        //**判断协议类型
//                        if(p_ParallelReadMeter_F1F1_Up->unit_up_.protocol_type_==OOP)
//                        {
//                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("debug<<收到F1F1上报_报文长度不为0_协议类型为oop"));

//                            QString spendTime;
//                            bool res=true;
//                            shared_ptr<FrameOOPBase> MsgBase_OOP_ptr = FrameOOPHelper::DecodeMsg(&msgBuf,res);
//                            if(MsgBase_OOP_ptr!=nullptr)
//                            {
//                                p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("debug<<收到F1F1上报_报文长度不为0_协议类型为oop_报文不为空"));
////                                    //**收到日冻结-复合数据上报
//                                    //if(MsgBase_OOP_ptr->service_sub_type_==uchar(ReportNotificationType::kReportNotificationRecordList))
//                                    if(MsgBase_OOP_ptr->service_type_==REPORT_NOTIFICATION_SERVER && MsgBase_OOP_ptr->service_sub_type_==uchar(ReportNotificationType::kReportNotificationRecordList))
//                                    {
//                                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("debug<<收到日冻结-复合数据上报"));
////                                            if(emSendOopProtype==pro_ActionRequest_RollCallRepor)
////                                            {
//                                            shared_ptr<ReportNotificationRecordList> p_ReportNotificationRecordList=dynamic_pointer_cast<ReportNotificationRecordList>(MsgBase_OOP_ptr);

//                                            //**根据数据类型判断上报OOP报文类型
//                                            OAD oad;
//                                            oad.OI=6012;
//                                            oad.attribute.feature=0;
//                                            oad.attribute.seq=3;
//                                            oad.element_index=0;

//                                            ROAD road;
//                                            road.oad.OI=5006;
//                                            road.oad.attribute.feature=0;
//                                            road.oad.attribute.seq=2;
//                                            road.oad.element_index=0;

//                                            p_ReportNotificationRecordList->list_result_record_[0].rcsd_ptr = make_shared<RCSD>();
//                                            //p_ReportNotificationRecordList->list_result_record_[0].response_data_ptr = make_shared<RecordResponseDataParent>();

//                                            if(p_ReportNotificationRecordList->list_result_record_.at(0).oad==oad
//                                                    && std::dynamic_pointer_cast<CsdROad>(p_ReportNotificationRecordList->list_result_record_[0].rcsd_ptr->list_csd_[0])->road_==road)
//                                            {
//                                                p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到日冻结-复合数据上报"));
//                                                emSendOopProtype=pro_reportResponseRecordList;
//                                                sendMsg(CCO_GW,dvcId,meterIndex,p_ParallelReadMeter_F1F1);
//                                                p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("发送--集中器响应上报报文--F1F1"));
//                                                p_timer->start((maxMonitorTime*2)*1000);
//                                            }
////                                            }

//                                        //**无后续数据上报
////                                            else if(emSendOopProtype==pro_reportResponseRecordList)
////                                            {
//                                            //shared_ptr<ReportNotificationRecordList> p_ReportNotificationRecordList=dynamic_pointer_cast<ReportNotificationRecordList>(MsgBase_OOP_ptr);
//                                            //**根据数据类型判断上报OOP报文类型
//                                                OAD oad_1;
//                                                oad_1.OI=6012;
//                                                oad_1.attribute.feature=0;
//                                                oad_1.attribute.seq=3;
//                                                oad_1.element_index=0;
//                                                ROAD road_1;
//                                                road_1.oad.OI=5006;
//                                                road_1.oad.attribute.feature=0;
//                                                road_1.oad.attribute.seq=2;
//                                                road_1.oad.element_index=0;
//                                                if(p_ReportNotificationRecordList->list_result_record_.at(0).oad==oad_1
//                                                        && std::dynamic_pointer_cast<CsdROad>(p_ReportNotificationRecordList->list_result_record_[0].rcsd_ptr->list_csd_[0])->road_==road_1
//                                                        && std::dynamic_pointer_cast<RecordResponseDAR>(p_ReportNotificationRecordList->list_result_record_[0].response_data_ptr)->dar_==0xc8)
//                                                {
//                                                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到--采集器上报复合数据之后无后续数据！"));
//                                                    p_AbstractScriptHost->updateProgress(ProcessState_Success, QString("收到--采集器正常上报否认报文！测试成功！"));
//                                                }
//                                                emSendOopProtype=pro_minuteFroRecordList;

////                                            }
//                                        //**收到分钟冻结上报-单相/三相
////                                            else if(emSendOopProtype==pro_minuteFroRecordList)
////                                            {
//                                            //shared_ptr<ReportNotificationRecordList> p_ReportNotificationRecordList=dynamic_pointer_cast<ReportNotificationRecordList>(MsgBase_OOP_ptr);
//                                            OAD oad_2;
//                                            oad_2.OI=6012;
//                                            oad_2.attribute.feature=0;
//                                            oad_2.attribute.seq=3;
//                                            oad_2.element_index=0;
//                                            ROAD road_2;
//                                            road_2.oad.OI=5002;
//                                            road_2.oad.attribute.feature=0;
//                                            road_2.oad.attribute.seq=2;
//                                            road_2.oad.element_index=0;
//                                            if(p_ReportNotificationRecordList->list_result_record_.at(0).oad==oad_2
//                                                    && std::dynamic_pointer_cast<CsdROad>(p_ReportNotificationRecordList->list_result_record_[0].rcsd_ptr->list_csd_[0])->road_==road_2)
//                                            {
//                                                p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到--采集器正常上报分钟冻结数据！"));
//                                                emSendOopProtype=pro_minuteFroRecordList;
//                                                p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("发送--集中器响应上报报文--F1F1"));
//                                                sendMsg(CCO_GW,dvcId,meterIndex,p_ParallelReadMeter_F1F1);
//                                                p_timer->start((maxMonitorTime*2)*1000);
//                                            }
//                                            //**收到分钟冻结采集后续上报报文
//                                            else if(std::dynamic_pointer_cast<RecordResponseDAR>(p_ReportNotificationRecordList->list_result_record_[2].response_data_ptr)->dar_==0xc8)
//                                            {
//                                                p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到--采集器上报分钟冻结之后无后续数据！"));
//                                            }

////                                            }
//                                    }
//                                    //**采集器无后续响应数据（否认报文）
//                                    else if(MsgBase_OOP_ptr->service_type_==ACTION_RESPONSE_SERVER)
//                                    {
//                                        shared_ptr<ActionResponseNormal> p_ActionResponseNormal=dynamic_pointer_cast<ActionResponseNormal>(MsgBase_OOP_ptr);
//                                        DAR dar_=p_ActionResponseNormal->a_action_result_.dar;
//                                        if(0x06==dar_)
//                                        {
//                                            p_AbstractScriptHost->updateProgress(ProcessState_Success, QString("收到--采集器正常上报否认报文！测试成功！"));
//                                            emScriptRunState=ScriptSuccess;
//                                        }
//                                    }

////                                }
//                            }
////                            if(isAllRead(currentMeterIndex)==true)
////                                readInfoList[currentMeterIndex].readFlag=ReadSuccess;
//                        }
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

void Script_MinuteCollect_CalibrationTime::processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID)
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

void Script_MinuteCollect_CalibrationTime::processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID)
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

void Script_MinuteCollect_CalibrationTime::sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame)
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
    else if(frame==p_QueryRouterState_10F4)
    {
        p_QueryRouterState_10F4->ctrl_field_.dir=kDirDown;
        p_QueryRouterState_10F4->ctrl_field_.prm=kActive;
        p_QueryRouterState_10F4->ctrl_field_.comn_type=kHplc;

        p_QueryRouterState_10F4->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_QueryRouterState_10F4->info_field_.info_field_down.comu_rate=0;
        p_QueryRouterState_10F4->info_field_.info_field_down.comu_module_ident=0;

        sendMsgOct=p_QueryRouterState_10F4->EncodeFrame();
        sendMsgLog=QString("》》查询路由运行状态10F4：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
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
    else if(frame==p_ParallelReadMeter_F1F1)
    {
        p_ParallelReadMeter_F1F1->ctrl_field_.dir=kDirDown;
        p_ParallelReadMeter_F1F1->ctrl_field_.prm=kActive;
        p_ParallelReadMeter_F1F1->ctrl_field_.comn_type=kHplc;

        p_ParallelReadMeter_F1F1->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_ParallelReadMeter_F1F1->info_field_.info_field_down.comu_rate=0;
        p_ParallelReadMeter_F1F1->info_field_.info_field_down.comu_module_ident=1;

        uchar tmpAddr[6];
        memcpy(tmpAddr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(0)->cjqAddr,6);
        //memcpy(tmpAddr,readInfoList.at(meterID).meterNo.addr,6);
        uchar tmpCcoAddr[6];
        memcpy(tmpCcoAddr,p_CtrInfoList->at(0)->ccoAddr,6);
        memcpy(p_ParallelReadMeter_F1F1->address_field_.src_addr,tmpCcoAddr,6);
        memcpy(p_ParallelReadMeter_F1F1->address_field_.dst_addr,tmpAddr,6);


            if(emSendOopProtype==pro_ActionRequest_RollCallRepor)
            {
                //**ActionRequest操作一个对象方法请求
                shared_ptr<ActionRequest> p_ActionRequest_RollCallReport = make_shared<ActionRequest>();

                p_ActionRequest_RollCallReport->ctrl_field_.dir = 0;
                p_ActionRequest_RollCallReport->ctrl_field_.prm = 1;
                p_ActionRequest_RollCallReport->ctrl_field_.fra = 0;
                p_ActionRequest_RollCallReport->ctrl_field_.res = 0;
                p_ActionRequest_RollCallReport->ctrl_field_.sc = 0;
                p_ActionRequest_RollCallReport->ctrl_field_.func = 3;

                p_ActionRequest_RollCallReport->address_field_.sa.addr_type = 0;
                p_ActionRequest_RollCallReport->address_field_.sa.logic_addr = 0;
                p_ActionRequest_RollCallReport->address_field_.sa.addr_len = 5;
                p_ActionRequest_RollCallReport->address_field_.sa.address = QString((QByteArray(reinterpret_cast<char*>(tmpAddr),6).toHex()));
                p_ActionRequest_RollCallReport->address_field_.ca.address = 0x00;

                //**PIID
                p_ActionRequest_RollCallReport->piid_.serve_priority = 0;
                p_ActionRequest_RollCallReport->piid_.serve_seq = 1;
                p_ActionRequest_RollCallReport->piid_.reserve = 0;

                //**OMD
                p_ActionRequest_RollCallReport->a_action_.omd.OI = 0x6043;
                p_ActionRequest_RollCallReport->a_action_.omd.operate_mode = 0; //**默认为0
                p_ActionRequest_RollCallReport->a_action_.omd.method_index = 127;

                p_ActionRequest_RollCallReport->a_action_.data_ptr=make_shared<DataBasic>();
                p_ActionRequest_RollCallReport->a_action_.data_ptr->type_=kUnsigned;
                //**Data
                std::dynamic_pointer_cast<DataBasic>(p_ActionRequest_RollCallReport->a_action_.data_ptr)->data_.append(char(0x01));
                //std::dynamic_pointer_cast<DataString>(p_ActionRequest_RollCallReport)->data_.append(char(0x17));


                //**TimeTag
                p_ActionRequest_RollCallReport->time_tag_field_.optional_ = 0;

                QByteArray msg698=p_ActionRequest_RollCallReport->EncodeFrame();

                p_ParallelReadMeter_F1F1->unit_down_.subsidiary_node_num_=0x00;
                p_ParallelReadMeter_F1F1->unit_down_.protocol_type_=0x03;
                p_ParallelReadMeter_F1F1->unit_down_.frame_content_=msg698;
                p_ParallelReadMeter_F1F1->unit_down_.frame_length_=uchar(msg698.size());

                sendMsgOct=p_ParallelReadMeter_F1F1->EncodeFrame();
                sendMsgLog=QString("》》并发抄表F1F1,启动点名上报%0：%1\n").arg(QString((QByteArray(reinterpret_cast<char*>(tmpAddr),6).toHex()))).arg(QString(sendMsgOct.toHex()));

                startTime=QDateTime::currentDateTime().toString(CURRENT_TIME_FORMAT);
            }
            else if(emSendOopProtype==pro_reportResponseRecordList)
            {
                //**ReportResponseRecordList上报响应记录列表
                shared_ptr<ReportResponseRecordList> p_ReportResponseRecordList = make_shared<ReportResponseRecordList>();

                p_ReportResponseRecordList->ctrl_field_.dir = 0;
                p_ReportResponseRecordList->ctrl_field_.prm = 1;
                p_ReportResponseRecordList->ctrl_field_.fra = 0;
                p_ReportResponseRecordList->ctrl_field_.res = 0;
                p_ReportResponseRecordList->ctrl_field_.sc = 0;
                p_ReportResponseRecordList->ctrl_field_.func = 3;

                p_ReportResponseRecordList->address_field_.sa.addr_type = 0;
                p_ReportResponseRecordList->address_field_.sa.logic_addr = 0;
                p_ReportResponseRecordList->address_field_.sa.addr_len = 5;
                p_ReportResponseRecordList->address_field_.sa.address = QString((QByteArray(reinterpret_cast<char*>(tmpAddr),6).toHex()));
                p_ReportResponseRecordList->address_field_.ca.address = 0x00;


                //**PIID
                p_ReportResponseRecordList->piid_.serve_priority = 0;
                p_ReportResponseRecordList->piid_.serve_seq = 2;
                p_ReportResponseRecordList->piid_.reserve = 0;

                //**OMD.OAD
                OAD oad;
                oad.OI=0x6012;
                oad.attribute.seq=3;
                oad.attribute.feature=0;
                oad.element_index=0;
                p_ReportResponseRecordList->list_oad_.append(oad);

                //**TimeTag
                p_ReportResponseRecordList->time_tag_field_.optional_ = 0;


                QByteArray msg698=p_ReportResponseRecordList->EncodeFrame();

                p_ParallelReadMeter_F1F1->unit_down_.subsidiary_node_num_=0x00;
                p_ParallelReadMeter_F1F1->unit_down_.protocol_type_=0x03;
                p_ParallelReadMeter_F1F1->unit_down_.frame_content_=msg698;
                p_ParallelReadMeter_F1F1->unit_down_.frame_length_=uchar(msg698.size());

                sendMsgOct=p_ParallelReadMeter_F1F1->EncodeFrame();
                sendMsgLog=QString("》》并发抄表F1F1,集中器响应记录型对象列表上报%0：%1\n").arg(QString((QByteArray(reinterpret_cast<char*>(tmpAddr),6).toHex()))).arg(QString(sendMsgOct.toHex()));

                startTime=QDateTime::currentDateTime().toString(CURRENT_TIME_FORMAT);
            }
            else if(emSendOopProtype==pro_minuteFroRecordList)
            {
                QByteArray msg698;
                msg698.append(QByteArray::fromHex(QString("681300430524000000000000d22b080602003ef716").toLatin1()));

                p_ParallelReadMeter_F1F1->unit_down_.subsidiary_node_num_=0x00;
                p_ParallelReadMeter_F1F1->unit_down_.protocol_type_=0x03;
                p_ParallelReadMeter_F1F1->unit_down_.frame_content_=msg698;
                p_ParallelReadMeter_F1F1->unit_down_.frame_length_=uchar(msg698.size());

                sendMsgOct=p_ParallelReadMeter_F1F1->EncodeFrame();
                sendMsgLog=QString("》》并发抄表F1F1,集中器响应精简记录型对象属性上报%0：%1\n").arg(QString((QByteArray(reinterpret_cast<char*>(tmpAddr),6).toHex()))).arg(QString(sendMsgOct.toHex()));

                startTime=QDateTime::currentDateTime().toString(CURRENT_TIME_FORMAT);
            }

        else
            return;
    }
    else if(frame==p_ReqCtrTime_14F2)
    {
        QDateTime crntDateTime=QDateTime::currentDateTime();

        QString sDate=crntDateTime.date().toString("yyMMdd");
        QString sTime=crntDateTime.time().toString("hhmmss");
        uchar dateTime[6]={0,0,0,0,0,0};

        for(int i=0; i<3; i++)
        {
            dateTime[i] = (sTime.at(5-i*2-1).cell()-'0')*16+(sTime.at(5-i*2).cell()-'0');
            dateTime[i+3] = (sDate.at(5-i*2-1).cell()-'0')*16+(sDate.at(5-i*2).cell()-'0');
        }
        memcpy(&p_ReqCtrTime_14F2->current_time_,dateTime,6);

        p_ReqCtrTime_14F2->ctrl_field_.dir=kDirDown;
        p_ReqCtrTime_14F2->ctrl_field_.prm=kActive;
        p_ReqCtrTime_14F2->ctrl_field_.comn_type=kHplc;

        p_ReqCtrTime_14F2->info_field_.info_field_down.msg_seq=char(msgSeq);
        p_ReqCtrTime_14F2->info_field_.info_field_down.comu_module_ident=0;

        sendMsgOct=p_ReqCtrTime_14F2->EncodeFrame();
        sendMsgLog=QString("》》路由请求集中器时钟14F2：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
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

void Script_MinuteCollect_CalibrationTime::sendSrcMsg(DvcType dvcType, int dvcId, QByteArray msg)
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

void Script_MinuteCollect_CalibrationTime::timer_timeoutProc()
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
        case Wait_Finish_F1F1:
        {
            p_timer->stop();
            if(parallelCount>0)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("存在%1条并发没有上行的情况").arg(parallelCount));
                emScriptRunState=ScriptSuccess;
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("当前F1F1并发抄表成功率：%1%;").arg(calSuccessRate()*100));
                if(isAllMeterReadSuccess())
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Success, QString("F1F1并发抄表成功率：%1%; 抄表平均耗时：%2秒;").arg(calSuccessRate()*100).arg(calCostTime()));
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("失败电表清单如下%1\n").arg(getFailMeterNo()));
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("存在%0条并发没有上行的情况，F1F1并发抄表成功率：%1%; 抄表平均耗时：%2秒;").arg(parallelCount).arg(calSuccessRate()*100).arg(calCostTime()));

                }
            }
            else
            {
                emScriptRunState=ScriptSuccess;
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("当前F1F1并发抄表成功率：%1%;").arg(calSuccessRate()*100));
                if(isAllMeterReadSuccess())
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Success, QString("F1F1并发抄表成功率：%1%; 抄表平均耗时：%2秒;").arg(calSuccessRate()*100).arg(calCostTime()));
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("失败电表清单如下%1\n").arg(getFailMeterNo()));
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("F1F1并发抄表成功率：%1%; 抄表平均耗时：%2秒;").arg(calSuccessRate()*100).arg(calCostTime()));

                }
            }
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

void Script_MinuteCollect_CalibrationTime::maxAllowTimer_timeoutProc()
{
    switch(emScriptRunState)
    {
        case Wait_Finish_F1F1:
        {
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "Wait_Finish_F1F1 timeout!!!"+QString("  F1F1抄表成功率：%1%").arg(calSuccessRate()*100));
        p_AbstractScriptHost->updateProgress(ProcessState_Processing,getFailMeterNo());
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

void Script_MinuteCollect_CalibrationTime::delayTimer_timeoutProc()
{
    p_delayTimer->stop();
    if(emScriptRunState==Wait_00F1_for_12F2_Pause)
    {
        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_RouterPause_12F2);
        emScriptRunState=Wait_00F1_for_12F2_Pause;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--路由暂停（12F2），等待--确认");
        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
    }
    else if(emScriptRunState==Wait_QueryRouterState_Finish)
    {
        emScriptRunState=Wait_QueryRouterState_Finish;
        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryRouterState_10F4);
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询路由运行状态（10F4），等待--回复");
        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
    }
    else if(emScriptRunState==Wait_Finish_F1F1)
    {
        emSendOopProtype=pro_ActionRequest_RollCallRepor;
        //**通过自定义int dvcNo传入dvcId
        sendMsg(CCO_GW,dvcNo,meterIndex,p_ParallelReadMeter_F1F1);
        parallelCount++;
        meterIndex++;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--并发抄表（F1F1），等待--抄表完成");
    }
    else
    {
        index=0;
        p_maxAllowTimer->stop();
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, getFailMeterNo());

        if(calSuccessRate()>=netSucRateThresld)
        {
            emScriptRunState=ScriptSuccess;
            resultFlag=true;
            p_AbstractScriptHost->updateProgress(ProcessState_Success, QString("  F1F1抄表成功率：%1%; F1F1抄表平均耗时：%2秒;").arg(calSuccessRate()*100).arg(calCostTime()));
        }
        else
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, getFailMeterNo());
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("  F1F1抄表成功率：%1%; F1F1抄表平均耗时：%2秒;").arg(calSuccessRate()*100).arg(calCostTime()));
        }
    }
}

void Script_MinuteCollect_CalibrationTime::readInfoInit()
{
    struct DataID_Struct
    {
        uchar dataID[4];
    };

    QList<DataID_Struct> dataIDList;
    DataID_Struct dataID_ST0;
    memcpy(dataID_ST0.dataID,PosActEne_Total,4);
    DataID_Struct dataID_ST1;
    memcpy(dataID_ST1.dataID,Last_DailyFre_ActEne,4);
    DataID_Struct dataID_ST2;
    memcpy(dataID_ST2.dataID,Last_DailyFre_NegActEne,4);
    DataID_Struct dataID_ST3;
    memcpy(dataID_ST3.dataID,Last_DailyFre_Time,4);
    DataID_Struct dataID_ST4;
    memcpy(dataID_ST4.dataID,Volt_A,4);
    DataID_Struct dataID_ST5;
    memcpy(dataID_ST5.dataID,Volt_Blck,4);
    DataID_Struct dataID_ST6;
    memcpy(dataID_ST6.dataID,Crnt_A,4);
    DataID_Struct dataID_ST7;
    memcpy(dataID_ST7.dataID,Crnt_Blck,4);
    DataID_Struct dataID_ST8;
    memcpy(dataID_ST8.dataID,ActPower_Total,4);
    DataID_Struct dataID_ST9;
    memcpy(dataID_ST9.dataID,ActPower_A,4);
    DataID_Struct dataID_ST10;
    memcpy(dataID_ST10.dataID,ActPower_Blck,4);
    DataID_Struct dataID_ST11;
    memcpy(dataID_ST11.dataID,MeterType,4);
    DataID_Struct dataID_ST12;
    memcpy(dataID_ST12.dataID,DateAndWeek,4);
    DataID_Struct dataID_ST13;
    memcpy(dataID_ST13.dataID,Time645,4);
    DataID_Struct dataID_ST14;
    memcpy(dataID_ST14.dataID,CapOpenTimes,4);
    DataID_Struct dataID_ST15;
    memcpy(dataID_ST15.dataID,PhaseAngle_A,4);
    DataID_Struct dataID_ST16;
    memcpy(dataID_ST16.dataID,PhaseAngle_Blck,4);
    DataID_Struct dataID_ST17;
    memcpy(dataID_ST17.dataID,PhaseAngle_B,4);
    DataID_Struct dataID_ST18;
    memcpy(dataID_ST18.dataID,PhaseAngle_C,4);
    dataIDList<<dataID_ST0<<dataID_ST1<<dataID_ST2<<dataID_ST3<<dataID_ST4<<dataID_ST5
                <<dataID_ST6<<dataID_ST7<<dataID_ST8<<dataID_ST9<<dataID_ST10<<dataID_ST11
                  <<dataID_ST12<<dataID_ST13<<dataID_ST14<<dataID_ST15<<dataID_ST16<<dataID_ST17;
      //当前正向、日冻结正向、日冻结反向、日冻结时间、A相电压、电压数据块
    //A相电流、电流数据块、瞬时总有功功率、瞬时A相有功功率、瞬时有功功率数据块、电表型号
    //日期及星期、时间、开表盖总次数、A相相角、相角数据块、B相相角、C相相角
    readInfoList.clear();
    if(dataIdIndexList.isEmpty())
    {
        while(dataIDList.size()>maxFrameNum)
        {
            dataIDList.removeLast();
        }
    }
    else
    {
        QList<DataID_Struct> dataIDList1;
        for(int item=0;item<dataIdIndexList.size();item++)
        {
            dataIDList1.append(dataIDList.at(dataIdIndexList.at(item)));
        }
        dataIDList=dataIDList1;
    }
    for(int i=0;i<p_CtrInfoList->size();i++)
    {
        QList<MeterInfoForSingleNet*> meterInfoList=p_CtrInfoList->at(i)->p_MeterInfoForSingleNetList->values();
        for(int j=0;j<meterInfoList.size();j++)
        {
            ReadInfo_F1F1 readInfo_ST;
            memcpy(readInfo_ST.meterNo.addr,meterInfoList.at(j)->mtrAddr,6);
            readInfo_ST.protocolType=meterInfoList.at(j)->prtcl;
            readInfo_ST.readFlag=Reading;
            if(readInfo_ST.protocolType==0x02)
            {
                for(int n=0;n<dataIDList.size();n++)
                {
                    ReadDataUnit readData;
                    char id[4];
                    memcpy(id,dataIDList.at(n).dataID,4);
                    readData.dataID.append(QByteArray(id,4));
                    readData.notRead=true;
                    readData.costTime=0.0;
                    readInfo_ST.dataUnitList.append(readData);
                }
            }
            else if(readInfo_ST.protocolType==0x03)
            {
                ReadDataUnit readData;
                char id[4]={0x00,0x10,0x02,0x00};
                readData.dataID.append(QByteArray(id,4));
                readData.notRead=true;
                readData.costTime=0.0;
                readInfo_ST.dataUnitList.append(readData);
            }
            readInfoList.append(readInfo_ST);
        }
    }
}

bool Script_MinuteCollect_CalibrationTime::isMeterExist(Address address)
{
    for(int i=0;i<readInfoList.size();i++)
    {
        if(address==readInfoList.at(i).meterNo)
        {
            return true;
        }
    }
    return false;
}

int Script_MinuteCollect_CalibrationTime::getReadInfo(Address address)
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

double Script_MinuteCollect_CalibrationTime::calSuccessRate()
{
    double successCount=0.0;
    for(int i=0;i<readInfoList.size();i++)
    {
        if(readInfoList.at(i).readFlag==ReadSuccess)
            successCount++;
    }
    return successCount/double(readInfoList.size());
}

QString Script_MinuteCollect_CalibrationTime::calCostTime()
{
    double totalConsume=0.0;
    double successCount=0.0;
    for(int i=0;i<readInfoList.size();i++)
    {
        if(readInfoList.at(i).readFlag==ReadSuccess)
        {
            double singleConsume=0.0;
            successCount++;
            for(int j=0;j<readInfoList.at(i).dataUnitList.size();j++)
            {
                singleConsume+=readInfoList.at(i).dataUnitList.at(j).costTime;
            }
            totalConsume+=singleConsume/double(readInfoList.at(i).dataUnitList.size());
        }
    }
    return QString::number(totalConsume/successCount,'g',3);
}

QString Script_MinuteCollect_CalibrationTime::getFailMeterNo()
{
    QString failMeterNo;
    int count=0;
    for(int i=0;i<readInfoList.size();i++)
    {
        if(readInfoList.at(i).readFlag!=ReadSuccess)
        {
            count++;
            failMeterNo+=QString(QByteArray(readInfoList.at(i).meterNo.addr,6).toHex())+";";
            if(count%8==0)
                failMeterNo+="\n";
        }

    }
    return failMeterNo;
}

bool Script_MinuteCollect_CalibrationTime::isAllRead(int meterID)
{
    bool sus=true;
    for(int i=0;i<readInfoList.at(meterID).dataUnitList.size();i++)
    {
        if(readInfoList.at(meterID).dataUnitList.at(i).notRead==true)
        {
            sus=false;
            break;
        }
    }
    return sus;
}

bool Script_MinuteCollect_CalibrationTime::isAllMeterReadSuccess()
{
    bool sus=true;
    for(int i=0;i<readInfoList.size();i++)
    {
        if(readInfoList.at(i).readFlag==Reading)
        {
            sus=false;
            break;
        }
    }
    return sus;
}

void Script_MinuteCollect_CalibrationTime::CalcAvrgConsumeTimeLen(uchar rdFlag)
{
    if(rdFlag!=1 && rdFlag!=2 && rdFlag!=3)
        return;
    double totalConsume=0.0;
    for(int i=0; i<p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().size(); i++)
    {
        if(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(i)->testResultList[rdFlag])
            totalConsume+=p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(i)->timeConsumList[rdFlag];
    }
    if(p_CtrInfoList->at(0)->successCnt[rdFlag]!=0)
        p_CtrInfoList->at(0)->successConsume[rdFlag]=totalConsume/(p_CtrInfoList->at(0)->successCnt[rdFlag]);
    if(rdFlag==2)
    {
        totalConsume=double(timerForReachThresld*1000-p_maxAllowTimer->remainingTime())/1000.0;
        if(p_CtrInfoList->at(0)->successCnt[rdFlag]!=0)
            p_CtrInfoList->at(0)->successConsume[rdFlag]=totalConsume/(p_CtrInfoList->at(0)->successCnt[rdFlag]);
    }
}
