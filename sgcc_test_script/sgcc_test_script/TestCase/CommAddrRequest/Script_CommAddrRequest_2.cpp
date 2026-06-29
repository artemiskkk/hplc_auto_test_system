#include "Script_CommAddrRequest_2.h"
#include <QDataStream>
#include <QDir>

Script_CommAddrRequest_2::Script_CommAddrRequest_2(QObject *parent) : QObject(parent)
{
    p_CtrInfoList = make_shared<QList<shared_ptr<CtrInfo>>>();

    sendMsgOct.clear();

    p_MsgBase_645=make_shared<dlt_645_Protocol::Frame645Helper>();
    p_MeterAddrResp_93=make_shared<dlt_645_Protocol::RspsNormal_ReadAddr_0x93>(addr,6);
    p_MeterDataRqst_11_Down=make_shared<dlt_645_Protocol::Rqst_ReadData_0x11>(addr,4,0);
  //  p_MsgBase_698_45=make_shared<object_oriented_electic_data_exchange_protocol::FrameOOPHelper>();
 //   p_GetResponseNormal_ReadAddr=make_shared<object_oriented_electic_data_exchange_protocol::GetResponseNormal>();

    p_timer=make_shared<QTimer>(this);
    p_maxAllowTimer=make_shared<QTimer>(this);
    connect(p_timer.get(),SIGNAL(timeout()),this,SLOT(timer_timeoutProc()));
    connect(p_maxAllowTimer.get(),SIGNAL(timeout()),this,SLOT(maxAllowTimer_timeoutProc()));
}

Script_CommAddrRequest_2::~Script_CommAddrRequest_2()
{
    p_timer->stop();
    p_maxAllowTimer->stop();

    powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost);
}

QList<int> Script_CommAddrRequest_2::getDvcIdList(DvcType dvcType)
{
    QList<int> dvcIdList;
    dvcIdList.clear();
 //   switch (dvcType)
  //  {
  //  case SingleSTA:
  //  {
    QList<int> tempIdList;
    QMap<int,MeterInfoForSingleNet*>::const_iterator const_it;
    for (const_it = (p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList)->constBegin(); const_it != (p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList)->constEnd(); const_it++)
    {
        if(const_it.value()->slotPosition == SingleSTA)
        {
            tempIdList.append(const_it.value()->dvcId);
        }
    }
    if(tempIdList.size() == 0)
    {
        qDebug()<<QString("单通槽位档案未找到");
    }
    foreach(int id,tempIdList)
    {
        if(!dvcIdList.contains(id))
        {
            dvcIdList.append(id);
        }
    }
  //      break;
   // }
    if(dvcIdList.size()==0)
    {
        dvcIdList.append(p_CtrInfoList->at(0)->dvcId);
    }
    return dvcIdList;
}

void  Script_CommAddrRequest_2::execute()
{
    p_AbstractScriptHost->updateProgress(ProcessState_Start, "Test start!");
 //   resultFlag=false;
    p_maxAllowTimer->start(maxAllowTime*1000);

    emScriptRunState=Init_SetBaudRate;
    sendParams.clear();
    idList.clear();
    sendParams.append(staBaudRate);
    idList = getDvcIdList(SingleSTA);
    p_AbstractScriptHost->controlDvc(SingleSTA,idList,CtrlCmd_SetBaudRate,sendParams);
    p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
    tryTimes=0;
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("设置STA单通波特率，等待--确认"));
}

void  Script_CommAddrRequest_2::stop()
{
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "Test stop!");
}

void Script_CommAddrRequest_2::addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq)
{
    p_CtrInfoList->clear();
    comnAddAddrsInfo(p_CtrInfoList, p_ConcentratorInfoList, p_MeterInfoList, p_SchemeCfgInfoList);
//   concentratorCnt=ushort(p_CtrInfoList->size());

   // uchar dstFreq=freq&0x0f;
    uchar dstPrtcl=(freq>>4)&0x0f;
    if(dstPrtcl==0x03)
        setMeterAddrsPrtcl(p_CtrInfoList,dstPrtcl);

//    p_BuildNetwork->setCtrInfoListAndFreq(p_CtrInfoList, dstFreq);
}

void  Script_CommAddrRequest_2::setHost(AbstractScriptHost *host)
{
    p_AbstractScriptHost=host;
  //  p_BuildNetwork->setHost(host);
}

bool  Script_CommAddrRequest_2::config(const QMap<QString,QString> *paraDic)
{
    bool result = false;
    if(paraDic!=nullptr)
    {
    //    p_BuildNetwork->config(paraDic);

        if(paraDic->keys().contains("maxAllowTime"))
        {
            this->maxAllowTime = (*paraDic)["maxAllowTime"].toUShort();
        }
        if(paraDic->keys().contains("staBaudRate"))
        {
            this->staBaudRate=(*paraDic)["staBaudRate"].toInt();
        }
        result = true;
    }
    return result;
}

void Script_CommAddrRequest_2::processMsg(DvcType dvcType, int id, uchar *data, int datalen)
{
    QByteArray tmpRecvTempData=QByteArray::fromRawData(reinterpret_cast<char*>(data),datalen);
    QByteArray recvTempData;
    recvTempData.append(tmpRecvTempData);
    delete[]data;

    if(p_CtrInfoList->size()==0)
        return;

    QString device;
    if(dvcType==CCO_GW)
        device="路由";
    else if(dvcType==SingleSTA)
        device="单通";
    else if(dvcType==ThreeSTA)
        device="三通";
    else if(dvcType==ReadCtrlDvc)
        device="抄控器";
    else
        device="未知设备";
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到%0的报文：%1").arg(device).arg(QString(recvTempData.toHex())));

    if(dvcType==CCO_GW)
    {
        //   p_CtrInfoList->at(0)->buf.append(recvTempData);
        //   processMsgFromCco(id);
    }
    else if(dvcType==SingleSTA || dvcType==ThreeSTA)
    {
        if(false==(*p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList).contains(id))
            return;

        if(emScriptRunState==Wait_StaReadAddr||emScriptRunState==Wait_645ReadMeter_Finish)
        {
            (*p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList)[id]->buf645.append(recvTempData);
            processMsgFromMeter645(id);
        }
        /*    else if((*p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList)[id]->prtcl==0x03)
            {
                (*p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList)[id]->buf698.append(recvTempData);
                processMsgFromMeter698(id);
            }*/
    }
    else if(dvcType==ReadCtrlDvc)
    {
        (*p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList)[id]->buf645.append(recvTempData);
        processMsgFromMeter645(id);
    }
    else
    {
        return;
    }
}

void Script_CommAddrRequest_2::processCtrlDvcRes(DvcType dvcType, QList<int> idList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params)
{
    if(isSucs==false)
        return;

    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到控制命令回复，参数个数=%1").arg(QString::number(params.size())));
    p_timer->stop();
    //QList<int> sendParams;
    switch(emScriptRunState)
    {
//    case Init_PowerOffSta:
//    {
//        if (dvcType == SingleSTA)
//        {
//            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("STA断电成功，等待90s开始用例..."));
//            p_timer->start(90*1000);
//        }
//        break;
//    }
    case Init_SetBaudRate:
    {
        if (dvcType == SingleSTA)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("单通槽位_设置波特率成功，12V上电"));
            emScriptRunState=Init_PowerOnSta;

            sendParams.clear();
            idList.clear();
            idList = getDvcIdList(SingleSTA);
            p_AbstractScriptHost->controlDvc(SingleSTA,idList,CtrlCmd_PowerOn_12V,sendParams);
            p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
            tryTimes=0;
        }
        break;
    }
    case Init_PowerOnSta:
    {
        if (dvcType == SingleSTA)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("单通上电成功，拉复位"));
            emScriptRunState=Init_ResetSta;

            sendParams.clear();
            idList.clear();
            idList = getDvcIdList(SingleSTA);
            p_AbstractScriptHost->controlDvc(SingleSTA,idList,CtrlCmd_ModuleRST,sendParams);
            p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
        }
        break;
    }
    case Init_ResetSta:
    {
        if (dvcType == SingleSTA)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("单通复位成功，等待645读表号"));
            emScriptRunState=Wait_StaReadAddr;
            p_timer->start(30*1000);
        }
        break;
    }
    case ScriptComplete:
    {
        break;
    }
    default:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==emScriptRunState");
        emScriptRunState=ScriptComplete;
        break;
    }
    }
}

void  Script_CommAddrRequest_2::timer_timeoutProc()
{
    p_timer->stop();
    switch(emScriptRunState)
    {
//    case Init_PowerOffSta:
//    {
//        emScriptRunState=Init_SetBaudRate;
//        sendParams.clear();
//        idList.clear();
//        sendParams.append(staBaudRate);
//        idList = getDvcIdList(SingleSTA);
//        p_AbstractScriptHost->controlDvc(SingleSTA,idList,CtrlCmd_SetBaudRate,sendParams);
//        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
//        tryTimes=0;
//        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("设置STA单通波特率，等待--确认"));
//        break;
//    }
    case Init_SetBaudRate:
    {
        if(++tryTimes>=3)
        {
         //   p_maxAllowTimer->stop();
          //  p_delayTimer->stop();
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_SetBaudRate_Finish timeout!!!");
            emScriptRunState=ScriptComplete;
        }
        else
        {
            p_AbstractScriptHost->controlDvc(SingleSTA,idList,CtrlCmd_SetBaudRate,sendParams);
            p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("重发设置单通波特率，等待--确认"));
        }
        break;
    }
    case Init_PowerOnSta:
    {
        if(++tryTimes>=3)
        {
         //   p_maxAllowTimer->stop();
          //  p_delayTimer->stop();
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_PowerOnSta_Finish timeout!!!");
            emScriptRunState=ScriptComplete;
        }
        else
        {
            p_AbstractScriptHost->controlDvc(SingleSTA,idList,CtrlCmd_PowerOn_12V,sendParams);
            p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("重发单通上电，等待--确认"));
        }
        break;
    }
    case Wait_StaReadAddr:
    {
        sendTimes=1;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "开始645厂商信息读取");
        emScriptRunState=Wait_645ReadMeter_Finish;
        sendMsg(SingleSTA,p_CtrInfoList->at(0)->ctrlID,p_MeterDataRqst_11_Down);
        p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
        break;
    }
    case Wait_645ReadMeter_Finish:
    {
        if(sendTimes==3)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "645商信息读取3次未回复");
            emScriptRunState=ScriptComplete;
        }
        else
        {
            sendTimes++;
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("重发645厂商信息读取，等待--"));
            sendMsg(SingleSTA,p_CtrInfoList->at(0)->ctrlID,p_MeterDataRqst_11_Down);
            p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
        }
        break;
    }
    default:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==p_timer");
        emScriptRunState=ScriptComplete;
        break;
    }
    }
}

void  Script_CommAddrRequest_2::maxAllowTimer_timeoutProc()
{
    switch(emScriptRunState)
    {
    default:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==p_maxAllowTimer");
        emScriptRunState=ScriptComplete;
        break;
    }
    }
}

void Script_CommAddrRequest_2::processMsgFromMeter645(int id)
{
    bool haveCompleteMsg=true;
    while(haveCompleteMsg)
    {
        qInfo()<<QString("id=%1; 解析前 buf645=%2").arg(id).arg(QString((*p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList)[id]->buf645.toHex()));
        shared_ptr<Frame645Base> MsgBase_645_ptr = p_MsgBase_645->DecodeLocalMsg(&((*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[id]->buf645),haveCompleteMsg);
        qInfo()<<QString("id=%1; 解析后 buf645=%2").arg(id).arg(QString((*p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList)[id]->buf645.toHex()));

        if(MsgBase_645_ptr==nullptr)
        {
            break;
        }

        if(MsgBase_645_ptr->ctrlCode_==READ_ADDR)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到645读表号报文，应答");
            //staReadAddrFlag=1;
            sendMsg(SingleSTA,id,p_MeterAddrResp_93);
        }
        else
        {
            uchar di[4]={0x00};
            if(MsgBase_645_ptr->ctrlCode_==READ_DATA)
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到645读数据报文，应答");
                shared_ptr<Rqst_ReadData_0x11> Rqst_ReadData_0x11_ptr = std::dynamic_pointer_cast<Rqst_ReadData_0x11>(MsgBase_645_ptr);
                memcpy(di,Rqst_ReadData_0x11_ptr->di,4);
                QByteArray tmpSendMsg=prcsOther645Msg(MsgBase_645_ptr->ctrlCode_,MsgBase_645_ptr->dataLen_,di,MsgBase_645_ptr->addr_,(*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[id]->mtrAddr,p_CtrInfoList);
                sendMsg(SingleSTA,id,tmpSendMsg);
            }
            else if(MsgBase_645_ptr->ctrlCode_==NORMAL_RESP)
            {
                p_timer->stop();
                shared_ptr<dlt_645_Protocol::RspsNormal_ReadData_0x91> p_RespData_91_Up=std::dynamic_pointer_cast<RspsNormal_ReadData_0x91>(MsgBase_645_ptr);
                if(MsgBase_645_ptr->dataLen_==19)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Success, "645测试成功");
                    emScriptRunState=ScriptComplete;
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, "645厂商信息读取回复报文数据域长度不为19："+p_RespData_91_Up->data.toHex());
                    emScriptRunState=ScriptComplete;
                }
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther645Msg(MsgBase_645_ptr->ctrlCode_,MsgBase_645_ptr->dataLen_,di,MsgBase_645_ptr->addr_,(*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[id]->mtrAddr,p_CtrInfoList);
                sendMsg(SingleSTA,id,tmpSendMsg);
            }
        }
    }
}

void Script_CommAddrRequest_2::sendMsg(DvcType dvcType,int id,shared_ptr<void> frame)
{
    sendMsgOct.clear();
    if(frame==p_MeterAddrResp_93)
    {
        memcpy(p_MeterAddrResp_93->addr,(*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[id]->mtrAddr,6);
        memcpy(p_MeterAddrResp_93->addr_,(*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[id]->mtrAddr,6);
        sendMsgOct=p_MeterAddrResp_93->EncodeFrame();
        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("》》<645>93回复表号：%1\n").arg(QString(sendMsgOct.toHex())));
    }
    else if(frame==p_MeterDataRqst_11_Down)
    {
        memcpy(p_MeterDataRqst_11_Down->addr_,(*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[id]->mtrAddr,6);
//        memcpy(p_MeterDataRqst_11_Down->di,dlt_645_Protocol::PosActEne_Blck,4);
        memcpy(p_MeterDataRqst_11_Down->di,dlt_645_Protocol::readVendorInfo_Blck,4);//DATA_ID_GET_VENDOR  读取厂商信息
        sendMsgOct=p_MeterDataRqst_11_Down->EncodeFrame();
        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("》》<645>11读取厂商信息：%1\n").arg(QString(sendMsgOct.toHex())));
    }
    else
    {
        return;
    }
    uchar *sendMsg=new uchar[sendMsgOct.size()];
    memcpy(sendMsg,reinterpret_cast<uchar*>(sendMsgOct.data()),uint(sendMsgOct.size()));
    p_AbstractScriptHost->sendMsg2Dvc(dvcType,id,sendMsg,sendMsgOct.size());
}

void Script_CommAddrRequest_2::sendMsg(DvcType dvcType, int id, QByteArray msg)
{
    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("》》报文发送：%1\n").arg(QString(msg.toHex())));
    uchar *sendMsg=new uchar[msg.size()];
    memcpy(sendMsg,reinterpret_cast<uchar*>(msg.data()),uint(msg.size()));
    p_AbstractScriptHost->sendMsg2Dvc(dvcType,id,sendMsg,msg.size());
}
