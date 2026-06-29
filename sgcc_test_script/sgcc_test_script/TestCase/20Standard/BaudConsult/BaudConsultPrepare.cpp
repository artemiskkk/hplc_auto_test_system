#include "BaudConsultPrepare.h"
BaudConsultPrepare::BaudConsultPrepare(shared_ptr<QList<shared_ptr<CtrInfo>>> p_CtrInfoList,AbstractScriptHost *host,int baud,bool isCCO,QObject *parent) :
    QObject(parent),p_AbstractScriptHost(host),p_CtrInfoList(p_CtrInfoList),m_Baud(baud)
{
    p_Frame3762Helper=make_shared<Frame3762Helper>();
    p_Frame645Helper=make_shared<Frame645Helper>();
    p_MeterAddrResp_93=make_shared<RspsNormal_ReadAddr_0x93>(addr,6);
    p_FrameOOPHelper=make_shared<FrameOOPHelper>();
    p_GetResponseNormal_ReadAddr=make_shared<GetResponseNormal>();
    p_Afn01F2=make_shared<Afn01F2>();
    p_Afn11F1=make_shared<Afn11F1>();
    p_Afn10F21=make_shared<Afn10F21>();
    p_timer=new QTimer(this);
    p_maxAllowTimer=new QTimer(this);
    p_delayTimer=new QTimer(this);
    connect(p_timer,SIGNAL(timeout()),this,SLOT(timer_timeoutProc()));
    connect(p_maxAllowTimer,SIGNAL(timeout()),this,SLOT(maxAllowTimer_timeoutProc()));
    connect(p_delayTimer,SIGNAL(timeout()),this,SLOT(delayTimer_timeoutProc()));
    m_DvcType=(isCCO==true?CCO_GW:ReadCtrlDvc);
}

BaudConsultPrepare::~BaudConsultPrepare()
{

}

void BaudConsultPrepare::execute()
{
    p_AbstractScriptHost->updateProgress(ProcessState_Start, "波特率协商准备流程开始!");
    emScriptRunState=Wait_DeviceInit_Finish;
    //设置单通工装波特率
    p_AbstractScriptHost->controlDvc(SingleSTA,QList<int>()<<p_CtrInfoList->at(0)->dvcId,CtrlCmd_SetBaudRate,QList<double>()<<m_Baud);
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("发送设置单通工装波特率%1bps，等待确认").arg(QString::number(m_Baud)));
    delay(2000);
    //复位单通模块
    p_AbstractScriptHost->controlDvc(SingleSTA,QList<int>()<<p_CtrInfoList->at(0)->dvcId,CtrlCmd_ModuleRST,QList<double>());
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("发送复位单通模块，等待确认").arg(QString::number(m_Baud)));
    //等待单通读表号，120s内
    p_timer->start(120*1000);
}

void BaudConsultPrepare::processMsg(DvcType dvcType, int id, uchar *data, int datalen)
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
        p_CtrInfoList->at(0)->buf.append(recvTempData);
        processMsgFromCCO(dvcType,id);
    }
    else if(dvcType==SingleSTA)//单通全部按照OOP处理
    {
        for(int i=0; i<p_CtrInfoList->at(0)->keyList.size(); i++)
        {
            if(dvcType == p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->value(p_CtrInfoList->at(0)->keyList.at(i))->slotPosition
                    && id == p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->value(p_CtrInfoList->at(0)->keyList.at(i))->dvcId)
            {
                index=i;
                (*p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList)[p_CtrInfoList->at(0)->keyList.at(i)]->buf698.append(recvTempData);
                processMsgFromMeterOOP(dvcType,id,p_CtrInfoList->at(0)->keyList.at(i));
                break;
            }
        }
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

void BaudConsultPrepare::processCtrlDvcRes(DvcType dvcType, QList<int> , CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params)
{
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("文件传输流程收到工装控制命令回复，设备类型=%1，命令类型=%2，参数个数=%3").arg(dvcType).arg(ctrlCmdType).arg(QString::number(params.size())));
    if(isSucs==false)
        return;
    p_timer->stop();
    switch(emScriptRunState)
    {
        default:
            break;
    }
}

void BaudConsultPrepare::processMsgFromCCO(DvcType dvcType, int dvcId)
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
            case Wait_BuildNet_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp&&uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq)==msgSeq)
                {
                    p_timer->stop();
                    tryTimes=0;
                    if(flagParaInit)
                    {
                        flagParaInit=false;
                        flagAddNode=true;
                        sendMsg(dvcType,p_CtrInfoList->at(0)->ctrlID,index,p_Afn11F1);
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("发送添加从节点11F1，等待回复"));
                        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                    {
                        flagAddNode=false;
                        p_delayTimer->start(15*1000);
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("等待15s，开始查询拓扑"));
                    }
                }
                else if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x02&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp&&uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq)==msgSeq)
                {
                    p_timer->stop();
                    if(flagParaInit)
                    {
                        QString state=QString("波特率协商准备流程%1，参数初始化回复异常，实际回复否认，期望回复确认")
                                .arg(metaEnum.valueToKey(emScriptRunState));
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,state);
                        emit signalNoticeBaudConsultPrepareState(ProcessState_Failed,state);
                    }
                    else
                    {
                        QString state=QString("波特率协商准备流程%1，添加从节点回复异常，实际回复否认，期望回复确认")
                                .arg(metaEnum.valueToKey(emScriptRunState));
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,state);
                        emit signalNoticeBaudConsultPrepareState(ProcessState_Failed,state);
                    }

                }
                else if(p_Frame3762Base->afn_ == 0x10&&p_Frame3762Base->dt1_==0x10&&p_Frame3762Base->dt2_==0x02&&p_Frame3762Base->ctrl_field_.dir == kDirUp&&uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq)==msgSeq)
                {
                    p_timer->stop();
                    shared_ptr<Afn10F21> ptr=dynamic_pointer_cast<Afn10F21>(p_Frame3762Base);
                    if(ptr->network_typelogy_info_unit_.this_node_num_==2)
                    {
                        if(ptr->network_typelogy_info_unit_.network_typelogy_info_List_.first().node_address_==Address(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->value(p_CtrInfoList->at(0)->keyList.at(index))->mtrAddr)
                           ||ptr->network_typelogy_info_unit_.network_typelogy_info_List_.last().node_address_==Address(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->value(p_CtrInfoList->at(0)->keyList.at(index))->mtrAddr))
                        {
                            QString state=QString("波特率协商准备流程%1，组网完成")
                                    .arg(metaEnum.valueToKey(emScriptRunState));
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing,state);
                            emit signalNoticeBaudConsultPrepareState(ProcessState_Processing,state);
                        }
                        else
                        {
                            QString state=QString("波特率协商准备流程%1，组网异常")
                                    .arg(metaEnum.valueToKey(emScriptRunState));
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing,state);
                            emit signalNoticeBaudConsultPrepareState(ProcessState_Failed,state);
                        }
                    }
                    else
                    {
                        p_delayTimer->start(15*1000);
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("等待15s，开始查询下一轮拓扑"));
                    }
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            default:
            {
                break;
            }
        }
    }
}

void BaudConsultPrepare::processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID)
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

void BaudConsultPrepare::processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID)
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
                        if(dvcType==SingleSTA&&emScriptRunState==Wait_DeviceInit_Finish)
                        {
                            p_timer->stop();
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing,"单通模块读表号完成");
                            if(m_DvcType==ReadCtrlDvc)
                            {
                                QString state=QString("波特率协商准备流程%1，抄控器设备准备完成")
                                        .arg(metaEnum.valueToKey(emScriptRunState));
                                p_AbstractScriptHost->updateProgress(ProcessState_Processing,state);
                                emit signalNoticeBaudConsultPrepareState(ProcessState_Processing,state);
                            }
                            else
                            {
                                emScriptRunState=Wait_BuildNet_Finish;
                                sendMsg(m_DvcType,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_Afn01F2);
                                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送参数初始化01F2，等待回复");
                                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                            }
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

void BaudConsultPrepare::sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame)
{
    if(frame==p_Afn01F2)
    {
        p_Afn01F2->ctrl_field_={kHplc,kActive,kDirDown};
        p_Afn01F2->info_field_.info_field_down.msg_seq=char(++msgSeq);

        sendMsgOct=p_Afn01F2->EncodeFrame();
        sendMsgLog=QString("》》参数初始化01F2：%1\n").arg(QString(sendMsgOct.toHex()));
    }
    else if(frame==p_Afn11F1)
    {
        p_Afn11F1->ctrl_field_={kHplc,kActive,kDirDown};
        p_Afn11F1->info_field_.info_field_down.msg_seq=char(++msgSeq);

        p_Afn11F1->node_parameter_list_.clear();
        NodeParameter node;
        node.node_address_=Address(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->value(p_CtrInfoList->at(0)->keyList.at(index))->mtrAddr);
        node.protocol_type_=0x03;
        p_Afn11F1->node_parameter_list_.append(node);
        p_Afn11F1->node_num_=uchar(p_Afn11F1->node_parameter_list_.size());

        sendMsgOct=p_Afn11F1->EncodeFrame();
        sendMsgLog=QString("》》添加从节点11F1：%1\n").arg(QString(sendMsgOct.toHex()));
    }
    else if(frame==p_Afn10F21)
    {
        p_Afn10F21->ctrl_field_={kHplc,kActive,kDirDown};
        p_Afn10F21->info_field_.info_field_down.msg_seq=char(++msgSeq);

        p_Afn10F21->node_num_=0x05;
        p_Afn10F21->node_start_no_=0x01;

        sendMsgOct=p_Afn10F21->EncodeFrame();
        sendMsgLog=QString("》》查询网络拓扑10F21：%1\n").arg(QString(sendMsgOct.toHex()));
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

void BaudConsultPrepare::sendSrcMsg(DvcType dvcType, int dvcId, QByteArray msg)
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

void BaudConsultPrepare::timer_timeoutProc()
{
    p_timer->stop();
    switch(emScriptRunState)
    {
        case Wait_BuildNet_Finish:
        {
            if(flagParaInit)
            {
                if(++tryTimes>=3)
                {
                    QString state=QString("波特率协商准备流程%1，参数初始化回复超时")
                            .arg(metaEnum.valueToKey(emScriptRunState));
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,state);
                    emit signalNoticeBaudConsultPrepareState(ProcessState_Failed,state);
                }
                else
                {
                    sendMsg(m_DvcType,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_Afn01F2);
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送参数初始化01F2，等待回复");
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                }
            }
            else if(flagAddNode)
            {
                if(++tryTimes>=3)
                {
                    QString state=QString("波特率协商准备流程%1，添加从节点回复超时")
                            .arg(metaEnum.valueToKey(emScriptRunState));
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,state);
                    emit signalNoticeBaudConsultPrepareState(ProcessState_Failed,state);
                }
                else
                {
                    sendMsg(m_DvcType,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_Afn11F1);
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送添加从节点11F1，等待回复");
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                }
            }
            else
            {
                if(++tryTimes>=3)
                {
                    QString state=QString("波特率协商准备流程%1，查询拓扑回复超时")
                            .arg(metaEnum.valueToKey(emScriptRunState));
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,state);
                    emit signalNoticeBaudConsultPrepareState(ProcessState_Failed,state);
                }
                else
                {
                    sendMsg(m_DvcType,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_Afn10F21);
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("发送查询网络拓扑10F21，等待回复，当前第%1轮").arg(++topoIndex));
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                }
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

void BaudConsultPrepare::maxAllowTimer_timeoutProc()
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

void BaudConsultPrepare::delayTimer_timeoutProc()
{
    switch(emScriptRunState)
    {
        case Wait_BuildNet_Finish:
        {
            p_delayTimer->stop();
            sendMsg(m_DvcType,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_Afn10F21);
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("发送查询网络拓扑10F21，等待回复，当前第%1轮").arg(++topoIndex));
            p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            break;
        }
        default:
        {
            break;
        }
    }
}
