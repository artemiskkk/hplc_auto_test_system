#include <memory>
using namespace std;
#include "Script_IdManage_STA.h"

Script_IdManage_STA::Script_IdManage_STA(QObject *parent) : QObject(parent)
{
    emScriptRunState=TestInit;
    resultFlag=false;
    sendMsgOct.clear();
    currentMeterIndex = -1;
    currentId = "";
    dstId = "";  // 初始化dstId


    p_CtrInfoList=make_shared<QList<shared_ptr<CtrInfo>>>();
    p_BuildNetwork_GW=make_shared<BuildNetwork_GW>();

    p_Frame3762Helper=make_shared<Frame3762Helper>();
    p_ParaInit_01F2=make_shared<Afn01F2>();
    p_QueryStaID_10F7=make_shared<Afn10F7>();
    p_QueryStaID_10F40=make_shared<Afn10F40>();
    p_TransmitData_02F1=make_shared<Afn02F1>();
    p_HardReset_01F1=make_shared<Afn01F1>();
    p_QueryNetScale_10F9=make_shared<Afn10F9>();
    p_Frame645Helper=make_shared<Frame645Helper>();
    p_MeterAddrResp_93=make_shared<RspsNormal_ReadAddr_0x93>(addr,6);
    p_FrameOOPHelper=make_shared<FrameOOPHelper>();
    p_GetResponseNormal_ReadAddr=make_shared<GetResponseNormal>();

    p_WriteId_14 = make_shared<dlt_645_Protocol::Rqst_WriteData_0x14>(addr, 0);
    // 正常响应对象
    p_WriteIdNormalResp_94 = make_shared<dlt_645_Protocol::RspsNormal_WriteData_0x94>(addr, 0);

    // 异常响应对象
    p_WriteIdAbNormalResp_D4 = make_shared<dlt_645_Protocol::RspsAbNormal_WriteData_0xD4>(addr, 0);

    p_SetRequestNormalId=make_shared<SetRequestNormal>();
    p_GetResponseNormalId=make_shared<GetResponseNormal>();

    p_timer=make_shared<QTimer>();
    p_maxAllowTimer=make_shared<QTimer>();
    p_delayTimer=make_shared<QTimer>();
    connect(p_timer.get(),SIGNAL(timeout()),this,SLOT(timer_timeoutProc()));
    connect(p_maxAllowTimer.get(),SIGNAL(timeout()),this,SLOT(maxAllowTimer_timeoutProc()));
    connect(p_delayTimer.get(),SIGNAL(timeout()),this,SLOT(delayTimer_timeoutProc()));

    idHeader=QDateTime::currentDateTime().toString("yyMMddhhmmss");
    idList<<id_0<<id_1<<id_2;
}

Script_IdManage_STA::~Script_IdManage_STA()
{
    // 1. 首先停止所有定时器并断开连接
    if(p_timer != nullptr) {
        p_timer->stop();
        disconnect(p_timer.get(), nullptr, this, nullptr);
    }
    if(p_maxAllowTimer != nullptr) {
        p_maxAllowTimer->stop();
        disconnect(p_maxAllowTimer.get(), nullptr, this, nullptr);
    }
    if(p_delayTimer != nullptr) {
        p_delayTimer->stop();
        disconnect(p_delayTimer.get(), nullptr, this, nullptr);
    }

    // 2. 然后清理 BuildNetwork_GW
    if(p_BuildNetwork_GW != nullptr) {
        p_BuildNetwork_GW->initBuildNetWork();
        if(needPowerOff == true && p_AbstractScriptHost != nullptr)
            powerOffAll12V(p_CtrInfoList, p_AbstractScriptHost);
    }
}
void Script_IdManage_STA::execute()
{
    p_AbstractScriptHost->updateProgress(ProcessState_Start, "Test start!");

    emScriptRunState=TestInit;
    resultFlag=false;
    addrList.clear();

    currentMeterIndex = -1;
    currentId = "";

    if(needBuildNet==true)
    {
        p_BuildNetwork_GW->execute();
        emScriptRunState=Wait_BuildNetFinish_Whole;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "等待全网组网完成");
        p_timer->start((timerForReachThresld+timerAfterReachThresld)*1000);
    }
    else
    {
        index=0;
        emScriptRunState=Wait_SetStaId_Finish;
        sendMsg(SingleSTA,p_CtrInfoList->at(0)->ctrlID,index,p_WriteId_14);
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--设置STA模块ID，等待--确认");
        p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
    }
    p_maxAllowTimer->start((timerForReachThresld+timerAfterReachThresld)*1000);

}
void Script_IdManage_STA::stop()
{
    // 首先停止所有定时器并断开连接，防止回调触发
    if(p_timer != nullptr) {
        p_timer->stop();
        disconnect(p_timer.get(), nullptr, this, nullptr);
    }
    if(p_maxAllowTimer != nullptr) {
        p_maxAllowTimer->stop();
        disconnect(p_maxAllowTimer.get(), nullptr, this, nullptr);
    }
    if(p_delayTimer != nullptr) {
        p_delayTimer->stop();
        disconnect(p_delayTimer.get(), nullptr, this, nullptr);
    }

    if(p_AbstractScriptHost==nullptr)
        return;
    p_AbstractScriptHost->updateProgress(ProcessState_Error, "Test stop!");
    if(needPowerOff==true)
        powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost);
}
void Script_IdManage_STA::addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq)
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
void Script_IdManage_STA::setHost(AbstractScriptHost *host)
{
    p_AbstractScriptHost=host;
    p_BuildNetwork_GW->setHost(host);
}
bool Script_IdManage_STA::config(const QMap<QString,QString> *paraDic)
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
        result = true;
    }
    return result;
}
void Script_IdManage_STA::processMsg(DvcType dvcType, int id, uchar *data, int datalen)
{
    if(p_CtrInfoList->size()==0) {
        return;
    }
    if(emScriptRunState==Wait_BuildNetFinish_Whole)
    {
        if(!p_BuildNetwork_GW->buildNetworkResultFlag)
            p_BuildNetwork_GW->processMsg(dvcType,id,data,datalen);
        else
        {
            // delete[] data; // Removed
            if(p_BuildNetwork_GW->emScriptRunState==BuildNetFinish&&p_BuildNetwork_GW->buildNetworkResultFlag==true)
            {
                tryTimes=0;
                index=0;
                emScriptRunState=Wait_SetStaId_Finish;
                sendMsg(SingleSTA,p_CtrInfoList->at(0)->ctrlID,index,p_WriteId_14);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--设置STA模块ID，等待--确认");
                p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
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
        else if(dvcType==ReadCtrlDvc)
        {
            p_CtrInfoList->at(0)->bufReadCtrlDvc.append(recvTempData);
            processMsgFromReadCtrlDvc(dvcType,id);
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
void Script_IdManage_STA::processCtrlDvcRes(DvcType dvcType, QList<int> snList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params)
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
            p_BuildNetwork_GW->processCtrlDvcRes(dvcType,snList,ctrlCmdType,isSucs,params);
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

void Script_IdManage_STA::processMsgFromCCO(DvcType dvcType, int dvcId)
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
            break;  // 改为break，防止无限循环
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
            case Wait_SetStaId_Finish:
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                break;
            }
            case Wait_QueryStaId_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x00&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "硬件初始化，回复确认！");
                }
                else if(p_Frame3762Base->afn_ == 0x03&&p_Frame3762Base->dt1_==0x02&&p_Frame3762Base->dt2_==0x01&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    index=0;
                    emScriptRunState=Wait_QueryStaId_Finish;
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryNetScale_10F9);
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询网络规模（10F9），等待--回复");
                    p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                }
                else if(p_Frame3762Base->afn_ == 0x10&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x01&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    shared_ptr<Afn10F9> p_QueryNetScale_10F9_Up=std::dynamic_pointer_cast<Afn10F9>(p_Frame3762Base);
                    if(p_QueryNetScale_10F9_Up->network_scale_==p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size()+1)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "网络规模与档案一致，组网完成！");
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("等待5分钟后，查询10F40！"));// 等待组网完成，路由定时3分5秒下发查询模块id
                        p_delayTimer->start(5*60*1000+10*1000);
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("查询网络规模（%1轮）！").arg(++index));
                        p_delayTimer->start(5*1000);
                    }
                }
                else if(p_Frame3762Base->afn_ == char(0x10)&&p_Frame3762Base->dt1_==char(0x80)&&p_Frame3762Base->dt2_==0x04&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    shared_ptr<Afn10F40> p_QueryStaID_10F40_Up=dynamic_pointer_cast<Afn10F40>(p_Frame3762Base);

                    if(memcmp(p_QueryStaID_10F40_Up->read_chip_id_unit_up_.mac_address_.addr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(index)->mtrAddr,6)==0
                            &&p_QueryStaID_10F40_Up->read_chip_id_unit_up_.id_type_==0x02
                            &&p_QueryStaID_10F40_Up->read_chip_id_unit_up_.device_type_==0x03)
                    {
                        // 获取查询到的模块ID
                        QByteArray receivedId = p_QueryStaID_10F40_Up->read_chip_id_unit_up_.id_content_;

                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("10F40查询到的原始模块ID（16进制）：%1，长度：%2").arg(QString(receivedId.toHex()).arg(receivedId.size())));

                        // 解析数据：DL/T 645格式（每个字节加了0x33）
                        QByteArray decodedId;
                        for(int i = 0; i < receivedId.size(); i++) {
                            uchar byte = static_cast<uchar>(receivedId.at(i));
                            // 减去0x33
                            if(byte >= 0x33) {
                                decodedId.append(byte - 0x33);
                            } else {
                                decodedId.append(byte);
                            }
                        }

                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("目标模块ID：%1").arg(dstId));
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("解码后的模块ID：%1").arg(QString(decodedId.toHex())));
                        if(dstId==QString(decodedId.toHex()))
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("STA模块ID为%1，符合要求").arg(QString(p_QueryStaID_10F40_Up->read_chip_id_unit_up_.id_content_.toHex())));
                            index=0;
                            emScriptRunState=Wait_QueryStaId_Finish;
                            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,index,p_QueryStaID_10F7);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询STA模块ID（10F7），等待--回复");
                            p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                        }
                        else
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("STA模块ID为%1，不符合要求").arg(QString(p_QueryStaID_10F40_Up->read_chip_id_unit_up_.id_content_.toHex())));
                        }
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("STA模块ID为%1，不符合要求").arg(QString(p_QueryStaID_10F40_Up->read_chip_id_unit_up_.id_content_.toHex())));
                    }
                }
                else if(p_Frame3762Base->afn_ == 0x10&&p_Frame3762Base->dt1_==0x40&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    shared_ptr<Afn10F7> p_QueryStaID_10F7_Up=dynamic_pointer_cast<Afn10F7>(p_Frame3762Base);

                    for(int i=0;i<p_QueryStaID_10F7_Up->node_id_info_unit_.node_id_info_list_.size();i++)
                    {
                        if(memcmp(p_QueryStaID_10F7_Up->node_id_info_unit_.node_id_info_list_.at(i).node_address_.addr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(0)->mtrAddr,6)==0)
                        {
                            flagFind=true;
                            index=0;
                            if(memcmp(p_QueryStaID_10F7_Up->node_id_info_unit_.node_id_info_list_.at(i).node_address_.addr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(0)->mtrAddr,6)==0)
                            {
                                QByteArray rawId = p_QueryStaID_10F7_Up->node_id_info_unit_.node_id_info_list_.at(i).node_id_;
                                // 解码：减去0x33
                               QByteArray decodedId;
                               for(int j = 0; j < rawId.size(); j++) {
                                   uchar byte = static_cast<uchar>(rawId.at(j));
                                   if(byte >= 0x33) {
                                       decodedId.append(byte - 0x33);
                                   } else {
                                       decodedId.append(byte);
                                   }
                               }

                               p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("目标节点找到，原始芯片ID：%1，解码后：%2").arg(QString(rawId.toHex())).arg(QString(decodedId.toHex())));

                                if(dstId==QString(decodedId.toHex()))
                                {
                                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("STA模块ID为%1，符合要求").arg(QString(decodedId.toHex())));
//                                    flag02F1=true;
                                    tryTimes=0;
                                    emScriptRunState=ScriptSuccess;
                                    resultFlag=true;
                                    p_AbstractScriptHost->updateProgress(ProcessState_Success, QString("STA模块ID管理测试成功;"));
                                }
                                else
                                {
                                    p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("STA模块ID为%1，不符合要求").arg(QString(decodedId.toHex())));
                                }
                            }
                            else
                            {
                                p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("STA模块ID为%1，不符合要求").arg(QString(p_QueryStaID_10F7_Up->node_id_info_unit_.node_id_info_list_.at(i).node_id_.toHex())));
                            }
                        }
                    }
//                    if(flagFind==false)
//                    {
//                        index+=topoCntPerTime;
//                        if(index<p_QueryStaID_10F7_Up->node_id_info_unit_.node_total_num_)
//                        {
//                            emScriptRunState=Wait_QueryStaId_Finish;
//                            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,index,p_QueryStaID_10F7);
//                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询STA模块ID（10F7），等待--回复");
//                            p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
//                        }
//                        else
//                        {
//                            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "没有目标节点");
//                        }
//                    }
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

void Script_IdManage_STA::processMsgFromReadCtrlDvc(DvcType dvcType, int dvcId)
{
    if(dvcId!=p_CtrInfoList->at(0)->ctrlID)
        return;
    bool haveCompleteMsg=true;
    while(haveCompleteMsg)
    {
        qInfo()<<QString("CCO-解析前 buf3762=%1").arg(QString((p_CtrInfoList->at(0)->bufReadCtrlDvc.toHex())));
        shared_ptr<Frame3762Base> p_Frame3762Base = p_Frame3762Helper->DecodeLocalMsg(&(p_CtrInfoList->at(0)->bufReadCtrlDvc),haveCompleteMsg);
        qInfo()<<QString("CCO-解析后 buf3762=%1").arg(QString((p_CtrInfoList->at(0)->bufReadCtrlDvc.toHex())));
        if(p_Frame3762Base==nullptr)
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
            case Wait_SetStaId_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x02&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    shared_ptr<Afn02F1> p_TransmitData_02F1_Up=dynamic_pointer_cast<Afn02F1>(p_Frame3762Base);
                    if(p_TransmitData_02F1_Up->frame_content_.size()!=0)
                    {
                        shared_ptr<SetParam_Up> p_SetParam_Up=SetParam_Up::decode_SetParam_Up(&p_TransmitData_02F1_Up->frame_content_);
                        if(p_SetParam_Up->paramSetRlstInfoList.at(0).id==28)
                        {
                            if(p_SetParam_Up->paramSetRlstInfoList.at(0).setRes==0x00)
                                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "模块ID设置成功");
                            else
                                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "模块ID设置失败");
                            flag02F1=false;
                            emScriptRunState=Wait_QueryStaId_Finish;
                            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_HardReset_01F1);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--硬件复位（01F1），等待--确认");
                            p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                        }
                        else
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "模块ID设置失败");
                        }
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "模块ID设置失败,02F1上报为空");
                    }
                }
                else
                {
                    QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                    sendSrcMsg(dvcType,dvcId,tmpSendMsg);
                }
                break;
            }
            case Wait_QueryStaId_Finish:
            {
                if(p_Frame3762Base->afn_ == 0x02&&p_Frame3762Base->dt1_==0x01&&p_Frame3762Base->dt2_==0x00&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
                {
                    p_timer->stop();
                    shared_ptr<Afn02F1> p_TransmitData_02F1_Up=dynamic_pointer_cast<Afn02F1>(p_Frame3762Base);
                    if(p_TransmitData_02F1_Up->frame_content_.size()!=0)
                    {
                        shared_ptr<ChkParam_Up> p_ChkParam_Up=ChkParam_Up::decode_ChkParam_Up(&p_TransmitData_02F1_Up->frame_content_);
                        if(p_ChkParam_Up->paramInfoList.at(0).id==28&&QString(p_ChkParam_Up->paramInfoList.at(0).idCntnt.toHex()).toUpper()==dstId+idType)
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("STA模块ID为%1，符合要求").arg(QString(p_ChkParam_Up->paramInfoList.at(0).idCntnt.toHex()).toUpper()));
                            idIndex++;
                            if(idIndex<idList.size())
                            {
                                flagFind=false;
                                flag02F1=true;
                                tryTimes=0;
                                emScriptRunState=Wait_SetStaId_Finish;
                                sendMsg(dvcType,p_CtrInfoList->at(0)->ctrlID,index,p_TransmitData_02F1);
                                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--透传设置STA模块ID，等待--确认");
                                p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
                            }
                            else
                            {
                                emScriptRunState=ScriptSuccess;
                                resultFlag=true;
                                p_AbstractScriptHost->updateProgress(ProcessState_Success, QString("STA模块ID管理测试成功;"));
                            }
                        }
                        else
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "模块ID查询失败");
                        }
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "模块ID查询失败,02F1上报为空");
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
void Script_IdManage_STA::processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID)
{
    bool haveCompleteMsg=true;
    // 安全检查
    if(!p_CtrInfoList || p_CtrInfoList->isEmpty() ||
       !p_CtrInfoList->at(0) ||
       !p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList) {
        p_AbstractScriptHost->updateProgress(ProcessState_Error,
            "processMsgFromMeter645: 数据结构未初始化");
        return;
    }

    // 获取电表信息
    auto meterInfo = p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->value(mtrlID);
    if(!meterInfo) {
        p_AbstractScriptHost->updateProgress(ProcessState_Error,
            QString("processMsgFromMeter645: 电表信息为nullptr，电表ID：%1").arg(mtrlID));
        return;
    }
    while(haveCompleteMsg)
    {
        shared_ptr<dlt_645_Protocol::Frame645Base> MsgBase_645_ptr = dlt_645_Protocol::Frame645Helper::DecodeLocalMsg(&((*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[mtrlID]->buf645),haveCompleteMsg);
        if(MsgBase_645_ptr==nullptr)
        {
            break;
        }
        p_AbstractScriptHost->updateProgress(ProcessState_Processing,
            QString("收到645协议报文，控制码：0x%1")
            .arg(QString::number(MsgBase_645_ptr->ctrlCode_, 16)));
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
        case Wait_SetStaId_Finish:
            {
                // 写数据正常应答 (0x94)
                if(MsgBase_645_ptr->ctrlCode_ == 0x94)
                {
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,
                        "收到0x94写数据正常应答");

                    // 验证当前正在处理的电表索引是否有效
                    if(currentMeterIndex < 0 ||
                       currentMeterIndex >= p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->size()) {
                        p_AbstractScriptHost->updateProgress(ProcessState_Error,
                            QString("currentMeterIndex %1 超出范围").arg(currentMeterIndex));
                        return;
                    }

                    // 获取当前电表信息
                    int actualMeterID = p_CtrInfoList->at(0)->keyList.at(currentMeterIndex);
                    auto currentMeterInfo = p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->value(actualMeterID);
                    if(!currentMeterInfo) {
                        p_AbstractScriptHost->updateProgress(ProcessState_Error,
                            QString("当前电表信息为nullptr，索引：%1").arg(currentMeterIndex));
                        return;
                    }

                    // 验证地址是否匹配
                    bool addrMatch = true;
                    QString logAddr1, logAddr2;
                    for(int i = 0; i < 6; i++) {
                        logAddr1.append(QString::number(MsgBase_645_ptr->addr_[i], 16).rightJustified(2, '0'));
                        logAddr2.append(QString::number(currentMeterInfo->mtrAddr[i], 16).rightJustified(2, '0'));
                        if(MsgBase_645_ptr->addr_[i] != currentMeterInfo->mtrAddr[i]) {
                            addrMatch = false;
                            break;
                        }
                    }

                    p_AbstractScriptHost->updateProgress(ProcessState_Processing,
                        QString("地址匹配检查：报文地址=%1，电表地址=%2，匹配=%3")
                        .arg(logAddr1).arg(logAddr2).arg(addrMatch ? "是" : "否"));

                    if(addrMatch)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing,
                            QString("STA %1 模块ID设置成功，模块ID：%2")
                            .arg(currentMeterIndex + 1)  // 显示为第几个STA，从1开始计数
                            .arg(currentId));

                        // 设置下一个STA的模块ID
                        index++;
                        if(index < p_CtrInfoList->at(0)->keyList.size())
                        {
                            emScriptRunState = Wait_SetStaId_Finish;
                            sendMsg(SingleSTA, p_CtrInfoList->at(0)->ctrlID, index, p_WriteId_14);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing,
                                QString("发送--设置第%1个STA模块ID，等待--确认").arg(index + 1));
                            p_timer->start(STA_CMD_TIMEOUT_TIME * 1000);
                        }
                        else
                        {
                            // 所有STA设置完成
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing,
                                QString("所有%1个STA模块ID设置成功！").arg(p_CtrInfoList->at(0)->keyList.size()));

                            // 发送硬件初始化命令
                            emScriptRunState = Wait_QueryStaId_Finish;
                            sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, INSIGNIFICANCE, p_HardReset_01F1);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--硬件复位（01F1），等待--确认");
                            p_timer->start(CTRL_CMD_TIMEOUT_TIME * 1000);
                        }
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Error,
                            "地址不匹配，设置模块ID失败");
                        // 继续下一个STA
                        index++;
                        if(index < p_CtrInfoList->at(0)->keyList.size())
                        {
                            emScriptRunState = Wait_SetStaId_Finish;
                            sendMsg(SingleSTA, p_CtrInfoList->at(0)->ctrlID, index, p_WriteId_14);
                            p_AbstractScriptHost->updateProgress(ProcessState_Processing,
                                QString("发送--设置下一个STA模块ID，等待--确认"));
                            p_timer->start(STA_CMD_TIMEOUT_TIME * 1000);
                        }
                        else
                        {
                            p_AbstractScriptHost->updateProgress(ProcessState_Error,
                                "设置模块ID失败，测试结束");
                            resultFlag = false;
                            emScriptRunState = ScriptSuccess;
                        }
                    }
                }
                // 写数据异常应答 (0xD4)
                else if(MsgBase_645_ptr->ctrlCode_ == 0xD4)
                {
                    p_timer->stop();
                    p_AbstractScriptHost->updateProgress(ProcessState_Error,
                        "收到0xD4写数据异常应答");

                    resultFlag = false;
                    emScriptRunState = ScriptSuccess;
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed,
                        "写模块ID失败，收到异常应答");
                }
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
void Script_IdManage_STA::processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID)
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
void Script_IdManage_STA::sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame)
{
    sendMsgOct.clear();
//    if(frame==p_TransmitData_02F1)
//    {
//        p_TransmitData_02F1->ctrl_field_.dir=kDirDown;
//        p_TransmitData_02F1->ctrl_field_.prm=kActive;
//        p_TransmitData_02F1->ctrl_field_.comn_type=kHplc;

//        p_TransmitData_02F1->info_field_.info_field_down.msg_seq=char(msgSeq++);
//        p_TransmitData_02F1->info_field_.info_field_down.comu_rate=0;
//        p_TransmitData_02F1->info_field_.info_field_down.comu_module_ident=1;

//        memcpy(p_TransmitData_02F1->address_field_.src_addr,p_CtrInfoList->at(0)->ccoAddr,6);
//        memcpy(p_TransmitData_02F1->address_field_.dst_addr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(meterID)->mtrAddr,6);

//        p_TransmitData_02F1->protocol_type_=0x04;
//        if(emScriptRunState==Wait_SetStaId_Finish)
//        {
//            p_TransmitData_02F1->frame_content_=getSetFrame();
//        }
//        else if(emScriptRunState==Wait_QueryStaId_Finish)
//        {
//            p_TransmitData_02F1->frame_content_=getQueryFrame();
//        }

//        p_TransmitData_02F1->frame_length_=uchar(p_TransmitData_02F1->frame_content_.size());

//        sendMsgOct=p_TransmitData_02F1->EncodeFrame();
//        sendMsgLog=QString("》》数据透传02F1：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
//        /////
//        dstId=idList.at(0);
//    }
    if(frame==p_HardReset_01F1)
    {
        p_HardReset_01F1->ctrl_field_.dir=kDirDown;
        p_HardReset_01F1->ctrl_field_.prm=kActive;
        p_HardReset_01F1->ctrl_field_.comn_type=kHplc;

        p_HardReset_01F1->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_HardReset_01F1->info_field_.info_field_down.comu_rate=0;
        p_HardReset_01F1->info_field_.info_field_down.comu_module_ident=0;
        sendMsgOct=p_HardReset_01F1->EncodeFrame();
        sendMsgLog=QString("》》硬件初始化01F1：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_QueryNetScale_10F9)
    {
        p_QueryNetScale_10F9->ctrl_field_.dir=kDirDown;
        p_QueryNetScale_10F9->ctrl_field_.prm=kActive;
        p_QueryNetScale_10F9->ctrl_field_.comn_type=kHplc;

        p_QueryNetScale_10F9->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_QueryNetScale_10F9->info_field_.info_field_down.comu_rate=0;
        p_QueryNetScale_10F9->info_field_.info_field_down.comu_module_ident=0;

        sendMsgOct=p_QueryNetScale_10F9->EncodeFrame();
        sendMsgLog=QString("》》查询网络规模10F9：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_QueryStaID_10F40)
    {
        p_QueryStaID_10F40->ctrl_field_.dir=kDirDown;
        p_QueryStaID_10F40->ctrl_field_.prm=kActive;
        p_QueryStaID_10F40->ctrl_field_.comn_type=kHplc;

        p_QueryStaID_10F40->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_QueryStaID_10F40->info_field_.info_field_down.comu_rate=0;
        p_QueryStaID_10F40->info_field_.info_field_down.comu_module_ident=0;

        p_QueryStaID_10F40->read_chip_id_unit_down_.device_type_=0x03;
        memcpy(p_QueryStaID_10F40->read_chip_id_unit_down_.mac_address_.addr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(meterID)->mtrAddr,6);
        p_QueryStaID_10F40->read_chip_id_unit_down_.id_type_=0x02;

        sendMsgOct=p_QueryStaID_10F40->EncodeFrame();
        sendMsgLog=QString("》》查询STA模块ID 10F40：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_QueryStaID_10F7)
    {
        p_QueryStaID_10F7->ctrl_field_.dir=kDirDown;
        p_QueryStaID_10F7->ctrl_field_.prm=kActive;
        p_QueryStaID_10F7->ctrl_field_.comn_type=kHplc;

        p_QueryStaID_10F7->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_QueryStaID_10F7->info_field_.info_field_down.comu_rate=0;
        p_QueryStaID_10F7->info_field_.info_field_down.comu_module_ident=0;

        p_QueryStaID_10F7->node_start_no_=1;
        p_QueryStaID_10F7->node_num_=topoCntPerTime;

        sendMsgOct=p_QueryStaID_10F7->EncodeFrame();
        sendMsgLog=QString("》》查询STA模块ID 10F7：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_ParaInit_01F2)
    {
        p_ParaInit_01F2->ctrl_field_.dir=kDirDown;
        p_ParaInit_01F2->ctrl_field_.prm=kActive;
        p_ParaInit_01F2->ctrl_field_.comn_type=kHplc;

        p_ParaInit_01F2->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_ParaInit_01F2->info_field_.info_field_down.comu_rate=0;
        p_ParaInit_01F2->info_field_.info_field_down.comu_module_ident=0;

        sendMsgOct=p_ParaInit_01F2->EncodeFrame();
        sendMsgLog=QString("》》路由参数初始化01F2：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
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
    else if(frame==p_WriteId_14)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "44444444444444444--设置STA模块ID，等待--确认");

            // 安全检查
            if(!p_CtrInfoList || p_CtrInfoList->isEmpty() ||
               !p_CtrInfoList->at(0) ||
               !p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList) {
                p_AbstractScriptHost->updateProgress(ProcessState_Error, "p_CtrInfoList无效");
                return;
            }

            // 检查是否有电表信息
            if(p_CtrInfoList->at(0)->keyList.isEmpty()) {
                p_AbstractScriptHost->updateProgress(ProcessState_Error, "电表列表为空");
                return;
            }

            if(meterID >= p_CtrInfoList->at(0)->keyList.size()) {
                p_AbstractScriptHost->updateProgress(ProcessState_Error,
                    QString("meterID %1 超出keyList范围，keyList大小：%2").arg(meterID).arg(p_CtrInfoList->at(0)->keyList.size()));
                return;
            }

            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "7777777777777777--设置STA模块ID，等待--确认");

            // 获取电表ID（从keyList中获取真正的电表ID）
            int actualMeterID = p_CtrInfoList->at(0)->keyList.at(meterID);

            // 通过电表ID获取电表信息
            auto meterInfo = p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->value(actualMeterID);
            if(!meterInfo) {
                p_AbstractScriptHost->updateProgress(ProcessState_Error,
                    QString("电表信息为nullptr，电表ID：%1").arg(actualMeterID));
                return;
            }

            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "8888888888888888--设置STA模块ID，等待--确认");

            // 获取电表地址 - 这里声明meterAddr变量
            uchar* meterAddr = meterInfo->mtrAddr;  // 添加这行声明

            // 检查模块ID列表
            if(meterID >= idList.size()) {
                p_AbstractScriptHost->updateProgress(ProcessState_Error,
                    QString("meterID %1 超出chipIdList范围，chipIdList大小：%2").arg(meterID).arg(idList.size()));
                return;
            }

            // 设置要写入的数据（11字节模块ID）
            QString IdStr = idList.at(meterID);
            p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("12121212121212--模块ID字符串：%1，长度：%2").arg(IdStr).arg(IdStr.length()));

            // 格式化模块ID字符串，确保是48个十六进制字符（11字节）
            if(IdStr.length() > 22) {
                IdStr = IdStr.left(22);
            }

            while(IdStr.length() < 22) {
                IdStr += "0";
            }

            p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("处理后的模块ID字符串：%1").arg(IdStr));

            QByteArray hexBytes = QByteArray::fromHex(IdStr.toLatin1());
            p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("1313131313--转换后的字节数组长度：%1").arg(hexBytes.size()));

            // 检查长度
            if(hexBytes.size() != 11) {
                p_AbstractScriptHost->updateProgress(ProcessState_Error,QString("模块ID长度错误：%1字节，期望11字节").arg(hexBytes.size()));
                return;
            }

            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "555555555--设置STA模块ID，等待--确认");

            // 手动组帧 - 不依赖 Rqst_WriteData_0x14 的 EncodeFrame()
            QByteArray frameData;

            // 1. 帧起始符 68H
            frameData.append(0x68);

            // 2. 地址域 A0~A5 (6字节，低字节在前)
            for(int i = 0; i < 6; i++) {
                frameData.append(meterAddr[i]);
            }

            // 3. 帧起始符 68H
            frameData.append(0x68);

            // 4. 控制码 C=14H (写数据)
            frameData.append(0x14);

            // 5. 数据域长度 L (15字节: 4+11)
            frameData.append(0x0F); // 15的十六进制

            // 6. 数据域 (每个字节需要加33H处理)
            // 6.1 数据标识: 01 00 F0 F0 (F0F00001，低字节在前)
            frameData.append(0x01 + 0x33);
            frameData.append(0x00 + 0x33);
            frameData.append(0xF0 + 0x33);
            frameData.append(0xF0 + 0x33);
//            frameData.append(0x01);
//            frameData.append(char(0x00));
//            frameData.append(0xF0);
//            frameData.append(0xF0);

            // 6.2 模块ID数据 (11字节)
            for(int i = 0; i < 11; i++) {
                uchar byte = static_cast<uchar>(hexBytes.at(i));
                frameData.append(byte + 0x33);
            }

            // 7. 计算校验码 CS (从第一个帧起始符开始到校验码之前的所有字节的模256和)
            uchar cs = 0;
            for(int i = 0; i < frameData.size(); i++) {
                cs += static_cast<uchar>(frameData.at(i));
            }
            frameData.append(cs);

            // 8. 结束符 16H
            frameData.append(0x16);

            sendMsgOct = frameData;
            p_AbstractScriptHost->updateProgress(ProcessState_Processing,
                QString("666666666666666--手动组帧成功，帧长度：%1").arg(sendMsgOct.size()));

            sendMsgLog = QString("》》 设置模块ID(0x14)：%1，模块ID：%2\n")
                            .arg(QString(sendMsgOct.toHex()))
                            .arg(IdStr);

            // 记录当前设置的模块ID，用于后续验证
            currentId = IdStr;
            currentMeterIndex = meterID;
            // 设置目标模块ID，用于后续验证
            dstId = IdStr;
            p_AbstractScriptHost->updateProgress(ProcessState_Processing,
                QString("设置目标模块ID：%1").arg(dstId));

            p_AbstractScriptHost->updateProgress(ProcessState_Processing, sendMsgLog);

            uchar *sendMsgData = new uchar[static_cast<uint>(sendMsgOct.size())];
            memcpy(sendMsgData, reinterpret_cast<uchar*>(sendMsgOct.data()), uint(sendMsgOct.size()));
            p_AbstractScriptHost->sendMsg2Dvc(dvcType, dvcId, sendMsgData, sendMsgOct.size());

            return; // 确保从这里返回
        }
    else if(frame==p_SetRequestNormalId)
    {
        // 根据OOP协议正确设置数据
        p_SetRequestNormalId->ctrl_field_.dir = 1;
        p_SetRequestNormalId->ctrl_field_.prm = 0;
        p_SetRequestNormalId->ctrl_field_.fra = 0;
        p_SetRequestNormalId->ctrl_field_.res = 0;
        p_SetRequestNormalId->ctrl_field_.sc = 0;
        p_SetRequestNormalId->ctrl_field_.func = 1;

        p_SetRequestNormalId->address_field_.sa.addr_type = 0;
        p_SetRequestNormalId->address_field_.sa.logic_addr = 0;
        p_SetRequestNormalId->address_field_.sa.addr_len = 5;
        p_SetRequestNormalId->address_field_.sa.address = QString((QByteArray(reinterpret_cast<char*>((*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[meterID]->mtrAddr),6).toHex()));
        p_SetRequestNormalId->address_field_.ca.address = 0x00;

        p_SetRequestNormalId->piid_.serve_priority = 0;
        p_SetRequestNormalId->piid_.reserve = 0;
        p_SetRequestNormalId->piid_.serve_seq = 1;

        // 设置模块ID的OAD对象 - 根据实际协议定义调整
        p_SetRequestNormalId->a_set_normal_.oad.OI = 0x0010; // 假设模块ID的OI是0x0010
        p_SetRequestNormalId->a_set_normal_.oad.attribute.feature = 0;
        p_SetRequestNormalId->a_set_normal_.oad.attribute.seq = 0;
        p_SetRequestNormalId->a_set_normal_.oad.element_index = 0;

        // 创建正确的数据对象
        // 注意：这里需要根据您的OOP协议实现来创建正确的Data对象
        // 如果您的DataParent基类有正确的虚函数，应该可以直接赋值
        auto data_ptr = std::make_shared<DataString>();
        data_ptr->type_ = DataType::kOctet_string;
        data_ptr->data_ = QByteArray::fromHex(currentId.toLatin1());

        // 正确赋值给data_ptr
        p_SetRequestNormalId->a_set_normal_.data_ptr = data_ptr;

        sendMsgOct = p_SetRequestNormalId->EncodeFrame();
        sendMsgLog = QString("》》 设置模块ID(OOP)：%1\n").arg(QString(sendMsgOct.toHex()));
    }

    p_AbstractScriptHost->updateProgress(ProcessState_Processing, sendMsgLog);

    uchar *sendMsg=new uchar[uint(sendMsgOct.size())];
    memcpy(sendMsg,reinterpret_cast<uchar*>(sendMsgOct.data()),uint(sendMsgOct.size()));
    p_AbstractScriptHost->sendMsg2Dvc(dvcType,dvcId,sendMsg,sendMsgOct.size());
}
void Script_IdManage_STA::sendSrcMsg(DvcType dvcType, int dvcId, QByteArray msg)
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

void Script_IdManage_STA::timer_timeoutProc()
{
    if(emScriptRunState==Wait_SetStaId_Finish)
    {
        if(++tryTimes<3)
        {
            emScriptRunState=Wait_SetStaId_Finish;
            sendMsg(ReadCtrlDvc,p_CtrInfoList->at(0)->ctrlID,index,p_WriteId_14);
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--设置STA模块ID，等待--确认");
            p_timer->start(STA_CMD_TIMEOUT_TIME*1000);
        }
        else
        {
            tryTimes=0;
            p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==p_Timer");
        }
    }
    else if(emScriptRunState==Wait_QueryStaId_Finish)
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==p_Timer");
    }
}
void Script_IdManage_STA::maxAllowTimer_timeoutProc()
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
void Script_IdManage_STA::delayTimer_timeoutProc()
{
    p_delayTimer->stop();
    if(p_delayTimer->interval()==(5*60*1000+10*1000))
    {
        index=0;
        emScriptRunState=Wait_QueryStaId_Finish;
        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,index,p_QueryStaID_10F40);
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询STA模块ID（10F40），等待--回复");
        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
    }
    else
    {
        emScriptRunState=Wait_QueryStaId_Finish;
        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryNetScale_10F9);
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询网络规模（10F9），等待--回复");
        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
    }
}

QByteArray Script_IdManage_STA::getSetFrame()
{
    QByteArray msg;
    shared_ptr<SetParam_Down> p_SetParam_Down=make_shared<SetParam_Down>();
    QList<ParamInfo> setParamInfoList;
    ParamInfo tmpParamInfo;
    tmpParamInfo.id=28;//模块ID
    if(idIndex<2)
        tmpParamInfo.idCntnt=QByteArray::fromHex((idList.at(idIndex)+idType).toLatin1());
    tmpParamInfo.idLen=ushort(tmpParamInfo.idCntnt.size());
    setParamInfoList.append(tmpParamInfo);

    p_SetParam_Down->msgSeq=msgSeq;
    memcpy(p_SetParam_Down->srcAddr,p_CtrInfoList->at(0)->ccoAddr,6);
    memcpy(p_SetParam_Down->dstAddr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(index)->mtrAddr,6);
    p_SetParam_Down->paramInfoList=setParamInfoList;
    p_SetParam_Down->idCnt=ushort(setParamInfoList.size());

    msg=SetParam_Down::encode_SetParam_Down(p_SetParam_Down);
    return msg;
}

QByteArray Script_IdManage_STA::getQueryFrame()
{
    QByteArray msg;
    shared_ptr<ChkParam_Down> p_ChkParam_Down=make_shared<ChkParam_Down>();
    QList<ushort> chkParamIdList;
    chkParamIdList.append(28);

    p_ChkParam_Down->msgSeq=msgSeq;
    memcpy(p_ChkParam_Down->srcAddr,p_CtrInfoList->at(0)->ccoAddr,6);
    memcpy(p_ChkParam_Down->dstAddr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(index)->mtrAddr,6);
    p_ChkParam_Down->idList=chkParamIdList;
    p_ChkParam_Down->idCnt=ushort(chkParamIdList.size());

    msg=ChkParam_Down::encode_ChkParam_Down(p_ChkParam_Down);
    return msg;
}
