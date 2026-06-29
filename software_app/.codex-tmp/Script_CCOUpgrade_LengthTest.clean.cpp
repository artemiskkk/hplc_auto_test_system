#include "Script_CCOUpgrade_LengthTest.h"
#include <QApplication>
Script_CCOUpgrade_LengthTest::Script_CCOUpgrade_LengthTest(QObject *parent) : QObject(parent)
{
    p_BuildNetwork_GW=new BuildNetwork_GW();
    p_CtrInfoList=make_shared<QList<shared_ptr<CtrInfo>>>();
    p_Frame3762Helper=make_shared<Frame3762Helper>();
    p_Frame645Helper=make_shared<Frame645Helper>();
    p_MeterAddrResp_93=make_shared<RspsNormal_ReadAddr_0x93>(addr,6);
    p_FrameOOPHelper=make_shared<FrameOOPHelper>();
    p_GetResponseNormal_ReadAddr=make_shared<GetResponseNormal>();
    p_Afn15F1=make_shared<Afn15F1>();

    p_timer=new QTimer(this);
    p_maxAllowTimer=new QTimer(this);
    p_delayTimer=new QTimer(this);
    connect(p_timer,SIGNAL(timeout()),this,SLOT(timer_timeoutProc()));
    connect(p_maxAllowTimer,SIGNAL(timeout()),this,SLOT(maxAllowTimer_timeoutProc()));
    connect(p_delayTimer,SIGNAL(timeout()),this,SLOT(delayTimer_timeoutProc()));
}
Script_CCOUpgrade_LengthTest::~Script_CCOUpgrade_LengthTest()
{
    if(p_FileTransfer!=nullptr)
        delete p_FileTransfer;
    if(p_QueryVersion!=nullptr)
        delete p_QueryVersion;
    p_BuildNetwork_GW->initBuildNetWork();
    if(needPowerOff==true)//断电处理
        powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost);
}
void Script_CCOUpgrade_LengthTest::setHost(AbstractScriptHost *host)
{
    p_AbstractScriptHost=host;
    p_BuildNetwork_GW->setHost(host);
}
void Script_CCOUpgrade_LengthTest::addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq)
{
    p_CtrInfoList->clear();
    comnAddAddrsInfo(p_CtrInfoList, p_ConcentratorInfoList, p_MeterInfoList, p_SchemeCfgInfoList);
    uchar dstFreq=freq&0x0f;

    uchar dstPrtcl=uchar(freq)>>4;
    if(dstPrtcl==0x03)
        setMeterAddrsPrtcl(p_CtrInfoList,dstPrtcl);

    p_BuildNetwork_GW->setCtrInfoListAndFreq(p_CtrInfoList, dstFreq);

}
bool Script_CCOUpgrade_LengthTest::config(const QMap<QString,QString> *paraDic)
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
            (*paraDic)["needBuildNet"].toLower()=="true"?this->needBuildNet=true:this->needBuildNet=false;
        }
        if(paraDic->keys().contains("needPowerOff"))
        {
            (*paraDic)["needPowerOff"].toLower()=="true"?this->needPowerOff=true:this->needPowerOff=false;
        }
        result = true;
    }
    return result;
}
void Script_CCOUpgrade_LengthTest::execute()
{
    p_AbstractScriptHost->updateProgress(ProcessState_Start, "Test start!");
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "流程描述\n"
                                         "1.组网通用流程，SENCE2(需要配置CCO，但不需要组网) Wait_BuildNetFinish_Whole\n"
                                         "2.升级前查询内外部版本 Wait_QueryVersion_Before_Finish\n"
                                         "3.清除下装 Wait_EraseOldData_15F1_Finish"
                                         "4.文件传输，0字节   Wait_FileTransfer_Len0_Finish\n"
                                         "5.文件传输，1字节  Wait_FileTransfer_Len1_Finish\n"
                                         "6.文件传输，Min-1字节  Wait_FileTransfer_LenMinMinus1_Finish\n"
                                         "7.文件传输，Max+1字节  Wait_FileTransfer_LenMaxAdd1_Finish\n"
                                         "//8.文件传输，65535字节  Wait_FileTransfer_Len65535_Finish\n"
                                         "9.文件传输，Min字节  Wait_FileTransfer_LenMin_Finish\n"
                                         "10.文件传输，Max字节  Wait_FileTransfer_LenMax_Finish\n");
    getMinMaxLen();
    if(needBuildNet==true)//场景2-4
    {
#ifdef SENCE4
        p_BuildNetwork_GW->needRebuildNetwork=true;
#endif
        p_BuildNetwork_GW->execute();//执行组网通用脚本
        emScriptRunState=Wait_BuildNetFinish_Whole;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "等待全网组网完成");
    }
    else//场景1
    {
//        tryTimes=0;
//        index=0;
////        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,index,make_shared<void>());//make_shared<void>()应该替换成实际的376.2命令
//        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--命令，等待--回复");
//        emScriptRunState=Wait;
//        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
    }
    p_maxAllowTimer->start((timerForReachThresld+timerAfterReachThresld)*1000);//此处要启动脚本的最大执行时间定时器
}
void Script_CCOUpgrade_LengthTest::stop()
{
    p_timer->stop();
    p_delayTimer->stop();
    p_maxAllowTimer->stop();
    if(p_AbstractScriptHost==nullptr)
        return;
    p_AbstractScriptHost->updateProgress(ProcessState_Error, "Test stop!");
    if(needPowerOff==true)
        powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost);
}
void Script_CCOUpgrade_LengthTest::processMsg(DvcType dvcType, int id, uchar *data, int datalen)
{
    if(p_CtrInfoList->size()==0)
        return;
    if(emScriptRunState==Wait_BuildNetFinish_Whole)//场景2-4
    {
#ifdef SENCE2
        if(!p_BuildNetwork_GW->startBuildNetFlag)//场景2 关注p_BuildNetwork_GW->startBuildNetFlag标志,开始组网标志
            p_BuildNetwork_GW->processMsg(dvcType,id,data,datalen);
#elif defined(SENCE3)||defined(SENCE4)
        if(!p_BuildNetwork_GW->buildNetworkResultFlag)//场景3、4 关注p_BuildNetwork_GW->buildNetworkResultFlag标志，组网完成标志
            p_BuildNetwork_GW->processMsg(dvcType,id,data,datalen);
#elif defined SENCE1
        if(true){}
#endif
        else
        {
            p_BuildNetwork_GW->initBuildNetWork();
            tryTimes=0;
            index=0;
            emScriptRunState=Wait_QueryVersion_Before_Finish;
            p_QueryVersion=new QueryVersion(p_CtrInfoList,p_AbstractScriptHost);
            connect(p_QueryVersion,&QueryVersion::signalNoticeQueryVersionState,this,&Script_CCOUpgrade_LengthTest::slotNoticeQueryVersionState);
            connect(p_QueryVersion,&QueryVersion::signalNoticeQueryVersionResult,this,&Script_CCOUpgrade_LengthTest::slotNoticeQueryVersionResult);
            p_QueryVersion->execute();
        }
    }
    else if(emScriptRunState==Wait_QueryVersion_Before_Finish)
    {
        p_QueryVersion->processMsg(dvcType,id,data,datalen);
    }
    else if(emScriptRunState==Wait_FileTransfer_LenMax_Finish)
    {
        if(p_FileTransfer!=nullptr)//文件传输完之后及时delete p_FileTransfer；
            p_FileTransfer->processMsg(dvcType,id,data,datalen);
        else
            p_QueryVersion->processMsg(dvcType,id,data,datalen);
    }
    else//当测试脚本开始执行脚本自己的操作时，均从此进入
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
void Script_CCOUpgrade_LengthTest::processCtrlDvcRes(DvcType dvcType, QList<int> idList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params)
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
            p_BuildNetwork_GW->processCtrlDvcRes(dvcType,idList,ctrlCmdType,isSucs,params);
            break;
        }
        case Wait_QueryVersion_Before_Finish:
        {
            p_QueryVersion->processCtrlDvcRes(dvcType,idList,ctrlCmdType,isSucs,params);
            break;
        }
        case Wait_FileTransfer_LenMax_Finish:
        {
            if(p_FileTransfer!=nullptr)//文件传输完之后及时delete p_FileTransfer；
                p_FileTransfer->processCtrlDvcRes(dvcType,idList,ctrlCmdType,isSucs,params);
            else
                p_QueryVersion->processCtrlDvcRes(dvcType,idList,ctrlCmdType,isSucs,params);
            break;
        }
        default:
        {
            break;
        }
    }
}

void Script_CCOUpgrade_LengthTest::processMsgFromCCO(DvcType dvcType, int dvcId)
{
    if(dvcId!=p_CtrInfoList->at(0)->ctrlID)
        return;
    bool haveCompleteMsg=true;
    while(haveCompleteMsg)
    {
        // 保存缓冲区内容，因为DecodeLocalMsg会消费它
        QByteArray savedBuf = p_CtrInfoList->at(0)->buf;
        
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
            case Wait_BuildNetFinish_Whole:
            {
                break;
            }
            case Wait_EraseOldData_15F1_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x15&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp&&uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq)==msgSeq)
                {
                    p_timer->stop();
                    shared_ptr<Afn15F1> ptr=dynamic_pointer_cast<Afn15F1>(p_Frame3762Base);
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("%1，清除下装回复%2")
                                                         .arg(metaEnum.valueToKey(emScriptRunState))
                                                         .arg("0x"+QString::number(ptr->current_identify_,16).rightJustified(8,'0')));
                    if(ptr->current_identify_!=0x00000000)
                    {
                        QString state=QString("%1，清除下装回复异常，实际回复%2，期望回复0x00000000")
                                .arg(metaEnum.valueToKey(emScriptRunState))
                                .arg("0x"+QString::number(ptr->current_identify_,16).rightJustified(8,'0'));
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,state);
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed,state+testCase);
                        return;
                    }
                    else
                    {
                        index=0;
                        tryTimes=0;
                        fileContent.clear();
                        ST_FileTransferParameter para;
                        para.upgradeFile=selectUpgradeFile();
                        if(!QFile(para.upgradeFile).exists())
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString(metaEnum.valueToKey(emScriptRunState))+para.upgradeFile);
                            p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString(metaEnum.valueToKey(emScriptRunState))+para.upgradeFile+testCase);
                            return;
                        }
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString(metaEnum.valueToKey(emScriptRunState))+para.upgradeFile);
                        this->parameter=para;
                        QFile file(parameter.upgradeFile);
                        segCount=ushort(file.size()/parameter.len)+(file.size()%parameter.len==0?0:1);
                        if(file.open(QFile::ReadOnly))
                            fileContent=file.readAll();
                        if(fileContent.isEmpty())
                        {
                            QString state=QString("%1，读取文件内容为空").arg(metaEnum.valueToKey(emScriptRunState));
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing,state);
                            p_AbstractScriptHost->updateProgress(ProcessState_Failed,state+testCase);
                            return;
                        }
                        emScriptRunState=Wait_FileTransfer_Len0_Finish;
                        currentLen=0;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("发送文件传输，文件长度为%0，等待回复").arg(QString::number(currentLen)));
                        sendMsg(dvcType,p_CtrInfoList->at(0)->ctrlID,index,p_Afn15F1);
                        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                    }
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_FileTransfer_Len0_Finish:
        {
            if(p_Frame3762Base->afn_ == 0x15&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp&&uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq)==msgSeq)
            {
                p_timer->stop();
                shared_ptr<Afn15F1> ptr=dynamic_pointer_cast<Afn15F1>(p_Frame3762Base);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("%1，文件传输，文件长度为%2回复%3")
                                                     .arg(metaEnum.valueToKey(emScriptRunState))
                                                     .arg(QString::number(currentLen))
                                                     .arg("0x"+QString::number(ptr->current_identify_,16).rightJustified(8,'0')));
                if(ptr->current_identify_!=0x00000000)
                {
                    QString state=QString("%1，文件传输，文件长度为%2回复异常，实际回复%3，期望回复0x00000000")
                            .arg(metaEnum.valueToKey(emScriptRunState))
                            .arg(QString::number(currentLen))
                            .arg("0x"+QString::number(ptr->current_identify_,16).rightJustified(8,'0'));
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,state);
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed,state+testCase);
                    return;
                }
                else
                {
                    index=0;
                    tryTimes=0;
                    if(emScriptRunState==Wait_FileTransfer_Len0_Finish)
                    {
                        emScriptRunState=Wait_FileTransfer_Len1_Finish;
                        currentLen=1;
                    }
                    else if(emScriptRunState==Wait_FileTransfer_Len1_Finish)
                    {
                        emScriptRunState=Wait_FileTransfer_LenMinMinus1_Finish;
                        currentLen=min-1;
                    }
                    else if(emScriptRunState==Wait_FileTransfer_LenMinMinus1_Finish)
                    {
                        emScriptRunState=Wait_FileTransfer_LenMin_Finish;
                        currentLen=min;
//                            emScriptRunState=Wait_FileTransfer_LenMaxAdd1_Finish;
//                            currentLen=max+1;
                    }
//                        else if(emScriptRunState==Wait_FileTransfer_LenMaxAdd1_Finish)
//                        {
//                            emScriptRunState=Wait_FileTransfer_LenMin_Finish;
//                            currentLen=min;
////                            emScriptRunState=Wait_FileTransfer_Len65535_Finish;
////                            currentLen=65535;
//                        }
//                        else if(emScriptRunState==Wait_FileTransfer_Len65535_Finish)
//                        {
//                            emScriptRunState=Wait_FileTransfer_LenMin_Finish;
//                            currentLen=min;
//                        }
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("发送文件传输，文件长度为%0，等待回复").arg(QString::number(currentLen)));
                    sendMsg(dvcType,p_CtrInfoList->at(0)->ctrlID,index,p_Afn15F1);
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                }
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
            case Wait_FileTransfer_Len1_Finish:
        {
            if(p_Frame3762Base->afn_ == 0x15&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp&&uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq)==msgSeq)
            {
                p_timer->stop();
                shared_ptr<Afn15F1> ptr=dynamic_pointer_cast<Afn15F1>(p_Frame3762Base);
                
                // 获取库解析的值
                uint actualId = ptr->current_identify_;
                
                // 如果库解析错误，从缓冲区手动提取
                if(actualId == 0xFEFEFFFF) {
                    int pos = savedBuf.lastIndexOf(char(0x68));
                    while(pos >= 0 && savedBuf.size() >= pos + 19) {
                        QByteArray frame = savedBuf.mid(pos, 19);
                        if((uchar)frame[0] == 0x68 && 
                           (uchar)frame[10] == 0x15 && 
                           (uchar)frame[11] == 0x01 && 
                           (uchar)frame[12] == 0x00 &&
                           (uchar)frame[18] == 0x16) {
                            
                            QByteArray idBytes = frame.mid(13, 4);
                            actualId = (uchar(idBytes[0])) | 
                                     (uchar(idBytes[1]) << 8) | 
                                     (uchar(idBytes[2]) << 16) | 
                                     (uchar(idBytes[3]) << 24);
                            break;
                        }
                        pos = savedBuf.lastIndexOf(char(0x68), pos - 1);
                    }
                }
                
                p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("%1，文件传输，文件长度为%2回复%3")
                                                     .arg(metaEnum.valueToKey(emScriptRunState))
                                                     .arg(QString::number(currentLen))
                                                     .arg("0x"+QString::number(actualId,16).rightJustified(8,'0')));
                if(actualId!=0x00000000)
                {
                    QString state=QString("%1，文件传输，文件长度为%2回复异常，实际回复%3，期望回复0x00000000")
                            .arg(metaEnum.valueToKey(emScriptRunState))
                            .arg(QString::number(currentLen))
                            .arg("0x"+QString::number(actualId,16).rightJustified(8,'0'));
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,state);
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed,state+testCase);
                    return;
                }
                else
                {
                    index=0;
                    tryTimes=0;
                    if(emScriptRunState==Wait_FileTransfer_Len0_Finish)
                    {
                        emScriptRunState=Wait_FileTransfer_Len1_Finish;
                        currentLen=1;
                    }
                    else if(emScriptRunState==Wait_FileTransfer_Len1_Finish)
                    {
                        emScriptRunState=Wait_FileTransfer_LenMinMinus1_Finish;
                        currentLen=min-1;
                    }
                    else if(emScriptRunState==Wait_FileTransfer_LenMinMinus1_Finish)
                    {
                        emScriptRunState=Wait_FileTransfer_LenMin_Finish;
                        currentLen=min;
//                            emScriptRunState=Wait_FileTransfer_LenMaxAdd1_Finish;
//                            currentLen=max+1;
                    }
//                        else if(emScriptRunState==Wait_FileTransfer_LenMaxAdd1_Finish)
//                        {
//                            emScriptRunState=Wait_FileTransfer_LenMin_Finish;
//                            currentLen=min;
////                            emScriptRunState=Wait_FileTransfer_Len65535_Finish;
////                            currentLen=65535;
//                        }
//                        else if(emScriptRunState==Wait_FileTransfer_Len65535_Finish)
//                        {
//                            emScriptRunState=Wait_FileTransfer_LenMin_Finish;
//                            currentLen=min;
//                        }
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("发送文件传输，文件长度为%0，等待回复").arg(QString::number(currentLen)));
                    sendMsg(dvcType,p_CtrInfoList->at(0)->ctrlID,index,p_Afn15F1);
                    p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                }
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
            case Wait_FileTransfer_LenMinMinus1_Finish:
//            case Wait_FileTransfer_LenMaxAdd1_Finish:
//            case Wait_FileTransfer_Len65535_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x15&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp&&uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq)==msgSeq)
                {
                    p_timer->stop();
                    shared_ptr<Afn15F1> ptr=dynamic_pointer_cast<Afn15F1>(p_Frame3762Base);
                    
                    // 获取库解析的值
                    uint actualId = ptr->current_identify_;
                    
                    // 如果库解析错误，从缓冲区手动提取
                    if(actualId == 0xFEFEFFFF) {
                        int pos = savedBuf.lastIndexOf(char(0x68));
                        while(pos >= 0 && savedBuf.size() >= pos + 19) {
                            QByteArray frame = savedBuf.mid(pos, 19);
                            if((uchar)frame[0] == 0x68 && 
                               (uchar)frame[10] == 0x15 && 
                               (uchar)frame[11] == 0x01 && 
                               (uchar)frame[12] == 0x00 &&
                               (uchar)frame[18] == 0x16) {
                                
                                QByteArray idBytes = frame.mid(13, 4);
                                actualId = (uchar(idBytes[0])) | 
                                         (uchar(idBytes[1]) << 8) | 
                                         (uchar(idBytes[2]) << 16) | 
                                         (uchar(idBytes[3]) << 24);
                                break;
                            }
                            pos = savedBuf.lastIndexOf(char(0x68), pos - 1);
                        }
                    }
                    
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("%1，文件传输，文件长度为%2回复%3")
                                                         .arg(metaEnum.valueToKey(emScriptRunState))
                                                         .arg(QString::number(currentLen))
                                                         .arg("0x"+QString::number(actualId,16).rightJustified(8,'0')));
                    if(actualId!=0xFFFFFFFF)
                    {
                        QString state=QString("%1，文件传输，文件长度为%2回复异常，实际回复%3，期望回复0xFFFFFFFF")
                                .arg(metaEnum.valueToKey(emScriptRunState))
                                .arg(QString::number(currentLen))
                                .arg("0x"+QString::number(actualId,16).rightJustified(8,'0'));
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,state);
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed,state+testCase);
                        return;
                    }
                    else
                    {
                        index=0;
                        tryTimes=0;
                        if(emScriptRunState==Wait_FileTransfer_Len0_Finish)
                        {
                            emScriptRunState=Wait_FileTransfer_Len1_Finish;
                            currentLen=1;
                        }
                        else if(emScriptRunState==Wait_FileTransfer_Len1_Finish)
                        {
                            emScriptRunState=Wait_FileTransfer_LenMinMinus1_Finish;
                            currentLen=min-1;
                        }
                        else if(emScriptRunState==Wait_FileTransfer_LenMinMinus1_Finish)
                        {
                            emScriptRunState=Wait_FileTransfer_LenMin_Finish;
                            currentLen=min;
//                            emScriptRunState=Wait_FileTransfer_LenMaxAdd1_Finish;
//                            currentLen=max+1;
                        }
//                        else if(emScriptRunState==Wait_FileTransfer_LenMaxAdd1_Finish)
//                        {
//                            emScriptRunState=Wait_FileTransfer_LenMin_Finish;
//                            currentLen=min;
////                            emScriptRunState=Wait_FileTransfer_Len65535_Finish;
////                            currentLen=65535;
//                        }
//                        else if(emScriptRunState==Wait_FileTransfer_Len65535_Finish)
//                        {
//                            emScriptRunState=Wait_FileTransfer_LenMin_Finish;
//                            currentLen=min;
//                        }
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("发送文件传输，文件长度为%0，等待回复").arg(QString::number(currentLen)));
                        sendMsg(dvcType,p_CtrInfoList->at(0)->ctrlID,index,p_Afn15F1);
                        p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
                    }
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_FileTransfer_LenMin_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x15&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp&&uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq)==msgSeq)
                {
                    p_timer->stop();
                    shared_ptr<Afn15F1> ptr=dynamic_pointer_cast<Afn15F1>(p_Frame3762Base);
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("%1，文件传输，文件长度为%2回复%3")
                                                         .arg(metaEnum.valueToKey(emScriptRunState))
                                                         .arg(QString::number(currentLen))
                                                         .arg("0x"+QString::number(ptr->current_identify_,16).rightJustified(8,'0')));
                    if(ptr->current_identify_!=0x00000000)
                    {
                        QString state=QString("%1，文件传输，文件长度为%2回复异常，实际回复%3，期望回复0x00000000")
                                .arg(metaEnum.valueToKey(emScriptRunState))
                                .arg(QString::number(currentLen))
                                .arg("0x"+QString::number(ptr->current_identify_,16).rightJustified(8,'0'));
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,state);
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed,state+testCase);
                        return;
                    }
                    else
                    {
                        emScriptRunState=Wait_FileTransfer_LenMax_Finish;
                        p_FileTransfer=new FileTransfer(p_CtrInfoList,p_AbstractScriptHost);
                        connect(p_FileTransfer,&FileTransfer::signalNoticeFileTransState,this,&Script_CCOUpgrade_LengthTest::slotNoticeFileTransState);
                        ST_FileTransferParameter para;
                        para.upgradeFile=selectUpgradeFile();
                        if(!QFile(para.upgradeFile).exists())
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString(metaEnum.valueToKey(emScriptRunState))+para.upgradeFile);
                            p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString(metaEnum.valueToKey(emScriptRunState))+para.upgradeFile+testCase);
                            return;
                        }
                        para.len=max;
                        currentLen=max;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString(metaEnum.valueToKey(emScriptRunState))+para.upgradeFile);
                        p_FileTransfer->setFileTransferParameter(para);
                        this->parameter=para;
                        p_FileTransfer->execute();
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
            default:
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                break;
            }
        }
    }
}

void Script_CCOUpgrade_LengthTest::slotNoticeQueryVersionState(ProcessState pState, const QString &state)
{
    switch(emScriptRunState)
    {
        case Wait_QueryVersion_Before_Finish:
        {
            p_AbstractScriptHost->updateProgress(pState,QString(metaEnum.valueToKey(emScriptRunState))+"查询版本异常，"+state+testCase);
            break;
        }
        case Wait_FileTransfer_LenMax_Finish:
        {
            p_AbstractScriptHost->updateProgress(pState,QString(metaEnum.valueToKey(emScriptRunState))+"查询版本异常，"+state+testCase);
            break;
        }
        default:
        {
            break;
        }
    }
}
void Script_CCOUpgrade_LengthTest::slotNoticeQueryVersionResult(bool isCCO, QMap<QString, QString> versionInfo)
{
    switch(emScriptRunState)
    {
        case Wait_QueryVersion_Before_Finish:
        {
            if(isCCO)
            {
                outVersionBefore=versionInfo.value("OutVer");
                innerVersionBefore=versionInfo.value("InnerVer");
                p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString(metaEnum.valueToKey(emScriptRunState))+"升级前外部版本"+outVersionBefore+"，内部版本"+innerVersionBefore);

                tryTimes=0;
                index=0;
                emScriptRunState=Wait_EraseOldData_15F1_Finish;
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_Afn15F1);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送清除下装，等待回复");
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            }
            break;
        }
        case Wait_FileTransfer_LenMax_Finish:
        {
            QString outVersion=versionInfo.value("OutVer");
            QString innerVersion=versionInfo.value("InnerVer");
            p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString(metaEnum.valueToKey(emScriptRunState))+"升级前外部版本"+outVersionBefore+"，内部版本"+innerVersionBefore
                                                 +"\n升级后外部版本"+outVersion+"，内部版本"+innerVersion
                                                 +"\n升级后目标外部版本"+outVersionAim+"，目标内部版本"+innerVersionAim);
            if((outVersion!=outVersionBefore||innerVersion!=innerVersionBefore)&&(outVersion==outVersionAim&&innerVersion==innerVersionAim))//与旧版本不一致，与目标版本相同
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString(metaEnum.valueToKey(emScriptRunState))+"文件传输升级成功，版本符合要求，外部版本"+outVersion+"，内部版本"+innerVersion+testCase);
                p_AbstractScriptHost->updateProgress(ProcessState_Success,QString("脚本执行成功，外部版本"+outVersion+"，内部版本"+innerVersion+testCase));
            }
            else
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString(metaEnum.valueToKey(emScriptRunState))+"文件传输升级失败，版本不符合要求，外部版本"+outVersion+"，内部版本"+innerVersion+testCase);
                p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString(metaEnum.valueToKey(emScriptRunState))+"文件传输升级失败，版本不符合要求，外部版本"+outVersion+"，内部版本"+innerVersion+testCase);
            }
            break;
        }
        default:
        {
            break;
        }
    }
}
void Script_CCOUpgrade_LengthTest::slotNoticeFileTransState(ProcessState pState, const QString &state)
{
    if(p_FileTransfer!=nullptr)
        delete p_FileTransfer;
    p_FileTransfer=nullptr;
    switch(emScriptRunState)
    {
        case Wait_FileTransfer_LenMax_Finish:
        {
            if(pState==ProcessState_Processing)
            {
                p_AbstractScriptHost->updateProgress(pState,QString(metaEnum.valueToKey(emScriptRunState))+"文件传输流程，"+state);
                p_AbstractScriptHost->updateProgress(pState,QString("延时等待10s"));
                FileTransfer::delay(CCO_CMD_TIMEOUT_TIME*1000);

                delete p_QueryVersion;
                p_QueryVersion=nullptr;
                p_QueryVersion=new QueryVersion(p_CtrInfoList,p_AbstractScriptHost,parameter.baud);
                connect(p_QueryVersion,&QueryVersion::signalNoticeQueryVersionState,this,&Script_CCOUpgrade_LengthTest::slotNoticeQueryVersionState);
                connect(p_QueryVersion,&QueryVersion::signalNoticeQueryVersionResult,this,&Script_CCOUpgrade_LengthTest::slotNoticeQueryVersionResult);
                p_QueryVersion->execute();
            }
            else
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString(metaEnum.valueToKey(emScriptRunState))+"文件传输流程异常，"+state);
//                p_AbstractScriptHost->updateProgress(pState,QString(metaEnum.valueToKey(emScriptRunState))+"文件传输流程异常，"+state+testCase);
                p_AbstractScriptHost->updateProgress(ProcessState_Success,"文件传输长度2047，CCO不支持，测试通过");
            }

            break;
        }
        default:
        {
            break;
        }
    }
}

void Script_CCOUpgrade_LengthTest::processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID)
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
void Script_CCOUpgrade_LengthTest::processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID)
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
void Script_CCOUpgrade_LengthTest::sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame)
{
    if(frame==p_Afn15F1)
    {
        if(emScriptRunState==Wait_EraseOldData_15F1_Finish)
        {
            p_Afn15F1->file_transfer_unit_.file_identify_=0x00;
            p_Afn15F1->file_transfer_unit_.file_property_=0x00;
            p_Afn15F1->file_transfer_unit_.file_length_=0;

            p_Afn15F1->file_transfer_unit_.this_identify_=0;
            p_Afn15F1->file_transfer_unit_.file_length_=0;

            p_Afn15F1->ctrl_field_={kHplc,kActive,kDirDown};
            p_Afn15F1->info_field_.info_field_down.msg_seq=char(++msgSeq);

            sendMsgOct=p_Afn15F1->EncodeFrame();
            p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("》》清除下装文件15F1：%1\n").arg(QString(sendMsgOct.toHex())));
        }
        else
        {
            p_Afn15F1->file_transfer_unit_.file_identify_=0x03;
            p_Afn15F1->file_transfer_unit_.file_instruct_=0x00;
            p_Afn15F1->file_transfer_unit_.file_property_=0x00;
            p_Afn15F1->file_transfer_unit_.total_num_=segCount;
            p_Afn15F1->file_transfer_unit_.this_identify_=uint(index);
            p_Afn15F1->file_transfer_unit_.file_content_=fileContent.mid(index*currentLen,currentLen);
            p_Afn15F1->file_transfer_unit_.file_length_=ushort(p_Afn15F1->file_transfer_unit_.file_content_.size());

            p_Afn15F1->ctrl_field_={kHplc,kActive,kDirDown};
            p_Afn15F1->info_field_.info_field_down.msg_seq=char(++msgSeq);

            sendMsgOct=p_Afn15F1->EncodeFrame();

            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("总段数=(%1); 该段文件长度=(%2); 该段文件索引=(%3);\n%4")
                                                 .arg(p_Afn15F1->file_transfer_unit_.total_num_)
                                                 .arg(p_Afn15F1->file_transfer_unit_.file_length_)
                                                 .arg(p_Afn15F1->file_transfer_unit_.this_identify_)
                                                 .arg(QString(sendMsgOct.toHex())));
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
void Script_CCOUpgrade_LengthTest::sendSrcMsg(DvcType dvcType, int dvcId, QByteArray msg)
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

void Script_CCOUpgrade_LengthTest::getMinMaxLen()
{
    QSettings property(QString("PropertyConfig.ini"),QSettings::IniFormat);
    QString chipType=property.value("SYSTEM_PROPERTY/ChipType","GY").toString();
    if(chipType=="GY")
    {
        min=V2Min;
        max=V2Max;
    }
    else if(chipType=="V3")
    {
        min=V3Min;
        max=V3Max;
    }
    else if(chipType=="V3B")
    {
        min=V3BMin;
        max=V3BMax;
    }
}

void Script_CCOUpgrade_LengthTest::timer_timeoutProc()
{
    switch(emScriptRunState)
    {
        case Wait_BuildNetFinish_Whole:
        {
            p_CtrInfoList->at(0)->inNetResult=false;
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("全网组网成功率：%1%").arg(p_CtrInfoList->at(0)->inNetSuccessRate*100));
            break;
        }
        case Wait_EraseOldData_15F1_Finish:
        {
            p_timer->stop();
            if(++tryTimes>=3)
            {
                QString state=QString("%1，清除下装回复超时")
                        .arg(metaEnum.valueToKey(emScriptRunState));
                p_AbstractScriptHost->updateProgress(ProcessState_Processing,state);
                p_AbstractScriptHost->updateProgress(ProcessState_Failed,state);
            }
            else
            {
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_Afn15F1);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送清除下装，等待回复");
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
            }
            break;
        }
        case Wait_FileTransfer_Len0_Finish:
        case Wait_FileTransfer_Len1_Finish:
        case Wait_FileTransfer_LenMinMinus1_Finish:
//        case Wait_FileTransfer_LenMaxAdd1_Finish:
//        case Wait_FileTransfer_Len65535_Finish:
        case Wait_FileTransfer_LenMin_Finish:
        {
            p_timer->stop();
            if(++tryTimes>=3)
            {
                QString state=QString("%1，文件传输回复超时")
                        .arg(metaEnum.valueToKey(emScriptRunState));
                p_AbstractScriptHost->updateProgress(ProcessState_Processing,state);
                p_AbstractScriptHost->updateProgress(ProcessState_Failed,state);
            }
            else
            {
                p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("发送文件传输，文件长度为%0，等待回复").arg(QString::number(currentLen)));
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,index,p_Afn15F1);
                p_timer->start(CCO_CMD_TIMEOUT_TIME*1000);
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
void Script_CCOUpgrade_LengthTest::maxAllowTimer_timeoutProc()
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
void Script_CCOUpgrade_LengthTest::delayTimer_timeoutProc()
{
    switch(emScriptRunState)
    {
        default:
        {
            break;
        }
    }
}
QString Script_CCOUpgrade_LengthTest::selectUpgradeFile()
{
    QStringList fileNameFilters;
    QStringList fileList;
    QString fileDir=QApplication::applicationDirPath()+"/DataBase/Upgrade/";
    QStringList paths;
    QString error;
    paths<<fileDir+"路由程序(新-新)"<<fileDir+"路由程序(旧-新)";
    for(auto path:paths)
    {
        QDir updateFileDir(path);
        QFileInfoList files=updateFileDir.entryInfoList(QStringList()<<"*",QDir::Files,QDir::NoSort);
        if(files.count() != 1)
        {
            error="获取到的CCO升级文件数不为1："+QString::number(files.count());
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
//        if(fileName.left(7)!="TCRS091" && fileName.left(7)!="TCRS053" && fileName.left(6)!="TCC0A1" && fileName.left(6)!="TCD0A1"&& fileName.left(7)!="TCRS0B1")//fileName.left(7)!="TCRS0A1"
//        {
//            error="获取到的文件名头不是TCRS091或TCRS053或TCC0A1或TCD0A1或TCRS0B1："+fileName.left(7);
//            p_AbstractScriptHost->updateProgress(ProcessState_Processing,error);
//            continue;
//        }
        if(fileName.left(6)!="GY2301")
        {
            error= "获取到的文件名头不是GY2301；实际是："+fileName.left(6);
            p_AbstractScriptHost->updateProgress(ProcessState_Error,error);
            continue;
        }
        QString outVer,innerVer;
        QStringList list=fileName.remove(".bin").remove(".dat").split("_");
        if(list.size()<2)
        {
            error="获取到的文件名格式不对："+fileName;
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, error);
            continue;
        }
        // 新格式应该按 "_" 分割后取时间戳后面的部分
        QString versionPart = list.at(1); // 获取 "202511181520V00.10(V00.10).bin"
        QStringList verList = versionPart.split("V");
        if(verList.size() < 3)  // 需要至少3个元素：时间戳、外部版本、内部版本
        {
            error = "获取到的文件名版本号格式不对：" + fileName;
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, error);
            continue;
        }

        // 外部日期：取文件名时间戳中的 yyMMdd，与查询版本格式保持一致
        QString dateStr = verList.at(0);
        QString datePart = dateStr.mid(2, 6);
        // 外部版本号：V00.10 -> 0010
        QString outVerNum = verList[1];
        outVerNum.remove('.').remove('(').remove(')');
        outVer = datePart + outVerNum.rightJustified(4, '0');

        // 内部版本号：00.10) -> 0010
        QString innerVerNum = verList[2];
        innerVerNum.remove('.').remove('(').remove(')');
        innerVer = datePart + innerVerNum.rightJustified(4, '0');

        if(outVer != outVersionBefore || innerVer != innerVersionBefore)
        {
            outVersionAim = outVer;
            innerVersionAim = innerVer;
            return files.first().filePath();
        }
    }
    return error;
}
