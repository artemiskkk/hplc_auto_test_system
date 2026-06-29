#include "Script_VersionHead_CcoFail.h"

Script_VersionHead_CcoFail::Script_VersionHead_CcoFail(QObject *parent) : QObject(parent)
{
    emScriptRunState=TestInit;
    resultFlag=false;
    sendMsgOct.clear();

    p_CtrInfoList=make_shared<QList<shared_ptr<CtrInfo>>>();
    p_BuildNetwork_GW=new BuildNetwork_GW();

    p_Frame3762Helper=make_shared<Frame3762Helper>();
    p_Confirm_00F1=make_shared<Afn00F1>();
    p_QueryVersionHeadF0F7_Down=make_shared<AfnF0F7>();
    p_SetVersionHeadF0F8_Down=make_shared<AfnF0F8>();
    p_ChkCcoOutVrsnInfo_10F104_Down=make_shared<Afn10F104>();
    p_ChkCcoOutVrsn_03F1_Down=make_shared<Afn03F1>();
    p_ChkCcoInVrsn_F0F15_Down=make_shared<AfnF0F15>();

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
Script_VersionHead_CcoFail::~Script_VersionHead_CcoFail()
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
void Script_VersionHead_CcoFail::execute()
{
    p_AbstractScriptHost->updateProgress(ProcessState_Start, "Test start!");

    resultFlag=false;

    if(needBuildNet==true)
    {
        p_BuildNetwork_GW->execute();
        emScriptRunState=Wait_BuildNetFinish_Whole;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "等待全网组网完成");
        p_timer->start((timerForReachThresld+timerAfterReachThresld)*1000);
    }
    else
    {
        //发送电表断电
        //sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryNodeInfo_10F2);
    //    emScriptRunState=Wait_EventReport_Finish;
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "电表断电--等待事件上报");
        //时间再定
        //p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
    }
    p_maxAllowTimer->start((timerForReachThresld+timerAfterReachThresld)*1000);
}
void Script_VersionHead_CcoFail::stop()
{
    if(p_AbstractScriptHost==nullptr)
        return;
    p_AbstractScriptHost->updateProgress(ProcessState_Error, "Test stop!");
    if(needPowerOff==true)
        powerOffAll12V(p_CtrInfoList,p_AbstractScriptHost);
}
void Script_VersionHead_CcoFail::addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq)
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
void Script_VersionHead_CcoFail::setHost(AbstractScriptHost *host)
{
    p_AbstractScriptHost=host;
    p_BuildNetwork_GW->setHost(host);
}
bool Script_VersionHead_CcoFail::config(const QMap<QString,QString> *paraDic)
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
        if(paraDic->keys().contains("BaseHead"))
        {
            this->BaseHead = (*paraDic)["BaseHead"];
        }
        if(paraDic->keys().contains("SpecialHead"))
        {
            this->SpecialHead = (*paraDic)["SpecialHead"];
        }
        result = true;
    }
    return result;
}
void Script_VersionHead_CcoFail::processMsg(DvcType dvcType, int id, uchar *data, int datalen)
{
    if(p_CtrInfoList->size()==0)
        return;
    if(emScriptRunState==Wait_BuildNetFinish_Whole)
    {
        if(!p_BuildNetwork_GW->startBuildNetFlag)
            p_BuildNetwork_GW->processMsg(dvcType,id,data,datalen);
        else
        {
            p_BuildNetwork_GW->initBuildNetWork();
            p_timer->stop();

            sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryVersionHeadF0F7_Down);
            emScriptRunState=ReadInfoBefore;
            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "F0F7查询路由初始版本头");
            p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
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
void Script_VersionHead_CcoFail::processCtrlDvcRes(DvcType dvcType, QList<int> idList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params)
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
    case ScriptSuccess:
    {
        break;
    }
    default:
        break;
    }
}

void Script_VersionHead_CcoFail::processMsgFromCCO(DvcType dvcType, int dvcId)
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
        case ReadInfoBefore:
        {
            dtValue3762 = get3762Dt(p_Frame3762Base->dt1_,p_Frame3762Base->dt2_);
            if(p_Frame3762Base->afn_==char(0xf0) && dtValue3762==7 && p_Frame3762Base->ctrl_field_.dir==kDirUp)
            {
                p_timer->stop();
                shared_ptr<AfnF0F7> p_QueryVersionHeadF0F7_Up=dynamic_pointer_cast<AfnF0F7>(p_Frame3762Base);
                initHead = p_QueryVersionHeadF0F7_Up->data_unit_up_.parameter_content_.toHex();
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "路由初始版本头："+initHead);
                char province = p_QueryVersionHeadF0F7_Up->data_unit_up_.parameter_content_.at(4);
                switch (province) {
                case 13:
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "初始版本头为浙江(13)，设置基准版本头");
                    chkInitHead = "特殊省份";
                    break;
                case 17:
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "初始版本头为江苏(17)，设置基准版本头");
                    chkInitHead = "特殊省份";
                    break;
                case 2:
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "初始版本头为湖南(2)，设置基准版本头");
                    chkInitHead = "特殊省份";
                    break;
                case 27:
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "初始版本头为陕西(27)，设置基准版本头");
                    chkInitHead = "特殊省份";
                    break;
                case 18:
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "初始版本头为北京(18)，设置基准版本头");
                    chkInitHead = "特殊省份";
                    break;
                case 1:
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "初始版本头为福建(1)，设置基准版本头");
                    chkInitHead = "特殊省份";
                    break;
                default:
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("初始版本头为基准省份(%0)，设置特殊省份版本头").arg(QString::number(province)));
                    chkInitHead = "基准省份";
                    break;
                }
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_ChkCcoOutVrsn_03F1_Down);
                p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "03F1查询路由初始外部版本");
            }
            else if(p_Frame3762Base->afn_==0x03 && dtValue3762==1 && p_Frame3762Base->ctrl_field_.dir==kDirUp)
            {
                p_timer->stop();
                shared_ptr<Afn03F1> p_ChkCcoOutVrsn_03F1_Up=std::dynamic_pointer_cast<Afn03F1>(p_Frame3762Base);
                ccoOutVrsn_init = (p_ChkCcoOutVrsn_03F1_Up->vendor_code_+p_ChkCcoOutVrsn_03F1_Up->chip_code_).toLatin1().toHex() + p_ChkCcoOutVrsn_03F1_Up->version_time_+p_ChkCcoOutVrsn_03F1_Up->version_;
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "03F1路由初始厂商芯片代码"+ccoOutVrsn_init.left(8)+"  外部版本"+ccoOutVrsn_init.right(10));
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "10F104查询路由初始外部版本");
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_ChkCcoOutVrsnInfo_10F104_Down);
                p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
            }
            else if(p_Frame3762Base->afn_==0x10 && dtValue3762==104 && p_Frame3762Base->ctrl_field_.dir==kDirUp)
            {
                p_timer->stop();
                shared_ptr<Afn10F104> p_ChkCcoOutVrsnInfo_10F104_Up=std::dynamic_pointer_cast<Afn10F104>(p_Frame3762Base);
                if(memcmp(p_CtrInfoList->at(0)->ccoAddr,p_ChkCcoOutVrsnInfo_10F104_Up->network_node_info_unit_.node_info_group_list_[0].node_address_.addr,6)==0)
                {
                    QByteArray rptVrsnDate;
                    rptVrsnDate.append(p_ChkCcoOutVrsnInfo_10F104_Up->network_node_info_unit_.node_info_group_list_[0].node_info_.ver_year_)
                               .append(p_ChkCcoOutVrsnInfo_10F104_Up->network_node_info_unit_.node_info_group_list_[0].node_info_.ver_month_)
                               .append(p_ChkCcoOutVrsnInfo_10F104_Up->network_node_info_unit_.node_info_group_list_[0].node_info_.ver_day_);

                    QByteArray rptVrsn;
                    rptVrsn.append(p_ChkCcoOutVrsnInfo_10F104_Up->network_node_info_unit_.node_info_group_list_[0].node_info_.software_version_[0])
                            .append(p_ChkCcoOutVrsnInfo_10F104_Up->network_node_info_unit_.node_info_group_list_[0].node_info_.software_version_[1]);

                    QByteArray rptVrsnMnfcCode;
                    rptVrsnMnfcCode.append(p_ChkCcoOutVrsnInfo_10F104_Up->network_node_info_unit_.node_info_group_list_[0].node_info_.vendor_code_[0])
                            .append(p_ChkCcoOutVrsnInfo_10F104_Up->network_node_info_unit_.node_info_group_list_[0].node_info_.vendor_code_[1]);

                    QByteArray rptVrsnChipCode;
                    rptVrsnChipCode.append(p_ChkCcoOutVrsnInfo_10F104_Up->network_node_info_unit_.node_info_group_list_[0].node_info_.chip_code_[0])
                            .append(p_ChkCcoOutVrsnInfo_10F104_Up->network_node_info_unit_.node_info_group_list_[0].node_info_.chip_code_[1]);
                    QString ccoOutVrsn_10F104 = (rptVrsnMnfcCode+rptVrsnChipCode).toHex() + (rptVrsnDate+rptVrsn).toHex();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "10F104路由初始厂商芯片代码"+ccoOutVrsn_10F104.left(8)+"  外部版本"+ccoOutVrsn_10F104.right(10));
                    if(ccoOutVrsn_10F104 == ccoOutVrsn_init)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "10F104路由外部版本和03F1一致，开始F0F15查询路由内部版本");
                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_ChkCcoInVrsn_F0F15_Down);
                        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "10F104路由外部版本和03F1不一致");
                    }
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("10F104回复节点非主节点地址"));
                }
            }
            else if(p_Frame3762Base->afn_==char(0xF0) && dtValue3762==15 && p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                shared_ptr<AfnF0F15> p_ChkCcoInVrsn_F0F15_Up=std::dynamic_pointer_cast<AfnF0F15>(p_Frame3762Base);
                ccoInVrsn_init = (p_ChkCcoInVrsn_F0F15_Up->vendor_code_+p_ChkCcoInVrsn_F0F15_Up->chip_code_).toLatin1().toHex() + p_ChkCcoInVrsn_F0F15_Up->version_time_+p_ChkCcoInVrsn_F0F15_Up->version_;
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "F0F15厂商芯片代码"+ccoInVrsn_init.left(8)+"  路由初始内部版本"+ccoInVrsn_init.right(10));

                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "开始F0F8设置路由版本头");
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,1,p_SetVersionHeadF0F8_Down);
                p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                emScriptRunState = SetAndReadInfo;
            }
            else
            {
                QByteArray tmpSendMsg=prcsOther3762Msg(uchar(p_Frame3762Base->afn_),p_Frame3762Base->dt1_,p_Frame3762Base->dt2_,uchar(p_Frame3762Base->info_field_.info_field_up.msg_seq));
                sendSrcMsg(dvcType,dvcId,tmpSendMsg);
            }
            break;
        }
        case SetAndReadInfo:
        {
            dtValue3762 = get3762Dt(p_Frame3762Base->dt1_,p_Frame3762Base->dt2_);
            if(p_Frame3762Base->afn_==0x00 && dtValue3762==1 && p_Frame3762Base->ctrl_field_.dir==kDirUp)
            {
                p_timer->stop();
                p_AbstractScriptHost->updateProgress(ProcessState_Failed, "设置版本头路由回复确认，不符合要求");
            }
            else if(p_Frame3762Base->afn_==0x00 && dtValue3762==2 && p_Frame3762Base->ctrl_field_.dir==kDirUp)
            {
                p_timer->stop();
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "设置版本头路由回复否认，符合要求，再F0F7查询版本头");
                sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_QueryVersionHeadF0F7_Down);
                p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
            }
            else if(p_Frame3762Base->afn_==char(0xf0) && dtValue3762==7 && p_Frame3762Base->ctrl_field_.dir==kDirUp)
            {
                p_timer->stop();
                shared_ptr<AfnF0F7> p_QueryVersionHeadF0F7_Up=dynamic_pointer_cast<AfnF0F7>(p_Frame3762Base);
                if(p_QueryVersionHeadF0F7_Up->data_unit_up_.parameter_content_.toHex() == initHead)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "查询路由版本头未改变，开始03F1读路由外部版本");
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_ChkCcoOutVrsn_03F1_Down);
                    p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, "查询路由版本头发生改变："+p_QueryVersionHeadF0F7_Up->data_unit_up_.parameter_content_.toHex());
                }
            }
            else if(p_Frame3762Base->afn_==0x03 && dtValue3762==1 && p_Frame3762Base->ctrl_field_.dir==kDirUp)
            {
                p_timer->stop();
                shared_ptr<Afn03F1> p_ChkCcoOutVrsn_03F1_Up=std::dynamic_pointer_cast<Afn03F1>(p_Frame3762Base);

                QString code_03F1 = (p_ChkCcoOutVrsn_03F1_Up->vendor_code_+p_ChkCcoOutVrsn_03F1_Up->chip_code_).toLatin1().toHex();
                QString vrsn_03F1 = p_ChkCcoOutVrsn_03F1_Up->version_time_+p_ChkCcoOutVrsn_03F1_Up->version_;
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "03F1路由厂商芯片代码"+code_03F1+"  外部版本"+vrsn_03F1);
                if(code_03F1+vrsn_03F1 == ccoOutVrsn_init)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "03F1路由外部版本未改变，开始10F104查询外部版本");
                    sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_ChkCcoOutVrsnInfo_10F104_Down);
                    p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, "03F1路由外部版本发生改变，初始外部版本"+ccoOutVrsn_init);
                }
            }
            else if(p_Frame3762Base->afn_==0x10 && dtValue3762==104 && p_Frame3762Base->ctrl_field_.dir==kDirUp)
            {
                p_timer->stop();
                shared_ptr<Afn10F104> p_ChkCcoOutVrsnInfo_10F104_Up=std::dynamic_pointer_cast<Afn10F104>(p_Frame3762Base);
                if(memcmp(p_CtrInfoList->at(0)->ccoAddr,p_ChkCcoOutVrsnInfo_10F104_Up->network_node_info_unit_.node_info_group_list_[0].node_address_.addr,6)==0)
                {
                    QByteArray rptVrsnDate;
                    rptVrsnDate.append(p_ChkCcoOutVrsnInfo_10F104_Up->network_node_info_unit_.node_info_group_list_[0].node_info_.ver_year_)
                               .append(p_ChkCcoOutVrsnInfo_10F104_Up->network_node_info_unit_.node_info_group_list_[0].node_info_.ver_month_)
                               .append(p_ChkCcoOutVrsnInfo_10F104_Up->network_node_info_unit_.node_info_group_list_[0].node_info_.ver_day_);

                    QByteArray rptVrsn;
                    rptVrsn.append(p_ChkCcoOutVrsnInfo_10F104_Up->network_node_info_unit_.node_info_group_list_[0].node_info_.software_version_[0])
                            .append(p_ChkCcoOutVrsnInfo_10F104_Up->network_node_info_unit_.node_info_group_list_[0].node_info_.software_version_[1]);

                    QByteArray rptVrsnMnfcCode;
                    rptVrsnMnfcCode.append(p_ChkCcoOutVrsnInfo_10F104_Up->network_node_info_unit_.node_info_group_list_[0].node_info_.vendor_code_[0])
                            .append(p_ChkCcoOutVrsnInfo_10F104_Up->network_node_info_unit_.node_info_group_list_[0].node_info_.vendor_code_[1]);

                    QByteArray rptVrsnChipCode;
                    rptVrsnChipCode.append(p_ChkCcoOutVrsnInfo_10F104_Up->network_node_info_unit_.node_info_group_list_[0].node_info_.chip_code_[0])
                            .append(p_ChkCcoOutVrsnInfo_10F104_Up->network_node_info_unit_.node_info_group_list_[0].node_info_.chip_code_[1]);

                    QString code_10F104 = (rptVrsnMnfcCode+rptVrsnChipCode).toHex();
                    QString vrsn_10F104 = (rptVrsnDate+rptVrsn).toHex();
                    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "10F104路由厂商芯片代码"+code_10F104+"  外部版本"+vrsn_10F104);
                    if(code_10F104+vrsn_10F104 == ccoOutVrsn_init)
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Processing, "10F104路由外部版本未改变，开始F0F15查询内部版本");
                        sendMsg(CCO_GW,p_CtrInfoList->at(0)->ctrlID,INSIGNIFICANCE,p_ChkCcoInVrsn_F0F15_Down);
                        p_timer->start(CTRL_CMD_TIMEOUT_TIME*1000);
                    }
                    else
                    {
                        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "10F104路由外部版本发生改变，初始外部版本"+ccoOutVrsn_init);
                    }
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("10F104回复节点非主节点地址"));
                }
            }
            else if(p_Frame3762Base->afn_==char(0xF0) && dtValue3762==15 && p_Frame3762Base->ctrl_field_.dir == kDirUp)
            {
                p_timer->stop();
                shared_ptr<AfnF0F15> p_ChkCcoInVrsn_F0F15_Up=std::dynamic_pointer_cast<AfnF0F15>(p_Frame3762Base);
                QString ccoInVrsn_F0F15 = (p_ChkCcoInVrsn_F0F15_Up->vendor_code_+p_ChkCcoInVrsn_F0F15_Up->chip_code_).toLatin1().toHex() + p_ChkCcoInVrsn_F0F15_Up->version_time_+p_ChkCcoInVrsn_F0F15_Up->version_;
                p_AbstractScriptHost->updateProgress(ProcessState_Processing, "F0F15厂商芯片代码"+ccoInVrsn_F0F15.left(8)+"  路由内部版本"+ccoInVrsn_F0F15.right(10));
                if(ccoInVrsn_F0F15 == ccoInVrsn_init)
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Success, "路由内部版本未改变");
                }
                else
                {
                    p_AbstractScriptHost->updateProgress(ProcessState_Failed, "路由内部版本发生改变，初始内部版本"+ccoInVrsn_init);
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
            break;
        }
    }
}
void Script_VersionHead_CcoFail::processMsgFromMeter645(DvcType dvcType, int dvcId, int mtrlID)
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
void Script_VersionHead_CcoFail::processMsgFromMeterOOP(DvcType dvcType, int dvcId, int mtrlID)
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
void Script_VersionHead_CcoFail::sendMsg(DvcType dvcType, int dvcId, int meterID, shared_ptr<void> frame)
{
    if(frame==p_Confirm_00F1)
    {
        p_Confirm_00F1->ctrl_field_.dir=kDirDown;
        p_Confirm_00F1->ctrl_field_.prm=kPassive;
        p_Confirm_00F1->ctrl_field_.comn_type=kHplc;

        p_Confirm_00F1->info_field_.info_field_down.msg_seq=char(msgSeq);
        p_Confirm_00F1->info_field_.info_field_down.comu_rate=0;
        p_Confirm_00F1->info_field_.info_field_down.comu_module_ident=0;

        memset(p_Confirm_00F1->data_info_,char(0xff),4);
        p_Confirm_00F1->data_info_[4]=0x00;
        p_Confirm_00F1->data_info_[5]=0x00;

        sendMsgOct=p_Confirm_00F1->EncodeFrame();
        sendMsgLog=QString("》》确认00F1：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_QueryVersionHeadF0F7_Down)
    {
        p_QueryVersionHeadF0F7_Down->ctrl_field_.dir=kDirDown;
        p_QueryVersionHeadF0F7_Down->ctrl_field_.prm=kActive;
        p_QueryVersionHeadF0F7_Down->ctrl_field_.comn_type=kHplc;

        p_QueryVersionHeadF0F7_Down->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_QueryVersionHeadF0F7_Down->info_field_.info_field_down.comu_rate=0;
        p_QueryVersionHeadF0F7_Down->info_field_.info_field_down.comu_module_ident=0;

        p_QueryVersionHeadF0F7_Down->data_unit_down_.parameter_id_[0]=0x02;
        p_QueryVersionHeadF0F7_Down->data_unit_down_.parameter_id_[1]=0x5d;

        sendMsgOct=p_QueryVersionHeadF0F7_Down->EncodeFrame();
        sendMsgLog=QString("》》查询版本头F0F7：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_SetVersionHeadF0F8_Down)
    {
        p_SetVersionHeadF0F8_Down->ctrl_field_.dir=kDirDown;
        p_SetVersionHeadF0F8_Down->ctrl_field_.prm=kActive;
        p_SetVersionHeadF0F8_Down->ctrl_field_.comn_type=kHplc;

        p_SetVersionHeadF0F8_Down->info_field_.info_field_down.msg_seq=char(msgSeq++);
        p_SetVersionHeadF0F8_Down->info_field_.info_field_down.comu_rate=0;
        p_SetVersionHeadF0F8_Down->info_field_.info_field_down.comu_module_ident=0;

        p_SetVersionHeadF0F8_Down->data_unit_down_.parameter_id_[0]=0x02;
        p_SetVersionHeadF0F8_Down->data_unit_down_.parameter_id_[1]=0x5d;

        if(chkInitHead == "基准省份")
        {
            p_SetVersionHeadF0F8_Down->data_unit_down_.parameter_length_=ushort(SpecialHead.length()/2);
            p_SetVersionHeadF0F8_Down->data_unit_down_.parameter_content_=QByteArray::fromHex(SpecialHead.toLatin1());
        }
        else if(chkInitHead == "特殊省份")
        {
            p_SetVersionHeadF0F8_Down->data_unit_down_.parameter_length_=ushort(BaseHead.length()/2);
            p_SetVersionHeadF0F8_Down->data_unit_down_.parameter_content_=QByteArray::fromHex(BaseHead.toLatin1());
        }
        else
        {
            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "chkInitHead出现错误："+chkInitHead);
        }

        sendMsgOct=p_SetVersionHeadF0F8_Down->EncodeFrame();
        sendMsgLog=QString("》》设置版本头F0F8：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_ChkCcoOutVrsnInfo_10F104_Down)
    {
        p_ChkCcoOutVrsnInfo_10F104_Down->start_no_=0;
        p_ChkCcoOutVrsnInfo_10F104_Down->this_query_num_=1;

        p_ChkCcoOutVrsnInfo_10F104_Down->ctrl_field_={kHplc,kActive,kDirDown};
        p_ChkCcoOutVrsnInfo_10F104_Down->info_field_.info_field_down.comu_module_ident=0;
        p_ChkCcoOutVrsnInfo_10F104_Down->info_field_.info_field_down.msg_seq=char(msgSeq++);

        sendMsgOct=p_ChkCcoOutVrsnInfo_10F104_Down->EncodeFrame();
        sendMsgLog=QString("》》查询CCO外部版本信息10F104：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_ChkCcoOutVrsn_03F1_Down)
    {
        p_ChkCcoOutVrsn_03F1_Down->ctrl_field_={kHplc,kActive,kDirDown};
        p_ChkCcoOutVrsn_03F1_Down->info_field_.info_field_down.comu_module_ident=0;
        p_ChkCcoOutVrsn_03F1_Down->info_field_.info_field_down.msg_seq=char(msgSeq++);

        sendMsgOct=p_ChkCcoOutVrsn_03F1_Down->EncodeFrame();
        sendMsgLog=QString("》》查询CCO外部版本03F1：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
    }
    else if(frame==p_ChkCcoInVrsn_F0F15_Down)
    {
        p_ChkCcoInVrsn_F0F15_Down->ctrl_field_={kHplc,kActive,kDirDown};
        p_ChkCcoInVrsn_F0F15_Down->info_field_.info_field_down.comu_module_ident=0;
        p_ChkCcoInVrsn_F0F15_Down->info_field_.info_field_down.msg_seq=char(msgSeq++);

        sendMsgOct=p_ChkCcoInVrsn_F0F15_Down->EncodeFrame();
        sendMsgLog=QString("》》查询CCO内部版本F0F15：%1\n").arg(QString(sendMsgOct.toHex().toUpper()));
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
void Script_VersionHead_CcoFail::sendSrcMsg(DvcType dvcType, int dvcId, QByteArray msg)
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

void Script_VersionHead_CcoFail::timer_timeoutProc()
{
    p_timer->stop();
    switch(emScriptRunState)
    {
//    case Wait_BuildNetFinish_Whole:
//    {
//        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_BuildNetFinish_Whole timeout!!!");
//        break;
//    }
//    case Wait_HardwareInit_01F1_Finish:
//    {
//        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "Wait_HardwareInit_01F1_Finish timeout!!!");
//        break;
//    }
//    case Wait_EventReport_Finish:
//    {
//        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "QueryNetScale_10F9 timeout!!!");
//      //  powerOn220V_CJQ(p_CtrInfoList,p_AbstractScriptHost);
//        break;
//    }
    default:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Error, "超时未回复");
        break;
    }
    }
}
void Script_VersionHead_CcoFail::maxAllowTimer_timeoutProc()
{
    p_maxAllowTimer->stop();
    switch(emScriptRunState)
    {
//    case Wait_EventReport_Finish:
//    {
//        if(receiveEventFlag==true)
//        {
//            p_AbstractScriptHost->updateProgress(ProcessState_Processing, "组网到达最大等待时间，普通电表事件汇总：\n"+eventMsgList);
//            p_AbstractScriptHost->updateProgress(ProcessState_Success, "收到电表事件上报");
//        }
//        else
//        {
//            p_AbstractScriptHost->updateProgress(ProcessState_Failed, "组网到达最大等待时间，未收到电表事件上报");
//        }
//        break;
//    }
    default:
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Error, "State machine run error!!!==p_maxAllowTimer");
        break;
    }
    }
}
void Script_VersionHead_CcoFail::delayTimer_timeoutProc()
{
    p_delayTimer->stop();
//    if(emScriptRunState == SetAndReadInfo)
//    {
//        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "等待3min未收到路由复位后03F10运行模式信息上报");
//    }
//    else if(emScriptRunState == SetAndReadInfo_2)
//    {
//        p_AbstractScriptHost->updateProgress(ProcessState_Success, "设置相同省份版本头后等待3min路由未出现复位");
//    }
//    else
//    {
//        p_AbstractScriptHost->updateProgress(ProcessState_Failed, "状态机出现错误");
//    }
}

void Script_VersionHead_CcoFail::calPowerOnReportRate()
{
    QString powerOnFailMeter;
    QString powerOffFailMeter;
    QString otherEventReportMeter;
    struct MeterStruct
    {
        Address meterNo;
        bool powerOnReportFlag=false;
        bool powerOffReportFlag=false;
    };
    QList<MeterStruct> allMeterList;
    for(int i=0;i<p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().size();i++)
    {
        if(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(i)->slotPosition==SingleSTA||p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(i)->slotPosition==ThreeSTA)
            continue;
        MeterStruct meter;
        memcpy(meter.meterNo.addr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(i)->mtrAddr,6);
        allMeterList.append(meter);
    }
    for(int i=0;i<allMeterList.size();i++)
    {
        for(int j=0;j<powerOnReportList.size();j++)
        {
            if(allMeterList.at(i).meterNo==powerOnReportList.at(j).reportNodeAddress)
            {
                allMeterList[i].powerOnReportFlag=true;
            }
        }
        for(int j=0;j<powerOffReportList.size();j++)
        {
            if(allMeterList.at(i).meterNo==powerOffReportList.at(j).reportNodeAddress)
            {
                allMeterList[i].powerOffReportFlag=true;
            }
        }
    }
    for(int i=0;i<otherEventReportList.size();i++)
    {
        otherEventReportMeter.append(QString(QByteArray(otherEventReportList.at(i).reportNodeAddress.addr,6).toHex())+";");
    }
    for(int i=0;i<allMeterList.size();i++)
    {
        if(allMeterList.at(i).powerOnReportFlag==false)
            powerOnFailMeter.append(QString(QByteArray(allMeterList.at(i).meterNo.addr,6).toHex())+";");
        if(allMeterList.at(i).powerOffReportFlag==false)
            powerOffFailMeter.append(QString(QByteArray(allMeterList.at(i).meterNo.addr,6).toHex())+";");
    }
    if(!otherEventReportMeter.isEmpty())
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("上报其它事件的电表如下：\n%1").arg(powerOffFailMeter));
    }
    if(powerOnFailMeter.isEmpty()==true&&powerOffFailMeter.isEmpty()==true)
    {
        emScriptRunState=ScriptSuccess;
        resultFlag=true;
        p_AbstractScriptHost->updateProgress(ProcessState_Success, QString("停复电事件上报测试成功！"));
    }
    else if(powerOnFailMeter.isEmpty()!=true&&powerOffFailMeter.isEmpty()!=true)
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("停电事件上报不全！\n失败电表如下：\n%1").arg(powerOffFailMeter));
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("上电事件上报不全！\n失败电表如下：\n%1").arg(powerOnFailMeter));
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("停复电事件均上报不全！"));
    }
    else if(powerOnFailMeter.isEmpty()!=true&&powerOffFailMeter.isEmpty()==true)
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("上电事件上报不全！\n失败电表如下：\n%1").arg(powerOnFailMeter));
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("复电事件上报不全！"));
    }
    else if(powerOnFailMeter.isEmpty()==true&&powerOffFailMeter.isEmpty()!=true)
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Processing, QString("停电事件上报不全！\n失败电表如下：\n%1").arg(powerOffFailMeter));
        p_AbstractScriptHost->updateProgress(ProcessState_Failed, QString("停电事件上报不全！"));
    }
}
