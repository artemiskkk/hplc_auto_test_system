#include "BaudConsult.h"

BaudConsult::BaudConsult(shared_ptr<QList<shared_ptr<CtrInfo> > > p_CtrInfoList, AbstractScriptHost *host, int baud, bool isCCO, BaudConsult::BaudConsultSpecialType type,QObject *parent)
    :QObject(parent),p_AbstractScriptHost(host),p_CtrInfoList(p_CtrInfoList),m_Baud(baud),m_SpecialType(type)
{
    p_Frame3762Helper=make_shared<Frame3762Helper>();
    p_Frame645Helper=make_shared<Frame645Helper>();
    p_MeterAddrResp_93=make_shared<RspsNormal_ReadAddr_0x93>(addr,6);
    p_FrameOOPHelper=make_shared<FrameOOPHelper>();
    p_GetResponseNormal_ReadAddr=make_shared<GetResponseNormal>();
    p_Afn02F1=make_shared<Afn02F1>();
    p_timer=new QTimer(this);
    p_maxAllowTimer=new QTimer(this);
    p_delayTimer=new QTimer(this);
    connect(p_timer,SIGNAL(timeout()),this,SLOT(timer_timeoutProc()));
    connect(p_maxAllowTimer,SIGNAL(timeout()),this,SLOT(maxAllowTimer_timeoutProc()));
    connect(p_delayTimer,SIGNAL(timeout()),this,SLOT(delayTimer_timeoutProc()));
    m_DvcType=(isCCO==true?CCO_GW:ReadCtrlDvc);
}
BaudConsult::~BaudConsult()
{

}

void BaudConsult::execute()
{
    p_AbstractScriptHost->updateProgress(ProcessState_Start, "波特率协商流程开始!");
    for(int i=0; i<p_CtrInfoList->at(0)->keyList.size(); i++)
    {
        if(SingleSTA == p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->value(p_CtrInfoList->at(0)->keyList.at(i))->slotPosition)
        {
            index=i;
            break;
        }
    }
    emScriptRunState=Wait_BaudConsult_Finish;
    sendMsg(m_DvcType,p_CtrInfoList->at(0)->ctrlID,index,p_Afn02F1);
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("发送波特率协商02F1，波特率%1bps，等待回复").arg(QString::number(m_Baud)));
    p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
}

void BaudConsult::processMsg(DvcType dvcType, int id, uchar *data, int datalen)
{
    QByteArray tmpRecvTempData=QByteArray::fromRawData(reinterpret_cast<char*>(data),datalen);
    QByteArray recvTempData;
    recvTempData.append(tmpRecvTempData);
    delete[]data;

    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到报文：%1").arg(QString(recvTempData.toHex())));

    if(p_CtrInfoList->size()==0)
        return;

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
        (*p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList)[p_CtrInfoList->at(0)->keyList.at(index)]->buf698.append(recvTempData);
        processMsgFromMeterOOP(dvcType,id,p_CtrInfoList->at(0)->keyList.at(index));
    }
    else if(dvcType==SingleSTA)//单通全部按照OOP处理
    {
        (*p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList)[p_CtrInfoList->at(0)->keyList.at(index)]->buf698.append(recvTempData);
        processMsgFromMeterOOP(dvcType,id,p_CtrInfoList->at(0)->keyList.at(index));
    }
    else if(dvcType==ThreeSTA)
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
}

void BaudConsult::processCtrlDvcRes(DvcType dvcType, QList<int> , CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params)
{
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("查询版本流程收到工装控制命令回复，设备类型=%1，命令类型=%2，参数个数=%3").arg(dvcType).arg(ctrlCmdType).arg(QString::number(params.size())));
    if(isSucs==false)
        return;
    switch(emScriptRunState)
    {
        default:
            break;
    }
}

void BaudConsult::processMsgFromCCO(DvcType dvcType, int dvcId)
{
    if(dvcId!=p_CtrInfoList->at(0)->ctrlID)
        return;
    bool haveCompleteMsg=true;
    while(haveCompleteMsg)
    {
        shared_ptr<Frame3762Base> p_Frame3762Base = p_Frame3762Helper->DecodeLocalMsg(&(p_CtrInfoList->at(0)->buf),haveCompleteMsg);
        if(p_Frame3762Base==nullptr)
        {
            continue;
        }
        if(p_Frame3762Base->afn_ == 0x03&&p_Frame3762Base->dt1_==0x02&&p_Frame3762Base->dt2_==0x01&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("收到CCO上报03F10，疑似复位，运行状态为%1").arg(metaEnum.valueToKey(emScriptRunState)));
            p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("收到CCO上报03F10，疑似复位，运行状态为%1").arg(metaEnum.valueToKey(emScriptRunState)));
            return;
        }
        switch(emScriptRunState)
        {
            case ScriptInit:
            {
                break;
            }
            case Wait_BaudConsult_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x02&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp&&uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq)==msgSeq)
                {
                    p_timer->stop();
                    shared_ptr<Afn02F1> ptr=dynamic_pointer_cast<Afn02F1>(p_Frame3762Base);
                    if(m_SpecialType==ConfirmDelay)
                    {
                        if(Address(QByteArray(ptr->address_field_.src_addr,6))==Address(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->value(p_CtrInfoList->at(0)->keyList.at(index))->mtrAddr)
                                &&ptr->frame_content_.isEmpty())
                        {
                            QString state=QString("波特率协商流程%1，波特率%2bps，%3波特率协商收到数据为空，符合要求")
                                    .arg(metaEnum.valueToKey(emScriptRunState))
                                    .arg(QString::number(m_Baud)
                                    .arg(metaSpecialType.valueToKey(m_SpecialType)));
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing,state);
                            delay(500);
                            tryTimes=0;
                            meterMsg.clear();
                            emScriptRunState=Wait_ReadMeter_Finish;
                            sendMsg(m_DvcType,p_CtrInfoList->at(0)->ctrlID,index,p_Afn02F1);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("发送抄读电表02F1，等待回复"));
                            p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
                        }
                        else
                        {
                            QString state=QString("波特率协商流程%1，波特率%2bps，%3波特率协商回复异常")
                                    .arg(metaEnum.valueToKey(emScriptRunState))
                                    .arg(QString::number(m_Baud)
                                    .arg(metaSpecialType.valueToKey(m_SpecialType)));
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing,state);
                            emit signalNoticeBaudConsultState(ProcessState_Failed,state);
                        }
                        return;
                    }
                    if(m_SpecialType==NotSearchMeterRate)
                    {
                        if(Address(QByteArray(ptr->address_field_.src_addr,6))==Address(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->value(p_CtrInfoList->at(0)->keyList.at(index))->mtrAddr)
                                &&!ptr->frame_content_.isEmpty())
                        {
                            QString state=QString("波特率协商流程%1，波特率%2bps，%3波特率协商收到确认回复且STA不往电表转发，符合要求")
                                    .arg(metaEnum.valueToKey(emScriptRunState))
                                    .arg(QString::number(m_Baud)
                                    .arg(metaSpecialType.valueToKey(m_SpecialType)));
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing,state);
                            delay(500);
                            tryTimes=0;
                            meterMsg.clear();
                            emScriptRunState=Wait_ReadMeter_Finish;
                            sendMsg(m_DvcType,p_CtrInfoList->at(0)->ctrlID,index,p_Afn02F1);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("发送抄读电表02F1，等待回复"));
                            p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
                        }
                        else
                        {
                            QString state=QString("波特率协商流程%1，波特率%2bps，%3波特率协商回复异常")
                                    .arg(metaEnum.valueToKey(emScriptRunState))
                                    .arg(QString::number(m_Baud)
                                    .arg(metaSpecialType.valueToKey(m_SpecialType)));
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing,state);
                            emit signalNoticeBaudConsultState(ProcessState_Failed,state);
                        }
                        return;
                    }
                    if(Address(QByteArray(ptr->address_field_.src_addr,6))==Address(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->value(p_CtrInfoList->at(0)->keyList.at(index))->mtrAddr)
                            &&ptr->frame_content_==meterMsg&&recvFlag==true)//
                    {
                        QString state=QString("波特率协商流程%1，波特率%2bps，%3波特率协商收到正确回复")
                                .arg(metaEnum.valueToKey(emScriptRunState))
                                .arg(QString::number(m_Baud)
                                .arg(metaSpecialType.valueToKey(m_SpecialType)));
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,state);
                        delay(500);
                        recvFlag=false;
                        tryTimes=0;
                        meterMsg.clear();
                        emScriptRunState=Wait_ReadMeter_Finish;
                        sendMsg(m_DvcType,p_CtrInfoList->at(0)->ctrlID,index,p_Afn02F1);
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("发送抄读电表02F1，等待回复"));
                        p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                    {
                        if(++tryTimes>=3)
                        {
                            QString state=QString("波特率协商流程%1，波特率%2bps，%3波特率协商回复异常")
                                    .arg(metaEnum.valueToKey(emScriptRunState))
                                    .arg(QString::number(m_Baud)
                                    .arg(metaSpecialType.valueToKey(m_SpecialType)));
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing,state);
                            emit signalNoticeBaudConsultState(ProcessState_Failed,state);
                            return;
                        }
                        else
                        {
                            emScriptRunState=Wait_BaudConsult_Finish;
                            sendMsg(m_DvcType,p_CtrInfoList->at(0)->ctrlID,index,p_Afn02F1);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("发送波特率协商02F1，波特率%1bps，等待回复").arg(QString::number(m_Baud)));
                            p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
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
            case Wait_ReadMeter_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x02&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp&&uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq)==msgSeq)
                {
                    p_timer->stop();
                    shared_ptr<Afn02F1> ptr=dynamic_pointer_cast<Afn02F1>(p_Frame3762Base);
                    if(m_SpecialType==ConfirmDelay)
                    {
                        if(Address(QByteArray(ptr->address_field_.src_addr,6))==Address(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->value(p_CtrInfoList->at(0)->keyList.at(index))->mtrAddr)
                                &&ptr->frame_content_.isEmpty())
                        {
                            QString state=QString("波特率协商流程%1，抄表收到数据为空，符合要求")
                                    .arg(metaEnum.valueToKey(emScriptRunState));
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing,state);
                            emit signalNoticeBaudConsultState(ProcessState_Processing,state+":"+QString(ptr->frame_content_.toHex()));
                            return;
                        }
                        else
                        {
                            QString state=QString("波特率协商流程%1，抄表收到异常回复")
                                    .arg(metaEnum.valueToKey(emScriptRunState));
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing,state);
                            emit signalNoticeBaudConsultState(ProcessState_Failed,state+":"+QString(ptr->frame_content_.toHex()));
                            return;
                        }
                    }

                    if(Address(QByteArray(ptr->address_field_.src_addr,6))==Address(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->value(p_CtrInfoList->at(0)->keyList.at(index))->mtrAddr)
                            &&ptr->frame_content_==meterMsg&&recvFlag==true)//
                    {
                        QString state=QString("波特率协商流程%1，抄表收到正确回复")
                                .arg(metaEnum.valueToKey(emScriptRunState));
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,state);
                        emit signalNoticeBaudConsultState(ProcessState_Processing,state+":"+QString(ptr->frame_content_.toHex()));
                        return;
                    }
                    else
                    {
                        if(++tryTimes>=3)
                        {
                            QString state=QString("波特率协商流程%1，抄表收到异常回复")
                                    .arg(metaEnum.valueToKey(emScriptRunState));
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing,state);
                            emit signalNoticeBaudConsultState(ProcessState_Failed,state+":"+QString(ptr->frame_content_.toHex()));
                            return;
                        }
                        else
                        {
                            emScriptRunState=Wait_ReadMeter_Finish;
                            sendMsg(m_DvcType,p_CtrInfoList->at(0)->ctrlID,index,p_Afn02F1);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("发送抄读电表02F1，等待回复"));
                            p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
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
            case ScriptSuccess:
            {
                break;
            }
        }
    }
}

void BaudConsult::processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID)
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

void BaudConsult::processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID)
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
            case ScriptSuccess:
            {
                break;
            }
            case Wait_BaudConsult_Finish:
            {
                if(dvcType==SingleSTA)
                {
                    if(MsgBase_OOP_ptr->service_type_==ACTION_REQUEST_CLIENT && MsgBase_OOP_ptr->service_sub_type_==uchar(ActionRequestType::kActionRequest)
                            &&dynamic_pointer_cast<ActionRequest>(MsgBase_OOP_ptr)->a_action_.omd.OI==0xF209)
                    {
                        recvFlag=true;
                        if(m_SpecialType==Confirm)
                        {
                            shared_ptr<ActionResponseNormal> p_ActionResponseNormal=make_shared<ActionResponseNormal>();
                            p_ActionResponseNormal->ctrl_field_={1,1,0,0,0,3};
                            p_ActionResponseNormal->ctrl_field_.dir = 1;
                            p_ActionResponseNormal->ctrl_field_.prm = 1;
                            p_ActionResponseNormal->ctrl_field_.fra = 0;
                            p_ActionResponseNormal->ctrl_field_.res = 0;
                            p_ActionResponseNormal->ctrl_field_.sc = 0;
                            p_ActionResponseNormal->ctrl_field_.func = 3;

                            p_ActionResponseNormal->address_field_.sa.addr_type = 0;
                            p_ActionResponseNormal->address_field_.sa.logic_addr = 0;
                            p_ActionResponseNormal->address_field_.sa.addr_len = 5;
                            p_ActionResponseNormal->address_field_.sa.address = MsgBase_OOP_ptr->address_field_.sa.address;
                            p_ActionResponseNormal->address_field_.ca.address = MsgBase_OOP_ptr->address_field_.ca.address;

                            p_ActionResponseNormal->piid_acd_={0,0,0};
                            p_ActionResponseNormal->a_action_result_.omd.OI=0xF209;
                            p_ActionResponseNormal->a_action_result_.omd.method_index=0x80;
                            p_ActionResponseNormal->a_action_result_.omd.operate_mode=0x00;

                            p_ActionResponseNormal->a_action_result_.dar=kSuccess;
                            p_ActionResponseNormal->a_action_result_.optional=0x00;

                            p_ActionResponseNormal->follow_report_field_=0x00;
                            p_ActionResponseNormal->time_tag_field_.optional_ = 0;
                            meterMsg=p_ActionResponseNormal->EncodeFrame();
                            sendSrcMsg(dvcType,dvcId,meterMsg);

                            delay(100);
                            //设置单通工装波特率
                            p_AbstractScriptHost->controlDvc(SingleSTA,QList<int>()<<p_CtrInfoList->at(0)->dvcId,CtrlCmd_SetBaudRate,QList<double>()<<m_Baud);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("发送设置单通工装波特率%1bps，等待确认").arg(QString::number(m_Baud)));
                        }
                        else if(m_SpecialType==Deny)
                        {
                            shared_ptr<ActionResponseNormal> p_ActionResponseNormal=make_shared<ActionResponseNormal>();
                            p_ActionResponseNormal->ctrl_field_={1,1,0,0,0,3};
                            p_ActionResponseNormal->ctrl_field_.dir = 1;
                            p_ActionResponseNormal->ctrl_field_.prm = 1;
                            p_ActionResponseNormal->ctrl_field_.fra = 0;
                            p_ActionResponseNormal->ctrl_field_.res = 0;
                            p_ActionResponseNormal->ctrl_field_.sc = 0;
                            p_ActionResponseNormal->ctrl_field_.func = 3;

                            p_ActionResponseNormal->address_field_.sa.addr_type = 0;
                            p_ActionResponseNormal->address_field_.sa.logic_addr = 0;
                            p_ActionResponseNormal->address_field_.sa.addr_len = 5;
                            p_ActionResponseNormal->address_field_.sa.address = MsgBase_OOP_ptr->address_field_.sa.address;
                            p_ActionResponseNormal->address_field_.ca.address = MsgBase_OOP_ptr->address_field_.ca.address;

                            p_ActionResponseNormal->piid_acd_={0,0,0};
                            p_ActionResponseNormal->a_action_result_.omd.OI=0xF209;
                            p_ActionResponseNormal->a_action_result_.omd.method_index=0x80;
                            p_ActionResponseNormal->a_action_result_.omd.operate_mode=0x00;

                            p_ActionResponseNormal->a_action_result_.dar=kCommRateCannotChange;
                            p_ActionResponseNormal->a_action_result_.optional=0x00;

                            p_ActionResponseNormal->follow_report_field_=0x00;
                            p_ActionResponseNormal->time_tag_field_.optional_ = 0;
                            meterMsg=p_ActionResponseNormal->EncodeFrame();
                            sendSrcMsg(dvcType,dvcId,meterMsg);
                        }
                        else if(m_SpecialType==Abnormal_NoResponse)
                        {
                            meterMsg.clear();
                        }
                        else if(m_SpecialType==Abnormal_WrongResponse)
                        {
                            shared_ptr<ActionResponseNormal> p_ActionResponseNormal=make_shared<ActionResponseNormal>();
                            p_ActionResponseNormal->ctrl_field_={1,1,0,0,0,3};
                            p_ActionResponseNormal->ctrl_field_.dir = 1;
                            p_ActionResponseNormal->ctrl_field_.prm = 1;
                            p_ActionResponseNormal->ctrl_field_.fra = 0;
                            p_ActionResponseNormal->ctrl_field_.res = 0;
                            p_ActionResponseNormal->ctrl_field_.sc = 0;
                            p_ActionResponseNormal->ctrl_field_.func = 3;

                            p_ActionResponseNormal->address_field_.sa.addr_type = 0;
                            p_ActionResponseNormal->address_field_.sa.logic_addr = 0;
                            p_ActionResponseNormal->address_field_.sa.addr_len = 5;
                            p_ActionResponseNormal->address_field_.sa.address = MsgBase_OOP_ptr->address_field_.sa.address;
                            p_ActionResponseNormal->address_field_.ca.address = MsgBase_OOP_ptr->address_field_.ca.address;

                            p_ActionResponseNormal->piid_acd_={0,0,0};
                            p_ActionResponseNormal->a_action_result_.omd.OI=0xF209;
                            p_ActionResponseNormal->a_action_result_.omd.method_index=0x80;
                            p_ActionResponseNormal->a_action_result_.omd.operate_mode=0x00;

                            p_ActionResponseNormal->a_action_result_.dar=kObjctNotExist;
                            p_ActionResponseNormal->a_action_result_.optional=0x00;

                            p_ActionResponseNormal->follow_report_field_=0x00;
                            p_ActionResponseNormal->time_tag_field_.optional_ = 0;
                            meterMsg=p_ActionResponseNormal->EncodeFrame();
                            sendSrcMsg(dvcType,dvcId,meterMsg);
                        }
                        else if(m_SpecialType==ConfirmDelay)
                        {
                            shared_ptr<ActionResponseNormal> p_ActionResponseNormal=make_shared<ActionResponseNormal>();
                            p_ActionResponseNormal->ctrl_field_={1,1,0,0,0,3};
                            p_ActionResponseNormal->ctrl_field_.dir = 1;
                            p_ActionResponseNormal->ctrl_field_.prm = 1;
                            p_ActionResponseNormal->ctrl_field_.fra = 0;
                            p_ActionResponseNormal->ctrl_field_.res = 0;
                            p_ActionResponseNormal->ctrl_field_.sc = 0;
                            p_ActionResponseNormal->ctrl_field_.func = 3;

                            p_ActionResponseNormal->address_field_.sa.addr_type = 0;
                            p_ActionResponseNormal->address_field_.sa.logic_addr = 0;
                            p_ActionResponseNormal->address_field_.sa.addr_len = 5;
                            p_ActionResponseNormal->address_field_.sa.address = MsgBase_OOP_ptr->address_field_.sa.address;
                            p_ActionResponseNormal->address_field_.ca.address = MsgBase_OOP_ptr->address_field_.ca.address;

                            p_ActionResponseNormal->piid_acd_={0,0,0};
                            p_ActionResponseNormal->a_action_result_.omd.OI=0xF209;
                            p_ActionResponseNormal->a_action_result_.omd.method_index=0x80;
                            p_ActionResponseNormal->a_action_result_.omd.operate_mode=0x00;

                            p_ActionResponseNormal->a_action_result_.dar=kSuccess;
                            p_ActionResponseNormal->a_action_result_.optional=0x00;

                            p_ActionResponseNormal->follow_report_field_=0x00;
                            p_ActionResponseNormal->time_tag_field_.optional_ = 0;
                            meterMsg=p_ActionResponseNormal->EncodeFrame();

                            //设置单通工装波特率
                            p_AbstractScriptHost->controlDvc(SingleSTA,QList<int>()<<p_CtrInfoList->at(0)->dvcId,CtrlCmd_SetBaudRate,QList<double>()<<m_Baud);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("发送设置单通工装波特率%1bps，等待确认").arg(QString::number(m_Baud)));
                            delay(500);
                            //回复确认
                            sendSrcMsg(dvcType,dvcId,meterMsg);
                        }
                        else if(m_SpecialType==NotSearchMeterRate)
                        {
                            meterMsg.clear();
                            QString state=QString("波特率协商流程%1，波特率%2bps，%3非搜表波特率STA不应该往电表转发")
                                    .arg(metaEnum.valueToKey(emScriptRunState))
                                    .arg(QString::number(m_Baud)
                                    .arg(metaSpecialType.valueToKey(m_SpecialType)));
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing,state);
                            emit signalNoticeBaudConsultState(ProcessState_Failed,state);
                            return;
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
                }
                else if(dvcType==ReadCtrlDvc)
                {
                    if(MsgBase_OOP_ptr->service_type_==ACTION_RESPONSE_SERVER && MsgBase_OOP_ptr->service_sub_type_==uchar(ActionResponseType::kActionResponseNormal)
                            &&dynamic_pointer_cast<ActionResponseNormal>(MsgBase_OOP_ptr)->a_action_result_.omd.OI==0xF209)
                    {
                        p_timer->stop();
                        QString state=QString("波特率协商流程%1，波特率%2bps，%3波特率协商收到正确回复")
                                .arg(metaEnum.valueToKey(emScriptRunState))
                                .arg(QString::number(m_Baud)
                                .arg(metaSpecialType.valueToKey(m_SpecialType)));
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,state);
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("等待10s之后开始抄表"));
                        delay(10000);
                        recvFlag=false;
                        tryTimes=0;
                        meterMsg.clear();
                        emScriptRunState=Wait_ReadMeter_Finish;
                        sendMsg(m_DvcType,p_CtrInfoList->at(0)->ctrlID,index,p_Afn02F1);
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("发送抄读电表02F1，等待回复"));
                        p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
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
                        if(dvcType==SingleSTA) {
                            QString tmp=tmpSendMsg.toHex().toUpper();
                            meterMsg.clear();
                            if(tmp.contains("00100200"))
                            {
                                meterMsg=tmpSendMsg;
                                recvFlag=true;
                            }
                        }
                        sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                    }
                }
                else if(MsgBase_OOP_ptr->service_type_==GET_RESPONSE_SERVER && MsgBase_OOP_ptr->service_sub_type_==uchar(GetResponseType::kGetResponseNormal))
                {
                    if(dvcType==ReadCtrlDvc)
                    {
                        p_timer->stop();
                        if(dynamic_pointer_cast<GetResponseNormal>(MsgBase_OOP_ptr)->a_result_normal_.oad.OI==0x0010)
                        {
                            QString state=QString("波特率协商流程%1，抄表收到正确回复")
                                    .arg(metaEnum.valueToKey(emScriptRunState));
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing,state);
                            emit signalNoticeBaudConsultState(ProcessState_Processing,state+":"+QString::number(0x0010,16));
                            return;
                        }
                        else if(MsgBase_OOP_ptr->service_type_==ACTION_RESPONSE_SERVER && MsgBase_OOP_ptr->service_sub_type_==uchar(ActionResponseType::kActionResponseNormal)
                                &&dynamic_pointer_cast<ActionResponseNormal>(MsgBase_OOP_ptr)->a_action_result_.omd.OI==0xF209)
                        {
                            QString state=QString("波特率协商流程%1，抄表收到异常回复")
                                    .arg(metaEnum.valueToKey(emScriptRunState));
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing,state);
                            emit signalNoticeBaudConsultState(ProcessState_Failed,state+":");
                            return;
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

void BaudConsult::sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame)
{
    if(frame==p_Afn02F1)
    {
        p_Afn02F1->ctrl_field_={kHplc,kActive,kDirDown};
        p_Afn02F1->info_field_.info_field_down.comu_module_ident=1;
        p_Afn02F1->info_field_.info_field_down.msg_seq=char(++msgSeq);

        p_Afn02F1->protocol_type_=0x03;
        QByteArray cco;
        if(m_DvcType==CCO_GW)
            cco=QByteArray::fromHex(Address(p_CtrInfoList->at(0)->ccoAddr).toString().toLatin1());
        else
            cco=QByteArray::fromHex(QString("000000000000").toLatin1());
        QByteArray meter=QByteArray::fromHex(Address(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(index)->mtrAddr).toString().toLatin1());
        if(emScriptRunState==Wait_BaudConsult_Finish)
        {
            uchar tmpAddr[6];
            memcpy(tmpAddr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(index)->mtrAddr,6);

            shared_ptr<ActionRequest> p_ActionRequest=make_shared<ActionRequest>();
            p_ActionRequest->ctrl_field_={0,1,0,0,0,3};
            p_ActionRequest->ctrl_field_.dir = 0;
            p_ActionRequest->ctrl_field_.prm = 1;
            p_ActionRequest->ctrl_field_.fra = 0;
            p_ActionRequest->ctrl_field_.res = 0;
            p_ActionRequest->ctrl_field_.sc = 0;
            p_ActionRequest->ctrl_field_.func = 3;

            p_ActionRequest->address_field_.sa.addr_type = 0;
            p_ActionRequest->address_field_.sa.logic_addr = 0;
            p_ActionRequest->address_field_.sa.addr_len = 5;
            p_ActionRequest->address_field_.sa.address = QString((QByteArray(reinterpret_cast<char*>(tmpAddr),6).toHex()));
            p_ActionRequest->address_field_.ca.address = 0x10;

            p_ActionRequest->piid_={0,0,0};
            p_ActionRequest->a_action_.omd.OI=0xF209;
            p_ActionRequest->a_action_.omd.method_index=0x80;
            p_ActionRequest->a_action_.omd.operate_mode=0x00;
            p_ActionRequest->a_action_.data_ptr=make_shared<DataList>();
            p_ActionRequest->a_action_.data_ptr->type_=DataType::kStructure;
            shared_ptr<DataBasic> data1=make_shared<DataBasic>();
            data1->type_=DataType::kOAD;
            data1->data_=QByteArray::fromHex(QString("F20902FD").toLatin1());
            shared_ptr<DataBasic> data2=make_shared<DataBasic>();
            data2->type_=DataType::kCOMDCB;
            data2->data_=QByteArray::fromHex(QString("0002080100").toLatin1());
            data2->data_[0]=matchBaud(m_Baud);
            dynamic_pointer_cast<DataList>(p_ActionRequest->a_action_.data_ptr)->list_data_member_<<data1<<data2;
            p_ActionRequest->time_tag_field_.optional_ = 0;
            p_Afn02F1->frame_content_=p_ActionRequest->EncodeFrame();

            p_Afn02F1->frame_length_=uchar(p_Afn02F1->frame_content_.size());

            memcpy(&p_Afn02F1->address_field_.src_addr,cco,6);
            memcpy(&p_Afn02F1->address_field_.dst_addr,meter,6);

            sendMsgOct=p_Afn02F1->EncodeFrame();
            if(m_DvcType==ReadCtrlDvc)
                sendMsgOct=p_Afn02F1->frame_content_;
            p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("》》波特率协商02F1：%1\n").arg(QString(sendMsgOct.toHex())));
        }
        else if(emScriptRunState==Wait_ReadMeter_Finish)
        {
            uchar tmpAddr[6];
            memcpy(tmpAddr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(index)->mtrAddr,6);

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
            p_Afn02F1->frame_content_=p_GetRequestNormal_ReadData->EncodeFrame();
            p_Afn02F1->frame_length_=uchar(p_Afn02F1->frame_content_.size());

            memcpy(&p_Afn02F1->address_field_.src_addr,cco,6);
            memcpy(&p_Afn02F1->address_field_.dst_addr,meter,6);

            sendMsgOct=p_Afn02F1->EncodeFrame();
            if(m_DvcType==ReadCtrlDvc)
                sendMsgOct=p_Afn02F1->frame_content_;
            p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("》》抄读电表02F1：%1\n").arg(QString(sendMsgOct.toHex())));
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

void BaudConsult::sendSrcMsg(DvcType dvcType, int dvcId, QByteArray msg)
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

char BaudConsult::matchBaud(int baud)
{
    QMap<int,char> baudMap;
    baudMap.insert(300,0x00);
    baudMap.insert(600,0x01);
    baudMap.insert(1200,0x02);
    baudMap.insert(2400,0x03);
    baudMap.insert(4800,0x04);
    baudMap.insert(7200,0x05);
    baudMap.insert(9600,0x06);
    baudMap.insert(19200,0x07);
    baudMap.insert(38400,0x08);
    baudMap.insert(57600,0x09);
    baudMap.insert(115200,0x0a);
    return baudMap.keys().contains(baud)?baudMap.value(baud):char(0xff);
}

void BaudConsult::timer_timeoutProc()
{
    switch(emScriptRunState)
    {
        case Wait_BaudConsult_Finish:
        {
            if(m_DvcType==ReadCtrlDvc)
            {
                if(++tryTimes>=5)
                {
                    QString state=QString("波特率协商流程%1，波特率%2bps，%3波特率协商回复异常")
                            .arg(metaEnum.valueToKey(emScriptRunState))
                            .arg(QString::number(m_Baud)
                            .arg(metaSpecialType.valueToKey(m_SpecialType)));
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,state);
                    emit signalNoticeBaudConsultState(ProcessState_Failed,state);
                    return;
                }
                else
                {
                    emScriptRunState=Wait_BaudConsult_Finish;
                    sendMsg(m_DvcType,p_CtrInfoList->at(0)->ctrlID,index,p_Afn02F1);
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("发送波特率协商02F1，波特率%1bps，等待回复").arg(QString::number(m_Baud)));
                    p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
                }
            }
            else
            {
                QString state=QString("波特率协商流程%1，回复超时")
                        .arg(metaEnum.valueToKey(emScriptRunState));
                p_AbstractScriptHost->updateProgress(ProcessState_Processing,state);
                emit signalNoticeBaudConsultState(ProcessState_Failed,state);
                break;
            }
            break;
        }
        case Wait_ReadMeter_Finish:
        {
            if(m_DvcType==ReadCtrlDvc)
            {
                if(++tryTimes>=5)
                {
                    QString state=QString("波特率协商流程%1，抄表收到异常回复")
                            .arg(metaEnum.valueToKey(emScriptRunState));
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,state);
                    emit signalNoticeBaudConsultState(ProcessState_Failed,state);
                    return;
                }
                else
                {
                    emScriptRunState=Wait_ReadMeter_Finish;
                    sendMsg(m_DvcType,p_CtrInfoList->at(0)->ctrlID,index,p_Afn02F1);
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("发送抄读电表02F1，等待回复"));
                    p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
                }
            }
            else
            {
                QString state=QString("波特率协商流程%1，回复超时")
                        .arg(metaEnum.valueToKey(emScriptRunState));
                p_AbstractScriptHost->updateProgress(ProcessState_Processing,state);
                emit signalNoticeBaudConsultState(ProcessState_Failed,state);
                break;
            }
            break;
        }
        default:
        {
            QString state=QString("波特率协商流程%1，回复超时")
                    .arg(metaEnum.valueToKey(emScriptRunState));
            p_AbstractScriptHost->updateProgress(ProcessState_Processing,state);
            emit signalNoticeBaudConsultState(ProcessState_Failed,state);
            break;
        }
    }
}

void BaudConsult::maxAllowTimer_timeoutProc()
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

void BaudConsult::delayTimer_timeoutProc()
{
    switch(emScriptRunState)
    {
        default:
        {
            break;
        }
    }
}
