#include "Script_TransparentTrans.h"

Script_TransparentTrans::Script_TransparentTrans(QObject *parent) : QObject(parent)
{
    emScriptRunState=TestInit;
    emSetQueryState=Wait_SetDAC_Finish;
    resultFlag=false;
    sendMsgOct.clear();

    p_CtrInfoList=make_shared<QList<shared_ptr<CtrInfo>>>();
    p_BuildNetwork_GW=new BuildNetwork_GW();

    p_Frame3762Helper=make_shared<Frame3762Helper>();
    p_TransmitData_02F1=make_shared<Afn02F1>();

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
Script_TransparentTrans::~Script_TransparentTrans()
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
void Script_TransparentTrans::execute()
{
    // 使用全部档案
    QString step_desc = "流程描述: \r";
    step_desc += "1.组网通用流程，SENCE3(需要重新组网)  Wait_TransmitData02F1_1_Finish\r";
    step_desc += "2.02F1数据转发：针对档案类型为97的，通信协议类型0x01，抄读97电表数据；针对档案类型为645的，通信协议类型0x02，抄读645电表数据；针对档案类型为oop的，通信协议类型0x03，抄读oop电表数据 Wait_TransmitData02F1_1_Finish\r";
    step_desc += "3.02F1数据转发：通信协议类型均为0x00（透明传输），针对档案类型为97的，抄读97电表数据；针对档案类型为645的，抄读645电表数据；针对档案类型为oop的，抄读oop电表数据 Wait_TransmitData02F1_2_Finish\r";
    step_desc += "4.02F1数据转发：通信协议类型0x04，报文内容为应用层扩展报文（“参数查询”）Wait_TransmitData02F1_3_Finish\n";
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, step_desc);

    p_AbstractScriptHost->updateProgress(ProcessState_Start, "Test start!");

    resultFlag=false;
    index=0;
    if(needBuildNet==true)
    {
        p_BuildNetwork_GW->execute();
        emScriptRunState=Wait_BuildNetFinish_Whole;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "等待全网组网完成");
        p_timer->start((timerForReachThresld+timerAfterReachThresld)*1000);
    }
    else
    {
        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_TransmitData_02F1);
        emScriptRunState=Wait_TransmitData02F1_1_Finish;
        emSetQueryState=Wait_SetDAC_Finish;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--转发数据（02F1），等待--确认");
        p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
    }
    p_maxAllowTimer->start((timerForReachThresld+timerAfterReachThresld)*1000);

}
void Script_TransparentTrans::stop()
{
    if(p_AbstractScriptHost==nullptr)
        return;
    p_AbstractScriptHost->updateProgress(ProcessState_Error, "Test stop!");
    if(needPowerOff==true)
        powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost);
}
void Script_TransparentTrans::addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq)
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
void Script_TransparentTrans::setHost(AbstractScriptHost *host)
{
    p_AbstractScriptHost=host;
    p_BuildNetwork_GW->setHost(host);
}
bool Script_TransparentTrans::config(const QMap<QString,QString> *paraDic)
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
        if(paraDic->keys().contains("transSucRateThresld"))
        {
            this->transSucRateThresld = (*paraDic)["transSucRateThresld"].toDouble();
        }
        result = true;
    }
    return result;
}
void Script_TransparentTrans::processMsg(DvcType dvcType, int id, uchar *data, int datalen)
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
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "开始第一轮透传");
                emScriptRunState=Wait_TransmitData02F1_1_Finish;

                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_TransmitData_02F1);
          //      emSetQueryState=Wait_SetDAC_Finish;
                p_timer->start(35*1000);
            }
        }
    }
    else
    {
        QByteArray tmpRecvTempData=QByteArray::fromRawData(reinterpret_cast<char*>(data),datalen);
        QByteArray recvTempData;
        recvTempData.append(tmpRecvTempData);
        delete[] data;

//        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到报文：%1").arg(QString(recvTempData.toHex())));

        if(dvcType==CCO_GW || dvcType==CCO_NW)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到来自CCO报文：%1").arg(QString(recvTempData.toHex())));

            if(dvcType == p_CtrInfoList->at(0)->slotPosition && id == p_CtrInfoList->at(0)->dvcId)
            {
                p_CtrInfoList->at(0)->buf.append(recvTempData);
                processMsgFromCCO(dvcType,id);
            }
        }
        else if(dvcType==SingleSTA || dvcType==ThreeSTA)
        {
            if(dvcType==SingleSTA)
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到来自单通报文：%1").arg(QString(recvTempData.toHex())));
            else
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到来自三通报文：%1").arg(QString(recvTempData.toHex())));

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
void Script_TransparentTrans::processCtrlDvcRes(DvcType dvcType, QList<int> idList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params)
{
    if(isSucs==false)
        return;
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到控制命令回复，参数个数=%1").arg(QString::number(params.size())));
    QList<int> sendParams;
    switch(emScriptRunState)
    {
        case TestInit:
        {
            break;
        }
        case Wait_BuildNetFinish_Whole:
        {
            p_BuildNetwork_GW->processCtrlDvcRes(dvcType,idList,ctrlCmdType,isSucs,params);
            break;
        }
        case Wait_TransmitData02F1_1_Finish:
        {
            break;
        }
        case ScriptSuccess:
        {
            break;
        }
        default:
        {
            break;
        }
    }
}

void Script_TransparentTrans::processMsgFromCCO(DvcType dvcType, int dvcId)
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
            case TestInit:
            {
                break;
            }
            case Wait_BuildNetFinish_Whole:
            {
                break;
            }
            case Wait_TransmitData02F1_1_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x02&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    shared_ptr<Afn02F1> p_TransmitData_02F1_Up=dynamic_pointer_cast<Afn02F1>(p_Frame3762Base);

                    if(p_TransmitData_02F1_Up->frame_length_>16)
                    {
                        sucNum++;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("透传回复数据正常，继续下一个：%1").arg(QString::fromLatin1(p_TransmitData_02F1_Up->frame_content_.toHex().toUpper())));
                        if(++index == p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size())
                        {
                            if(failAddrs.size()>0)
                                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "全部档案透传一轮结束，失败表：\n" + failAddrs);
                            else
                                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "全部档案透传一轮结束，无失败表\n");

                            double sucRate = double(sucNum)/double(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size());
                            int failNum = p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size() - sucNum;
                            if(sucRate >= transSucRateThresld)
                            {
                                p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("透传成功率%1%达到要求，失败表数%2 \n").arg(sucRate*100).arg(failNum));

                                index = 0;
                                sucNum = 0;
                                emScriptRunState = Wait_TransmitData02F1_2_Finish;
                                tryTimes=0;
                                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "开始第二轮透传--通信协议类型均为0x00（透明传输）");
                                sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, INSIGNIFICANCE, p_TransmitData_02F1);
                                p_timer->start(35*1000);

                            }
                            else
                            {
                                p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("Wait_TransmitData02F1_1_Finish 透传成功率%1%未达到要求，失败表数%2 【GW-CCO-F006-0001-V01】").arg(sucRate*100).arg(failNum));
                            }
                        }
                        else
                        {
                            // 继续剩余档案的透传
                            tryTimes = 0;
                            sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, INSIGNIFICANCE, p_TransmitData_02F1);
                            p_timer->start(35*1000);
                        }
                    }
                    else if(p_TransmitData_02F1_Up->frame_length_==0)
                    {
                        if(++tryTimes>=3)
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("透传3次回复数据均为空，表号%0，换下一个表").arg(QString(QByteArray(p_TransmitData_02F1_Up->address_field_.src_addr,6).toHex())));
                            failAddrs.append(QByteArray(p_TransmitData_02F1_Up->address_field_.src_addr,6).toHex()).append('\n');

                            if(++index==p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size())
                            {
                                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "全部档案透传一轮结束，失败表：\n" + failAddrs);
                                double sucRate = double(sucNum)/double(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size());
                                int failNum = p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size() - sucNum;
                                if(sucRate >= transSucRateThresld)
                                {
                                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("透传成功率%1%达到要求，失败表数%2").arg(sucRate*100).arg(failNum));

                                    index = 0;
                                    sucNum = 0;
                                    emScriptRunState = Wait_TransmitData02F1_2_Finish;
                                    tryTimes=0;
                                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "开始第二轮透传--通信协议类型均为0x00（透明传输）");
                                    sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, INSIGNIFICANCE, p_TransmitData_02F1);
                                    p_timer->start(35*1000);
                                }
                                else
                                {
                                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("Wait_TransmitData02F1_1_Finish 透传成功率%1%未达到要求，失败表数%2 【GW-CCO-F006-0001-V01】").arg(sucRate*100).arg(failNum));
                                }
                            }
                            else
                            {
                                tryTimes=0;
                                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_TransmitData_02F1);
                                p_timer->start(35*1000);
                            }
                        }
                        else
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "透传回复数据为空，重发02F1--");

                            sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, INSIGNIFICANCE, p_TransmitData_02F1);
                            p_timer->start(35*1000);
                        }
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("Wait_TransmitData02F1_1_Finish 透传回复数据异常，数据:%1 【GW-CCO-F006-0001-V01】").arg(QString::fromLatin1(p_TransmitData_02F1_Up->frame_content_.toHex().toUpper())));
                    }
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
        case Wait_TransmitData02F1_2_Finish:
        {
            if(p_Frame3762Base->afn_ == 0x02&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                shared_ptr<Afn02F1> p_TransmitData_02F1_Up=dynamic_pointer_cast<Afn02F1>(p_Frame3762Base);

                if(p_TransmitData_02F1_Up->frame_length_>16)
                {
                    sucNum++;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("透传回复数据正常，继续下一个：%1").arg(QString::fromLatin1(p_TransmitData_02F1_Up->frame_content_.toHex().toUpper())));
                    if(++index==p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size())
                    {
                        // 档案表全部抄读完成
                        if(failAddrs.size()>0)
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "全部档案透传一轮结束，失败表：\n" + failAddrs);
                        else
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "全部档案透传一轮结束，无失败表\n");

                        double sucRate = double(sucNum)/double(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size());
                        int failNum = p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size() - sucNum;
                        if(sucRate >= transSucRateThresld)
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("透传成功率%1%达到要求，失败表数%2 \n").arg(sucRate*100).arg(failNum));

                            index = 0;
                            tryTimes=0;
                            p_AbstractScriptHost->updateProgress(ProcessState_Success, QString("脚本执行成功【GW-CCO-F006-0001-V01】"));
                            emScriptRunState = ScriptSuccess;
//                            index = 0;
//                            sucNum = 0;
//                            emScriptRunState = Wait_TransmitData02F1_3_Finish;
//                            tryTimes=0;
//                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "开始第三轮透传--通信协议类型0x04，报文内容为应用层扩展报文，如“参数查询”");
//                            emSetQueryState = Wait_QueryDAC_Finish;
//                            sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, INSIGNIFICANCE, p_TransmitData_02F1);
//                            p_timer->start(35*1000);
                        }
                        else
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("Wait_TransmitData02F1_2_Finish 透传成功率%1%未达到要求，失败表数%2 【GW-CCO-F006-0001-V01】").arg(sucRate*100).arg(failNum));
                        }
                    }
                    else
                    {
                        // 继续剩余档案的透传
                        tryTimes=0;
                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_TransmitData_02F1);
                        p_timer->start(35*1000);
                    }
                }
                else if(p_TransmitData_02F1_Up->frame_length_==0)
                {
                    if(++tryTimes>=3)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("透传3次回复数据均为空，表号%0，换下一个表").arg(QString(QByteArray(p_TransmitData_02F1_Up->address_field_.src_addr,6).toHex())));
                        failAddrs.append(QByteArray(p_TransmitData_02F1_Up->address_field_.src_addr,6).toHex()).append('\n');

                        if(++index == p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size())
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "全部档案透传一轮结束，失败表：\n" + failAddrs);
                            double sucRate = double(sucNum)/double(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size());
                            int failNum = p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size() - sucNum;
                            if(sucRate >= transSucRateThresld)
                            {
                                p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("Wait_TransmitData02F1_2_Finish 透传成功率%1%达到要求，失败表数%2").arg(sucRate*100).arg(failNum));

                                index = 0;
                                tryTimes=0;
                                p_AbstractScriptHost->updateProgress(ProcessState_Success, QString("脚本执行成功【GW-CCO-F006-0001-V01】"));
                                emScriptRunState = ScriptSuccess;
//                                index = 0;
//                                emScriptRunState = Wait_TransmitData02F1_3_Finish;
//                                tryTimes=0;
//                                emSetQueryState = Wait_QueryDAC_Finish;
//                                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "开始第三轮透传--通信协议类型0x04，报文内容为应用层扩展报文，如“参数查询");
//                                sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, INSIGNIFICANCE, p_TransmitData_02F1);
//                                p_timer->start(35*1000);
                            }
                            else
                            {
                                p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("Wait_TransmitData02F1_2_Finish 透传成功率%1%未达到要求，失败表数%2 【GW-CCO-F006-0001-V01】").arg(sucRate*100).arg(failNum));
                            }
                        }
                        else
                        {
                            tryTimes=0;
                            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_TransmitData_02F1);
                            p_timer->start(35*1000);
                        }
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "透传回复数据为空，重发02F1--");

                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_TransmitData_02F1);
                        p_timer->start(35*1000);
                    }
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("Wait_TransmitData02F1_2_Finish 透传回复数据异常，数据:%1 【GW-CCO-F006-0001-V01】").arg(QString::fromLatin1(p_TransmitData_02F1_Up->frame_content_.toHex().toUpper())));
                }
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case Wait_TransmitData02F1_3_Finish:
        {
            if(p_Frame3762Base->afn_ == 0x02&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                shared_ptr<Afn02F1> p_TransmitData_02F1_Up=dynamic_pointer_cast<Afn02F1>(p_Frame3762Base);

                if(p_TransmitData_02F1_Up->frame_length_>16)
                {
                    shared_ptr<ChkParam_Up> p_ChkParam_Up = ChkParam_Up::decode_ChkParam_Up(&p_TransmitData_02F1_Up->frame_content_);

                    uchar head_app[4] = {0xfe,0xf8,0x0f,0x00};
                    if(memcmp(head_app, p_ChkParam_Up->fixedHead, 4)==0)
                    {
                        sucNum++;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("透传应用层报文-回复数据正常：%1").arg(QString::fromLatin1(p_TransmitData_02F1_Up->frame_content_.toHex().toUpper())));
                    }

                    if(++index==p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size())
                    {
                        // 档案表全部透传查询完毕
                        if(failAddrs.size()>0)
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "全部档案透传一轮结束，失败表：\n" + failAddrs);
                        else
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "全部档案透传一轮结束，无失败表\n");

                        double sucRate = double(sucNum)/double(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size());
                        int failNum = p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size() - sucNum;
                        if(sucRate >= transSucRateThresld)
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("透传成功率%1%达到要求，失败表数%2").arg(sucRate*100).arg(failNum));
                            p_AbstractScriptHost->updateProgress(ProcessState_Success, QString("脚本执行成功【GW-CCO-F006-0001-V01】"));
                            emScriptRunState = ScriptSuccess;
                        }
                        else
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("Wait_TransmitData02F1_3_Finish 透传成功率%1%未达到要求，失败表数%2 【GW-CCO-F006-0001-V01】")
                                                                 .arg(sucRate*100).arg(failNum));
                        }
                    }
                    else
                    {
                        // 继续剩余档案的透传
                        tryTimes=0;
                        sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, INSIGNIFICANCE, p_TransmitData_02F1);
                        p_timer->start(35*1000);
                    }
                }
                else if(p_TransmitData_02F1_Up->frame_length_==0)
                {
                    if(++tryTimes>=3)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("透传3次回复数据均为空，表号%1").arg(QString(QByteArray(p_TransmitData_02F1_Up->address_field_.src_addr,6).toHex())));
                        failAddrs.append(QByteArray(p_TransmitData_02F1_Up->address_field_.src_addr,6).toHex()).append('\n');

                        if(++index == p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size())
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "全部档案透传一轮结束，失败表：\n" + failAddrs);
                            double sucRate = double(sucNum)/double(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size());
                            int failNum = p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size() - sucNum;
                            if(sucRate >= transSucRateThresld)
                            {
                                p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("透传成功率%1%达到要求，失败表数%2").arg(sucRate*100).arg(failNum));
                                p_AbstractScriptHost->updateProgress(ProcessState_Success, QString("脚本执行成功【GW-CCO-F006-0001-V01】"));
                                emScriptRunState = ScriptSuccess;
                            }
                            else
                            {
                                p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("Wait_TransmitData02F1_3_Finish 透传成功率%1%未达到要求，失败表数%2 【GW-CCO-F006-0001-V01】").arg(sucRate*100).arg(failNum));
                            }
                        }
                        else
                        {
                            tryTimes=0;
                            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE, p_TransmitData_02F1);
                            p_timer->start(35*1000);
                        }
                    }
                    else
                    {
                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_TransmitData_02F1);
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "透传回复数据为空，重发02F1--");
                        p_timer->start(35*1000);
                    }
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("Wait_TransmitData02F1_3_Finish 透传回复数据异常，数据:%1 【GW-CCO-F006-0001-V01】").arg(QString::fromLatin1(p_TransmitData_02F1_Up->frame_content_.toHex().toUpper())));
                }
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }

            //                    switch(emSetQueryState)
            //                    {
            //                        case Wait_SetDAC_Finish:
            //                        {
            //                            shared_ptr<SetParam_Up> p_SetParam_Up=SetParam_Up::decode_SetParam_Up(&p_TransmitData_02F1_Up->frame_content_);
            //                            if(p_SetParam_Up->paramSetRlstInfoList.at(0).setRes==0x00&&p_SetParam_Up->paramSetRlstInfoList.at(1).setRes==0x00)
            //                            {
            //                                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "参数1设置成功");
            //                                emSetQueryState=Wait_QueryDAC_Finish;
            //                                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_TransmitData_02F1);
            //                                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--转发数据（02F1），等待--确认");
            //                                p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
            //                            }
            //                            else
            //                            {
            //                                p_AbstractScriptHost->updateProgress(ProcessState_Failed, "参数1设置失败");
            //                            }
            //                            break;
            //                        }
            //                        case Wait_QueryDAC_Finish:
            //                        {
            //                            shared_ptr<ChkParam_Up> p_ChkParam_Up=ChkParam_Up::decode_ChkParam_Up(&p_TransmitData_02F1_Up->frame_content_);
            //                            if(true)//(p_ChkParam_Up->paramInfoList.at(0).idCntnt.at(0)==0x04&&p_ChkParam_Up->paramInfoList.at(1).idCntnt.at(0)==0x00)
            //                            {
            //                                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "参数1查询成功");
            //                                emSetQueryState=Wait_SetFrameDetect_Finish;
            //                                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_TransmitData_02F1);
            //                                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--转发数据（02F1），等待--确认");
            //                                p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
            //                            }
            //                            else
            //                            {
            //                                p_AbstractScriptHost->updateProgress(ProcessState_Failed, "参数1查询失败");
            //                            }
            //                            break;
            //                        }
            //                        case Wait_SetFrameDetect_Finish:
            //                        {
            //                            shared_ptr<SetParam_Up> p_SetParam_Up=SetParam_Up::decode_SetParam_Up(&p_TransmitData_02F1_Up->frame_content_);
            //                            //存在问题
            //                            if(p_SetParam_Up->paramSetRlstInfoList.at(0).setRes==0x00&&p_SetParam_Up->paramSetRlstInfoList.at(1).setRes==0x00)
            //                            {
            //                                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "参数2设置成功");
            //                                emSetQueryState=Wait_QueryFrameDetect_Finish;
            //                                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_TransmitData_02F1);
            //                                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--转发数据（02F1），等待--确认");
            //                                p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
            //                            }
            //                            else
            //                            {
            //                                p_AbstractScriptHost->updateProgress(ProcessState_Failed, "参数2设置失败");
            //                            }
            //                            break;
            //                        }
            //                        case Wait_QueryFrameDetect_Finish:
            //                        {
            //                            shared_ptr<ChkParam_Up> p_ChkParam_Up=ChkParam_Up::decode_ChkParam_Up(&p_TransmitData_02F1_Up->frame_content_);
            //                            QByteArray multiRes=QByteArray::fromHex(multi.toLatin1());
            //                            QByteArray thresholdRes=QByteArray::fromHex(threshold.toLatin1());
            //                            if(true)//(p_ChkParam_Up->paramInfoList.at(0).idCntnt==multiRes&&p_ChkParam_Up->paramInfoList.at(1).idCntnt==thresholdRes)
            //                            {
            //                                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "参数2查询成功，符合要求");
            //                                emSetQueryState=Wait_SetSuppress_Finish;
            //                                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_TransmitData_02F1);
            //                                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--转发数据（02F1），等待--确认");
            //                                p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
            //                            }
            //                            else
            //                            {
            //                                p_AbstractScriptHost->updateProgress(ProcessState_Failed, "参数2查询失败，不符合要求");
            //                            }
            //                            break;
            //                        }
            //                        case Wait_SetSuppress_Finish:
            //                        {
            //                            shared_ptr<SetParam_Up> p_SetParam_Up=SetParam_Up::decode_SetParam_Up(&p_TransmitData_02F1_Up->frame_content_);
            //                            if(p_SetParam_Up->paramSetRlstInfoList.at(0).setRes==0x00)
            //                            {
            //                                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "参数3设置成功");
            //                                emSetQueryState=Wait_QuerySuppress_Finish;
            //                                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_TransmitData_02F1);
            //                                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--转发数据（02F1），等待--确认");
            //                                p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
            //                            }
            //                            else
            //                            {
            //                                p_AbstractScriptHost->updateProgress(ProcessState_Failed, "参数3设置失败");
            //                            }
            //                            break;
            //                        }
            //                        case Wait_QuerySuppress_Finish:
            //                        {
            //                            shared_ptr<ChkParam_Up> p_ChkParam_Up=ChkParam_Up::decode_ChkParam_Up(&p_TransmitData_02F1_Up->frame_content_);
            //                            QByteArray suppressRes=QByteArray::fromHex(suppress.toLatin1());
            //                            if(true)//(p_ChkParam_Up->paramInfoList.at(0).idCntnt==suppressRes)
            //                            {
            //                                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "参数3查询成功，符合要求");
            //                                emSetQueryState=Wait_SetRelateThreshold_Finish;
            //                                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_TransmitData_02F1);
            //                                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--转发数据（02F1），等待--确认");
            //                                p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
            //                            }
            //                            else
            //                            {
            //                                p_AbstractScriptHost->updateProgress(ProcessState_Failed, "参数3查询失败，不符合要求");
            //                            }
            //                            break;
            //                        }
            //                        case Wait_SetRelateThreshold_Finish:
            //                        {
            //                            shared_ptr<SetParam_Up> p_SetParam_Up=SetParam_Up::decode_SetParam_Up(&p_TransmitData_02F1_Up->frame_content_);
            //                            //不能设置一样的参数
            //                            if(p_SetParam_Up->paramSetRlstInfoList.at(0).setRes==0x03||p_SetParam_Up->paramSetRlstInfoList.at(0).setRes==0x00)
            //                            {
            //                                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "参数4设置成功");
            //                                emSetQueryState=Wait_QueryRelateThreshold_Finish;
            //                                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_TransmitData_02F1);
            //                                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--转发数据（02F1），等待--确认");
            //                                p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
            //                            }
            //                            else
            //                            {
            //                                p_AbstractScriptHost->updateProgress(ProcessState_Failed, "参数4设置失败");
            //                            }
            //                            break;
            //                        }
            //                        case Wait_QueryRelateThreshold_Finish:
            //                        {
            //                            shared_ptr<ChkParam_Up> p_ChkParam_Up=ChkParam_Up::decode_ChkParam_Up(&p_TransmitData_02F1_Up->frame_content_);

            //                            if(true)//(p_ChkParam_Up->paramInfoList.at(0).idCntnt.at(0)==relateThreshold)
            //                            {
            //                                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "参数4查询成功，符合要求");
            //                                emScriptRunState=ScriptSuccess;
            //                                resultFlag=true;
            //                                p_AbstractScriptHost->updateProgress(ProcessState_Success, QString("透传功能测试成功;"));
            //                            }
            //                            else
            //                            {
            //                                p_AbstractScriptHost->updateProgress(ProcessState_Failed, "参数4查询失败，不符合要求");
            //                            }
            //                            break;
            //                        }
            //                    }

            case ScriptSuccess:
            {
                break;
            }
        }
    }
}
void Script_TransparentTrans::processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID)
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
            case TestInit:
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
void Script_TransparentTrans::processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID)
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
            case TestInit:
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
void Script_TransparentTrans::sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame)
{
    if(frame==p_TransmitData_02F1)
    {
        p_TransmitData_02F1->ctrl_field_.dir=kDirDown;
        p_TransmitData_02F1->ctrl_field_.prm=kActive;
        p_TransmitData_02F1->ctrl_field_.comn_type=kHplc;

        p_TransmitData_02F1->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_TransmitData_02F1->info_field_.info_field_down.comu_rate=0;
        p_TransmitData_02F1->info_field_.info_field_down.comu_module_ident=1;

        uchar tmpAddr[6];
        memcpy(tmpAddr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(index)->mtrAddr,6);
        uchar comPrtclType=p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(index)->prtcl;

        memcpy(&p_TransmitData_02F1->address_field_.src_addr,p_CtrInfoList->at(0)->ccoAddr,6);
        memcpy(&p_TransmitData_02F1->address_field_.dst_addr,tmpAddr,6);

        if(emScriptRunState == Wait_TransmitData02F1_1_Finish)
        {
            if(comPrtclType == 0x01)
            {
                // 97表-645
                uchar CrntPosEneTotal[4]={0x00,0x00,0x01,0x00}; //DI0_DI3
                shared_ptr<Rqst_ReadData_0x11> p_ReadData_0x11=make_shared<Rqst_ReadData_0x11>(addr,4,0);
                //reverseAddr(tmpAddr, 6);
                memcpy(p_ReadData_0x11->addr_,tmpAddr,6);
                memcpy(p_ReadData_0x11->di,CrntPosEneTotal,4);
                QByteArray msg645=p_ReadData_0x11->EncodeFrame();
                p_TransmitData_02F1->protocol_type_=0x01;
                p_TransmitData_02F1->frame_content_=msg645;
            }
            else if(comPrtclType == 0x02)
            {
                // 07表-645
                uchar CrntPosEneTotal[4]={0x00,0x00,0x01,0x00}; //DI0_DI3
                shared_ptr<Rqst_ReadData_0x11> p_ReadData_0x11=make_shared<Rqst_ReadData_0x11>(addr,4,0);
                //reverseAddr(tmpAddr, 6);
                memcpy(p_ReadData_0x11->addr_,tmpAddr,6);
                memcpy(p_ReadData_0x11->di,CrntPosEneTotal,4);
                QByteArray msg645=p_ReadData_0x11->EncodeFrame();
                p_TransmitData_02F1->protocol_type_=0x02;
                p_TransmitData_02F1->frame_content_=msg645;
            }
            else if(comPrtclType == 0x03)
            {
                // oop表-698
                shared_ptr<GetRequestNormal> p_GetRequestNormal_ReadData = make_shared<GetRequestNormal>();
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

                QByteArray msg698 = p_GetRequestNormal_ReadData->EncodeFrame();
                p_TransmitData_02F1->protocol_type_=0x03;
                p_TransmitData_02F1->frame_content_=msg698;
            }
            else
                return;
        }
        else if(emScriptRunState == Wait_TransmitData02F1_2_Finish)
        {
            p_TransmitData_02F1->protocol_type_=0x00;
            if(comPrtclType == 0x01)
            {
                // 97表-645
                uchar CrntPosEneTotal[4]={0x00,0x00,0x01,0x00}; //DI0_DI3
                shared_ptr<Rqst_ReadData_0x11> p_ReadData_0x11=make_shared<Rqst_ReadData_0x11>(addr,4,0);
                //reverseAddr(tmpAddr, 6);
                memcpy(p_ReadData_0x11->addr_,tmpAddr,6);
                memcpy(p_ReadData_0x11->di,CrntPosEneTotal,4);
                QByteArray msg645=p_ReadData_0x11->EncodeFrame();

                p_TransmitData_02F1->frame_content_=msg645;
            }
            else if(comPrtclType == 0x02)
            {
                // 07表-645
                uchar CrntPosEneTotal[4]={0x00,0x00,0x01,0x00}; //DI0_DI3
                shared_ptr<Rqst_ReadData_0x11> p_ReadData_0x11=make_shared<Rqst_ReadData_0x11>(addr,4,0);
                //reverseAddr(tmpAddr, 6);
                memcpy(p_ReadData_0x11->addr_,tmpAddr,6);
                memcpy(p_ReadData_0x11->di,CrntPosEneTotal,4);
                QByteArray msg645=p_ReadData_0x11->EncodeFrame();

                p_TransmitData_02F1->frame_content_=msg645;
            }
            else if(comPrtclType == 0x03)
            {
                // oop表-698
                shared_ptr<GetRequestNormal> p_GetRequestNormal_ReadData = make_shared<GetRequestNormal>();
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

                QByteArray msg698 = p_GetRequestNormal_ReadData->EncodeFrame();

                p_TransmitData_02F1->frame_content_=msg698;
            }
            else
                return;
        }
        else if(emScriptRunState == Wait_TransmitData02F1_3_Finish)
        {
            // 应用层扩展报文--系统信息查询
            p_TransmitData_02F1->protocol_type_=0x04;

            switch(emSetQueryState)
            {
                case Wait_SetDAC_Finish:
                {
                    p_TransmitData_02F1->frame_content_=getSetFrame(DAC);
                    break;
                }
                case Wait_QueryDAC_Finish:
                {
                    p_TransmitData_02F1->frame_content_=getQueryFrame(DAC);
                    break;
                }
                case Wait_SetFrameDetect_Finish:
                {
                    p_TransmitData_02F1->frame_content_=getSetFrame(FrameDetect);
                    break;
                }
                case Wait_QueryFrameDetect_Finish:
                {
                    p_TransmitData_02F1->frame_content_=getQueryFrame(FrameDetect);
                    break;
                }
                case Wait_SetSuppress_Finish:
                {
                    p_TransmitData_02F1->frame_content_=getSetFrame(Suppress);
                    break;
                }
                case Wait_QuerySuppress_Finish:
                {
                    p_TransmitData_02F1->frame_content_=getQueryFrame(Suppress);
                    break;
                }
                case Wait_SetRelateThreshold_Finish:
                {
                    p_TransmitData_02F1->frame_content_=getSetFrame(RelateThreshold);
                    break;
                }
                case Wait_QueryRelateThreshold_Finish:
                {
                    p_TransmitData_02F1->frame_content_=getQueryFrame(RelateThreshold);
                    break;
                }
            }
        }

        p_TransmitData_02F1->frame_length_ = uchar(p_TransmitData_02F1->frame_content_.size());

        sendMsgOct=p_TransmitData_02F1->EncodeFrame();
        sendMsgLog=QString("》》转发数据02F1：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
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
void Script_TransparentTrans::sendSrcMsg(DvcType dvcType, int dvcId, QByteArray msg)
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

void Script_TransparentTrans::timer_timeoutProc()
{
    switch(emScriptRunState)
    {
        case Wait_BuildNetFinish_Whole:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_BuildNetFinish_Whole timeout!!!【GW-CCO-F006-0001-V01】");
            break;
        }
        case Wait_TransmitData02F1_1_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_TransmitData02F1_1_Finish timeout!!!【GW-CCO-F006-0001-V01】");
            break;
        }
        case Wait_TransmitData02F1_2_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_TransmitData02F1_2_Finish timeout!!!【GW-CCO-F006-0001-V01】");
            break;
        }
        case Wait_TransmitData02F1_3_Finish:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_TransmitData02F1_3_Finish timeout!!!【GW-CCO-F006-0001-V01】");
            break;
        }
        default:
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==p_timer");
            break;
        }
    }
}
void Script_TransparentTrans::maxAllowTimer_timeoutProc()
{
    p_maxAllowTimer->stop();
    p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==p_maxAllowTimer");
}

void Script_TransparentTrans::delayTimer_timeoutProc()
{
    p_delayTimer->stop();
    index=0;
    p_maxAllowTimer->stop();
}

QByteArray Script_TransparentTrans::getSetFrame(ParaType type)
{
    QByteArray msg;
    shared_ptr<SetParam_Down> p_SetParam_Down=make_shared<SetParam_Down>();
    QList<ParamInfo> setParamInfoList;
    if(type==DAC)
    {
        ParamInfo tmpParamInfo;
        tmpParamInfo.id=39;
        tmpParamInfo.idLen=1;
        tmpParamInfo.idCntnt.append(vol);
        setParamInfoList.append(tmpParamInfo);
        ParamInfo tmpParamInfo1;
        tmpParamInfo1.id=40;
        tmpParamInfo1.idLen=1;
        tmpParamInfo1.idCntnt.append(factor);
        setParamInfoList.append(tmpParamInfo1);
    }
    else if(type==FrameDetect)
    {
        ParamInfo tmpParamInfo;
        tmpParamInfo.id=41;
        tmpParamInfo.idLen=4;
        tmpParamInfo.idCntnt=QByteArray::fromHex(multi.toLatin1());
        setParamInfoList.append(tmpParamInfo);

        tmpParamInfo.id=42;
        tmpParamInfo.idLen=4;
        tmpParamInfo.idCntnt=QByteArray::fromHex(threshold.toLatin1());
        setParamInfoList.append(tmpParamInfo);
    }
    else if(type==Suppress)
    {
        ParamInfo tmpParamInfo;
        tmpParamInfo.id=43;
        tmpParamInfo.idLen=4;
        tmpParamInfo.idCntnt=QByteArray::fromHex(suppress.toLatin1());
        setParamInfoList.append(tmpParamInfo);
    }
    else if(type==RelateThreshold)
    {
        ParamInfo tmpParamInfo;
        tmpParamInfo.id=44;
        tmpParamInfo.idLen=1;
        tmpParamInfo.idCntnt.append(char(relateThreshold));
        setParamInfoList.append(tmpParamInfo);
    }

    uchar tmpCcoAddr[6];
    uchar tmpStaAddr[6];
    memcpy(tmpCcoAddr,p_CtrInfoList->at(0)->ccoAddr,6);
    memcpy(tmpStaAddr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(0)->mtrAddr,6);
    p_SetParam_Down->msgSeq=msgSeq;
    memcpy(p_SetParam_Down->srcAddr,tmpCcoAddr,6);
    memcpy(p_SetParam_Down->dstAddr,tmpStaAddr,6);
    p_SetParam_Down->paramInfoList=setParamInfoList;
    p_SetParam_Down->idCnt=ushort(setParamInfoList.size());

    msg=SetParam_Down::encode_SetParam_Down(p_SetParam_Down);
    return msg;
}

QByteArray Script_TransparentTrans::getQueryFrame(ParaType type)
{
    QByteArray msg;
    shared_ptr<ChkParam_Down> p_ChkParam_Down=make_shared<ChkParam_Down>();
    QList<ushort> chkParamIdList;
    if(type==DAC)
    {
        chkParamIdList.append(39);
        chkParamIdList.append(40);
    }
    else if(type==FrameDetect)
    {
        chkParamIdList.append(41);
        chkParamIdList.append(42);
    }
    else if(type==Suppress)
    {
        chkParamIdList.append(43);
    }
    else if(type==RelateThreshold)
    {
        chkParamIdList.append(44);
    }

    uchar tmpCcoAddr[6];
    uchar tmpStaAddr[6];
    memcpy(tmpCcoAddr,p_CtrInfoList->at(0)->ccoAddr,6);
    memcpy(tmpStaAddr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(index)->mtrAddr,6);
    p_ChkParam_Down->msgSeq=msgSeq;
    memcpy(p_ChkParam_Down->srcAddr,tmpCcoAddr,6);
    memcpy(p_ChkParam_Down->dstAddr,tmpStaAddr,6);
    p_ChkParam_Down->idList=chkParamIdList;
    p_ChkParam_Down->idCnt=ushort(chkParamIdList.size());

    msg=ChkParam_Down::encode_ChkParam_Down(p_ChkParam_Down);
    return msg;
}
