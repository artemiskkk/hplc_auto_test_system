#include <memory>
using namespace std;
#include "Script_ChipIdManage_CCO.h"

Script_ChipIdManage_CCO::Script_ChipIdManage_CCO(QObject *parent) : QObject(parent)
{
    emScriptRunState = TestInit;
    resultFlag = false;
    sendMsgOct.clear();

    p_CtrInfoList = make_shared<QList<shared_ptr<CtrInfo>>>();
    p_BuildNetwork_GW=make_shared<BuildNetwork_GW>(); // lazy init

    p_Frame3762Helper=make_shared<Frame3762Helper>();
    p_ParaInit_01F2=make_shared<Afn01F2>();
    p_SetRouterChipID_F0F1=make_shared<AfnF0F1>();
//    p_QueryRouterChipID_F0F40=make_shared<AfnF0F40>();
    p_QueryRouterChipID_10F112=make_shared<Afn10F112>();
    p_QueryRouterChipID_10F40=make_shared<Afn10F40>();

    p_Frame645Helper=make_shared<Frame645Helper>();
    p_MeterAddrResp_93=make_shared<RspsNormal_ReadAddr_0x93>(addr,6);
    p_FrameOOPHelper=make_shared<FrameOOPHelper>();
    p_GetResponseNormal_ReadAddr=make_shared<GetResponseNormal>();

    p_timer=make_shared<QTimer>();
    p_maxAllowTimer=make_shared<QTimer>();
    p_delayTimer=make_shared<QTimer>();
    connect(p_timer.get(),SIGNAL(timeout()),this,SLOT(timer_timeoutProc()));
    connect(p_maxAllowTimer.get(),SIGNAL(timeout()),this,SLOT(maxAllowTimer_timeoutProc()));
    connect(p_delayTimer.get(),SIGNAL(timeout()),this,SLOT(delayTimer_timeoutProc()));

    chipIdHeader=QDateTime::currentDateTime().toString("yyMMddhhmmss");
    chipIdList<<chipIdHeader+chipId_0<<chipIdHeader+chipId_1;//<<chipId_2
}
Script_ChipIdManage_CCO::~Script_ChipIdManage_CCO()
{
    if(p_BuildNetwork_GW != nullptr) {
        p_BuildNetwork_GW->initBuildNetWork();
        if(needPowerOff==true && p_AbstractScriptHost != nullptr)
            powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost);
    }
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
}
void Script_ChipIdManage_CCO::execute()
{
    // 安全检查
    if(p_AbstractScriptHost == nullptr) {
        return;
    }
    if(p_CtrInfoList == nullptr || p_CtrInfoList->size() == 0) {
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "错误：设备列表为空，请先配置设备信息");
        return;
    }

    if (p_BuildNetwork_GW != nullptr) {
        p_BuildNetwork_GW->setHost(p_AbstractScriptHost);
        p_BuildNetwork_GW->config(&m_paraDic);
        uchar dstFreq = m_cachedFreq & 0x0f;
        p_BuildNetwork_GW->setCtrInfoListAndFreq(p_CtrInfoList, dstFreq);
    }
    p_AbstractScriptHost->updateProgress(ProcessState_Start, "Test start!");

    emScriptRunState = TestInit;
    resultFlag = false;
    addrList.clear();
    // 重置状态变量，避免残留值干扰
    msgSeq = static_cast<uchar>(QDateTime::currentMSecsSinceEpoch() & 0xFF);
    chipIdIndex = 0;
    tryTimes = 0;
    // 清空缓冲区
    p_CtrInfoList->at(0)->buf.clear();

    if (needBuildNet == true)
    {
        p_BuildNetwork_GW->execute();
        emScriptRunState = Wait_BuildNetFinish_Whole;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "等待全网组网完成");
        p_timer->start((timerForReachThresld + timerAfterReachThresld) * 1000);
    }
    else
    {
        sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, chipIdIndex, p_SetRouterChipID_F0F1);
        emScriptRunState = Wait_SetRouterChipID_Finish;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--设置CCO芯片ID，等待--确认");
        p_timer->start(CTRL_CMD_TIMEOUT_TIME * 1000);
    }
    p_maxAllowTimer->start((timerForReachThresld + timerAfterReachThresld) * 1000);
}
void Script_ChipIdManage_CCO::stop()
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
    
    // Safety check
    if (p_BuildNetwork_GW == nullptr) return;

    if(needPowerOff==true)
        powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost);
}
void Script_ChipIdManage_CCO::addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq)
{
    p_CtrInfoList->clear();
    comnAddAddrsInfo(p_CtrInfoList, p_ConcentratorInfoList, p_MeterInfoList, p_SchemeCfgInfoList);
    concentratorCnt = ushort(p_CtrInfoList->size());
    uchar dstFreq = freq & 0x0f;

    uchar dstPrtcl = uchar(freq) >> 4;
    if (dstPrtcl == 0x03)
        setMeterAddrsPrtcl(p_CtrInfoList, dstPrtcl);

    m_cachedFreq = freq;
    if (p_BuildNetwork_GW != nullptr) {
        p_BuildNetwork_GW->setCtrInfoListAndFreq(p_CtrInfoList, dstFreq);
    }
}
void Script_ChipIdManage_CCO::setHost(AbstractScriptHost *host)
{
    p_AbstractScriptHost = host;
    if (p_BuildNetwork_GW != nullptr) {
        p_BuildNetwork_GW->setHost(host);
    }
}
bool Script_ChipIdManage_CCO::config(const QMap<QString, QString> *paraDic)
{
    bool result = false;
    if (paraDic != nullptr)
    {
        m_paraDic = *paraDic; // Cache config
        if (p_BuildNetwork_GW != nullptr) {
            p_BuildNetwork_GW->config(paraDic);
        }

        if (paraDic->keys().contains("timerForReachThresld"))
        {
            this->timerForReachThresld = (*paraDic)["timerForReachThresld"].toUShort();
        }
        if (paraDic->keys().contains("timerAfterReachThresld"))
        {
            this->timerAfterReachThresld = (*paraDic)["timerAfterReachThresld"].toUShort();
        }

        if (paraDic->keys().contains("netSucRateThresld"))
        {
            this->netSucRateThresld = (*paraDic)["netSucRateThresld"].toDouble();
        }
        if (paraDic->keys().contains("needBuildNet"))
        {
            if ((*paraDic)["needBuildNet"].toLower() == "false")
            {
                this->needBuildNet = false;
            }
            else
            {
                this->needBuildNet = true;
            }
        }
        if (paraDic->keys().contains("needPowerOff"))
        {
            if ((*paraDic)["needPowerOff"].toLower() == "false")
            {
                this->needPowerOff = false;
            }
            else
            {
                this->needPowerOff = true;
            }
        }
        result = true;
    }
    return result;
}
void Script_ChipIdManage_CCO::processMsg(DvcType dvcType, int id, uchar *data, int datalen)
{
    if (p_CtrInfoList == nullptr || p_CtrInfoList->size() == 0) {
        delete[] data;  // 防止内存泄漏
        return;
    }
    // Safety check
    if (p_BuildNetwork_GW == nullptr) {
        delete[] data;  // 防止内存泄漏
        return;
    }

    if (emScriptRunState == Wait_BuildNetFinish_Whole)
    {
        if (!p_BuildNetwork_GW->startBuildNetFlag)
            p_BuildNetwork_GW->processMsg(dvcType, id, data, datalen);
        else
        {
            delete[] data;  // 释放数据，防止内存泄漏
            p_BuildNetwork_GW->initBuildNetWork();
            p_CtrInfoList->at(0)->buf.clear();  // 清空缓冲区
            emScriptRunState = Wait_SetRouterChipID_Finish;
            sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, chipIdIndex, p_SetRouterChipID_F0F1);
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--设置CCO芯片ID，等待--确认");
            p_timer->start(CTRL_CMD_TIMEOUT_TIME * 1000);
        }
    }
    else
    {
        // 安全拷贝数据，不删除原始data指针
        QByteArray recvTempData = QByteArray(reinterpret_cast<char *>(data), datalen);

        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到报文：%1").arg(QString(recvTempData.toHex())));

        if (dvcType == CCO_GW || dvcType == CCO_NW)
        {
            if (dvcType == p_CtrInfoList->at(0)->slotPosition && id == p_CtrInfoList->at(0)->dvcId)
            {
                p_CtrInfoList->at(0)->buf.append(recvTempData);
                processMsgFromCCO(dvcType, id);
            }
        }
        else if (dvcType == SingleSTA || dvcType == ThreeSTA)
        {
            for (int i = 0; i < p_CtrInfoList->at(0)->keyList.size(); i++)
            {
                if (dvcType == p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->value(p_CtrInfoList->at(0)->keyList.at(i))->slotPosition 
						&& id == p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->value(p_CtrInfoList->at(0)->keyList.at(i))->dvcId)
                {
                    if ((*p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList)[p_CtrInfoList->at(0)->keyList.at(i)]->prtcl == 0x02)
                    {
                        (*p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList)[p_CtrInfoList->at(0)->keyList.at(i)]->buf645.append(recvTempData);
                        processMsgFromMeter645(dvcType, id, p_CtrInfoList->at(0)->keyList.at(i));
                        break;
                    }
                    else if ((*p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList)[p_CtrInfoList->at(0)->keyList.at(i)]->prtcl == 0x03)
                    {
                        (*p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList)[p_CtrInfoList->at(0)->keyList.at(i)]->buf698.append(recvTempData);
                        processMsgFromMeterOOP(dvcType, id, p_CtrInfoList->at(0)->keyList.at(i));
                        break;
                    }
                }
            }
        }
        else if (dvcType == CJQ)
        {
        }
        else
        {
            return;
        }
    }
}
void Script_ChipIdManage_CCO::processCtrlDvcRes(DvcType dvcType, QList<int> chipIdList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params)
{
    if (p_CtrInfoList == nullptr || isSucs == false)
        return;
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("收到控制命令回复，参数个数=%1").arg(QString::number(params.size())));
    QList<int> sendParams;
    switch (emScriptRunState)
    {
    case TestInit:
    {
        break;
    }
    case Wait_BuildNetFinish_Whole:
    {
        p_BuildNetwork_GW->processCtrlDvcRes(dvcType, chipIdList, ctrlCmdType, isSucs, params);
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

void Script_ChipIdManage_CCO::processMsgFromCCO(DvcType dvcType, int dvcId)
{
    if (dvcId != p_CtrInfoList->at(0)->ctrlID)
        return;
    bool haveCompleteMsg = true;
    while (haveCompleteMsg)
    {
        qInfo() << QString("CCO-解析前 buf3762=%1").arg(QString((p_CtrInfoList->at(0)->buf.toHex())));
        shared_ptr<Frame3762Base> p_Frame3762Base = p_Frame3762Helper->DecodeLocalMsg(&(p_CtrInfoList->at(0)->buf), haveCompleteMsg);
        qInfo() << QString("CCO-解析后 buf3762=%1").arg(QString((p_CtrInfoList->at(0)->buf.toHex())));
        if (p_Frame3762Base == nullptr)
        {
            break;  // 必须是break，continue会导致无限循环！
        }
        switch (emScriptRunState)
        {
        case TestInit:
        {
            break;
        }
        case Wait_BuildNetFinish_Whole:
        {
            break;
        }
        case Wait_SetRouterChipID_Finish:
        {
            if (p_Frame3762Base->afn_ == 0x00 && p_Frame3762Base->dt1_ == 0x01 && p_Frame3762Base->dt2_ == 0x00 && p_Frame3762Base->ctrl_field_.dir == kDirUp && p_Frame3762Base->ctrl_field_.dir == kDirUp && p_Frame3762Base->info_field_.info_field_up.msg_seq == char(msgSeq - 1))
            {
                p_timer->stop();
                sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, INSIGNIFICANCE, p_QueryRouterChipID_10F40);
                emScriptRunState = Wait_QueryRouterChipID_Finish;
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询CCO芯片ID（10F40），等待--回复");
                p_timer->start(CTRL_CMD_TIMEOUT_TIME * 1000);
            }
            else if (p_Frame3762Base->afn_ == 0x00 && p_Frame3762Base->dt1_ == 0x02 && p_Frame3762Base->dt2_ == 0x00 && p_Frame3762Base->ctrl_field_.dir == kDirUp && p_Frame3762Base->info_field_.info_field_up.msg_seq == char(msgSeq - 1))
            {
                p_timer->stop();
                if (chipIdIndex != 0)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "设置CCO芯片ID（10F40）回复否认");
                    sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, INSIGNIFICANCE, p_QueryRouterChipID_10F40);
                    emScriptRunState = Wait_QueryRouterChipID_Finish;
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询CCO芯片ID（10F40），等待--回复");
                    p_timer->start(CTRL_CMD_TIMEOUT_TIME * 1000);
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "设置CCO芯片ID（10F40）回复否认");
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, "设置CCO芯片ID（10F40）回复否认");
                }
            }
            else
            {
                QByteArray tmpSendMsg = prcsOther3762Msg(uchar(p_Frame3762Base->afn_), p_Frame3762Base->dt1_, p_Frame3762Base->dt2_, uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType, dvcId, tmpSendMsg);
            }
            break;
        }
        case Wait_ParaInit_Finish:
        {
            if (p_Frame3762Base->afn_ == 0x00 && p_Frame3762Base->dt1_ == 0x01 && p_Frame3762Base->dt2_ == 0x00 && p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, INSIGNIFICANCE, p_QueryRouterChipID_10F40);
                emScriptRunState = Wait_QueryRouterChipID_Finish;
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询CCO芯片ID（10F40），等待--回复");
                p_timer->start(CTRL_CMD_TIMEOUT_TIME * 1000);
            }
            else
            {
                QByteArray tmpSendMsg = prcsOther3762Msg(uchar(p_Frame3762Base->afn_), p_Frame3762Base->dt1_, p_Frame3762Base->dt2_, uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType, dvcId, tmpSendMsg);
            }
            break;
        }
        case Wait_QueryRouterChipID_Finish:
        {
            //                if(p_Frame3762Base->afn_ == char(0xF0)&&p_Frame3762Base->dt1_==char(0x80)&&p_Frame3762Base->dt2_==0x04&&p_Frame3762Base->ctrl_field_.dir == kDirUp)
            //                {
            //                    p_timer->stop();
            //                    shared_ptr<AfnF0F40> p_QueryRouterChipID_F0F40_Up=dynamic_pointer_cast<AfnF0F40>(p_Frame3762Base);

            //                    if(memcmp(p_QueryRouterChipID_F0F40_Up->read_chip_id_unit_up_.mac_address_.addr,p_CtrInfoList->at(0)->ccoAddr,6)==0
            //                            &&p_QueryRouterChipID_F0F40_Up->read_chip_id_unit_up_.id_type_==0x01)
            //                    {
            //                        if(dstChipId==QString(p_QueryRouterChipID_F0F40_Up->read_chip_id_unit_up_.id_content_.toHex()))
            //                        {
            //                            p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("路由芯片ID为%1，符合要求").arg(QString(p_QueryRouterChipID_F0F40_Up->read_chip_id_unit_up_.id_content_.toHex())));

            //                            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,chipIdIndex,p_QueryRouterChipID_10F40);
            //                            emScriptRunState=Wait_QueryRouterChipID_Finish;
            //                            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询CCO芯片ID（10F40），等待--回复");
            //                            p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
            //                        }
            //                        else
            //                        {
            //                            p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("CCO芯片ID为%1，不符合要求").arg(QString(p_QueryRouterChipID_F0F40_Up->read_chip_id_unit_up_.id_content_.toHex())));
            //                        }
            //                    }
            //                    else
            //                    {
            //                        p_AbstractScriptHost->updateProgress(ProcessState_Failed,QString("CCO芯片ID为%1，不符合要求").arg(QString(p_QueryRouterChipID_F0F40_Up->read_chip_id_unit_up_.id_content_.toHex())));
            //                    }
            //                }
            if (p_Frame3762Base->afn_ == char(0x10) && p_Frame3762Base->dt1_ == char(0x80) && p_Frame3762Base->dt2_ == 0x04 && p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                shared_ptr<Afn10F40> p_QueryRouterChipID_10F40_Up = dynamic_pointer_cast<Afn10F40>(p_Frame3762Base);

                if (memcmp(p_QueryRouterChipID_10F40_Up->read_chip_id_unit_up_.mac_address_.addr, p_CtrInfoList->at(0)->ccoAddr, 6) == 0 
				         && p_QueryRouterChipID_10F40_Up->read_chip_id_unit_up_.id_type_ == 0x01 
				         && p_QueryRouterChipID_10F40_Up->read_chip_id_unit_up_.device_type_ == 0x02)
                {
                    if (dstChipId == QString(p_QueryRouterChipID_10F40_Up->read_chip_id_unit_up_.id_content_.toHex()))
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("路由芯片ID为%1，符合要求").arg(QString(p_QueryRouterChipID_10F40_Up->read_chip_id_unit_up_.id_content_.toHex())));

                        sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, chipIdIndex, p_QueryRouterChipID_10F112);
                        emScriptRunState = Wait_QueryRouterChipID_Finish;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "发送--查询CCO芯片ID（10F112），等待--回复");
                        p_timer->start(CTRL_CMD_TIMEOUT_TIME * 1000);
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("CCO芯片ID为%1，不符合要求").arg(QString(p_QueryRouterChipID_10F40_Up->read_chip_id_unit_up_.id_content_.toHex())));
                    }
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("CCO芯片ID为%1，不符合要求").arg(QString(p_QueryRouterChipID_10F40_Up->read_chip_id_unit_up_.id_content_.toHex())));
                }
            }
            else if (p_Frame3762Base->afn_ == 0x10 && p_Frame3762Base->dt1_ == char(0x80) && p_Frame3762Base->dt2_ == 0x0D && p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                shared_ptr<Afn10F112> p_QueryRouterChipID_10F112_Up = dynamic_pointer_cast<Afn10F112>(p_Frame3762Base);

                if (memcmp(p_QueryRouterChipID_10F112_Up->node_chip_info_unit_.node_chip_info_list_.at(0).node_address_.addr, p_CtrInfoList->at(0)->ccoAddr, 6) == 0 
				         && p_QueryRouterChipID_10F112_Up->node_chip_info_unit_.node_chip_info_list_.at(0).node_device_type_ == 0x02)
                {
                    if (dstChipId == QString(p_QueryRouterChipID_10F112_Up->node_chip_info_unit_.node_chip_info_list_.at(0).node_chip_id_.toHex()))
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("路由芯片ID为%1，符合要求").arg(QString(p_QueryRouterChipID_10F112_Up->node_chip_info_unit_.node_chip_info_list_.at(0).node_chip_id_.toHex())));
                        chipIdIndex++;
                        if (chipIdIndex < chipIdList.size())
                        {
                            sendMsg(CCO_GW, p_CtrInfoList->at(0)->ctrlID, chipIdIndex, p_SetRouterChipID_F0F1);
                            emScriptRunState = Wait_SetRouterChipID_Finish;
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("发送--设置CCO芯片ID，等待--确认"));
                            p_timer->start(CTRL_CMD_TIMEOUT_TIME * 1000);
                        }
                        else
                        {
                            // 测试成功时停止所有定时器，防止对象销毁后定时器触发导致崩溃
                            p_BuildNetwork_GW->initBuildNetWork();  // 停止BuildNetwork_GW内部的定时器
                            p_timer->stop();
                            p_maxAllowTimer->stop();
                            p_delayTimer->stop();
                            emScriptRunState = ScriptSuccess;
                            resultFlag = true;
                            p_AbstractScriptHost->updateProgress(ProcessState_Success, QString("CCO芯片ID管理测试成功;"));
                        }
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("CCO芯片ID为%1，不符合要求").arg(QString(p_QueryRouterChipID_10F112_Up->node_chip_info_unit_.node_chip_info_list_.at(0).node_chip_id_.toHex())));
                    }
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("CCO芯片ID为%1，不符合要求").arg(QString(p_QueryRouterChipID_10F112_Up->node_chip_info_unit_.node_chip_info_list_.at(0).node_chip_id_.toHex())));
                }
            }
            else
            {
                QByteArray tmpSendMsg = prcsOther3762Msg(uchar(p_Frame3762Base->afn_), p_Frame3762Base->dt1_, p_Frame3762Base->dt2_, uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType, dvcId, tmpSendMsg);
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
void Script_ChipIdManage_CCO::processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID)
{
    bool haveCompleteMsg = true;
    while (haveCompleteMsg)
    {
        shared_ptr<dlt_645_Protocol::Frame645Base> MsgBase_645_ptr = dlt_645_Protocol::Frame645Helper::DecodeLocalMsg(&((*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[mtrlID]->buf645), haveCompleteMsg);
        if (MsgBase_645_ptr == nullptr)
        {
            break;
        }
        switch (emScriptRunState)
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
            if (MsgBase_645_ptr->ctrlCode_ == READ_ADDR)
            {
                sendMsg(dvcType, dvcId, mtrlID, p_MeterAddrResp_93);
            }
            else
            {
                uchar di[4] = {0x00};
                if (MsgBase_645_ptr->ctrlCode_ == READ_DATA)
                {
                    shared_ptr<dlt_645_Protocol::Rqst_ReadData_0x11> p_ReadData_0x11 = std::dynamic_pointer_cast<dlt_645_Protocol::Rqst_ReadData_0x11>(MsgBase_645_ptr);
                    memcpy(di, p_ReadData_0x11->di, 4);

                    QByteArray tmpSendMsg = prcsOther645Msg(MsgBase_645_ptr->ctrlCode_, MsgBase_645_ptr->dataLen_, di, MsgBase_645_ptr->addr_, (*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[mtrlID]->mtrAddr, p_CtrInfoList);
                    sendSrcMsg(dvcType, dvcId, tmpSendMsg);
                }
                else
                {
                    QByteArray tmpSendMsg = prcsOther645Msg(MsgBase_645_ptr->ctrlCode_, MsgBase_645_ptr->dataLen_, di, MsgBase_645_ptr->addr_, (*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[mtrlID]->mtrAddr, p_CtrInfoList);
                    sendSrcMsg(dvcType, dvcId, tmpSendMsg);
                }
            }
            break;
        }
        }
    }
}
void Script_ChipIdManage_CCO::processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID)
{
    bool haveCompleteMsg = true;
    while (haveCompleteMsg)
    {
        shared_ptr<FrameOOPBase> MsgBase_OOP_ptr = FrameOOPHelper::DecodeMsg(&((*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[mtrlID]->buf698), haveCompleteMsg);

        if (MsgBase_OOP_ptr == nullptr)
        {
            break;
        }
        switch (emScriptRunState)
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
            if (MsgBase_OOP_ptr->service_type_ == GET_REQUEST_CLIENT && MsgBase_OOP_ptr->service_sub_type_ == uchar(GetRequestType::kGetRequestNormal))
            {
                OAD oad;
                oad.OI = ComuAddr;
                oad.attribute.feature = 0;
                oad.attribute.seq = 2;
                oad.element_index = 0;
                shared_ptr<GetRequestNormal> p_GetRequestNormal = dynamic_pointer_cast<GetRequestNormal>(MsgBase_OOP_ptr);
                if (p_GetRequestNormal->oad_.OI == oad.OI)
                {
                    sendMsg(dvcType, dvcId, mtrlID, p_GetResponseNormal_ReadAddr);
                }
                else
                {
                    QByteArray tmpSendMsg = prcsOther698Msg(MsgBase_OOP_ptr, (*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[mtrlID]->mtrAddr, p_CtrInfoList);
                    sendSrcMsg(dvcType, dvcId, tmpSendMsg);
                }
            }
            else
            {
                QByteArray tmpSendMsg = prcsOther698Msg(MsgBase_OOP_ptr, (*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[mtrlID]->mtrAddr, p_CtrInfoList);
                sendSrcMsg(dvcType, dvcId, tmpSendMsg);
            }
            break;
        }
        }
    }
}

QByteArray Script_ChipIdManage_CCO::buildFrameManually(const QByteArray& chipIdData)
{
    if (!p_AbstractScriptHost) {
        return QByteArray();
    }

    if (chipIdData.size() != 24) {
        p_AbstractScriptHost->updateProgress(ProcessState_Error,
            QString("buildFrameManually: 芯片ID数据长度错误: %1").arg(chipIdData.size()));
        return QByteArray();
    }

    // 376.2帧结构
    QByteArray frame;

    // 1. 起始字符
    frame.append((char)0x68);

    // 2. 长度L（2字节，先占位，总帧长度）
    frame.append((char)0x00);  // 长度低字节
    frame.append((char)0x00);  // 长度高字节

    // 3. 控制域C（1字节）
    frame.append((char)0x43);  // DIR=0, PRM=1, 通信方式=3

    // 4. 信息域R（6字节）
    frame.append((char)0x00);  // 字节1
    frame.append((char)0x00);  // 字节2
    frame.append((char)0x00);  // 字节3
    frame.append((char)0x00);  // 字节4（低字节）
    frame.append((char)0x00);  // 字节5（高字节）
    frame.append(char(msgSeq++));  // 字节6：报文序列号，使用后递增

    // 5. 应用数据域
    frame.append((char)0xF0);  // AFN=F0H
    frame.append((char)0x01);  // DT1=01H
    frame.append((char)0x00);  // DT2=00H
    frame.append(chipIdData);   // 24字节芯片ID

    // 6. 计算总帧长度（从起始符到结束符的所有字节）
    int totalLength = 39;  // 固定为39字节

    // 将长度转换为2字节（低字节在前）
    uchar lenLow = totalLength & 0xFF;
    uchar lenHigh = (totalLength >> 8) & 0xFF;

    frame[1] = lenLow;
    frame[2] = lenHigh;

    // 7. 计算校验和CS（控制域和用户数据区所有字节的八位位组算术和）
    uchar cs = 0;
    int startIndex = 3;  // 控制域的索引
    int endIndex = frame.size();  // 当前frame的最后一个字节索引（应用数据域结束）

    for (int i = startIndex; i < endIndex; i++) {
        cs += (uchar)frame.at(i);
    }

    frame.append(cs);

    // 8. 添加结束字符
    frame.append((char)0x16);

    // 验证帧结构
    int expectedFrameSize = 39;
    if (frame.size() != expectedFrameSize) {
        p_AbstractScriptHost->updateProgress(ProcessState_Processing,
            QString("帧大小不匹配: 实际%1字节, 期望%2字节").arg(frame.size()).arg(expectedFrameSize));
    }

    return frame;
}

void Script_ChipIdManage_CCO::sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame)
{
    sendMsgOct.clear();

    if (frame == p_SetRouterChipID_F0F1)
    {
        // 获取芯片ID字符串
        if (meterID < 0 || meterID >= chipIdList.size())
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("meterID索引越界: %1").arg(meterID));
            return;
        }

        QString chipIdStr = chipIdList.at(meterID);

        if (chipIdStr.length() > 48) {
            chipIdStr = chipIdStr.left(48); // 截断为24字节
        }

        while (chipIdStr.length() < 48) {
            chipIdStr += "0";
        }

        QByteArray hexBytes = QByteArray::fromHex(chipIdStr.toLatin1());

        // 检查长度
        if (hexBytes.size() != 24)
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("芯片ID长度错误: %1字节，期望24字节").arg(hexBytes.size()));
            return;
        }

        // 手动构建帧
        sendMsgOct = buildFrameManually(hexBytes);

        if (sendMsgOct.isEmpty())
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "手动构建帧失败");
            return;
        }

        // 发送消息
        QString fullFrameHex = QString(sendMsgOct.toHex().toUpper());
        sendMsgLog = QString("》》设置CCO芯片ID F0F1：%1\n").arg(fullFrameHex);
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, sendMsgLog);

        uchar *sendMsg = new uchar[uint(sendMsgOct.size())];
        memcpy(sendMsg, reinterpret_cast<uchar *>(sendMsgOct.data()), uint(sendMsgOct.size()));
        p_AbstractScriptHost->sendMsg2Dvc(dvcType, dvcId, sendMsg, sendMsgOct.size());

        // 保存目标芯片ID用于后续验证
        dstChipId = chipIdStr;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing,QString("dstChipId = %1").arg(dstChipId));
    }
    else if (frame == p_ParaInit_01F2)
    {
        p_ParaInit_01F2->ctrl_field_.dir = kDirDown;
        p_ParaInit_01F2->ctrl_field_.prm = kActive;
        p_ParaInit_01F2->ctrl_field_.comn_type = kHplc;

        p_ParaInit_01F2->info_field_.info_field_down.msg_seq = char(msgSeq++);
        p_ParaInit_01F2->info_field_.info_field_down.comu_rate = 0;
        p_ParaInit_01F2->info_field_.info_field_down.comu_module_ident = 0;

        sendMsgOct = p_ParaInit_01F2->EncodeFrame();
        sendMsgLog = QString("》》路由参数初始化01F2：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if (frame == p_QueryRouterChipID_10F40)
    {
        p_QueryRouterChipID_10F40->ctrl_field_.dir = kDirDown;
        p_QueryRouterChipID_10F40->ctrl_field_.prm = kActive;
        p_QueryRouterChipID_10F40->ctrl_field_.comn_type = kHplc;

        p_QueryRouterChipID_10F40->info_field_.info_field_down.msg_seq = char(msgSeq++);
        p_QueryRouterChipID_10F40->info_field_.info_field_down.comu_rate = 0;
        p_QueryRouterChipID_10F40->info_field_.info_field_down.comu_module_ident = 0;

        p_QueryRouterChipID_10F40->read_chip_id_unit_down_.device_type_ = 0x02;
        memcpy(p_QueryRouterChipID_10F40->read_chip_id_unit_down_.mac_address_.addr, p_CtrInfoList->at(0)->ccoAddr, 6);
        p_QueryRouterChipID_10F40->read_chip_id_unit_down_.id_type_ = 0x01;

        sendMsgOct = p_QueryRouterChipID_10F40->EncodeFrame();
        sendMsgLog = QString("》》查询CCO芯片ID 10F40：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    //    else if(frame==p_QueryRouterChipID_F0F40)
    //    {
    //        p_QueryRouterChipID_F0F40->ctrl_field_.dir=kDirDown;
    //        p_QueryRouterChipID_F0F40->ctrl_field_.prm=kActive;
    //        p_QueryRouterChipID_F0F40->ctrl_field_.comn_type=kHplc;

    //        p_QueryRouterChipID_F0F40->info_field_.info_field_down.msg_seq=char(msgSeq++);
    //        p_QueryRouterChipID_F0F40->info_field_.info_field_down.comu_rate=0;
    //        p_QueryRouterChipID_F0F40->info_field_.info_field_down.comu_module_ident=0;

    //        memcpy(p_QueryRouterChipID_F0F40->read_chip_id_unit_down_.mac_address_.addr,p_CtrInfoList->at(0)->ccoAddr,6);
    //        p_QueryRouterChipID_F0F40->read_chip_id_unit_down_.id_type_=0x01;
    //        p_QueryRouterChipID_F0F40->read_chip_id_unit_down_.id_format_=0x00;
    //        p_QueryRouterChipID_F0F40->read_chip_id_unit_down_.id_length_=0x18;

    //        sendMsgOct=p_QueryRouterChipID_F0F40->EncodeFrame();
    //        sendMsgLog=QString("》》查询CCO芯片ID F0F40：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    //    }
    else if (frame == p_QueryRouterChipID_10F112)
    {
        p_QueryRouterChipID_10F112->ctrl_field_.dir = kDirDown;
        p_QueryRouterChipID_10F112->ctrl_field_.prm = kActive;
        p_QueryRouterChipID_10F112->ctrl_field_.comn_type = kHplc;

        p_QueryRouterChipID_10F112->info_field_.info_field_down.msg_seq = char(msgSeq++);
        p_QueryRouterChipID_10F112->info_field_.info_field_down.comu_rate = 0;
        p_QueryRouterChipID_10F112->info_field_.info_field_down.comu_module_ident = 0;

        p_QueryRouterChipID_10F112->node_num_ = 0x01;
        p_QueryRouterChipID_10F112->node_start_no_ = 0x01;

        sendMsgOct = p_QueryRouterChipID_10F112->EncodeFrame();
        sendMsgLog = QString("》》查询CCO芯片ID 10F112：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if (frame == p_MeterAddrResp_93)
    {
        memcpy(p_MeterAddrResp_93->addr, (*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[meterID]->mtrAddr, 6);
        memcpy(p_MeterAddrResp_93->addr_, (*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[meterID]->mtrAddr, 6);

        sendMsgOct = p_MeterAddrResp_93->EncodeFrame();
        sendMsgLog = QString("》》 读通信地址应答(0x93)：%1\n").arg(QString(sendMsgOct.toHex()));
    }
    else if (frame == p_GetResponseNormal_ReadAddr)
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
        p_GetResponseNormal_ReadAddr->address_field_.sa.address = QString((QByteArray(reinterpret_cast<char *>((*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[meterID]->mtrAddr), 6).toHex()));
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

        std::dynamic_pointer_cast<DataString>(std::dynamic_pointer_cast<GetResultData>(p_GetResponseNormal_ReadAddr->a_result_normal_.get_result_ptr)->value_ptr_)->data_ = QByteArray(reinterpret_cast<char *>((*(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList))[meterID]->mtrAddr), 6);

        p_GetResponseNormal_ReadAddr->follow_report_field_.optional_ = 0;
        p_GetResponseNormal_ReadAddr->time_tag_field_.optional_ = 0;

        sendMsgOct = p_GetResponseNormal_ReadAddr->EncodeFrame();
        sendMsgLog = QString("》》 读通信地址应答(OOP)：%1\n").arg(QString(sendMsgOct.toHex()));
    }

    p_AbstractScriptHost->updateProgress(ProcessState_Processing, sendMsgLog);

    uchar *sendMsg = new uchar[uint(sendMsgOct.size())];
    memcpy(sendMsg, reinterpret_cast<uchar *>(sendMsgOct.data()), uint(sendMsgOct.size()));
    p_AbstractScriptHost->sendMsg2Dvc(dvcType, dvcId, sendMsg, sendMsgOct.size());
}
void Script_ChipIdManage_CCO::sendSrcMsg(DvcType dvcType, int dvcId, QByteArray msg)
{
    if (msg.size() < 1)
        return;
    uchar *sendMsg = new uchar[static_cast<uint>(msg.size())];
    memcpy(sendMsg, reinterpret_cast<uchar *>(msg.data()), uint(msg.size()));
    p_AbstractScriptHost->sendMsg2Dvc(dvcType, dvcId, sendMsg, msg.size());
    QStringList dvcList;
    dvcList << "单通" << "三通" << "国网路由" << "南网路由" << "采集器";
    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("测试脚本==>>>>上位机【设备类型:%1】的报文=%2").arg(dvcList.at(int(dvcType))).arg(QString(msg.toHex())));
}

void Script_ChipIdManage_CCO::timer_timeoutProc()
{
    switch (emScriptRunState)
    {
    case Wait_BuildNetFinish_Whole:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_BuildNetFinish_Whole timeout!!!");
        break;
    }
    case Wait_SetRouterChipID_Finish:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_SetRouterChipID_Finish timeout!!!");
        break;
    }
    case Wait_QueryRouterChipID_Finish:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_QueryRouterChipID_Finish timeout!!!");
        break;
    }
    default:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==p_timer");
        break;
    }
    }
}
void Script_ChipIdManage_CCO::maxAllowTimer_timeoutProc()
{
    switch (emScriptRunState)
    {
    case Wait_SetRouterChipID_Finish:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_SetRouterChipID_Finish maxtimeout!!!");
        break;
    }
    case Wait_QueryRouterChipID_Finish:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_QueryRouterChipID_Finish maxtimeout!!!");
        break;
    }
    default:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==p_maxAllowTimer");
        break;
    }
    }
}
void Script_ChipIdManage_CCO::delayTimer_timeoutProc()
{
    p_delayTimer->stop();
    index = 0;
    p_maxAllowTimer->stop();
}
