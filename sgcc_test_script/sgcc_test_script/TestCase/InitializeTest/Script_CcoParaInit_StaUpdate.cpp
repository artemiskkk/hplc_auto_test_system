#include "Script_CcoParaInit_StaUpdate.h"
#include <QDataStream>
#include <QDir>
#include <QApplication>

Script_CcoParaInit_StaUpdate::Script_CcoParaInit_StaUpdate(QObject *parent) : QObject(parent)
{
    emScriptRunState=ScriptInit;
    p_CtrInfoList = make_shared<QList<shared_ptr<CtrInfo>>>();

    p_BuildNetwork=make_shared<BuildNetwork_GW>();
    p_MsgBase_1376_2=make_shared<qgdw_3762_protocol::Frame3762Helper>();
    p_FileTransfer_15F1_Down=make_shared<qgdw_3762_protocol::Afn15F1>();
    p_CcoRunStateInfo_10F4_Down=make_shared<qgdw_3762_protocol::Afn10F4>();
    p_CcoRunModeInfo_03F10_Down=make_shared<qgdw_3762_protocol::Afn03F10>();
    p_CcoCtrlPause_12F2=make_shared<qgdw_3762_protocol::Afn12F2>();
    p_StopSlaveNodeReg_11F6=make_shared<qgdw_3762_protocol::Afn11F6>();
    p_ParameterInit_01F2=make_shared<Afn01F2>();
    sendMsgOct.clear();

    p_MsgBase_645=make_shared<dlt_645_Protocol::Frame645Helper>();
    p_MeterAddrResp_93=make_shared<dlt_645_Protocol::RspsNormal_ReadAddr_0x93>(addr,6);
    p_MsgBase_698_45=make_shared<object_oriented_electic_data_exchange_protocol::FrameOOPHelper>();
    p_GetResponseNormal_ReadAddr=make_shared<object_oriented_electic_data_exchange_protocol::GetResponseNormal>();

    dataLen=0;
    totalSegs=0;
    transResList.clear();
    cfgCntPerTime=10;
    msgSeq=0;

    /***可配置参数***/
    timerForReachThresld=1800; //单位:s
    netSucRateThresld=1.0;
    timerAfterReachThresld=300; //单位:s

    timerForReachThresld_Upgrade=1200; //单位:s
    sucRateThresld_Upgrade=1.0;
    timerAfterTransferFinished=3600; //单位:s

    timerForReachThresld_QueryNodeVrsnInfo02F1=1200; //单位:s
    sucRateThresld_QueryNodeVrsnInfo02F1=1.0;
    timerAfterReachThresld_QueryNodeVrsnInfo02F1=180; //单位:s

    needBuildNet=true;
    dstUpgradeDvc=1;

    staOutVrsn.clear();
    staInVrsn.clear();

    p_timer=make_shared<QTimer>(this);
    p_send10F4Timer=make_shared<QTimer>(this);
    p_maxAllowTimer=make_shared<QTimer>(this);
    p_delayTimer=new QTimer(this);
    connect(p_timer.get(),SIGNAL(timeout()),this,SLOT(timer_timeoutProc()));
    connect(p_send10F4Timer.get(),SIGNAL(timeout()),this,SLOT(timer_send10F4TimeoutProc()));
    connect(p_maxAllowTimer.get(),SIGNAL(timeout()),this,SLOT(maxAllowTimer_timeoutProc()));
    connect(p_delayTimer,SIGNAL(timeout()),this,SLOT(delayTimer_timeoutProc()));
}

Script_CcoParaInit_StaUpdate::~Script_CcoParaInit_StaUpdate()
{
    p_timer->stop();
    p_maxAllowTimer->stop();
    p_send10F4Timer->stop();
    powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost);
}

void  Script_CcoParaInit_StaUpdate::execute()
{
    p_AbstractScriptHost->updateProgress(ProcessState_Start, "Test start!");
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "脚本执行流程描述打印：-----whc(2024年1月20)\r\n"
                                                                  "1.组网通用流程(需要组网) Wait_BuildNetFinish_Whole\n"
                                                                  "2.10F4查询路由运行状态 Wait_QueryRouterRunState_10F4\n"
                                                                  "3.调入文件进行文件传输 Wait_00F1_for_12F2_UpgrdSta，Wait_00F1_for_11F6_UpgrdSta，Wait_15F1_for_15F1_BeforeUpgrdSta，Wait_Res_for_10F4_BeforeUpgrdSta，Wait_FileTransferFinish，Wait_Res_for_03F10_WaitTimeLen，Wait_StaUpgradeFinish\n"
                                                                  "4.CCO参数初始化，10F4查询路由当前状态,Wait_CcoParaInit_01F2_Finish\n"
                                                                  "5.测试结束 ScriptSuccess\n");

    emScriptRunState=ScriptInit;
    if(needBuildNet==true)
    {
        p_BuildNetwork->execute();
        emScriptRunState=Wait_BuildNetFinish_Whole;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("等待全网组网完成"));
        p_timer->start((timerForReachThresld+timerAfterReachThresld)*1000);
    }
    else
    {
        if(isStdPrcs)
        {
            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,0,p_CcoCtrlPause_12F2);
            emScriptRunState=Wait_00F1_for_12F2_UpgrdSta;
            p_timer->start(10*1000);
        }
        else
        {
            /****调入升级包并计算每段长度和总段数*****/
            LoadUpdateFile();
            fileIndex=0;
            emScriptRunState=Wait_FileTransferFinish;
            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,0,p_FileTransfer_15F1_Down);
            qDebug()<<"发送文件传输报文"<<"段值索引："<<fileIndex;
            p_timer->start(10*1000);
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "等待升级文件传输完成");
            p_maxAllowTimer->start(timerForReachThresld_Upgrade*1000);
        }
    }
}

void  Script_CcoParaInit_StaUpdate::stop()
{
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "Test stop!");
}

void Script_CcoParaInit_StaUpdate::addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq)
{
    p_CtrInfoList->clear();
    comnAddAddrsInfo(p_CtrInfoList, p_ConcentratorInfoList, p_MeterInfoList, p_SchemeCfgInfoList);
    concentratorCnt=ushort(p_CtrInfoList->size());

    uchar dstFreq=freq&0x0f;
    uchar dstPrtcl=(freq>>4)&0x0f;
    if(dstPrtcl==0x03)
        setMeterAddrsPrtcl(p_CtrInfoList,dstPrtcl);

    p_BuildNetwork->setCtrInfoListAndFreq(p_CtrInfoList, dstFreq);
}

void  Script_CcoParaInit_StaUpdate::setHost(AbstractScriptHost *host)
{
    p_AbstractScriptHost=host;
    p_BuildNetwork->setHost(host);
}

bool  Script_CcoParaInit_StaUpdate::config(const QMap<QString,QString> *paraDic)
{
    bool result = false;
    if(paraDic!=nullptr)
    {
        p_BuildNetwork->config(paraDic);

        if(paraDic->keys().contains("timerForReachThresld"))
        {
            this->timerForReachThresld = (*paraDic)["timerForReachThresld"].toUShort();
        }
        if(paraDic->keys().contains("netSucRateThresld"))
        {
            this->netSucRateThresld = (*paraDic)["netSucRateThresld"].toDouble();
        }
        if(paraDic->keys().contains("timerAfterReachThresld"))
        {
            this->timerAfterReachThresld = (*paraDic)["timerAfterReachThresld"].toUShort();
        }
        if(paraDic->keys().contains("timerForReachThresld_Upgrade"))
        {
            this->timerForReachThresld_Upgrade = (*paraDic)["timerForReachThresld_Upgrade"].toUShort();
        }
        if(paraDic->keys().contains("sucRateThresld_Upgrade"))
        {
            this->sucRateThresld_Upgrade = (*paraDic)["sucRateThresld_Upgrade"].toDouble();
        }
        if(paraDic->keys().contains("timerAfterTransferFinished"))
        {
            this->timerAfterTransferFinished = (*paraDic)["timerAfterTransferFinished"].toUShort();
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
        if(paraDic->keys().contains("isStdPrcs"))
        {
            if((*paraDic)["isStdPrcs"].toLower()=="false")
            {
                this->isStdPrcs=false;
            }
            else
            {
                this->isStdPrcs=true;
            }
        }
        if(paraDic->keys().contains("dstUpgradeDvc"))
        {
            this->dstUpgradeDvc=(*paraDic)["dstUpgradeDvc"].toUShort();
        }
        if(paraDic->keys().contains("timerForReachThresld_QueryNodeVrsnInfo02F1"))
        {
            this->timerForReachThresld_QueryNodeVrsnInfo02F1 = (*paraDic)["timerForReachThresld_QueryNodeVrsnInfo02F1"].toUShort();
        }
        if(paraDic->keys().contains("sucRateThresld_QueryNodeVrsnInfo02F1"))
        {
            this->sucRateThresld_QueryNodeVrsnInfo02F1 = (*paraDic)["sucRateThresld_QueryNodeVrsnInfo02F1"].toDouble();
        }
        if(paraDic->keys().contains("timerAfterReachThresld_QueryNodeVrsnInfo02F1"))
        {
            this->timerAfterReachThresld_QueryNodeVrsnInfo02F1 = (*paraDic)["timerAfterReachThresld_QueryNodeVrsnInfo02F1"].toUShort();
        }
        if(paraDic->keys().contains("staVendorChipCode"))
        {
            staVendorChipCode.clear();
            this->staVendorChipCode=(*paraDic)["staVendorChipCode"];
        }
        result = true;
    }
    return result;
}

void Script_CcoParaInit_StaUpdate::processMsg(DvcType dvcType, int id, uchar *data, int datalen)
{
    if(emScriptRunState==Wait_BuildNetFinish_Whole)
    {
        if(!p_BuildNetwork->buildNetworkResultFlag)
            p_BuildNetwork->processMsg(dvcType,id,data,datalen);
        else
        {
            p_timer->stop();
            sendTimes=0;
            emScriptRunState = Wait_QueryRouterRunState_10F4;
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("Wait_QueryRouterRunState_10F4，组网完成，查询路由工作状态10F4"));
            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,0,p_CcoRunStateInfo_10F4_Down);
            p_timer->start(5*1000);
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

        if(dvcType==CCO_GW)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到路由报文：%1").arg(QString(recvTempData.toHex())));
            p_CtrInfoList->at(0)->buf.append(recvTempData);
            processMsgFromCco(id);
        }
        else if(dvcType==SingleSTA || dvcType==ThreeSTA)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到模块报文：%1").arg(QString(recvTempData.toHex())));
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

void Script_CcoParaInit_StaUpdate::processCtrlDvcRes(DvcType dvcType, QList<int> idList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params)
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
        p_BuildNetwork->processCtrlDvcRes(dvcType,idList,ctrlCmdType,isSucs,params);
        break;
    }
    case Wait_QueryRouterRunState_10F4:
    {
        break;
    }
    case Wait_00F1_for_12F2_UpgrdSta:
    {
        break;
    }
    case Wait_00F1_for_11F6_UpgrdSta:
    {
        break;
    }
    case Wait_15F1_for_15F1_BeforeUpgrdSta:
    {
        break;
    }
    case Wait_Res_for_10F4_BeforeUpgrdSta:
    {
        break;
    }
    case Wait_FileTransferFinish:
    {
        break;
    }
    case Wait_Res_for_03F10_WaitTimeLen:
    {
        break;
    }
    case Wait_StaUpgradeFinish:
    {
        break;
    }
    case Wait_CcoParaInit_01F2_Finish:
    {
        break;
    }
    default:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Error, QString("State machine run error!!!==p_timer, 状态机的值：")+emScriptRunState +test_name_);
        emScriptRunState=ScriptSuccess;
        break;
    }
    }
}

void  Script_CcoParaInit_StaUpdate::timer_timeoutProc()
{
    p_timer->stop();
    switch(emScriptRunState)
    {
    case Wait_BuildNetFinish_Whole:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString ("Wait build whole net finish timeout!!!,测试失败")+test_name_);
        emScriptRunState=ScriptSuccess;
        break;
    }
    case Wait_QueryRouterRunState_10F4:
    {
        if(++sendTimes>=3)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("Wait_QueryRouterRunState_10F4 timeout!!!") + test_name_);
        }
        else
        {
            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,0,p_CcoRunStateInfo_10F4_Down);
            qDebug()<<"定时器超时，重发10F4_Down******"<<sendMsgOct.toHex()<<"状态机的值："<<emScriptRunState;
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "超时重发10F4");
            p_timer->start(5*1000);
        }
        break;
    }
    case Wait_00F1_for_12F2_UpgrdSta:
    {
        if(++sendTimes>=3)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_00F1_for_12F2_UpgrdSta timeout!!!"+test_name_);
        }
        else
        {
            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,0,p_CcoCtrlPause_12F2);
            p_timer->start(5*1000);
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("超时，重发12F2暂停路由，等待--确认"));
        }
        break;
    }
    case Wait_00F1_for_11F6_UpgrdSta:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_11F6_UpgrdSta timeout!!!"+test_name_);
        emScriptRunState=ScriptSuccess;
        break;
    }
    case Wait_15F1_for_15F1_BeforeUpgrdSta:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_15F1_BeforeUpgrdSta timeout!!!"+test_name_);
        emScriptRunState=ScriptSuccess;
        break;
    }
    case Wait_Res_for_10F4_BeforeUpgrdSta:
    {
        if(++sendTimes>=3)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("Wait_Res_for_10F4_BeforeUpgrdSta timeout!!!") + test_name_);
        }
        else
        {
            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,0,p_CcoRunStateInfo_10F4_Down);
            qDebug()<<"定时器超时，重发10F4_Down******"<<sendMsgOct.toHex()<<"状态机的值："<<emScriptRunState;
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "超时重发10F4");
            p_timer->start(5*1000);
        }
        break;
    }
    case Wait_FileTransferFinish:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait Upgrade timeout!!!"+QString("文件索引%1").arg(fileIndex)+ test_name_);
        emScriptRunState=ScriptSuccess;
        break;
    }
    case Wait_Res_for_03F10_WaitTimeLen:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_Res_for_03F10_WaitTimeLen timeout!!!"+ test_name_);
        emScriptRunState=ScriptSuccess;
        break;
    }
    case Wait_StaUpgradeFinish:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_Res_for_10F4 timeout during Wait_StaUpgradeFinish!!!"+ test_name_);
        emScriptRunState=ScriptSuccess;
        break;
    }
    case Wait_CcoParaInit_01F2_Finish:
    {
        if(++sendTimes>=3)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_CcoParaInit_01F2_Finish timeout!!!"+ test_name_);
        }
        else
        {
            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,0,p_ParameterInit_01F2);
            qDebug()<<"定时器超时，重发01F2_Down******"<<sendMsgOct.toHex()<<"状态机的值："<<emScriptRunState;
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "超时重发01F2");
            p_timer->start(5*1000);
        }
        break;
    }
    default:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Error, QString("State machine run error!!!==p_timer, 状态机的值：")+emScriptRunState +test_name_);
        emScriptRunState=ScriptSuccess;
        break;
    }
    }
}

void Script_CcoParaInit_StaUpdate::timer_send10F4TimeoutProc()
{
    p_send10F4Timer->stop();
    switch(emScriptRunState)
    {
    case Wait_StaUpgradeFinish:
    {
        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,0,p_CcoRunStateInfo_10F4_Down);
        p_timer->start(5*1000);
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString(">>>>60s定时时间到，循环发送10F4查询路由运行状态"));
        break;
    }
    default:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Error, QString("State machine run error!!!==p_timer, 状态机的值：")+emScriptRunState + test_name_);
        emScriptRunState=ScriptSuccess;
        break;
    }
    }
}

void  Script_CcoParaInit_StaUpdate::maxAllowTimer_timeoutProc()
{
    p_maxAllowTimer->stop();
    switch(emScriptRunState)
    {
    case Wait_FileTransferFinish:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait Upgrade Finish timeout!!!");
        emScriptRunState=ScriptSuccess;
        break;
    }
    case ScriptSuccess:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Success, QString("定时时间到（非标准流程）  升级文件传输成功率：%1%; 升级文件传输耗时：%2秒;").arg(100).arg(p_CtrInfoList->at(0)->successConsume[0]));
        break;
    }
    default:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Error, QString("State machine run error!!!==p_maxAllowTimer,测试失败，当前状态机的值为：") + emScriptRunState + test_name_);
        emScriptRunState=ScriptSuccess;
        break;
    }
    }
}

void Script_CcoParaInit_StaUpdate::Refresh_TestResult_15F1(shared_ptr<Afn15F1> p_FileTransfer_15F1_Up)
{
   if(p_FileTransfer_15F1_Up->current_identify_==fileIndex)
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("第(%1)段传输完成;").arg(fileIndex));
        transResList[fileIndex]=true;
    }
}

ushort Script_CcoParaInit_StaUpdate::Refresh_SuccessCnt_15F1()
{
    ushort succSefgs=0;

    for(int i=0; i<p_FileTransfer_15F1_Down->file_transfer_unit_.total_num_; i++)
    {
        if(transResList[i]==true)
        {
            succSefgs+=1;
        }
    }

    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("TotalCnt=%1; SuccessCnt=%2; SuccessRate=%3%").arg(totalSegs).arg(succSefgs).arg(((double)succSefgs/(double)totalSegs)*100));
    return succSefgs;
}

void Script_CcoParaInit_StaUpdate::processMsgFromCco(int id)
{
    if(id!=p_CtrInfoList->at(0)->ctrlID)
        return;

    bool haveCompleteMsg=true;
    while(haveCompleteMsg)
    {
        qInfo()<<QString("id=%1; 解析前 buf3762=%2").arg(id).arg(QString((p_CtrInfoList->at(0)->buf.toHex())));
        // haveCompleteMsg=p_MsgBase_1376_2->decode_3762_MsgUp(&(p_CtrInfoList->at(0)->buf));
        shared_ptr<Frame3762Base> p_Frame3762Base = p_MsgBase_1376_2->DecodeLocalMsg(&(p_CtrInfoList->at(0)->buf),haveCompleteMsg);
        qInfo()<<QString("id=%1; 解析后 buf3762=%2").arg(id).arg(QString((p_CtrInfoList->at(0)->buf.toHex())));
        if(p_Frame3762Base==nullptr)
        {
            continue;
        }

        uchar dtValue3762 =get3762Dt(p_Frame3762Base->dt1_,p_Frame3762Base->dt2_);
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
        case Wait_QueryRouterRunState_10F4:
        {
            if(p_Frame3762Base->afn_ == 0x10 && dtValue3762==4 && p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                shared_ptr<Afn10F4> p_QueryRouterRunState_10F4_Up=dynamic_pointer_cast<Afn10F4>(p_Frame3762Base);
                uchar current_state =  uchar(p_QueryRouterRunState_10F4_Up->router_operate_state_unit_.work_switch_.current_state_);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString(">>>>收到10F40回复报文，期望当前状态为：3(路由当前状态为：11-其他)，实际收到值为：%1").arg(current_state));
                if (current_state == 3)
                {
                    sendTimes=0;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--12F2路由暂停命令，等待--回复");
                    emScriptRunState=Wait_00F1_for_12F2_UpgrdSta;
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,0,p_CcoCtrlPause_12F2);
                    p_timer->start(5*1000);

                }
                else
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString(">>>>Wait_QueryRouterRunState_10F4，10F4回复路由当前状态与期望状态不一致测试失败，期望当前状态为：3(路由当前状态为：11-其他)，实际收到值为：%1").arg(current_state));
            }
            break;
        }
        case Wait_00F1_for_12F2_UpgrdSta:
        {
            dtValue3762=get3762Dt(p_Frame3762Base->dt1_,p_Frame3762Base->dt2_);
            if(p_Frame3762Base->afn_ == 0x00&&dtValue3762==1&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                qDebug()<<"收到00F1_Up报文******"<<"状态机的值："<<emScriptRunState;
                p_timer->stop();
                emScriptRunState=Wait_15F1_for_15F1_BeforeUpgrdSta;
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,0,p_FileTransfer_15F1_Down);
                p_timer->start(20*1000);
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendMsg(CCO_GW,id,tmpSendMsg);
            }
            break;
        }
        case Wait_00F1_for_11F6_UpgrdSta:
        {
            dtValue3762=get3762Dt(p_Frame3762Base->dt1_,p_Frame3762Base->dt2_);
            if(p_Frame3762Base->afn_ == 0x00&&dtValue3762==1&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                qDebug()<<"收到00F1_Up报文******"<<"状态机的值："<<emScriptRunState;
                p_timer->stop();
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,0,p_FileTransfer_15F1_Down);
                emScriptRunState=Wait_15F1_for_15F1_BeforeUpgrdSta;
                p_timer->start(10*1000);
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendMsg(CCO_GW,id,tmpSendMsg);
            }
            break;
        }
        case Wait_15F1_for_15F1_BeforeUpgrdSta:
        {
            dtValue3762=get3762Dt(p_Frame3762Base->dt1_,p_Frame3762Base->dt2_);
            if(p_Frame3762Base->afn_ == 0x15&&dtValue3762==1&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                qDebug()<<"收到15F1_Up清除下装文件回复******"<<"状态机的值："<<emScriptRunState;
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到15F1_Up清除下装文件应答，查询10F4路由运行状态");
                p_timer->stop();

                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,0,p_CcoRunStateInfo_10F4_Down);
                emScriptRunState=Wait_Res_for_10F4_BeforeUpgrdSta;
                p_timer->start(10*1000);
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendMsg(CCO_GW,id,tmpSendMsg);
            }
            break;
        }
        case Wait_Res_for_10F4_BeforeUpgrdSta:
        {
            dtValue3762=get3762Dt(p_Frame3762Base->dt1_,p_Frame3762Base->dt2_);
            if(p_Frame3762Base->afn_ == 0x10&&dtValue3762==4&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                qDebug()<<"收到10F4_Up报文******"<<"状态机的值："<<emScriptRunState;
                p_timer->stop();
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到10F4_Up报文");
                shared_ptr<Afn10F4> p_CcoRunStateInfo_10F4_Up=std::dynamic_pointer_cast<Afn10F4>(p_Frame3762Base);
                if(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.operate_state_word_.report_work_flag_==1 || p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.work_switch_.current_state_!=3)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("路由运行状态错误：工作标志%1，当前状态%2").arg(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.operate_state_word_.report_work_flag_)
                                                                                      .arg(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.work_switch_.current_state_));
                }
                /****调入升级包并计算每段长度和总段数*****/
                LoadUpdateFile();
                fileIndex=0;
                emScriptRunState=Wait_FileTransferFinish;
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,0,p_FileTransfer_15F1_Down);
                p_timer->start(30*1000);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "开始15F1文件传输");
                p_maxAllowTimer->start(timerForReachThresld_Upgrade*1000);
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendMsg(CCO_GW,id,tmpSendMsg);
            }
            break;
        }
        case Wait_FileTransferFinish:
        {
            dtValue3762=get3762Dt(p_Frame3762Base->dt1_,p_Frame3762Base->dt2_);
            if(p_Frame3762Base->afn_ == 0x15&&dtValue3762==1&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                qDebug()<<"收到15F1_Up报文******"<<"状态机的值："<<emScriptRunState;
                p_timer->stop();
                shared_ptr<Afn15F1> p_FileTransfer_15F1_Up=std::dynamic_pointer_cast<Afn15F1>(p_Frame3762Base);
                Refresh_TestResult_15F1(p_FileTransfer_15F1_Up);

                ushort sucSegs=0;
                if(++fileIndex==p_FileTransfer_15F1_Down->file_transfer_unit_.total_num_)
                {
                    sucSegs=Refresh_SuccessCnt_15F1();
                }

                if(sucSegs==p_FileTransfer_15F1_Down->file_transfer_unit_.total_num_)
                {
                    p_CtrInfoList->at(0)->successConsume[0]=(double)(timerForReachThresld_Upgrade*1000-p_maxAllowTimer->remainingTime())/1000.0;
                    if(isStdPrcs)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("文件传输结束，成功率%1%，耗时%2秒，查询03F10本地通信模块运行模式信息").arg((double)(sucSegs)/(double)(totalSegs)*100).arg(p_CtrInfoList->at(0)->successConsume[0]));
                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,0,p_CcoRunModeInfo_03F10_Down);
                        emScriptRunState=Wait_Res_for_03F10_WaitTimeLen;
                        p_timer->start(10*1000);
                    }
                    else
                    {
                        emScriptRunState=ScriptSuccess;
                        p_maxAllowTimer->start(timerAfterTransferFinished*1000);
                    }
                }
                else
                {
                    if(fileIndex<totalSegs)
                    {
                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,0,p_FileTransfer_15F1_Down);
                        p_timer->start(30*1000);
                    }
                    else
                    {
                        p_maxAllowTimer->stop();
                        emScriptRunState=ScriptSuccess;
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("  升级文件传输成功率：%1%; 升级文件传输耗时：%2秒;").arg((double)(sucSegs)/(double)(totalSegs)*100).arg(p_CtrInfoList->at(0)->successConsume[0]));
                    }
                }
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendMsg(CCO_GW,id,tmpSendMsg);
            }
            break;
        }
        case Wait_Res_for_03F10_WaitTimeLen:
        {
            dtValue3762=get3762Dt(p_Frame3762Base->dt1_,p_Frame3762Base->dt2_);
            if(p_Frame3762Base->afn_ == 0x03&&dtValue3762==10&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                qDebug()<<"收到03F10_Up报文******"<<"状态机的值："<<emScriptRunState;
                p_timer->stop();
                emScriptRunState=Wait_StaUpgradeFinish;
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到03F10回复，开始等待模块升级，20s后查询10F4路由运行状态");
                p_send10F4Timer->start(20*1000);
                p_maxAllowTimer->start(timerAfterTransferFinished*1000);
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendMsg(CCO_GW,id,tmpSendMsg);
            }
            break;
        }
        case Wait_StaUpgradeFinish:
        {
            dtValue3762=get3762Dt(p_Frame3762Base->dt1_,p_Frame3762Base->dt2_);
            if(p_Frame3762Base->afn_ == 0x10&&dtValue3762==4&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                qDebug()<<"收到10F4_Up报文******"<<"状态机的值："<<emScriptRunState;
                p_timer->stop();
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到10F4_Up报文");
                shared_ptr<Afn10F4> p_CcoRunStateInfo_10F4_Up=std::dynamic_pointer_cast<Afn10F4>(p_Frame3762Base);

                QStringList rptEvntFlagList;
                QStringList workFlagList;
                QStringList ccoFinishFlagList;
                QStringList crntStateList;
                QStringList platIdentFlagList;
                QStringList evntRptStateFlagList;
                QStringList registerAllowStateList;
                QStringList workStateList;
                QStringList workStepList;

                rptEvntFlagList.clear();
                rptEvntFlagList<<"无从节点上报事件"<<"有从节点上报事件";
                workFlagList.clear();
                workFlagList<<"停止工作"<<"正在工作";
                ccoFinishFlagList.clear();
                ccoFinishFlagList<<"未完成"<<"路由学习完成";
                crntStateList.clear();
                crntStateList<<"抄表"<<"搜表"<<"升级"<<"其他";
                platIdentFlagList.clear();
                platIdentFlagList<<"不允许"<<"允许";
                evntRptStateFlagList.clear();
                evntRptStateFlagList<<"不允许"<<"允许";
                registerAllowStateList.clear();
                registerAllowStateList<<"不允许"<<"允许";
                workStateList.clear();
                workStateList<<"不允许"<<"允许";
                workStepList.clear();
                workStepList<<"未定义"<<"初始状态"<<"直抄"<<"中继"<<"监控状态"<<"广播状态"<<"召读电表"<<"侦听信息"<<"空闲";

                QString res=QString("路由运行状态信息如下：\n");
                res+=QString("[运行状态字]\r\n纠错编码: %1;\r\n上报事件标志: %2;\r\n工作标志: %3;\r\n路由完成标志: %4;\r\n")
                           .arg(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.operate_state_word_.error_correction_coding_)
                           .arg(rptEvntFlagList.at(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.operate_state_word_.report_event_flag_))
                           .arg(workFlagList.at(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.operate_state_word_.report_work_flag_))
                           .arg(ccoFinishFlagList.at(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.operate_state_word_.router_complete_flag_));
                res+=QString("从节点总数量: %1;\r\n")
                           .arg(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.node_total_num_);
                res+=QString("已抄从节点数量: %1;\r\n")
                           .arg(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.have_read_node_num_);
                res+=QString("中继抄到从节点数量: %1;\r\n")
                           .arg(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.read_by_relay_node_num_);
                res+=QString("[工作开关]\r\n当前状态: %1;\r\n台区识别使能标志: %2;\r\n事件上报状态标志: %3;\r\n注册允许标志: %4;\r\n工作状态: %5;\r\n")
                           .arg(crntStateList.at(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.work_switch_.current_state_))
                           .arg(platIdentFlagList.at(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.work_switch_.area_difference_flag_))
                           .arg(evntRptStateFlagList.at(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.work_switch_.event_report_flag_))
                           .arg(registerAllowStateList.at(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.work_switch_.register_allow_flag_))
                           .arg(workStateList.at(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.work_switch_.work_state));
                res+=QString("载波通信速率: %1;\r\n")
                           .arg(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.communication_rate_);
                res+=QString("第1相中继级别: %1;\r\n")
                           .arg(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.relay_level_phase_1_);
                res+=QString("第2相中继级别: %1;\r\n")
                           .arg(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.relay_level_phase_2_);
                res+=QString("第3相中继级别: %1;\r\n")
                           .arg(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.relay_level_phase_3_);
                res+=QString("第1相工作步骤: %1;\r\n")
                           .arg(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.work_procedure_phase_1_<9?workStepList.at(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.work_procedure_phase_1_):"备用");
                res+=QString("第2相工作步骤: %1;\r\n")
                           .arg(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.work_procedure_phase_2_<9?workStepList.at(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.work_procedure_phase_2_):"备用");
                res+=QString("第3相工作步骤: %1;\r\n")
                           .arg(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.work_procedure_phase_3_<9?workStepList.at(p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.work_procedure_phase_3_):"备用");

                p_AbstractScriptHost->updateProgress(ProcessState_Processing, res);

                uchar current_state = p_CcoRunStateInfo_10F4_Up->router_operate_state_unit_.work_switch_.current_state_;
                if(current_state == 2)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("路由当前状态为:%1,进行01F2参数初始化路由").arg(current_state));
                    p_maxAllowTimer->stop();
                    p_send10F4Timer->stop();
                    sendTimes=0;
                    emScriptRunState= Wait_CcoParaInit_01F2_Finish;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--参数初始化命令（01F2），等待--回复");
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_ParameterInit_01F2);
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "路由运行状态显示升级STA未完成，等待60秒后继续读取10F4");
                    p_send10F4Timer->start(60*1000);
                }
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendMsg(CCO_GW,id,tmpSendMsg);
            }
            break;
        }
        case Wait_CcoParaInit_01F2_Finish:
        {
            if(p_Frame3762Base->afn_== char(0x00) && dtValue3762==1 && p_Frame3762Base->ctrl_field_.dir==kDirUp)
            {

                p_timer->stop();
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>收到确认，01F2参数初始化成功。");
                sendTimes=0;
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, ">>>>发送--10F4查询路由运行状态，等待--确认\r\n");
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_CcoRunStateInfo_10F4_Down);
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            }
            else if(p_Frame3762Base->afn_ == 0x10 && dtValue3762==4 && p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                shared_ptr<Afn10F4> p_QueryWorkSwitch_10F4_Up=dynamic_pointer_cast<Afn10F4>(p_Frame3762Base);
                uchar router_complete_flag = uchar(p_QueryWorkSwitch_10F4_Up->router_operate_state_unit_.operate_state_word_.router_complete_flag_);
                uchar current_state =  uchar(p_QueryWorkSwitch_10F4_Up->router_operate_state_unit_.work_switch_.current_state_);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString(">>>>收到10F40查询路由运行状态报文，路由完成标志为：%1，当前状态为：%2").arg(router_complete_flag).arg(current_state));
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString(">>>>期望10F40查询回来路由运行状态，路由完成标志为：0(路由完成标志: 未完成;)"));
                if (router_complete_flag == 0)
                {
                    sendTimes = 0;
                    p_AbstractScriptHost->updateProgress(ProcessState_Success, QString("脚本执行成功，"+test_name_+"\r\n"));
                    p_maxAllowTimer->stop();
                    emScriptRunState=ScriptSuccess;
                }
                else
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString(">>>>Wait_CcoHardWareInit_01F1_Finish，10F4查回来状态与期望状态不一致测试失败,%1").arg(test_name_));

            }
            else
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing,"收到其他376.2报文，等待处理");
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendMsg(CCO_GW,id,tmpSendMsg);
            }
            break;
        }
        default:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==emScriptRunState");
            emScriptRunState=ScriptSuccess;
            break;
        }
        }
    }
}
void Script_CcoParaInit_StaUpdate::processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID)
{
    bool haveCompleteMsg=true;
    while(haveCompleteMsg)
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("收到645报文%1").arg(QString(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->value(mtrlID)->buf645.toHex())));
        shared_ptr<dlt_645_Protocol::Frame645Base> MsgBase_645_ptr = dlt_645_Protocol::Frame645Helper::DecodeLocalMsg(&((*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[mtrlID]->buf645),haveCompleteMsg);
        if(MsgBase_645_ptr==nullptr)
        {
            break;
        }
        switch(emScriptRunState)
        {
        case Wait_BuildNetFinish_Whole:
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
                    sendMsg(dvcType,dvcId,tmpSendMsg);
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther645Msg(MsgBase_645_ptr->ctrlCode_,MsgBase_645_ptr->dataLen_,di,MsgBase_645_ptr->addr_,(*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[mtrlID]->mtrAddr,p_CtrInfoList);
                    sendMsg(dvcType,dvcId,tmpSendMsg);
                }
            }
            break;
        }
        }
    }
}
void Script_CcoParaInit_StaUpdate::processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID)
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
        case Wait_BuildNetFinish_Whole:
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
                    sendMsg(dvcType,dvcId,tmpSendMsg);
                }
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther698Msg(MsgBase_OOP_ptr,(*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[mtrlID]->mtrAddr,p_CtrInfoList);
                sendMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        }
    }
}


void Script_CcoParaInit_StaUpdate::sendMsg(DvcType dvcType,int id,int meterID, shared_ptr<void> frame)
{
    sendMsgOct.clear();
    if(frame==p_ParameterInit_01F2)
    {
        p_ParameterInit_01F2->ctrl_field_.dir=kDirDown;
        p_ParameterInit_01F2->ctrl_field_.prm=kActive;
        p_ParameterInit_01F2->ctrl_field_.comn_type=kHplc;

        p_ParameterInit_01F2->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_ParameterInit_01F2->info_field_.info_field_down.comu_rate=0;
        p_ParameterInit_01F2->info_field_.info_field_down.comu_module_ident=0;

        sendMsgOct=p_ParameterInit_01F2->EncodeFrame();
        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("》》路由参数初始化01F2：%1\n").arg(QString(sendMsgOct.toHex().toUpper())));
    }
    else if(frame==p_CcoRunStateInfo_10F4_Down)
    {
        p_CcoRunStateInfo_10F4_Down->ctrl_field_={kHplc,kActive,kDirDown};
        p_CcoRunStateInfo_10F4_Down->info_field_.info_field_down.comu_module_ident=0;
        p_CcoRunStateInfo_10F4_Down->info_field_.info_field_down.msg_seq=char(msgSeq++);

        sendMsgOct=p_CcoRunStateInfo_10F4_Down->EncodeFrame();
        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("》》查询路由工作状态10F4：%1\n").arg(QString(sendMsgOct.toHex())));
    }
    else if(frame==p_CcoRunModeInfo_03F10_Down)
    {
        p_CcoRunModeInfo_03F10_Down->ctrl_field_={kHplc,kActive,kDirDown};
        p_CcoRunModeInfo_03F10_Down->info_field_.info_field_down.comu_module_ident=0;
        p_CcoRunModeInfo_03F10_Down->info_field_.info_field_down.msg_seq=char(msgSeq++);

        sendMsgOct=p_CcoRunModeInfo_03F10_Down->EncodeFrame();
        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("》》查询路由运行模式03F10：%1\n").arg(QString(sendMsgOct.toHex())));
    }
    else if(frame==p_CcoCtrlPause_12F2)
    {
        p_CcoCtrlPause_12F2->ctrl_field_={kHplc,kActive,kDirDown};
        p_CcoCtrlPause_12F2->info_field_.info_field_down.comu_module_ident=0;
        p_CcoCtrlPause_12F2->info_field_.info_field_down.msg_seq=char(msgSeq++);

        sendMsgOct=p_CcoCtrlPause_12F2->EncodeFrame();
        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("》》路由暂停12F2：%1\n").arg(QString(sendMsgOct.toHex())));
    }
    else if(frame==p_StopSlaveNodeReg_11F6)
    {
        p_StopSlaveNodeReg_11F6->ctrl_field_={kHplc,kActive,kDirDown};
        p_StopSlaveNodeReg_11F6->info_field_.info_field_down.comu_module_ident=0;
        p_StopSlaveNodeReg_11F6->info_field_.info_field_down.msg_seq=char(msgSeq++);

        sendMsgOct=p_StopSlaveNodeReg_11F6->EncodeFrame();
        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("》》终止从节点主动注册11F6：%1\n").arg(QString(sendMsgOct.toHex())));
    }
    else if(frame==p_FileTransfer_15F1_Down)
    {
        if(emScriptRunState==Wait_FileTransferFinish)
        {
            p_FileTransfer_15F1_Down->file_transfer_unit_.file_identify_=0x08;
            p_FileTransfer_15F1_Down->file_transfer_unit_.file_instruct_=0x00;
            if(fileIndex<totalSegs-1)
            {
                p_FileTransfer_15F1_Down->file_transfer_unit_.file_property_=0x00;
                p_FileTransfer_15F1_Down->file_transfer_unit_.file_length_=SEG_LEN_STA;
            }
            else
            {
                p_FileTransfer_15F1_Down->file_transfer_unit_.file_property_=0x01;
                p_FileTransfer_15F1_Down->file_transfer_unit_.file_length_=((dataLen%SEG_LEN_STA==0)?SEG_LEN_STA:(dataLen%SEG_LEN_STA));
            }
            p_FileTransfer_15F1_Down->file_transfer_unit_.this_identify_=fileIndex;
            p_FileTransfer_15F1_Down->file_transfer_unit_.file_content_=rawUpdateFile.mid(fileIndex*SEG_LEN_STA,p_FileTransfer_15F1_Down->file_transfer_unit_.file_length_);

            p_FileTransfer_15F1_Down->ctrl_field_={kHplc,kActive,kDirDown};
            p_FileTransfer_15F1_Down->info_field_.info_field_down.msg_seq=char(msgSeq++);

            sendMsgOct=p_FileTransfer_15F1_Down->EncodeFrame();
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("总段数=(%1); 该段文件长度=(%2); 该段文件索引=(%3); ").arg(p_FileTransfer_15F1_Down->file_transfer_unit_.total_num_).arg(p_FileTransfer_15F1_Down->file_transfer_unit_.file_length_).arg(p_FileTransfer_15F1_Down->file_transfer_unit_.this_identify_));
        }
        else
        {
            p_FileTransfer_15F1_Down->file_transfer_unit_.file_identify_=0x00;
            p_FileTransfer_15F1_Down->file_transfer_unit_.file_property_=0x00;
            p_FileTransfer_15F1_Down->file_transfer_unit_.file_length_=0;

            p_FileTransfer_15F1_Down->file_transfer_unit_.this_identify_=0;
            p_FileTransfer_15F1_Down->file_transfer_unit_.file_content_="";

            p_FileTransfer_15F1_Down->ctrl_field_={kHplc,kActive,kDirDown};
            p_FileTransfer_15F1_Down->info_field_.info_field_down.msg_seq=char(msgSeq++);

            sendMsgOct=p_FileTransfer_15F1_Down->EncodeFrame();
            p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("》》清除下装文件15F1：%1\n").arg(QString(sendMsgOct.toHex())));
        }
    }
    else if(frame==p_MeterAddrResp_93)
    {
        memcpy(p_MeterAddrResp_93->addr,(*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[meterID]->mtrAddr,6);
        memcpy(p_MeterAddrResp_93->addr_,(*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[meterID]->mtrAddr,6);

        sendMsgOct=p_MeterAddrResp_93->EncodeFrame();
        //sendMsgLog=QString("》》 读通信地址应答(0x93)：%1\n").arg(QString(sendMsgOct.toHex()));
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
        //   sendMsgLog=QString("》》 读通信地址应答(OOP)：%1\n").arg(QString(sendMsgOct.toHex()));
    }
    else
    {
        return;
    }
    uchar *sendMsg=new uchar[sendMsgOct.size()];
    memcpy(sendMsg,(uchar*)sendMsgOct.data(),sendMsgOct.size());
    p_AbstractScriptHost->sendMsg2Dvc(dvcType,id,sendMsg,sendMsgOct.size());
}

void Script_CcoParaInit_StaUpdate::sendMsg(DvcType dvcType, int dvcId, QByteArray msg)
{
    if(msg.size()<1)
    {
        return;
    }
    uchar *sendMsg=new uchar[static_cast<uint>(msg.size())];
    memcpy(sendMsg,reinterpret_cast<uchar*>(msg.data()),uint(msg.size()));
    p_AbstractScriptHost->sendMsg2Dvc(dvcType,dvcId,sendMsg,msg.size());

    //    p_AbstractScriptHost->sendMsg2Dvc(dvcType,dvcId,sendMsg,msg.size());
    //    uchar *sendMsg=new uchar[msg.size()];
    //    memcpy(sendMsg,(uchar*)msg.data(),msg.size());

    //    p_AbstractScriptHost->sendMsg2Dvc(dvcType,id,sendMsg,msg.size());
}

void Script_CcoParaInit_StaUpdate::LoadUpdateFile()
{
    QStringList fileNameFilters;
    QStringList fileList;
    QString path;

    if(1==dstUpgradeDvc)
    {
        path=tr("DataBase\\Upgrade\\模块程序(旧-新)");
    }
    else if(2==dstUpgradeDvc)
    {
        path=tr("DataBase\\Upgrade\\模块程序(新-新)");
    }
    else
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Error, "dstUpgradeDvc配置有误");
        return;
    }

    QDir *updateFileDir=new QDir(path);
    fileNameFilters << "*" ;
    QList<QFileInfo> *fileInfo=new QList<QFileInfo>(updateFileDir->entryInfoList(fileNameFilters,QDir::Files,QDir::NoSort));
    if(fileInfo->count() != 1)
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "获取到的STA升级文件数不为1："+QString::number(fileInfo->count()));
        return;
    }

    QString fileName = fileInfo->at(0).fileName();
    qDebug()<<fileName;
    delete fileInfo;
    delete updateFileDir;
    if(staOutVrsn.isEmpty() || staInVrsn.isEmpty())
    {
        if(fileName.left(6)!="TCC091" && fileName.left(6)!="TCC0A1" && fileName.left(6)!="TCD0A1"&& fileName.left(6)!="TCD0B1"&& fileName.left(6)!="GY2301")
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Error, "获取到的文件名头不是TCC091或TCC0A1或TCD0A1或TCD0B1或GY2301："+fileName.left(6));
            return;
        }
        if(fileName.right(3)!="bin" && fileName.right(3)!="dat")
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Error, "获取到的文件后缀名不是dat或bin："+fileName.right(3));
            return;
        }

        QString fileNamePart;
        for(int i=0;i<fileName.length();i++)
        {
            if(fileName[i]=='_')
            {
                fileNamePart = fileName.mid(i+1);
                break;
            }
        }
        qDebug()<<fileNamePart;

        if(staOutVrsn.isEmpty())
        {
            for(int i=0;i<fileNamePart.length();i++)
            {
                if(fileNamePart[i]=='V')
                {
                    if(fileNamePart[i+2]=='.')
                    {
                        staOutVrsn = fileNamePart.mid(0,6) + '0' + fileNamePart[i+1] + fileNamePart.mid(i+3,2);
                    }
                    else if(fileNamePart[i+3]=='.')
                    {
                        staOutVrsn = fileNamePart.mid(0,6) + fileNamePart.mid(i+1,2) + fileNamePart.mid(i+4,2);
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Error, "程序名称无法识别出外部版本号");
                        return;
                    }
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "获取到的文件名中STA外部版本："+staOutVrsn);
                    break;
                }
            }
        }
        //TCC091(SH)_190424_220118_181947V2.02(V2.10).bin/dat
        //TCC091(HN)_211224_220301_151426V43.02(V43.10).bin/dat
        if(staInVrsn.isEmpty())
        {
            for(int i=fileNamePart.length()-1;i>=0;i--)
            {
                if(fileNamePart[i]=='V')
                {
                    if(fileNamePart[i+2]=='.')
                    {
                        staInVrsn = fileNamePart.mid(7,6) + fileNamePart.mid(14,6) + '0' + fileNamePart[i+1] + QString("%1").arg(fileNamePart.mid(i+3,2).toInt(),2,16,QChar('0'));
                    }
                    else if(fileNamePart[i+3]=='.')
                    {
                        staInVrsn = fileNamePart.mid(7,6) + fileNamePart.mid(14,6) + QString("%1%2").arg(fileNamePart.mid(i+1,2).toInt(),2,16,QChar('0')).arg(fileNamePart.mid(i+4,2).toInt(),2,16,QChar('0'));
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Error, "程序名称无法识别出内部版本号");
                        return;
                    }
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "获取到的文件名中STA内部版本："+staInVrsn);
                    break;
                }
            }
        }
    }

    QString filePath=path+"\\"+fileName;
    char upgradeFileBuf[1024*1024];
    QFile file;
    file.setFileName(filePath);
    if(!file.open(QIODevice::ReadOnly))
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "打开STA升级文件失败!!!");
        return;
    }

    QDataStream in(&file);
    dataLen=in.readRawData(upgradeFileBuf,1024*1024);
    file.close();

    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("升级包大小=(%1);").arg(dataLen));
    rawUpdateFile=QByteArray::fromRawData(upgradeFileBuf,dataLen);

    totalSegs = (dataLen%SEG_LEN_STA==0)?dataLen/SEG_LEN_STA:(dataLen/SEG_LEN_STA+1);

    p_FileTransfer_15F1_Down->file_transfer_unit_.total_num_=totalSegs;
    for(int i=0; i<p_FileTransfer_15F1_Down->file_transfer_unit_.total_num_; i++)
    {
        transResList.append(false);
    }
    getVendorChipCode();
}

void Script_CcoParaInit_StaUpdate::getVendorChipCode()
{
    QSettings property("PropertyConfig.ini",QSettings::IniFormat);
    QString vendor;
    QString chip;
    property.beginGroup("SYSTEM_PROPERTY");
    vendor=property.value("STAVendorCoder").toString();
    chip=property.value("STAChipCode").toString();
    property.endGroup();
    if(!vendor.isEmpty()&&!chip.isEmpty())
    {
        staVendorChipCode=vendor+chip;
    }
    else
    {
        if(staVendorChipCode.isEmpty())
            staVendorChipCode="TCCA";
    }
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, staVendorChipCode);
}


void Script_CcoParaInit_StaUpdate::delayTimer_timeoutProc()
{
    p_delayTimer->stop();
    switch(emScriptRunState)
    {
    case Wait_BuildNetFinish_Whole:
    {
        break;
    }
    default:
    {
        break;
    }
    }
}
