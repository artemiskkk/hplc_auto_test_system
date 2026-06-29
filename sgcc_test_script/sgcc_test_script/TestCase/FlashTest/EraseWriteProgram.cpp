#include "EraseWriteProgram.h"

#include <QApplication>
#define CS_INDEX 8u
#define WBWCBL_LEN 13u
#define OPER_CODE_INDEX 0
#define ADDR_INDEX 2
#define TR_LEN_INDEX 6
EraseWriteProgram::EraseWriteProgram(shared_ptr<QList<shared_ptr<CtrInfo> > > p_CtrInfoList, AbstractScriptHost *host, DvcType type, QObject *parent) : QObject(parent),m_dvcType(type)
{
    p_Frame3762Helper=make_shared<Frame3762Helper>();
    p_Frame645Helper=make_shared<Frame645Helper>();
    p_MeterAddrResp_93=make_shared<RspsNormal_ReadAddr_0x93>(addr,6);
    p_FrameOOPHelper=make_shared<FrameOOPHelper>();
    p_GetResponseNormal_ReadAddr=make_shared<GetResponseNormal>();
    p_Afn03F1=make_shared<Afn03F1>();
    p_timer=new QTimer(this);
    p_maxAllowTimer=new QTimer(this);
    p_delayTimer=new QTimer(this);
    p_repeatTimer=new QTimer(this);
    connect(p_timer,SIGNAL(timeout()),this,SLOT(timer_timeoutProc()));
    connect(p_maxAllowTimer,SIGNAL(timeout()),this,SLOT(maxAllowTimer_timeoutProc()));
    connect(p_delayTimer,SIGNAL(timeout()),this,SLOT(delayTimer_timeoutProc()));
    this->p_CtrInfoList=p_CtrInfoList;
    this->p_AbstractScriptHost=host;
}

EraseWriteProgram::~EraseWriteProgram()
{

}

void EraseWriteProgram::execute()
{
    p_AbstractScriptHost->updateProgress(ProcessState_Start, "程序烧写流程开始!");
    emScriptRunState=Wait_DeviceInit_Finish;
    //设置槽位12V断电
    p_AbstractScriptHost->controlDvc(CCO_GW,QList<int>()<<p_CtrInfoList->at(0)->dvcId,CtrlCmd_PowerOff_12V,QList<double>());
    p_AbstractScriptHost->controlDvc(SingleSTA,QList<int>()<<p_CtrInfoList->at(0)->dvcId,CtrlCmd_PowerOff_12V,QList<double>());
    p_AbstractScriptHost->controlDvc(ThreeSTA,QList<int>()<<p_CtrInfoList->at(0)->dvcId,CtrlCmd_PowerOff_12V,QList<double>());
    p_AbstractScriptHost->controlDvc(m_dvcType,QList<int>()<<p_CtrInfoList->at(0)->dvcId,CtrlCmd_PowerOff_12V,QList<double>());
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("发送设置工装12V断电，等待确认"));
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("等待2min超级电容放电完成"));
    delay(2*60*1000);
    //设置槽位工装波特率115200bps，无校验
    p_AbstractScriptHost->controlDvc(m_dvcType,QList<int>()<<p_CtrInfoList->at(0)->dvcId,CtrlCmd_SetBaudRate,QList<double>()<<m_baud<<m_parity);
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("发送设置工装波特率%1bps，等待确认").arg(QString::number(m_baud)));
    delay(2000);
    emScriptRunState=Wait_ModeChange_Finish;
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("发送切换模式命令:%1").arg(QString(getChangeModeCMD().toHex())));
    sendSrcMsg(m_dvcType,p_CtrInfoList->at(0)->dvcId,getChangeModeCMD());//发送切换模式命令
    p_repeatTimer->start(50);
    connect(p_repeatTimer,&QTimer::timeout,this,[this]{
        p_repeatTimer->stop();
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("发送切换模式命令:%1").arg(QString(getChangeModeCMD().toHex())));
        sendSrcMsg(m_dvcType,p_CtrInfoList->at(0)->dvcId,getChangeModeCMD());//发送切换命令
        p_repeatTimer->start(50);
    });
    p_timer->start(120*1000);
    //设置槽位12V上电
    p_AbstractScriptHost->controlDvc(m_dvcType,QList<int>()<<p_CtrInfoList->at(0)->dvcId,CtrlCmd_PowerOn_12V,QList<double>());
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("发送设置工装12V上电，等待确认"));

//    //复位单通模块
//    p_AbstractScriptHost->controlDvc(SingleSTA,QList<int>()<<p_CtrInfoList->at(0)->dvcId,CtrlCmd_ModuleRST,QList<double>());
//    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("发送复位单通模块，等待确认").arg(QString::number(m_Baud)));
//    //等待单通读表号，120s内
//    p_timer->start(120*1000);
}

void EraseWriteProgram::processMsg(DvcType dvcType, int id, uchar *data, int datalen)
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
        switch (emScriptRunState)
        {
            case Wait_ModeChange_Finish:
            {
                p_CtrInfoList->at(0)->buf.append(recvTempData);
                if(p_CtrInfoList->at(0)->buf.contains(QByteArray::fromHex("49535053aa5555aa0000000000")))
                {
                    p_repeatTimer->stop();
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("模式切换成功:%1").arg(QString(recvTempData.toHex())));
                    p_CtrInfoList->at(0)->buf.clear();
                    emScriptRunState=Wait_UnlockFlash_Finish;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("发送解锁Flash命令:%1").arg(QString(getUnLockCMD().toHex())));
                    sendSrcMsg(m_dvcType,p_CtrInfoList->at(0)->dvcId,getUnLockCMD());
                    p_timer->start(5*1000);
                }
                break;
            }
            case Wait_UnlockFlash_Finish:
            {
                p_CtrInfoList->at(0)->buf.append(recvTempData);
                if(p_CtrInfoList->at(0)->buf.contains(QByteArray::fromHex("49535053aa5555aa0000000000")))
                {
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("解锁Flash成功:%1").arg(QString(recvTempData.toHex())));
                    p_CtrInfoList->at(0)->buf.clear();
                    emScriptRunState=Wait_EarseFlash_Finish;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("发送擦除Flash命令:%1").arg(QString(getEarseCMD().toHex())));
                    sendSrcMsg(m_dvcType,p_CtrInfoList->at(0)->dvcId,getEarseCMD());
                    p_timer->start(60*1000);
                }
                break;
            }
            case Wait_EarseFlash_Finish:
            {
                p_CtrInfoList->at(0)->buf.append(recvTempData);
                if(p_CtrInfoList->at(0)->buf.contains(QByteArray::fromHex("49535053aa5555aa0000000000")))
                {
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("擦除Flash成功:%1").arg(QString(recvTempData.toHex())));
                    p_CtrInfoList->at(0)->buf.clear();
                    emScriptRunState=Wait_WriteProgram_Finish;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("开始写入文件过程"));
                    writeData();
                    p_timer->start(10*1000);
                }
                break;
            }
            case Wait_WriteProgram_Finish:
            {
                p_CtrInfoList->at(0)->buf.append(recvTempData);
                if(p_CtrInfoList->at(0)->buf.contains(QByteArray::fromHex("49535053aa5555aa")))
                {
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("写入文件成功:%1").arg(QString(recvTempData.toHex())));
                    p_CtrInfoList->at(0)->buf.clear();
                    emScriptRunState=Wait_ReadVersion_Finish;
                    //设置槽位12V断电
                    p_AbstractScriptHost->controlDvc(m_dvcType,QList<int>()<<p_CtrInfoList->at(0)->dvcId,CtrlCmd_PowerOff_12V,QList<double>());
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("发送设置工装12V断电，等待确认"));
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("等待30s超级电容放电完成"));
                    delay(30*1000);
                    //设置槽位工装波特率9600bps，偶校验
                    p_AbstractScriptHost->controlDvc(m_dvcType,QList<int>()<<p_CtrInfoList->at(0)->dvcId,CtrlCmd_SetBaudRate,QList<double>()<<9600<<char(0x01));
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("发送设置工装波特率9600bps，等待确认"));
                    delay(2000);
                    //设置槽位12V上电
                    p_AbstractScriptHost->controlDvc(m_dvcType,QList<int>()<<p_CtrInfoList->at(0)->dvcId,CtrlCmd_PowerOn_12V,QList<double>());
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("发送设置工装12V上电，等待确认"));
                    if(m_dvcType==CCO_GW)
                    {
                        delay(5000);
                        tryTimes=0;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("开始查询CCO版本"));
                        sendMsg(m_dvcType,p_CtrInfoList->at(0)->dvcId,0,p_Afn03F1);
                        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                        p_timer->start(120*1000);//考虑到读表号
                }
                break;
            }
            default:
                p_CtrInfoList->at(0)->buf.append(recvTempData);
                processMsgFromCCO(dvcType,id);
                break;
        }
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

void EraseWriteProgram::processCtrlDvcRes(DvcType dvcType, QList<int> , CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params)
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

void EraseWriteProgram::processMsgFromCCO(DvcType dvcType, int dvcId)
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
        switch(emScriptRunState)
        {
            case ScriptInit:
            {
                break;
            }
            case Wait_ReadVersion_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x03&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp&&uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq)==msgSeq)
                {
                    p_timer->stop();
                    tryTimes=0;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("烧写完成后，读取版本成功"));
                    emit signalNoticeWriteState(ProcessState_Processing,QString("烧写完成后，读取版本成功"),freq);
                    return;
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

void EraseWriteProgram::processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID)
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

void EraseWriteProgram::processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID)
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
                        sendMsg(dvcType,dvcId,mtrlID,p_GetResponseNormal_ReadAddr);/*
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
                        }*/
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

void EraseWriteProgram::sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame)
{
    if(frame==p_Afn03F1)
    {
        p_Afn03F1->ctrl_field_={kHplc,kActive,kDirDown};
        p_Afn03F1->info_field_.info_field_down.msg_seq=char(++msgSeq);

        sendMsgOct=p_Afn03F1->EncodeFrame();
        sendMsgLog=QString("》》查询外部版本：%1\n").arg(QString(sendMsgOct.toHex()));
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

void EraseWriteProgram::sendSrcMsg(DvcType dvcType, int dvcId, QByteArray msg)
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

void EraseWriteProgram::timer_timeoutProc()
{
    p_timer->stop();
    switch(emScriptRunState)
    {
        case Wait_ReadVersion_Finish:
        {
            if(m_dvcType==CCO_GW)
            {
                if(++tryTimes>=4)
                {
                    QString state=QString("查询版本流程%1，查询CCO外部版本超时")
                            .arg(metaEnum.valueToKey(emScriptRunState));
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,state);
                    emit signalNoticeWriteState(ProcessState_Failed,state);
                    return;
                }
                else if(tryTimes>=2)
                {
                    p_AbstractScriptHost->controlDvc(CCO_GW,QList<int>()<<p_CtrInfoList->at(0)->dvcId,CtrlCmd_SetBaudRate,QList<double>()<<m_baud<<0x01);
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("发送设置工装波特率115200bps"));
                    delay(5000);
                }
                sendMsg(m_dvcType,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_Afn03F1);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送查询CCO外部版本03F1，等待回复");
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            }
            else
            {
                if(++tryTimes>=3)
                {
                    QString state=QString("查询版本流程%1，查询STA版本超时")
                            .arg(metaEnum.valueToKey(emScriptRunState));
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,state);
                    emit signalNoticeWriteState(ProcessState_Failed,state);
                    return;
                }
                sendMsg(m_dvcType,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_Afn03F1);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送查询STA版本03F1，等待回复");
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            }
            break;
        }
        default:
        {
            emit signalNoticeWriteState(ProcessState_Failed, QString("【%1】State machine run error!!!==p_timer").arg(metaEnum.valueToKey(emScriptRunState)));
            break;
        }
    }
}

void EraseWriteProgram::maxAllowTimer_timeoutProc()
{
    switch(emScriptRunState)
    {
        default:
        {
            emit signalNoticeWriteState(ProcessState_Failed, QString("【%1】State machine run error!!!==p_maxAllowTimer").arg(metaEnum.valueToKey(emScriptRunState)));
            break;
        }
    }
}

void EraseWriteProgram::delayTimer_timeoutProc()
{
    switch(emScriptRunState)
    {
        default:
        {
            break;
        }
    }
}

QByteArray EraseWriteProgram::getChangeModeCMD()
{
    QByteArray cbw = QByteArray::fromHex("49535043AA5555AA00000000000010");
    QByteArray cbwcb;
    cbwcb.fill(0x00,16);
    cbwcb[OPER_CODE_INDEX] = 0x40;
    cbwcb[1] = 0x02;
    cbw.append(cbwcb);
    return cbw;
}

QByteArray EraseWriteProgram::getUnLockCMD()
{
   QByteArray cbw = QByteArray::fromHex("49535043aa5555aa00000000000010520000a0000000100800000000000000");
   return cbw;
}
QByteArray EraseWriteProgram::getEarseCMD()
{
    QByteArray cbw = QByteArray::fromHex("49535043AA5555AA00000000000010");
    QByteArray cbwcb;
    cbwcb.fill(0x00,16);
    cbwcb[OPER_CODE_INDEX] = 0x4B;
    cbwcb[ADDR_INDEX] =address[3];
    cbwcb[ADDR_INDEX+1] =address[2];
    cbwcb[ADDR_INDEX+2] =address[1];
    cbwcb[ADDR_INDEX+3] =address[0];
    cbwcb[TR_LEN_INDEX]   = *(reinterpret_cast<char*>(&length));
    cbwcb[TR_LEN_INDEX+1] = *(reinterpret_cast<char*>(&length)+1);
    cbwcb[TR_LEN_INDEX+2] = *(reinterpret_cast<char*>(&length)+2);
    cbwcb[TR_LEN_INDEX+3] = *(reinterpret_cast<char*>(&length)+3);
    cbw.append(cbwcb);
    return cbw;
}

QByteArray EraseWriteProgram::getReadCMD()
{
    QByteArray cbw = QByteArray::fromHex("49535043AA5555AA00000000000010");
    QByteArray cbwcb;
    cbwcb.fill(0x00,16);
    cbwcb[OPER_CODE_INDEX] = 0x38;
    cbwcb[ADDR_INDEX] =address[3];
    cbwcb[ADDR_INDEX+1] =address[2];
    cbwcb[ADDR_INDEX+2] =address[1];
    cbwcb[ADDR_INDEX+3] =address[0];
    cbwcb[TR_LEN_INDEX] = *(reinterpret_cast<char*>(&length));
    cbwcb[TR_LEN_INDEX+1] =*(reinterpret_cast<char*>(&length)+1);
    cbwcb[TR_LEN_INDEX+2] =*(reinterpret_cast<char*>(&length)+2);
    cbwcb[TR_LEN_INDEX+3] =*(reinterpret_cast<char*>(&length)+3);
    cbw.append(cbwcb);
    return cbw;
}

QByteArray EraseWriteProgram::getWriteCMD(QByteArray data)
{
    QByteArray cbw = QByteArray::fromHex("49535043AA5555AA00000000000010");
    QByteArray cbwcb;
    uint length = uint(data.length());
    uchar cs = 0x00;
    for(int i=0;i<data.length();i++)
    {
        cs = cs+uchar(data.at(i));
    }
    cbw[CS_INDEX] = char(cs);
    cbwcb.fill(0x00,16);
    cbwcb[OPER_CODE_INDEX] = 0x3A;
    cbwcb[ADDR_INDEX] =address[3];
    cbwcb[ADDR_INDEX+1] =address[2];
    cbwcb[ADDR_INDEX+2] =address[1];
    cbwcb[ADDR_INDEX+3] =address[0];
    cbwcb[TR_LEN_INDEX] = *(reinterpret_cast<char*>(&length));
    cbwcb[TR_LEN_INDEX+1] =*(reinterpret_cast<char*>(&length)+1);
    cbwcb[TR_LEN_INDEX+2] =*(reinterpret_cast<char*>(&length)+2);
    cbwcb[TR_LEN_INDEX+3] =*(reinterpret_cast<char*>(&length)+3);
    cbw.append(cbwcb);
    return cbw;
}
QByteArray EraseWriteProgram::upgradeFileData()
{
    QStringList fileNameFilters;
    QStringList fileList;
    QString fileDir=QApplication::applicationDirPath()+"/DataBase/Upgrade/";
    QStringList paths;
    QString error;
    if(m_dvcType==CCO_GW)
        paths<<fileDir+"路由程序(新-新)";//<<fileDir+"路由程序(旧-新)";
    else
        paths<<fileDir+"模块程序(新-新)";//<<fileDir+"模块程序(旧-新)";
    for(auto path:paths)
    {
        QDir updateFileDir(path);
        QFileInfoList files=updateFileDir.entryInfoList(QStringList()<<"*",QDir::Files,QDir::NoSort);
        if(files.count() != 1)
        {
            error="获取到的升级文件数不为1："+QString::number(files.count());
            p_AbstractScriptHost->updateProgress(ProcessState_Processing,error);
            continue;
        }
        QString fileName=files.first().fileName();
        if(fileName.right(3)!="bin"&&fileName.right(3)!="dat")
        {
            error="获取到的文件后缀名不是dat或bin："+fileName.right(3);
            p_AbstractScriptHost->updateProgress(ProcessState_Processing,error);
            continue;
        }
        if(m_dvcType==CCO_GW)
        {
            if(fileName.left(7)!="TCRS091" && fileName.left(7)!="TCRS053" && fileName.left(6)!="TCC0A1" && fileName.left(6)!="TCD0A1"&& fileName.left(7)!="TCRS0B1")//fileName.left(7)!="TCRS0A1"
            {
                error="获取到的文件名头不是TCRS091或TCRS053或TCC0A1或TCD0A1或TCRS0B1："+fileName.left(7);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing,error);
                continue;
            }
        }
        else
        {
            if(fileName.left(6)!="TCC091" && fileName.left(6)!="TCC053" && fileName.left(6)!="TCC0A1" && fileName.left(6)!="TCD0A1"&& fileName.left(6)!="TCD0B1")//fileName.left(7)!="TCRS0A1"
            {
                error="获取到的文件名头不是TCC091或TCC053或TCC0A1或TCD0A1或TCD0B1："+fileName.left(7);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing,error);
                continue;
            }
        }
        QString outVer,innerVer;
        QStringList list=fileName.remove(".bin").remove(".dat").split("_");
        if(list.size()<4)
        {
            error="获取到的文件名日期格式不对："+fileName;
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, error);
            continue;
        }
        outVer=list.at(1);//外部日期TCRS0B1_230901_240221_100938V3.60(V0.02).bin
        innerVer=list.at(2);//内部日期
        list=list.at(3).split("V");
        if(list.size()<2)
        {
            error="获取到的文件名版本号格式不对："+fileName;
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, error);
            continue;
        }
        outVer+=list[1].remove('.').remove('(').remove(')').rightJustified(4,'0');//外部版本号
        innerVer+=list[2].remove('.').remove('(').remove(')').rightJustified(4,'0');//内部版本号
        if(files.first().filePath().contains("CQ"))
            freq=0x01;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, files.first().filePath());
        QFile file(files.first().filePath());
        QByteArray fileContent;
        if(file.open(QFile::ReadOnly))
            fileContent=file.readAll();

        return fileContent;
//        if(outVer!=outVersionBefore||innerVer!=innerVersionBefore)
//        {
//            outVersionAim=outVer;
//            innerVersionAim=innerVer;
//            return files.first().filePath();
//        }
    }
    return QByteArray();
}
void EraseWriteProgram::writeData()
{
    QByteArray data=upgradeFileData();
    QByteArray dataframe = getWriteCMD(data);
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("写入数据命令帧:%1").arg(QString(dataframe.toHex())));
    sendSrcMsg(m_dvcType,p_CtrInfoList->at(0)->dvcId,dataframe);
    delay(1500);
    QByteArray sendData;
    int sendCout=0;
    int sendLength=512;
    int i=0;
    while (data.length()!=sendCout)
    {
        delay(80);
        if((data.length()-sendCout)<sendLength)
        {
            sendLength = data.length()-sendCout;
        }
        sendData=data.mid(sendCout,sendLength);
        sendCout=sendCout+sendLength;
        if(sendData.length()==0)
        {
            QString s = "123";
        }
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("写入Flash数据【%0】:%1").arg(QString::number(i)).arg(QString(sendData.toHex())));
        sendSrcMsg(m_dvcType,p_CtrInfoList->at(0)->dvcId,sendData);
        i++;
    }
}
