#include "Script_CommAddrRequest_1.h"
#include <QDataStream>
#include <QDir>

Script_CommAddrRequest_1::Script_CommAddrRequest_1(QObject *parent) : QObject(parent)
{
    p_CtrInfoList = make_shared<QList<shared_ptr<CtrInfo>>>();

    sendMsgOct.clear();

    p_MsgBase_645=make_shared<dlt_645_Protocol::Frame645Helper>();
//    p_MeterAddrResp_93=make_shared<dlt_645_Protocol::RspsNormal_ReadAddr_0x93>(addr,6);
    p_MsgBase_698_45=make_shared<object_oriented_electic_data_exchange_protocol::FrameOOPHelper>();
//    p_GetResponseNormal_ReadAddr=make_shared<object_oriented_electic_data_exchange_protocol::GetResponseNormal>();

    p_timer=make_shared<QTimer>(this);
    p_maxAllowTimer=make_shared<QTimer>(this);
    connect(p_timer.get(),SIGNAL(timeout()),this,SLOT(timer_timeoutProc()));
    connect(p_maxAllowTimer.get(),SIGNAL(timeout()),this,SLOT(maxAllowTimer_timeoutProc()));
}

Script_CommAddrRequest_1::~Script_CommAddrRequest_1()
{
    p_timer->stop();
    p_maxAllowTimer->stop();

    powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost);
}

QList<int> Script_CommAddrRequest_1::getDvcIdList(DvcType dvcType)
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

void  Script_CommAddrRequest_1::execute()
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

void  Script_CommAddrRequest_1::stop()
{
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "Test stop!");
}

void Script_CommAddrRequest_1::addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq)
{
    p_CtrInfoList->clear();
    comnAddAddrsInfo(p_CtrInfoList, p_ConcentratorInfoList, p_MeterInfoList, p_SchemeCfgInfoList);
 //   concentratorCnt=ushort(p_CtrInfoList->size());

   // uchar dstFreq=freq&0x0f;
    uchar dstPrtcl=(freq>>4)&0x0f;
    if(dstPrtcl==0x03)
        setMeterAddrsPrtcl(p_CtrInfoList,dstPrtcl);

 //   p_BuildNetwork->setCtrInfoListAndFreq(p_CtrInfoList, dstFreq);
}

void  Script_CommAddrRequest_1::setHost(AbstractScriptHost *host)
{
    p_AbstractScriptHost=host;
 //   p_BuildNetwork->setHost(host);
}

bool  Script_CommAddrRequest_1::config(const QMap<QString,QString> *paraDic)
{
    bool result = false;
    if(paraDic!=nullptr)
    {
       // p_BuildNetwork->config(paraDic);

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

void Script_CommAddrRequest_1::processMsg(DvcType dvcType, int id, uchar *data, int datalen)
{
    QByteArray tmpRecvTempData=QByteArray::fromRawData(reinterpret_cast<char*>(data),datalen);
    QByteArray recvTempData;
    recvTempData.append(tmpRecvTempData);
    delete[]data;

    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到报文：%1").arg(QString(recvTempData.toHex())));

    if(p_CtrInfoList->size()==0)
        return;

    if(dvcType==CCO_GW)
    {
        //            p_CtrInfoList->at(0)->buf.append(recvTempData);
        //            processMsgFromCco(id);
    }
    else if(dvcType==SingleSTA || dvcType==ThreeSTA)
    {
        if(false==(*p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList).contains(id))
            return;

        if(emScriptRunState==Wait_StaReadAddr_645)
        {
            (*p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList)[id]->buf645.append(recvTempData);
            processMsgFromMeter645(id);
        }
        else if(emScriptRunState==Wait_StaReadAddr_OOP)
        {
            (*p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList)[id]->buf698.append(recvTempData);
            processMsgFromMeter698(id);
        }
    }
    else
    {
        return;
    }
}

void Script_CommAddrRequest_1::processCtrlDvcRes(DvcType dvcType, QList<int> idList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params)
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
            emScriptRunState=Wait_StaReadAddr_645;
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

void  Script_CommAddrRequest_1::timer_timeoutProc()
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
    case Wait_StaReadAddr_645:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_StaReadAddr_645 timeout!!!");
        emScriptRunState=ScriptComplete;
        break;
    }
    case Wait_StaReadAddr_OOP:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_StaReadAddr_OOP timeout!!!");
        emScriptRunState=ScriptComplete;
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

void  Script_CommAddrRequest_1::maxAllowTimer_timeoutProc()
{
    switch(emScriptRunState)
    {
   /* case Wait_StaReadAddr:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait StaReadAddr timeout!!!");
        emScriptRunState=ScriptComplete;
        break;
    }*/
    default:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==p_maxAllowTimer");
        emScriptRunState=ScriptComplete;
        break;
    }
    }
}

void Script_CommAddrRequest_1::processMsgFromMeter645(int id)
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
            p_timer->stop();
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "收到645读表号报文，等待OOP读表号");
            emScriptRunState=Wait_StaReadAddr_OOP;
            p_timer->start(20*1000);
            //staReadAddrFlag=1;
           // sendMsg(SingleSTA,id,p_MeterAddrResp_93);
        }
        else
        {/*
            uchar di[4]={0x00};
            if(MsgBase_645_ptr->ctrlCode_==READ_DATA)
            {
                shared_ptr<Rqst_ReadData_0x11> Rqst_ReadData_0x11_ptr = std::dynamic_pointer_cast<Rqst_ReadData_0x11>(MsgBase_645_ptr);
                memcpy(di,Rqst_ReadData_0x11_ptr->di,4);

                QByteArray tmpSendMsg=prcsOther645Msg(MsgBase_645_ptr->ctrlCode_,MsgBase_645_ptr->dataLen_,di,MsgBase_645_ptr->addr_,(*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[id]->mtrAddr,p_CtrInfoList);
                sendMsg(SingleSTA,id,tmpSendMsg);
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther645Msg(MsgBase_645_ptr->ctrlCode_,MsgBase_645_ptr->dataLen_,di,MsgBase_645_ptr->addr_,(*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[id]->mtrAddr,p_CtrInfoList);
                sendMsg(SingleSTA,id,tmpSendMsg);
            }*/
        }
    }
}

void Script_CommAddrRequest_1::processMsgFromMeter698(int id)
{
    bool haveCompleteMsg=true;
    while(haveCompleteMsg)
    {
        qInfo()<<QString("id=%1; 解析前 buf698=%2").arg(id).arg(QString((*p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList)[id]->buf698.toHex()));
        shared_ptr<FrameOOPBase> MsgBase_698_ptr=p_MsgBase_698_45->DecodeMsg(&((*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[id]->buf698),haveCompleteMsg);
        qInfo()<<QString("id=%1; 解析后 buf698=%2").arg(id).arg(QString((*p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList)[id]->buf698.toHex()));

        if(MsgBase_698_ptr==nullptr)
        {
            break;
        }

        if(MsgBase_698_ptr->service_type_==GET_REQUEST_CLIENT && MsgBase_698_ptr->service_sub_type_==uchar(GetRequestType::kGetRequestNormal))
        {
            OAD oad;
            oad.OI=ComuAddr;
            oad.attribute.feature=0;
            oad.attribute.seq=2;
            oad.element_index=0;
            shared_ptr<GetRequestNormal> p_IC_GetRequestNormal = std::dynamic_pointer_cast<GetRequestNormal>(MsgBase_698_ptr);

            if(p_IC_GetRequestNormal->oad_==oad)
            {
                p_timer->stop();
                p_AbstractScriptHost->updateProgress(ProcessState_Success, "收到645、OOP读表号报文");
                emScriptRunState=ScriptComplete;
               // sendMsg(SingleSTA,id,p_GetResponseNormal_ReadAddr);
            }
            else
            {
              //  QByteArray tmpSendMsg=prcsOther698Msg(MsgBase_698_ptr,(*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[id]->mtrAddr,p_CtrInfoList);
               // sendMsg(SingleSTA,id,tmpSendMsg);
            }
        }
        else
        {
           // QByteArray tmpSendMsg=prcsOther698Msg(MsgBase_698_ptr,(*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[id]->mtrAddr,p_CtrInfoList);
          //  sendMsg(SingleSTA,id,tmpSendMsg);
        }
    }
}
