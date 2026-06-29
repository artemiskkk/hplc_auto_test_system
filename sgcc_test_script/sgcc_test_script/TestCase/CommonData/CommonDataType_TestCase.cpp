#include "CommonDataType_TestCase.h"


MeterInfoForSingleNet::MeterInfoForSingleNet()
{

}
MeterInfoForSingleNet::~MeterInfoForSingleNet()
{

}



CtrInfo::CtrInfo()
{
    p_MeterInfoForSingleNetList = nullptr;
    memset(sucsCnt_CtrlCmd, 0, sizeof(sucsCnt_CtrlCmd));
    memset(successCnt, 0, sizeof(successCnt));
    inNetSuccessCnt = 0;
    totalMeterCnt = 0;
    totalNodeCnt = 0;
}
CtrInfo::~CtrInfo()
{
    if(p_MeterInfoForSingleNetList != nullptr)
    {
        qDeleteAll(*p_MeterInfoForSingleNetList);
        p_MeterInfoForSingleNetList->clear();
        delete p_MeterInfoForSingleNetList;
        p_MeterInfoForSingleNetList = nullptr;
    }
}
void memsetDouble(double *buffer, double c, int count)
{
    for(int i=0; i<count; i++)
        buffer[i]=c;
}
void Init_AttachedNodeInfo(ushort startIndex, ushort v_meterNum, QList<int> keyList, QMap<int,MeterInfoForSingleNet*> *p_MeterInfoForSingleNetList, shared_ptr<QList<NodeParameter>> p_AttachedNodeInfoList)
{
    ushort meterIndex=0;

    p_AttachedNodeInfoList->clear();
    for(meterIndex=startIndex; meterIndex<startIndex+v_meterNum; meterIndex=meterIndex+1)
    {
        if(p_MeterInfoForSingleNetList->contains(keyList.at(meterIndex)))
        {
            NodeParameter nodeParameter;
            memcpy(nodeParameter.node_address_.addr,p_MeterInfoForSingleNetList->value(keyList.at(meterIndex))->mtrAddr,6);
            nodeParameter.protocol_type_ = char(p_MeterInfoForSingleNetList->value(keyList.at(meterIndex))->prtcl);
            p_AttachedNodeInfoList->append(nodeParameter);
        }
    }
}

void Refresh_CtrInfo_Result_for_CtrlCmdRes(shared_ptr<CtrInfo> p_CtrInfo, DvcType dvcType, int id, CtrlCmdType ctrlCmdType)
{
    if(!p_CtrInfo->p_MeterInfoForSingleNetList->contains(id))
        return;

    memset(p_CtrInfo->sucsCnt_CtrlCmd,0,sizeof(p_CtrInfo->sucsCnt_CtrlCmd));

    if(dvcType==CCO_GW)
    {
        if(ctrlCmdType==CtrlCmd_PowerOn_12V)
        {
            p_CtrInfo->res_CtrlCmd[1]=true;
            p_CtrInfo->sucsCnt_CtrlCmd[1]+=1;
        }
        else if(ctrlCmdType==CtrlCmd_PowerOff_12V)
        {
            p_CtrInfo->res_CtrlCmd[2]=true;
            p_CtrInfo->sucsCnt_CtrlCmd[2]+=1;
        }
        else if(ctrlCmdType==CtrlCmd_PowerOn_220V)
        {
            p_CtrInfo->res_CtrlCmd[3]=true;
            p_CtrInfo->sucsCnt_CtrlCmd[3]+=1;
        }
        else if(ctrlCmdType==CtrlCmd_PowerOff_220V)
        {
            p_CtrInfo->res_CtrlCmd[4]=true;
            p_CtrInfo->sucsCnt_CtrlCmd[4]+=1;
        }
        else if(ctrlCmdType==CtrlCmd_PowerOnAll)
        {
            p_CtrInfo->res_CtrlCmd[5]=true;
            p_CtrInfo->sucsCnt_CtrlCmd[5]+=1;
        }
        else if(ctrlCmdType==CtrlCmd_PowerOffAll)
        {
            p_CtrInfo->res_CtrlCmd[6]=true;
            p_CtrInfo->sucsCnt_CtrlCmd[6]+=1;
        }
        else if(ctrlCmdType==CtrlCmd_SetBaudRate)
        {
            p_CtrInfo->res_CtrlCmd[7]=true;
            p_CtrInfo->sucsCnt_CtrlCmd[7]+=1;
        }
    }
    else if(dvcType==SingleSTA || dvcType==ThreeSTA)
    {
        if(ctrlCmdType==CtrlCmd_PowerOn_12V)
            (*(p_CtrInfo->p_MeterInfoForSingleNetList))[id]->ctrlCmdExecList[1]=true;
        else if(ctrlCmdType==CtrlCmd_PowerOff_12V)
            (*(p_CtrInfo->p_MeterInfoForSingleNetList))[id]->ctrlCmdExecList[2]=true;
        else if(ctrlCmdType==CtrlCmd_PowerOn_220V)
            (*(p_CtrInfo->p_MeterInfoForSingleNetList))[id]->ctrlCmdExecList[3]=true;
        else if(ctrlCmdType==CtrlCmd_PowerOff_220V)
            (*(p_CtrInfo->p_MeterInfoForSingleNetList))[id]->ctrlCmdExecList[4]=true;
        else if(ctrlCmdType==CtrlCmd_PowerOnAll)
            (*(p_CtrInfo->p_MeterInfoForSingleNetList))[id]->ctrlCmdExecList[5]=true;
        else if(ctrlCmdType==CtrlCmd_PowerOffAll)
            (*(p_CtrInfo->p_MeterInfoForSingleNetList))[id]->ctrlCmdExecList[6]=true;
        else if(ctrlCmdType==CtrlCmd_SetBaudRate)
            (*(p_CtrInfo->p_MeterInfoForSingleNetList))[id]->ctrlCmdExecList[7]=true;
        else if(ctrlCmdType==CtrlCmd_EventPinHigh)
            (*(p_CtrInfo->p_MeterInfoForSingleNetList))[id]->ctrlCmdExecList[8]=true;
        else if(ctrlCmdType==CtrlCmd_EventPinLow)
            (*(p_CtrInfo->p_MeterInfoForSingleNetList))[id]->ctrlCmdExecList[9]=true;
    }

    for(int i=0; i<p_CtrInfo->p_MeterInfoForSingleNetList->size(); i++)
    {
        for(int j=0; j<20; j++)
        {
            if(p_CtrInfo->p_MeterInfoForSingleNetList->values().at(i)->ctrlCmdExecList[j]==true)
                p_CtrInfo->sucsCnt_CtrlCmd[j]+=1;
        }
    }

    for(int i=0; i<20; i++)
    {
        p_CtrInfo->sucsRate_CtrlCmd[i]=p_CtrInfo->sucsCnt_CtrlCmd[i]/p_CtrInfo->totalNodeCnt;
    }
}

void Refresh_CtrInfo_Result_for_AssignAddr(shared_ptr<CtrInfo> p_CtrInfo, int mtrId)
{
    if(!p_CtrInfo->p_MeterInfoForSingleNetList->contains(mtrId))
        return;

    if(p_CtrInfo->p_MeterInfoForSingleNetList->value(mtrId)->slotPosition == SingleSTA
            || p_CtrInfo->p_MeterInfoForSingleNetList->value(mtrId)->slotPosition == ThreeSTA
            || p_CtrInfo->p_MeterInfoForSingleNetList->value(mtrId)->slotPosition == CJQ)
    {
        (*(p_CtrInfo->p_MeterInfoForSingleNetList))[mtrId]->ctrlCmdExecList[0]=true;
    }


    p_CtrInfo->sucsCnt_CtrlCmd[0]=0;
    int totalNodeCnt = 0;
    for(int i=0; i<p_CtrInfo->p_MeterInfoForSingleNetList->size(); i++)
    {
        if(p_CtrInfo->p_MeterInfoForSingleNetList->values().at(i)->ctrlCmdExecList[0])
            p_CtrInfo->sucsCnt_CtrlCmd[0]+=1;

        if(p_CtrInfo->p_MeterInfoForSingleNetList->values().at(i)->slotPosition == SingleSTA
                || p_CtrInfo->p_MeterInfoForSingleNetList->values().at(i)->slotPosition == ThreeSTA
                || p_CtrInfo->p_MeterInfoForSingleNetList->values().at(i)->slotPosition == CJQ)
        {
            totalNodeCnt +=1;
        }
    }

    if(totalNodeCnt >0)
    {
        p_CtrInfo->sucsRate_CtrlCmd[0]=static_cast<double>(p_CtrInfo->sucsCnt_CtrlCmd[0])/static_cast<double>(totalNodeCnt);
    }
    else
    {
        p_CtrInfo->sucsRate_CtrlCmd[0] = 1;
    }
}
void Refresh_CtrInfo_Result_for_BuildNet(ushort index, shared_ptr<CtrInfo> p_CtrInfo,shared_ptr<Afn10F21> p_ReqNetTopoInfo_10F21)
{
    if(p_CtrInfo->cjqAddressAccessNetFlag==false)
    {
        if(index==0)
        {

        }
        for(int i=0; i<p_ReqNetTopoInfo_10F21->network_typelogy_info_unit_.this_node_num_; i++)
        {
            for(int j=0; j<p_CtrInfo->p_MeterInfoForSingleNetList->size(); j++)
            {
                if(true==isArrayEqual(p_CtrInfo->p_MeterInfoForSingleNetList->values().at(j)->mtrAddr, reinterpret_cast<uchar*>(const_cast<char*>(p_ReqNetTopoInfo_10F21->network_typelogy_info_unit_.network_typelogy_info_List_.at(i).node_address_.addr)), 6))
                {
                    if(p_ReqNetTopoInfo_10F21->network_typelogy_info_unit_.network_typelogy_info_List_.at(i).node_info_.node_role_==Role_STA ||
                            p_ReqNetTopoInfo_10F21->network_typelogy_info_unit_.network_typelogy_info_List_.at(i).node_info_.node_role_==Role_PCO)
                    {
                        p_CtrInfo->p_MeterInfoForSingleNetList->values().at(j)->inNetResult=true;
                        break;
                    }
                }
            }
        }
        p_CtrInfo->inNetSuccessCnt=0;
        for(int k=0; k<p_CtrInfo->totalNodeCnt; k++)
        {
            if(p_CtrInfo->p_MeterInfoForSingleNetList->values().at(k)->inNetResult)
            {
                p_CtrInfo->inNetSuccessCnt=p_CtrInfo->inNetSuccessCnt+1;
            }
        }

        int needInNetNodeCount=0;
        int cjqNodeCount=0;
        for(int k=0;k<p_CtrInfo->cjqNodeList.size();k++)
        {
            cjqNodeCount+=p_CtrInfo->cjqNodeList.at(k).subsidiaryNodeAddrList.size();
        }
        needInNetNodeCount=p_CtrInfo->totalNodeCnt-cjqNodeCount+p_CtrInfo->cjqNodeList.size()+p_CtrInfo->notInParameterCjqNum;
        p_CtrInfo->inNetSuccessRate=double(p_CtrInfo->inNetSuccessCnt)/double(needInNetNodeCount);

        p_ReqNetTopoInfo_10F21->network_typelogy_info_unit_.network_typelogy_info_List_.clear();
    }
    else
    {
        int needInNetNodeCount=0;
        int cjqNodeCount=0;
        for(int k=0;k<p_CtrInfo->cjqNodeList.size();k++)
        {
            cjqNodeCount+=p_CtrInfo->cjqNodeList.at(k).subsidiaryNodeAddrList.size();
        }
        needInNetNodeCount=p_CtrInfo->totalNodeCnt-cjqNodeCount+p_CtrInfo->cjqNodeList.size()+p_CtrInfo->notInParameterCjqNum;
        p_CtrInfo->inNetSuccessRate=double(p_CtrInfo->inNetSuccessCnt)/double(needInNetNodeCount);
        p_CtrInfo->inNetSuccessRate=double(p_ReqNetTopoInfo_10F21->network_typelogy_info_unit_.node_total_num_-1)/double(needInNetNodeCount);
        p_ReqNetTopoInfo_10F21->network_typelogy_info_unit_.network_typelogy_info_List_.clear();
    }
}
//void Refresh_CtrInfo_Result_for_BuildNet_Special(ushort index, CtrInfo *p_CtrInfo, ReqNetTopoInfo_10F21_Up *p_ReqNetTopoInfo_10F21_Up)
//{
//    if(index==0)
//    {
//        for(int j=0; j<p_CtrInfo->p_MeterInfoForSingleNetList->size(); j++)
//        {
//            p_CtrInfo->p_MeterInfoForSingleNetList->values().at(j)->inNetResult=false;
//        }
//    }

//    for(int i=0; i<p_ReqNetTopoInfo_10F21_Up->rptNodeNumThisTime; i++)
//    {
//        for(int j=0; j<p_CtrInfo->p_MeterInfoForSingleNetList->size(); j++)
//        {
//            if(true==isArrayEqual(p_CtrInfo->p_MeterInfoForSingleNetList->values().at(j)->mtrAddr, p_ReqNetTopoInfo_10F21_Up->p_AddrAndNetTopoInfoList[i]->nodeAddr, 6))
//            {
//                if(p_ReqNetTopoInfo_10F21_Up->p_AddrAndNetTopoInfoList[i]->stNetTopoInfo.stNodeInfo.nodeRole==Role_STA ||
//                        p_ReqNetTopoInfo_10F21_Up->p_AddrAndNetTopoInfoList[i]->stNetTopoInfo.stNodeInfo.nodeRole==Role_PCO)
//                {
//                    p_CtrInfo->p_MeterInfoForSingleNetList->values().at(j)->inNetResult=true;
//                    break;
//                }
//            }
//        }
//    }

//    p_CtrInfo->inNetSuccessCnt=0;
//    for(int k=0; k<p_CtrInfo->totalNodeCnt; k++)
//    {
//        if(p_CtrInfo->p_MeterInfoForSingleNetList->values().at(k)->inNetResult)
//        {
//            p_CtrInfo->inNetSuccessCnt=p_CtrInfo->inNetSuccessCnt+1;
//        }
//    }

//    p_CtrInfo->inNetSuccessRate=(double)p_CtrInfo->inNetSuccessCnt/(double)p_CtrInfo->totalNodeCnt;



//    qDeleteAll(p_ReqNetTopoInfo_10F21_Up->p_AddrAndNetTopoInfoList.begin(), p_ReqNetTopoInfo_10F21_Up->p_AddrAndNetTopoInfoList.end());
//    p_ReqNetTopoInfo_10F21_Up->p_AddrAndNetTopoInfoList.clear();
//}

void Refresh_CtrInfo_Result_for_ReadMeter_13F1(shared_ptr<CtrInfo> p_CtrInfo, shared_ptr<Afn13F1> p_Afn13F1)
{
    for(int j=0; j<p_CtrInfo->totalNodeCnt; j++)
    {
        if(p_Afn13F1->data_field_up_.frame_length_<12)
            break;

        uchar rdResAddr[6]={0x00};
        if(p_Afn13F1->data_field_up_.protocol_type_==0x02)
            rvrsAddr(reinterpret_cast<uchar*>(p_Afn13F1->data_field_up_.frame_content_.data()+1),rdResAddr,6);
        else if(p_Afn13F1->data_field_up_.protocol_type_==0x03 || p_Afn13F1->data_field_up_.protocol_type_==0x00)
            rvrsAddr(reinterpret_cast<uchar*>(p_Afn13F1->data_field_up_.frame_content_.data()+5),rdResAddr,6);
        else
            rvrsAddr(reinterpret_cast<uchar*>(p_Afn13F1->data_field_up_.frame_content_.data()+1),rdResAddr,6);

        if(isArrayEqual(p_CtrInfo->p_MeterInfoForSingleNetList->values().at(j)->mtrAddr,rdResAddr,6))
        {
            p_CtrInfo->p_MeterInfoForSingleNetList->values().at(j)->testResultList[1]=true;
            break;
        }
    }

    p_CtrInfo->successCnt[1]=0;
    for(int k=0; k<p_CtrInfo->totalNodeCnt; k++)
    {
        if(p_CtrInfo->p_MeterInfoForSingleNetList->values().at(k)->testResultList[1])
        {
            p_CtrInfo->successCnt[1]=p_CtrInfo->successCnt[1]+1;
        }
    }

    p_CtrInfo->successRate[1]=double(p_CtrInfo->successCnt[1])/double(p_CtrInfo->totalNodeCnt);
}

//void Refresh_CtrInfo_Result_for_ReadMeter_F1F1(CtrInfo *p_CtrInfo, ParallelReadMeter_F1F1_Up *p_ParallelReadMeter_F1F1_Up)
//{
//    for(int j=0; j<p_CtrInfo->totalNodeCnt; j++)
//    {
//        if(p_ParallelReadMeter_F1F1_Up->msgLen<12)
//            break;

//        uchar rdResAddr[6]={0x00};
//        if(p_ParallelReadMeter_F1F1_Up->comuPrtlType==0x02)
//            rvrsAddr((uchar*)(p_ParallelReadMeter_F1F1_Up->msg.data()+1),rdResAddr,6);
//        else if(p_ParallelReadMeter_F1F1_Up->comuPrtlType==0x03 || p_ParallelReadMeter_F1F1_Up->comuPrtlType==0x00)
//            rvrsAddr((uchar*)(p_ParallelReadMeter_F1F1_Up->msg.data()+5),rdResAddr,6);
//        else
//            rvrsAddr((uchar*)(p_ParallelReadMeter_F1F1_Up->msg.data()+1),rdResAddr,6);

//        if(isArrayEqual(p_CtrInfo->p_MeterInfoForSingleNetList->values().at(j)->mtrAddr,rdResAddr,6))
//        {
//            p_CtrInfo->p_MeterInfoForSingleNetList->values().at(j)->testResultList[2]=true;
//            break;
//        }
//    }

//    p_CtrInfo->successCnt[2]=0;
//    for(int k=0; k<p_CtrInfo->totalNodeCnt; k++)
//    {
//        if(p_CtrInfo->p_MeterInfoForSingleNetList->values().at(k)->testResultList[2])
//        {
//            p_CtrInfo->successCnt[2]=p_CtrInfo->successCnt[2]+1;
//        }
//    }

//    p_CtrInfo->successRate[2]=(double)p_CtrInfo->successCnt[2]/(double)p_CtrInfo->totalNodeCnt;
//}

void Init_CtrInfo_Result_for_ReadMeter_F1F1(CtrInfo *p_CtrInfo)
{
    for(int j=0; j<p_CtrInfo->totalNodeCnt; j++)
    {
        p_CtrInfo->p_MeterInfoForSingleNetList->values().at(j)->testResultList[2]=false;
        memset(p_CtrInfo->p_MeterInfoForSingleNetList->values().at(j)->ctrlCmdExecList,false,20);
    }

    p_CtrInfo->successCnt[2]=0;
    for(int k=0; k<p_CtrInfo->totalNodeCnt; k++)
    {
        if(p_CtrInfo->p_MeterInfoForSingleNetList->values().at(k)->testResultList[2])
        {
            p_CtrInfo->successCnt[2]=p_CtrInfo->successCnt[2]+1;
        }
    }

    p_CtrInfo->successRate[2]=double(p_CtrInfo->successCnt[2])/double(p_CtrInfo->totalNodeCnt);
}


//void Refresh_CtrInfo_Result_for_SearchMeter_06F1(CtrInfo *p_CtrInfo, RptSlaveNodeInfo_06F1_Up *p_RptSlaveNodeInfo_06F1_Up)
//{
//    for(int i=0; i<p_RptSlaveNodeInfo_06F1_Up->slaveNodeNum; i++)
//    {
//        for(int j=0; j<p_CtrInfo->totalNodeCnt; j++)
//        {
//            if(true==isArrayEqual(p_CtrInfo->p_MeterInfoForSingleNetList->values().at(j)->mtrAddr, p_RptSlaveNodeInfo_06F1_Up->SlaveNodeInfo_06F1List.at(i)->nodeAddr, 6) &&
//                    p_CtrInfo->p_MeterInfoForSingleNetList->values().at(j)->prtcl==p_RptSlaveNodeInfo_06F1_Up->SlaveNodeInfo_06F1List.at(i)->protocolType)
//            {
//                p_CtrInfo->p_MeterInfoForSingleNetList->values().at(j)->testResultList[0]=true;
//                break;
//            }
//        }
//    }

//    p_CtrInfo->successCnt[0]=0;
//    for(int k=0; k<p_CtrInfo->totalNodeCnt; k++)
//    {
//        if(p_CtrInfo->p_MeterInfoForSingleNetList->values().at(k)->testResultList[0])
//        {
//            p_CtrInfo->successCnt[0]=p_CtrInfo->successCnt[0]+1;
//        }
//    }

//    p_CtrInfo->successRate[0]=(double)p_CtrInfo->successCnt[0]/(double)p_CtrInfo->totalNodeCnt;


//    qDeleteAll(p_RptSlaveNodeInfo_06F1_Up->SlaveNodeInfo_06F1List.begin(), p_RptSlaveNodeInfo_06F1_Up->SlaveNodeInfo_06F1List.end());
//    p_RptSlaveNodeInfo_06F1_Up->SlaveNodeInfo_06F1List.clear();
//}

//void Refresh_CtrInfo_Result_for_SearchMeter_06F4(CtrInfo *p_CtrInfo, RptSlaveNodeInfoAndDvcType_06F4_Up *p_RptSlaveNodeInfoAndDvcType_06F4_Up)
//{
//    QList<Addr> searchedAddrs;
//    QList<uchar> prtclList;

//    for(int i=0; i<p_RptSlaveNodeInfoAndDvcType_06F4_Up->slaveNodeNum; i++)
//    {
//        Addr tmpAddr;
//        memcpy(tmpAddr.addr,p_RptSlaveNodeInfoAndDvcType_06F4_Up->SlaveNodeInfo_06F4List.at(i)->nodeAddr,6);
//        searchedAddrs.append(tmpAddr);
//        prtclList.append(p_RptSlaveNodeInfoAndDvcType_06F4_Up->SlaveNodeInfo_06F4List.at(i)->protocolType);

//        for(int j=0; j<p_RptSlaveNodeInfoAndDvcType_06F4_Up->SlaveNodeInfo_06F4List.at(i)->transNumThisTime; j++)
//        {
//            Addr tmpAddr;
//            memcpy(tmpAddr.addr,p_RptSlaveNodeInfoAndDvcType_06F4_Up->SlaveNodeInfo_06F4List.at(i)->AttachedNodeInfoList.at(j)->nodeAddr,6);
//            searchedAddrs.append(tmpAddr);
//            prtclList.append(p_RptSlaveNodeInfoAndDvcType_06F4_Up->SlaveNodeInfo_06F4List.at(i)->AttachedNodeInfoList.at(j)->protocolType);
//        }
//    }
//    for(int i=0; i<searchedAddrs.size(); i++)
//    {
//        for(int j=0; j<p_CtrInfo->totalNodeCnt; j++)
//        {
//            if(true==isArrayEqual(p_CtrInfo->p_MeterInfoForSingleNetList->values().at(j)->mtrAddr, (uchar*)(searchedAddrs.at(i).addr), 6) &&
//                    p_CtrInfo->p_MeterInfoForSingleNetList->values().at(j)->prtcl==prtclList.at(i))
//            {
//                p_CtrInfo->p_MeterInfoForSingleNetList->values().at(j)->testResultList[0]=true;
//                break;
//            }
//        }
//    }

//    p_CtrInfo->successCnt[0]=0;
//    for(int k=0; k<p_CtrInfo->totalNodeCnt; k++)
//    {
//        if(p_CtrInfo->p_MeterInfoForSingleNetList->values().at(k)->testResultList[0])
//        {
//            p_CtrInfo->successCnt[0]=p_CtrInfo->successCnt[0]+1;
//        }
//    }

//    p_CtrInfo->successRate[0]=(double)p_CtrInfo->successCnt[0]/(double)p_CtrInfo->totalNodeCnt;



//    qDeleteAll(p_RptSlaveNodeInfoAndDvcType_06F4_Up->SlaveNodeInfo_06F4List.begin(), p_RptSlaveNodeInfoAndDvcType_06F4_Up->SlaveNodeInfo_06F4List.end());
//    p_RptSlaveNodeInfoAndDvcType_06F4_Up->SlaveNodeInfo_06F4List.clear();
//}
QString GenerateFailedMeterStr_Net(shared_ptr<CtrInfo> p_CtrInfo)
{
    QString result;
    if(p_CtrInfo->inNetSuccessRate<1)
    {
        result="组网失败的模块列表：\n";
        for(int i=0; i<p_CtrInfo->p_MeterInfoForSingleNetList->size(); i++)
        {
            if(p_CtrInfo->p_MeterInfoForSingleNetList->values().at(i)->inNetResult==false)
            {
                result=result+QString(QByteArray(reinterpret_cast<char*>(p_CtrInfo->p_MeterInfoForSingleNetList->values().at(i)->mtrAddr),6).toHex())+'\n';
            }
        }
    }
    return result;
}
QString GenerateFailedMeterStr_SiteLeave(shared_ptr<CtrInfo> p_CtrInfo, int cnt)
{
    ushort shiftIndex=ushort(p_CtrInfo->totalNodeCnt-cnt);
    QString result="\n\n\n\n\n\n";
    if(p_CtrInfo->inNetSuccessRate<1)
    {
        result="站点离线失败的模块列表：\n";
        for(int i=0; i<cnt; i++)
        {
            //            if(p_CtrInfo->p_MeterInfoForSingleNetList->values().at(shiftIndex+i)->inNetResult==true)
            //            {
            //                result=result+QString(QByteArray((char*)p_CtrInfo->p_MeterInfoForSingleNetList->values().at(shiftIndex+i)->mtrAddr,6).toHex())+";  ";
            //            }

            int key=p_CtrInfo->keyList.at(shiftIndex+i);
            if(!p_CtrInfo->p_MeterInfoForSingleNetList->contains(key))
                continue;
            if(p_CtrInfo->p_MeterInfoForSingleNetList->value(key)->inNetResult==true)
            {
                result=result+QString(QByteArray(reinterpret_cast<char*>(p_CtrInfo->p_MeterInfoForSingleNetList->value(key)->mtrAddr),6).toHex())+";  ";
            }
        }
        result=result+"\n\n\n\n\n\n";
    }

    return result;

}
QString GenerateFailedMeterStr_ProxyChange(shared_ptr<CtrInfo> p_CtrInfo, int cnt)
{
    QString result="\n\n\n\n\n\n";
    if(p_CtrInfo->inNetSuccessRate<1)
    {
        result="代理变更失败的模块列表：\n";
        for(int i=0; i<cnt; i++)
        {
            //            if(p_CtrInfo->p_MeterInfoForSingleNetList->values().at(i)->inNetResult==true)
            //            {
            //                result=result+QString(QByteArray((char*)p_CtrInfo->p_MeterInfoForSingleNetList->values().at(i)->mtrAddr,6).toHex())+";  ";
            //            }


            int key=p_CtrInfo->keyList.at(i);
            if(!p_CtrInfo->p_MeterInfoForSingleNetList->contains(key))
                continue;
            if(p_CtrInfo->p_MeterInfoForSingleNetList->value(key)->inNetResult==true)
            {
                result=result+QString(QByteArray(reinterpret_cast<char*>(p_CtrInfo->p_MeterInfoForSingleNetList->value(key)->mtrAddr),6).toHex())+";  ";
            }
        }
        result=result+"\n\n\n\n\n\n";
    }

    return result;
}
QString GenerateFailedMeterStr_ReadMeter(shared_ptr<CtrInfo> p_CtrInfo, int readMode)
{
    QString result="\n\n\n\n\n\n";

    if(readMode==1)
    {
        if(p_CtrInfo->successRate[1]<1)
        {
            result="点抄失败的模块列表：\n";
            for(int i=0; i<p_CtrInfo->p_MeterInfoForSingleNetList->size(); i++)
            {
                if(p_CtrInfo->p_MeterInfoForSingleNetList->values().at(i)->testResultList[1]==false)
                {
                    result=result+QString(QByteArray(reinterpret_cast<char*>(p_CtrInfo->p_MeterInfoForSingleNetList->values().at(i)->mtrAddr),6).toHex())+";  ";
                }
            }
            result=result+"\n\n\n\n\n\n";
        }
    }
    else if(readMode==2)
    {
        if(p_CtrInfo->successRate[1]<1)
        {
            result="点抄失败的模块列表：\n";
            for(int i=0; i<p_CtrInfo->p_MeterInfoForSingleNetList->size(); i++)
            {
                if(p_CtrInfo->p_MeterInfoForSingleNetList->values().at(i)->testResultList[1]==false)
                {
                    result=result+QString(QByteArray(reinterpret_cast<char*>(p_CtrInfo->p_MeterInfoForSingleNetList->values().at(i)->mtrAddr),6).toHex())+";  ";
                }
            }
            result=result+"\n\n\n\n\n\n";
        }

        if(p_CtrInfo->successRate[2]<1)
        {
            result=result+"并发失败的模块列表：\n";
            for(int i=0; i<p_CtrInfo->p_MeterInfoForSingleNetList->size(); i++)
            {
                if(p_CtrInfo->p_MeterInfoForSingleNetList->values().at(i)->testResultList[2]==false)
                {
                    result=result+QString(QByteArray(reinterpret_cast<char*>(p_CtrInfo->p_MeterInfoForSingleNetList->values().at(i)->mtrAddr),6).toHex())+";  ";
                }
            }
            result=result+"\n\n\n\n\n\n";
        }
    }
    else
    {
        if(p_CtrInfo->successRate[1]<1)
        {
            result="点抄失败的模块列表：\n";
            for(int i=0; i<p_CtrInfo->p_MeterInfoForSingleNetList->size(); i++)
            {
                if(p_CtrInfo->p_MeterInfoForSingleNetList->values().at(i)->testResultList[1]==false)
                {
                    result=result+QString(QByteArray(reinterpret_cast<char*>(p_CtrInfo->p_MeterInfoForSingleNetList->values().at(i)->mtrAddr),6).toHex())+";  ";
                }
            }
            result=result+"\n\n\n\n\n\n";
        }

        if(p_CtrInfo->successRate[2]<1)
        {
            result=result+"并发失败的模块列表：\n";
            for(int i=0; i<p_CtrInfo->p_MeterInfoForSingleNetList->size(); i++)
            {
                if(p_CtrInfo->p_MeterInfoForSingleNetList->values().at(i)->testResultList[2]==false)
                {
                    result=result+QString(QByteArray(reinterpret_cast<char*>(p_CtrInfo->p_MeterInfoForSingleNetList->values().at(i)->mtrAddr),6).toHex())+";  ";
                }
            }
            result=result+"\n\n\n\n\n\n";
        }

        if(p_CtrInfo->successRate[3]<1)
        {
            result=result+"路由主动抄表失败的模块列表：\n";
            for(int i=0; i<p_CtrInfo->p_MeterInfoForSingleNetList->size(); i++)
            {
                if(p_CtrInfo->p_MeterInfoForSingleNetList->values().at(i)->testResultList[3]==false)
                {
                    result=result+QString(QByteArray(reinterpret_cast<char*>(p_CtrInfo->p_MeterInfoForSingleNetList->values().at(i)->mtrAddr),6).toHex())+";  ";
                }
            }
            result=result+"\n\n\n\n\n\n";
        }
    }

    return result;
}
QString GenerateFailedMeterStr_ReadMeterSingleType(shared_ptr<CtrInfo> p_CtrInfo, int readMode)
{
    QString result="\n\n\n\n\n\n";

    if(readMode==1)
    {
        if(p_CtrInfo->successRate[1]<1)
        {
            result="点抄失败的模块列表：\n";
            for(int i=0; i<p_CtrInfo->p_MeterInfoForSingleNetList->size(); i++)
            {
                if(p_CtrInfo->p_MeterInfoForSingleNetList->values().at(i)->testResultList[1]==false)
                {
                    result=result+QString(QByteArray(reinterpret_cast<char*>(p_CtrInfo->p_MeterInfoForSingleNetList->values().at(i)->mtrAddr),6).toHex())+";  ";
                }
            }
            result=result+"\n\n\n\n\n\n";
        }
    }
    else if(readMode==2)
    {
        if(p_CtrInfo->successRate[2]<1)
        {
            result=result+"并发失败的模块列表：\n";
            for(int i=0; i<p_CtrInfo->p_MeterInfoForSingleNetList->size(); i++)
            {
                if(p_CtrInfo->p_MeterInfoForSingleNetList->values().at(i)->testResultList[2]==false)
                {
                    result=result+QString(QByteArray(reinterpret_cast<char*>(p_CtrInfo->p_MeterInfoForSingleNetList->values().at(i)->mtrAddr),6).toHex())+";  ";
                }
            }
            result=result+"\n\n\n\n\n\n";
        }
    }
    else if(readMode==3)
    {
        if(p_CtrInfo->successRate[3]<1)
        {
            result=result+"路由主动抄表失败的模块列表：\n";
            for(int i=0; i<p_CtrInfo->p_MeterInfoForSingleNetList->size(); i++)
            {
                if(p_CtrInfo->p_MeterInfoForSingleNetList->values().at(i)->testResultList[3]==false)
                {
                    result=result+QString(QByteArray(reinterpret_cast<char*>(p_CtrInfo->p_MeterInfoForSingleNetList->values().at(i)->mtrAddr),6).toHex())+";  ";
                }
            }
            result=result+"\n\n\n\n\n\n";
        }
    }
    else
    {
        return result;
    }

    return result;
}
QString GenerateFailedMeterStr_Others(shared_ptr<CtrInfo> p_CtrInfo)
{
    QString result;
    if(p_CtrInfo->successRate[0]<1.0)
    {
        result="查询内部版本失败的模块列表：\n";
        for(int i=0; i<p_CtrInfo->p_MeterInfoForSingleNetList->values().size(); i++)
        {
            if(p_CtrInfo->p_MeterInfoForSingleNetList->values().at(i)->testResultList[0]==false)
            {
                result+=QString(QByteArray(reinterpret_cast<char*>(p_CtrInfo->p_MeterInfoForSingleNetList->values().at(i)->mtrAddr),6).toHex())+"\n";
            }
        }
    }
    return result;
}
void comnAddAddrsInfo(shared_ptr<QList<shared_ptr<CtrInfo>>> p_CtrInfoList, const ConcentratorList *p_ConcentratorInfoList, const MeterList *p_MeterInfoList, const SchemeCfgList *p_SchemeCfgInfoList)
{
    for(int i=0; i<p_SchemeCfgInfoList->size(); i++)
    {
        for(int j=0; j<p_ConcentratorInfoList->size(); j++)
        {
            if(p_SchemeCfgInfoList->at(i).ctrID==p_ConcentratorInfoList->at(j).ctrID)
            {
                shared_ptr<CtrInfo> p_CtrInfo=make_shared<CtrInfo>();

                p_CtrInfo->ctrlID=p_ConcentratorInfoList->at(j).ctrID;
                memcpy(p_CtrInfo->ccoAddr,p_ConcentratorInfoList->at(j).ccoAddr,6);
                p_CtrInfo->slotPosition = p_ConcentratorInfoList->at(j).slotPosition;
                p_CtrInfo->dvcId = p_ConcentratorInfoList->at(j).dvcId;

                p_CtrInfo->inNetSuccessCnt=0;
                p_CtrInfo->inNetSuccessRate=0.0;
                p_CtrInfo->inNetConsume=0.0;
                p_CtrInfo->inNetResult=false;
                memset(p_CtrInfo->successCnt,0,sizeof(p_CtrInfo->successCnt));
                memsetDouble(p_CtrInfo->successRate,0,4);
                memsetDouble(p_CtrInfo->successConsume,0.0,4);
                memset(p_CtrInfo->testResult,false,4);
                memset(p_CtrInfo->res_CtrlCmd,false,20);
                memset(p_CtrInfo->sucsCnt_CtrlCmd,0,sizeof(p_CtrInfo->sucsCnt_CtrlCmd));
                memsetDouble(p_CtrInfo->sucsRate_CtrlCmd,0.0,20);
                p_CtrInfo->keyList.clear();
                p_CtrInfo->p_MeterInfoForSingleNetList = new QMap<int,MeterInfoForSingleNet*>();
                p_CtrInfo->p_MeterInfoForSingleNetList->clear();
                p_CtrInfo->buf.clear();

                p_CtrInfoList->append(p_CtrInfo);

                break;
            }
        }
    }


    for(int i=0; i<p_SchemeCfgInfoList->size(); i++)
    {
        p_CtrInfoList->at(i)->totalMeterCnt=0;
        p_CtrInfoList->at(i)->totalNodeCnt=0;

        for(int j=0; j<p_SchemeCfgInfoList->at(i).mtrIDs.size(); j++)
        {
            for(int k=0; k<p_MeterInfoList->size(); k++)
            {
                if(p_SchemeCfgInfoList->at(i).mtrIDs.at(j)==p_MeterInfoList->at(k).mtrID)
                {
                    MeterInfoForSingleNet *p_MeterInfoForSingleNet=new MeterInfoForSingleNet();

                    p_MeterInfoForSingleNet->mtrlID=p_MeterInfoList->at(k).mtrID;
                    memcpy(p_MeterInfoForSingleNet->mtrAddr,p_MeterInfoList->at(k).mtrAddr,6);
                    p_MeterInfoForSingleNet->slotPosition=p_MeterInfoList->at(k).slotPosition;
                    p_MeterInfoForSingleNet->realPhase=p_MeterInfoList->at(k).realPhase;
                    p_MeterInfoForSingleNet->phaseSeq=p_MeterInfoList->at(k).phaseSeq;
                    p_MeterInfoForSingleNet->prtcl=p_MeterInfoList->at(k).prtcl;
                    memcpy(p_MeterInfoForSingleNet->cjqAddr,p_MeterInfoList->at(k).CJQAddr,6);
                    p_MeterInfoForSingleNet->dvcId = p_MeterInfoList->at(k).dvcId;

                    p_MeterInfoForSingleNet->needAdd=true;
                    memcpy(p_MeterInfoForSingleNet->ccoAddr,p_CtrInfoList->at(i)->ccoAddr,6);
                    p_MeterInfoForSingleNet->inNetResult=false;
                    memsetDouble(p_MeterInfoForSingleNet->timeConsumList,0.0,4);
                    memsetDouble(p_MeterInfoForSingleNet->successRateList,0.0,4);
                    memset(p_MeterInfoForSingleNet->testResultList,false,4);
                    memset(p_MeterInfoForSingleNet->ctrlCmdExecList,false,20);
                    p_MeterInfoForSingleNet->buf645.clear();
                    p_MeterInfoForSingleNet->buf698.clear();

                    p_CtrInfoList->at(i)->keyList.append(p_MeterInfoForSingleNet->mtrlID);
                    p_CtrInfoList->at(i)->p_MeterInfoForSingleNetList->insert(p_MeterInfoForSingleNet->mtrlID,p_MeterInfoForSingleNet);
                    p_CtrInfoList->at(i)->totalMeterCnt+=1;
                    if(p_MeterInfoForSingleNet->needAdd)
                        p_CtrInfoList->at(i)->totalNodeCnt+=1;

                    break;
                }
            }
        }
    }

    //////////////////////
    ///////////////
    /// 默认为p_CtrInfoList->at(0)
    for(int i=0; i<p_CtrInfoList->at(0)->totalNodeCnt; i++)
    {
        if(p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(i)->slotPosition==CJQ||p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(i)->slotPosition==CJQMeter)
        {
            int cjqIndex=-1;
            for(int j=0;j<p_CtrInfoList->at(0)->cjqNodeList.size();j++)
            {
                if(memcmp(p_CtrInfoList->at(0)->cjqNodeList.at(j).cjqAddr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(i)->cjqAddr,6)==0)
                {
                    cjqIndex=j;
                }
            }
            if(cjqIndex==-1)
            {
                CjqNodeStruct cjqNode;
                memcpy(cjqNode.cjqAddr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(i)->cjqAddr,6);
                Address node;
                memcpy(node.addr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(i)->mtrAddr,6);
                cjqNode.subsidiaryNodeAddrList.append(node);
                p_CtrInfoList->at(0)->cjqNodeList.append(cjqNode);
            }
            else
            {
                Address node;
                memcpy(node.addr,p_CtrInfoList->at(0)->p_MeterInfoForSingleNetList->values().at(i)->mtrAddr,6);
                p_CtrInfoList->at(0)->cjqNodeList[cjqIndex].subsidiaryNodeAddrList.append(node);
            }
        }
    }

}
void setMeterAddrsPrtcl(shared_ptr<QList<shared_ptr<CtrInfo>>> p_CtrInfoList, uchar prtcl)
{
    if(p_CtrInfoList==nullptr)
        return;

    for(int i=0; i<p_CtrInfoList->size(); i++)
    {
        for(int j=0; j<p_CtrInfoList->at(i)->p_MeterInfoForSingleNetList->values().size(); j++)
        {
            (p_CtrInfoList->at(i)->p_MeterInfoForSingleNetList->values())[j]->prtcl=prtcl;
        }
    }
}
QString findDiNameAcordDiValue(uchar *di)
{
    uchar tmpAddr[4];
    memcpy(tmpAddr,di,4);
    //reverseAddr(tmpAddr,4);

    QString diName;
    if(isArrayEqual(const_cast<uchar*>(dlt_645_Protocol::Volt_A),tmpAddr,4))
        diName="A相电压";
    else if(isArrayEqual(const_cast<uchar*>(dlt_645_Protocol::Volt_B),tmpAddr,4))
        diName="B相电压";
    else if(isArrayEqual(const_cast<uchar*>(dlt_645_Protocol::Volt_C),tmpAddr,4))
        diName="C相电压";
    else if(isArrayEqual(const_cast<uchar*>(dlt_645_Protocol::Volt_Blck),tmpAddr,4))
        diName="电压数据块";
    else if(isArrayEqual(const_cast<uchar*>(dlt_645_Protocol::Crnt_A),tmpAddr,4))
        diName="A相电流";
    else if(isArrayEqual(const_cast<uchar*>(dlt_645_Protocol::Crnt_B),tmpAddr,4))
        diName="B相电流";
    else if(isArrayEqual(const_cast<uchar*>(dlt_645_Protocol::Crnt_C),tmpAddr,4))
        diName="C相电流";
    else if(isArrayEqual(const_cast<uchar*>(dlt_645_Protocol::Crnt_Blck),tmpAddr,4))
        diName="电流数据块";
    else if(isArrayEqual(const_cast<uchar*>(dlt_645_Protocol::Crnt_ZeroLine),tmpAddr,4))
        diName="零线电流";
    else if(isArrayEqual(const_cast<uchar*>(dlt_645_Protocol::PhaseAngle_A),tmpAddr,4))
        diName="A相相角";
    else if(isArrayEqual(const_cast<uchar*>(dlt_645_Protocol::PhaseAngle_B),tmpAddr,4))
        diName="B相相角";
    else if(isArrayEqual(const_cast<uchar*>(dlt_645_Protocol::PhaseAngle_C),tmpAddr,4))
        diName="C相相角";
    else if(isArrayEqual(const_cast<uchar*>(dlt_645_Protocol::PhaseAngle_Blck),tmpAddr,4))
        diName="相角数据块";
    else if(isArrayEqual(const_cast<uchar*>(dlt_645_Protocol::ActPower_Total),tmpAddr,4))
        diName="瞬时总有功功率";
    else if(isArrayEqual(const_cast<uchar*>(dlt_645_Protocol::ActPower_A),tmpAddr,4))
        diName="瞬时A相有功功率";
    else if(isArrayEqual(const_cast<uchar*>(dlt_645_Protocol::ActPower_B),tmpAddr,4))
        diName="瞬时B相有功功率";
    else if(isArrayEqual(const_cast<uchar*>(dlt_645_Protocol::ActPower_C),tmpAddr,4))
        diName="瞬时C相有功功率";
    else if(isArrayEqual(const_cast<uchar*>(dlt_645_Protocol::ActPower_Blck),tmpAddr,4))
        diName="瞬时有功功率数据块";
    else if(isArrayEqual(const_cast<uchar*>(dlt_645_Protocol::ReActPower_Total),tmpAddr,4))
        diName="瞬时总无功功率";
    else if(isArrayEqual(const_cast<uchar*>(dlt_645_Protocol::ReActPower_A),tmpAddr,4))
        diName="瞬时A相无功功率";
    else if(isArrayEqual(const_cast<uchar*>(dlt_645_Protocol::ReActPower_B),tmpAddr,4))
        diName="瞬时B相无功功率";
    else if(isArrayEqual(const_cast<uchar*>(dlt_645_Protocol::ReActPower_C),tmpAddr,4))
        diName="瞬时C相无功功率";
    else if(isArrayEqual(const_cast<uchar*>(dlt_645_Protocol::ReActPower_Blck),tmpAddr,4))
        diName="瞬时无功功率数据块";
    else if(isArrayEqual(const_cast<uchar*>(dlt_645_Protocol::PowerFactor_Total),tmpAddr,4))
        diName="总功率因数";
    else if(isArrayEqual(const_cast<uchar*>(dlt_645_Protocol::PowerFactor_A),tmpAddr,4))
        diName="A相功率因数";
    else if(isArrayEqual(const_cast<uchar*>(dlt_645_Protocol::PowerFactor_B),tmpAddr,4))
        diName="B相功率因数";
    else if(isArrayEqual(const_cast<uchar*>(dlt_645_Protocol::PowerFactor_C),tmpAddr,4))
        diName="C相功率因数";
    else if(isArrayEqual(const_cast<uchar*>(dlt_645_Protocol::PowerFactor_Blck),tmpAddr,4))
        diName="功率因数数据块";
    else if(isArrayEqual(const_cast<uchar*>(dlt_645_Protocol::CombActEne_Total),tmpAddr,4))
        diName="(当前)组合有功总电能";
    else if(isArrayEqual(const_cast<uchar*>(dlt_645_Protocol::CombActEne_Blck),tmpAddr,4))
        diName="(当前)组合有功电能数据块";
    else if(isArrayEqual(const_cast<uchar*>(dlt_645_Protocol::PosActEne_Total),tmpAddr,4))
        diName="(当前)正向有功总电能";
    else if(isArrayEqual(const_cast<uchar*>(dlt_645_Protocol::PosActEne_Blck),tmpAddr,4))
        diName="(当前)正向有功电能数据块";
    else if(isArrayEqual(const_cast<uchar*>(dlt_645_Protocol::NegActEne_Total),tmpAddr,4))
        diName="(当前)反向有功总电能";
    else if(isArrayEqual(const_cast<uchar*>(dlt_645_Protocol::NegActEne_Blck),tmpAddr,4))
        diName="(当前)反向有功电能数据块";
    else if(isArrayEqual(const_cast<uchar*>(dlt_645_Protocol::CombReActEne1_Total),tmpAddr,4))
        diName="(当前)组合无功1总电能";
    else if(isArrayEqual(const_cast<uchar*>(dlt_645_Protocol::CombReActEne2_Total),tmpAddr,4))
        diName="(当前)组合无功2总电能";
    else if(isArrayEqual(const_cast<uchar*>(dlt_645_Protocol::ReActEneQuadrant1_Total),tmpAddr,4))
        diName="(当前)第一象限无功总电能";
    else if(isArrayEqual(const_cast<uchar*>(dlt_645_Protocol::ReActEneQuadrant2_Total),tmpAddr,4))
        diName="(当前)第二象限无功总电能";
    else if(isArrayEqual(const_cast<uchar*>(dlt_645_Protocol::ReActEneQuadrant3_Total),tmpAddr,4))
        diName="(当前)第三象限无功总电能";
    else if(isArrayEqual(const_cast<uchar*>(dlt_645_Protocol::ReActEneQuadrant4_Total),tmpAddr,4))
        diName="(当前)第四象限无功总电能";
    else if(isArrayEqual(const_cast<uchar*>(dlt_645_Protocol::ActDemand),tmpAddr,4))
        diName="当前有功需量";
    else if(isArrayEqual(const_cast<uchar*>(dlt_645_Protocol::ReActDemand),tmpAddr,4))
        diName="当前无功需量";
    else if(isArrayEqual(const_cast<uchar*>(dlt_645_Protocol::LineFreq),tmpAddr,4))
        diName="电网频率";
    else if(isArrayEqual(const_cast<uchar*>(dlt_645_Protocol::MeterType),tmpAddr,4))
        diName="电表型号";
    else if(isArrayEqual(const_cast<uchar*>(dlt_645_Protocol::ActRptStatusWord),tmpAddr,4))
        diName="主动上报状态字";
    else if(isArrayEqual(const_cast<uchar*>(dlt_645_Protocol::MeterRunStatusWord7),tmpAddr,4))
        diName="电表运行状态字7";
    else if(isArrayEqual(const_cast<uchar*>(dlt_645_Protocol::MeterRunStatusWordBlck),tmpAddr,4))
        diName="电表运行状态字数据块";
    else if(isArrayEqual(const_cast<uchar*>(dlt_645_Protocol::DateAndWeek),tmpAddr,4))
        diName="日期及星期";
    else if(isArrayEqual(const_cast<uchar*>(dlt_645_Protocol::Time645),tmpAddr,4))
        diName="时间";
    else if(isArrayEqual(const_cast<uchar*>(dlt_645_Protocol::CapOpenTimes),tmpAddr,4))
        diName="开表盖总次数";
    else if(isArrayEqual(const_cast<uchar*>(dlt_645_Protocol::Last_DailyFre_Time),tmpAddr,4))
        diName="(上1次)日冻结时间";
    else if(isArrayEqual(const_cast<uchar*>(dlt_645_Protocol::Last_DailyFre_ActEne),tmpAddr,4))
        diName="(上1次)日冻结正向有功电能数据";
    else if(isArrayEqual(const_cast<uchar*>(dlt_645_Protocol::Last_DailyFre_NegActEne),tmpAddr,4))
        diName="(上1次)日冻结反向有功电能数据";
    else if(isArrayEqual(const_cast<uchar*>(dlt_645_Protocol::CurvePrid_Class1),tmpAddr,4))
        diName="第1类负荷记录间隔时间";
    else if(isArrayEqual(const_cast<uchar*>(dlt_645_Protocol::LastCurveBlc_Class1),tmpAddr,4))
        diName="第1类负荷最近一个记录块";
    else if(isArrayEqual(const_cast<uchar*>(dlt_645_Protocol::ComAddr),tmpAddr,4))
        diName="通信地址";

    return diName;
}
uchar get3762Dt(char dt1_,char dt2_)
{
    uchar dt=0;
    uchar dt1=uchar(dt1_);
    uchar dt2=uchar(dt2_);
    for(uchar i=1; i<=8; i++)
    {
        if(0==dt1)
        {
            dt=0;
            break;
        }
        if(1==uchar(dt1))
        {
            dt=i;
            break;
        }
        dt1=dt1>>1;
    }
    return uchar(dt2*8)+dt;
}
void delayTime(int msec)
{
    if(msec<=0)
        return;
    QEventLoop loop;//定义一个新的事件循环
    QTimer::singleShot(msec, &loop, &QEventLoop::quit);//创建单次定时器，槽函数为事件循环的退出函数
    loop.exec();//事件循环开始执行，程序会卡在这里，直到定时时间到，本循环被退出
}

QByteArray prcsOther3762Msg(uchar afn, char dt1,char dt2, uchar msgSeq)
{
    QByteArray sendMsgOct3762;
    QString crntDateTime=QDateTime::currentDateTime().toString(CURRENT_TIME_FORMAT);
    QString ctnt=QString("[%1]  ").arg(crntDateTime);

    uchar dt=get3762Dt(dt1,dt2);
    shared_ptr<qgdw_3762_protocol::Afn00F1> p_Afn00F1=make_shared<qgdw_3762_protocol::Afn00F1>();
    p_Afn00F1->ctrl_field_.dir=kDirDown;
    p_Afn00F1->ctrl_field_.prm=kActive;
    p_Afn00F1->ctrl_field_.comn_type=kHplc;

    p_Afn00F1->info_field_.info_field_down.msg_seq=char(msgSeq);
    p_Afn00F1->info_field_.info_field_down.comu_rate=0;
    p_Afn00F1->info_field_.info_field_down.comu_module_ident=0;

    memset(p_Afn00F1->data_info_,char(0xff),4);
    p_Afn00F1->data_info_[4]=0x00;
    p_Afn00F1->data_info_[5]=0x00;
    if(afn==0x06 && dt==5)
    {
        sendMsgOct3762=p_Afn00F1->EncodeFrame();
        ctnt+=QString("收到 06F5事件上报报文");
    }
    else if(afn==0x06 && dt==1)
    {
        sendMsgOct3762=p_Afn00F1->EncodeFrame();
        ctnt+=QString("收到 06F1上报从节点信息报文");
    }
    else if(afn==0x06 && dt==2)
    {
        sendMsgOct3762=p_Afn00F1->EncodeFrame();
        ctnt+=QString("收到 06F2上报抄读数据报文");
    }
    else if(afn==0x06 && dt==3)
    {
        sendMsgOct3762=p_Afn00F1->EncodeFrame();
        ctnt+=QString("收到 06F3上报路由工况变动信息");
    }
    else if(afn==0x06 && dt==4)
    {
        sendMsgOct3762=p_Afn00F1->EncodeFrame();
        ctnt+=QString("收到 06F4上报从节点信息与设备类型");
    }
    else if(afn==0x06 && dt==6)
    {
        sendMsgOct3762=p_Afn00F1->EncodeFrame();
        ctnt+=QString("收到 06F6上报事件从节点的地址");
    }
    else if(afn==0x06 && dt==5)
    {
        sendMsgOct3762=p_Afn00F1->EncodeFrame();
    }
    else if(afn==0x06 && dt==10)
    {
        sendMsgOct3762=p_Afn00F1->EncodeFrame();
    }
    else if(afn==0x03 && dt==10)
    {
        ctnt+=QString("收到 03F10本地通信模块运行模式信息");
    }
    else if(afn==0x14 && dt==1)
    {
        QStringList phaseList;
        phaseList<<"未知"<<"A相"<<"B相"<<"C相";

        shared_ptr<qgdw_3762_protocol::Afn14F1> p_Afn14F1=make_shared<qgdw_3762_protocol::Afn14F1>();

        ctnt+=QString("收到 14F1路由请求抄读内容      相位:%1; 表号:%2; 节点序号:%3").arg(p_Afn14F1->phase_<phaseList.size()?phaseList.at(p_Afn14F1->phase_):QString::number(p_Afn14F1->phase_))
                .arg(QString(QByteArray(p_Afn14F1->node_address_.addr).toHex()))
                .arg(QString::number(p_Afn14F1->node_no_));
    }
    else if(afn==0x14 && dt==2)
    {
        shared_ptr<qgdw_3762_protocol::Afn14F2> p_Afn14F2=make_shared<qgdw_3762_protocol::Afn14F2>();

        QDateTime crntDateTime=QDateTime::currentDateTime();

        QString sDate=crntDateTime.date().toString("yyMMdd");
        QString sTime=crntDateTime.time().toString("hhmmss");
        uchar dateTime[6]={0,0,0,0,0,0};

        for(int i=0; i<3; i++)
        {
            dateTime[i] = (sTime.at(5-i*2-1).cell()-'0')*16+(sTime.at(5-i*2).cell()-'0');
            dateTime[i+3] = (sDate.at(5-i*2-1).cell()-'0')*16+(sDate.at(5-i*2).cell()-'0');
        }
        memcpy(&p_Afn14F2->current_time_,dateTime,6);

        p_Afn14F2->ctrl_field_.dir=kDirDown;
        p_Afn14F2->ctrl_field_.prm=kActive;
        p_Afn14F2->ctrl_field_.comn_type=kHplc;

        p_Afn14F2->info_field_.info_field_down.msg_seq=char(msgSeq);
        p_Afn14F2->info_field_.info_field_down.comu_module_ident=0;

        sendMsgOct3762=p_Afn14F2->EncodeFrame();
        ctnt+=QString("收到 14F2路由请求集中器时钟");
    }
    else if(afn==0x14 && dt==3)
    {
        ctnt+=QString("收到 14F3路由请求依通信延时修正通信数据");
    }
    else if(afn==0x14 && dt==4)
    {
        //        uchar dataItemType14F4;
        //        uchar dataItem14F4[4];
        //        RouteRqstAcSampling_14F4_Up *p_RouteRqstAcSampling_14F4_Up=(RouteRqstAcSampling_14F4_Up*)p_MsgBase_1376_2->p_DT_3762;
        //        dataItemType14F4=p_RouteRqstAcSampling_14F4_Up->dataItemType;
        //        memcpy(dataItem14F4,p_RouteRqstAcSampling_14F4_Up->dataItem,4);
        //        ctnt+=QString("收到 14F4路由请求交采信息 数据项内容[%1] 交采数据项标识[%2]").arg(QString::number(dataItemType14F4)).arg(QString(QByteArray((const char*)dataItem14F4,4).toHex()));

        //        RouteRqstAcSampling_14F4_Down *p_RouteRqstAcSampling_14F4_Down=new RouteRqstAcSampling_14F4_Down();
        //        p_RouteRqstAcSampling_14F4_Down->dataItemType=dataItemType14F4;
        //        memcpy(p_RouteRqstAcSampling_14F4_Down->dataItem,dataItem14F4,4);
        //        QByteArray dataItemCntnt;
        //        dataItemCntnt.clear();
        //        if(dataItemType14F4==0x01)  //1:DL/T645-2007
        //        {
        //            if(true==isArrayEqual(dataItem14F4,const_cast<uchar*>(Volt_A),4))
        //                dataItemCntnt.append(0x22).append((char)0x00);
        //            else if(true==isArrayEqual(dataItem14F4,(uchar*)Volt_B,4))
        //                dataItemCntnt.append(0x22).append((char)0x00);
        //            else if(true==isArrayEqual(dataItem14F4,(uchar*)Volt_C,4))
        //                dataItemCntnt.append(0x22).append((char)0x00);
        //            else if(true==isArrayEqual(dataItem14F4,(uchar*)Volt_Blck,4))
        //                dataItemCntnt.append(0x22).append((char)0x00).append(0x22).append((char)0x00).append(0x22).append((char)0x00);
        //            else if(true==isArrayEqual(dataItem14F4,(uchar*)Crnt_A,4))
        //                dataItemCntnt.append(3,(char)0x00);
        //            else if(true==isArrayEqual(dataItem14F4,(uchar*)Crnt_B,4))
        //                dataItemCntnt.append(3,(char)0x00);
        //            else if(true==isArrayEqual(dataItem14F4,(uchar*)Crnt_C,4))
        //                dataItemCntnt.append(3,(char)0x00);
        //            else if(true==isArrayEqual(dataItem14F4,(uchar*)Crnt_Blck,4))
        //                dataItemCntnt.append(3,(char)0x00).append(3,(char)0x00).append(3,(char)0x00);
        //            else if(true==isArrayEqual(dataItem14F4,(uchar*)LineFreq,4))
        //                dataItemCntnt.append(0x50).append((char)0x00);
        //        }
        //        p_RouteRqstAcSampling_14F4_Down->dataItemCntnt.append(dataItemCntnt); //待完善

        //        p_MsgBase_1376_2->stCtrlField={DOWN_3762,FROM_ACTIVE_3762,HPLC_CARRIER_COMN};
        //        p_MsgBase_1376_2->unInfoField.stInfoFieldDown.comuModuleIdent=0;
        //        p_MsgBase_1376_2->unInfoField.stInfoFieldDown.msgSeq=msgSeq++;
        //        p_MsgBase_1376_2->ucAFN=0x14;

        //        sendMsgOct3762=p_MsgBase_1376_2->encode_3762_MsgDown(p_MsgBase_1376_2,p_RouteRqstAcSampling_14F4_Down);
        //        delete p_RouteRqstAcSampling_14F4_Down;
        //        p_RouteRqstAcSampling_14F4_Down=nullptr;
    }

    return sendMsgOct3762;
}
QByteArray prcsOther645Msg(uchar ctrlWord, uchar len, uchar *di, uchar *msgAddr, uchar *assignedAddr, shared_ptr<QList<shared_ptr<CtrInfo>>> p_CtrInfoList)
{
    uchar meterType=0;  //0单相;1三相
    if(nullptr!=p_CtrInfoList)
    {
        for(int i=0; i<p_CtrInfoList->size(); i++)
        {
            meterType=findMeterType(msgAddr,p_CtrInfoList->at(i));
            if(meterType!=0xff)
                break;
        }
    }

    QByteArray sendMsgOct645;
    QString crntDateTime=QDateTime::currentDateTime().toString(CURRENT_TIME_FORMAT);
    QString sendMsgLog;
    QString ctnt=QString("[%1]  ").arg(crntDateTime);

    if(ctrlWord==dlt_645_Protocol::READ_ADDR)
    {
        ctnt+=QString("收到 0x13通信地址请求报文，进行回复    分配的通信地址:%1").arg(QString(QByteArray(reinterpret_cast<const char*>(assignedAddr),6).toHex()));

        uchar addr[6];
        shared_ptr<dlt_645_Protocol::RspsNormal_ReadAddr_0x93>  RspsNormal_ReadAddr_0x93_ptr = make_shared<dlt_645_Protocol::RspsNormal_ReadAddr_0x93>(addr,6);
        memcpy(RspsNormal_ReadAddr_0x93_ptr->addr,assignedAddr,6);
        memcpy(RspsNormal_ReadAddr_0x93_ptr->addr_,assignedAddr,6);
        sendMsgOct645=RspsNormal_ReadAddr_0x93_ptr->EncodeFrame();
        sendMsgLog=QString("》》 读通信地址应答(0x93)：%1\n").arg(QString(sendMsgOct645.toHex()));
    }
    else if(ctrlWord==dlt_645_Protocol::BROADCAST)
    {
        ctnt+=QString("收到 0x08广播校时报文");
    }
    else if(ctrlWord==dlt_645_Protocol::READ_DATA)
    {
        if(len<4)
            return sendMsgOct645;

        double voltA=220.0;
        double voltB=220.0;
        double voltC=220.0;
        double crntA=0.000;
        double crntB=0.000;
        double crntC=0.000;
        double actPowerTotal=0.0000;
        double actPowerA=0.0000;
        double actPowerB=0.0000;
        double actPowerC=0.0000;
        double phaseAngleA=0;
        double phaseAngleB=0;
        double phaseAngleC=0;
        double capOpenTimes=0;
        double crntPosEneTotal=1;
        ushort curvePrid=15;

        QByteArray meterType_Single=QByteArray::fromStdString(QString("DDZY1710-Z").toStdString());
        QByteArray meterType_Three=QByteArray::fromStdString(QString("DTZY1710-Z").toStdString());

        QString dateStr=QDateTime::currentDateTime().toString("yy-MM-dd hh:mm:ss dddd").replace("-","").replace(":","").replace(" ","").left(6);  //日期星期
        QString dateWeekStr=QDateTime::currentDateTime().toString("yyMMddhhmmss ddd").right(1);
        if(dateWeekStr=="一")
            dateWeekStr="01";
        else if(dateWeekStr=="二")
            dateWeekStr="02";
        else if(dateWeekStr=="三")
            dateWeekStr="03";
        else if(dateWeekStr=="四")
            dateWeekStr="04";
        else if(dateWeekStr=="五")
            dateWeekStr="05";
        else if(dateWeekStr=="六")
            dateWeekStr="06";
        else if(dateWeekStr=="日")
            dateWeekStr="00";
        QByteArray tmpdateWeek=QByteArray::fromHex(dateStr.append(dateWeekStr).toStdString().data());
        QByteArray dateWeek;
        for(int i=tmpdateWeek.size()-1; i>=0; i--)
            dateWeek.append(tmpdateWeek.mid(i,1));

        QString crntDateTimeStr=QDateTime::currentDateTime().toString("yy-MM-dd hh:mm:ss dddd");
        QString timeStr=crntDateTimeStr.replace("-","").replace(":","").replace(" ","").remove(0,6).left(6);  //时间
        QByteArray timePos=QByteArray::fromHex(timeStr.toStdString().data());
        QByteArray time;
        for(int i=timePos.size()-1; i>=0; i--)
            time.append(timePos.mid(i,1));




        QByteArray status;  //主动上报状态字
        status.append(2,static_cast<char>(0xff));

        uchar addr[6];
        shared_ptr<dlt_645_Protocol::RspsNormal_ReadData_0x91>  RspsNormal_ReadData_0x91_ptr = make_shared<dlt_645_Protocol::RspsNormal_ReadData_0x91>(addr,6);
        memcpy(RspsNormal_ReadData_0x91_ptr->addr_,msgAddr,6);
        memcpy(RspsNormal_ReadData_0x91_ptr->di,di,4);

        QString diName=findDiNameAcordDiValue(di);
        if(diName=="A相电压")
        {

            uint volt_A=bcd2hex(static_cast<uint>(voltA*10));
            RspsNormal_ReadData_0x91_ptr->data.clear();
            RspsNormal_ReadData_0x91_ptr->data.append(reinterpret_cast<const char*>(&volt_A),2);
            sendMsgOct645=RspsNormal_ReadData_0x91_ptr->EncodeFrame();

            ctnt+=QString("收到 0x11正常请求数据报文，进行回复    请求抄读的DI:%1 {%2};    回复的A相电压值:%3V").arg(QString(QByteArray(reinterpret_cast<const char*>(di),4).toHex())).arg(diName).arg(QString::number(voltA));
        }
        else if(diName=="B相电压")
        {
            if(meterType==0x00)
            {
                return sendMsgOct645;
            }

            uint volt_B=bcd2hex(static_cast<uint>(voltB*10));
            RspsNormal_ReadData_0x91_ptr->data.clear();
            RspsNormal_ReadData_0x91_ptr->data.append(reinterpret_cast<const char*>(&volt_B),2);
            sendMsgOct645=RspsNormal_ReadData_0x91_ptr->EncodeFrame();

            ctnt+=QString("收到 0x11正常请求数据报文，进行回复    请求抄读的DI:%1 {%2};    回复的B相电压值:%3V").arg(QString(QByteArray(reinterpret_cast<const char*>(di),4).toHex())).arg(diName).arg(QString::number(voltB));
        }
        else if(diName=="C相电压")
        {
            if(meterType==0x00)
            {
                return sendMsgOct645;
            }

            uint volt_C=bcd2hex(static_cast<uint>(voltC*10));
            RspsNormal_ReadData_0x91_ptr->data.clear();
            RspsNormal_ReadData_0x91_ptr->data.append(reinterpret_cast<const char*>(&volt_C),2);
            sendMsgOct645=RspsNormal_ReadData_0x91_ptr->EncodeFrame();

            ctnt+=QString("收到 0x11正常请求数据报文，进行回复    请求抄读的DI:%1 {%2};    回复的C相电压值:%3V").arg(QString(QByteArray(reinterpret_cast<const char*>(di),4).toHex())).arg(diName).arg(QString::number(voltC));
        }
        else if(diName=="电压数据块")
        {
            if(meterType==0x00)
            {
                RspsNormal_ReadData_0x91_ptr->data.clear();
                uint volt_A=bcd2hex(static_cast<uint>(voltA*10));
                RspsNormal_ReadData_0x91_ptr->data.append(reinterpret_cast<const char*>(&volt_A),2);
                RspsNormal_ReadData_0x91_ptr->data.append(2,static_cast<char>(0xff));
                RspsNormal_ReadData_0x91_ptr->data.append(2,static_cast<char>(0xff));
                sendMsgOct645=RspsNormal_ReadData_0x91_ptr->EncodeFrame();

                ctnt+=QString("收到 0x11正常请求数据报文，进行回复    请求抄读的DI:%1 {%2};    回复的电压数据块的值:%3V, %4V, %5V").arg(QString(QByteArray(reinterpret_cast<const char*>(di),4).toHex())).arg(diName).arg(QString::number(voltA)).arg(QString::number(voltB)).arg(QString::number(voltC));
            }
            else if(meterType==0x01)
            {
                RspsNormal_ReadData_0x91_ptr->data.clear();
                uint volt_A=bcd2hex(static_cast<uint>(voltA*10));
                RspsNormal_ReadData_0x91_ptr->data.append(reinterpret_cast<const char*>(&volt_A),2);
                uint volt_B=bcd2hex(static_cast<uint>(voltB*10));
                RspsNormal_ReadData_0x91_ptr->data.append(reinterpret_cast<const char*>(&volt_B),2);
                uint volt_C=bcd2hex(static_cast<uint>(voltC*10));
                RspsNormal_ReadData_0x91_ptr->data.append(reinterpret_cast<const char*>(&volt_C),2);
                sendMsgOct645=RspsNormal_ReadData_0x91_ptr->EncodeFrame();

                ctnt+=QString("收到 0x11正常请求数据报文，进行回复    请求抄读的DI:%1 {%2};    回复的电压数据块的值:%3V, %4V, %5V").arg(QString(QByteArray(reinterpret_cast<const char*>(di),4).toHex())).arg(diName).arg(QString::number(voltA)).arg(QString::number(voltB)).arg(QString::number(voltC));
            }
        }
        else if(diName=="A相电流")
        {
            uint crnt_A=bcd2hex(static_cast<uint>(crntA*1000));
            RspsNormal_ReadData_0x91_ptr->data.clear();
            RspsNormal_ReadData_0x91_ptr->data.append(reinterpret_cast<const char*>(&crnt_A),3);
            sendMsgOct645=RspsNormal_ReadData_0x91_ptr->EncodeFrame();

            ctnt+=QString("收到 0x11正常请求数据报文，进行回复    请求抄读的DI:%1 {%2};    回复的A相电流值:%3A").arg(QString(QByteArray(reinterpret_cast<const char*>(di),4).toHex())).arg(diName).arg(QString::number(crntA));
        }
        else if(diName=="B相电流")
        {
            if(meterType==0x00)
            {
                return sendMsgOct645;
            }

            uint crnt_B=bcd2hex(static_cast<uint>(crntB*1000));
            RspsNormal_ReadData_0x91_ptr->data.clear();
            RspsNormal_ReadData_0x91_ptr->data.append(reinterpret_cast<const char*>(&crnt_B),3);
            sendMsgOct645=RspsNormal_ReadData_0x91_ptr->EncodeFrame();

            ctnt+=QString("收到 0x11正常请求数据报文，进行回复    请求抄读的DI:%1 {%2};    回复的B相电流值:%3A").arg(QString(QByteArray(reinterpret_cast<const char*>(di),4).toHex())).arg(diName).arg(QString::number(crntB));
        }
        else if(diName=="C相电流")
        {
            if(meterType==0x00)
            {
                return sendMsgOct645;
            }

            uint crnt_C=bcd2hex(static_cast<uint>(crntC*1000));
            RspsNormal_ReadData_0x91_ptr->data.clear();
            RspsNormal_ReadData_0x91_ptr->data.append(reinterpret_cast<const char*>(&crnt_C),3);
            sendMsgOct645=RspsNormal_ReadData_0x91_ptr->EncodeFrame();

            ctnt+=QString("收到 0x11正常请求数据报文，进行回复    请求抄读的DI:%1 {%2};    回复的C相电流值:%3A").arg(QString(QByteArray(reinterpret_cast<const char*>(di),4).toHex())).arg(diName).arg(QString::number(crntC));
        }
        else if(diName=="电流数据块")
        {
            if(meterType==0x00)
            {
                RspsNormal_ReadData_0x91_ptr->data.clear();
                uint crnt_A=bcd2hex(static_cast<uint>(crntA*1000));
                RspsNormal_ReadData_0x91_ptr->data.append(reinterpret_cast<const char*>(&crnt_A),3);
                RspsNormal_ReadData_0x91_ptr->data.append(3,static_cast<char>(0xff));
                RspsNormal_ReadData_0x91_ptr->data.append(3,static_cast<char>(0xff));
                sendMsgOct645=RspsNormal_ReadData_0x91_ptr->EncodeFrame();

                ctnt+=QString("收到 0x11正常请求数据报文，进行回复    请求抄读的DI:%1 {%2};    回复的电流数据块的值:%3A, %4A, %5A").arg(QString(QByteArray(reinterpret_cast<const char*>(di),4).toHex())).arg(diName).arg(QString::number(crntA)).arg(QString::number(crntB)).arg(QString::number(crntC));
            }
            else if(meterType==0x01)
            {
                RspsNormal_ReadData_0x91_ptr->data.clear();
                uint crnt_A=bcd2hex(static_cast<uint>(crntA*1000));
                RspsNormal_ReadData_0x91_ptr->data.append(reinterpret_cast<const char*>(&crnt_A),3);
                uint crnt_B=bcd2hex(static_cast<uint>(crntB*1000));
                RspsNormal_ReadData_0x91_ptr->data.append(reinterpret_cast<const char*>(&crnt_B),3);
                uint crnt_C=bcd2hex(static_cast<uint>(crntC*1000));
                RspsNormal_ReadData_0x91_ptr->data.append(reinterpret_cast<const char*>(&crnt_C),3);
                sendMsgOct645=RspsNormal_ReadData_0x91_ptr->EncodeFrame();

                ctnt+=QString("收到 0x11正常请求数据报文，进行回复    请求抄读的DI:%1 {%2};    回复的电流数据块的值:%3A, %4A, %5A").arg(QString(QByteArray(reinterpret_cast<const char*>(di),4).toHex())).arg(diName).arg(QString::number(crntA)).arg(QString::number(crntB)).arg(QString::number(crntC));
            }
        }
        else if(diName=="瞬时总有功功率")
        {
            uint actPower_Total=bcd2hex(static_cast<uint>(actPowerTotal*1000));
            RspsNormal_ReadData_0x91_ptr->data.clear();
            RspsNormal_ReadData_0x91_ptr->data.append(reinterpret_cast<const char*>(&actPower_Total),3);
            sendMsgOct645=RspsNormal_ReadData_0x91_ptr->EncodeFrame();

            ctnt+=QString("收到 0x11正常请求数据报文，进行回复    请求抄读的DI:%1 {%2};    回复的瞬时总有功功率值:%3kW").arg(QString(QByteArray(reinterpret_cast<const char*>(di),4).toHex())).arg(diName).arg(QString::number(actPower_Total));
        }
        else if(diName=="瞬时A相有功功率")
        {
            uint actPower_A=bcd2hex(static_cast<uint>(actPowerA*1000));
            RspsNormal_ReadData_0x91_ptr->data.clear();
            RspsNormal_ReadData_0x91_ptr->data.append(reinterpret_cast<const char*>(&actPower_A),3);
            sendMsgOct645=RspsNormal_ReadData_0x91_ptr->EncodeFrame();

            ctnt+=QString("收到 0x11正常请求数据报文，进行回复    请求抄读的DI:%1 {%2};    回复的瞬时A相有功功率值:%3kW").arg(QString(QByteArray(reinterpret_cast<const char*>(di),4).toHex())).arg(diName).arg(QString::number(actPower_A));
        }
        else if(diName=="瞬时B相有功功率")
        {
            if(meterType==0x00)
            {
                return sendMsgOct645;
            }

            uint actPower_B=bcd2hex(static_cast<uint>(actPowerB*1000));
            RspsNormal_ReadData_0x91_ptr->data.clear();
            RspsNormal_ReadData_0x91_ptr->data.append(reinterpret_cast<const char*>(&actPower_B),3);
            sendMsgOct645=RspsNormal_ReadData_0x91_ptr->EncodeFrame();

            ctnt+=QString("收到 0x11正常请求数据报文，进行回复    请求抄读的DI:%1 {%2};    回复的B相有功功率值:%3A").arg(QString(QByteArray(reinterpret_cast<const char*>(di),4).toHex())).arg(diName).arg(QString::number(actPowerB));
        }
        else if(diName=="瞬时C相有功功率")
        {
            if(meterType==0x00)
            {
                return sendMsgOct645;
            }

            uint actPower_C=bcd2hex(static_cast<uint>(actPowerC*1000));
            RspsNormal_ReadData_0x91_ptr->data.clear();
            RspsNormal_ReadData_0x91_ptr->data.append(reinterpret_cast<const char*>(&actPower_C),3);
            sendMsgOct645=RspsNormal_ReadData_0x91_ptr->EncodeFrame();

            ctnt+=QString("收到 0x11正常请求数据报文，进行回复    请求抄读的DI:%1 {%2};    回复的C相有功功率值:%3A").arg(QString(QByteArray(reinterpret_cast<const char*>(di),4).toHex())).arg(diName).arg(QString::number(actPowerC));
        }
        else if(diName=="瞬时有功功率数据块")
        {
            if(meterType==0x00)
            {
                RspsNormal_ReadData_0x91_ptr->data.clear();
                uint actPower_A=bcd2hex(static_cast<uint>(actPowerA*1000));
                RspsNormal_ReadData_0x91_ptr->data.append(reinterpret_cast<const char*>(&actPower_A),3);
                RspsNormal_ReadData_0x91_ptr->data.append(3,static_cast<char>(0xff));
                RspsNormal_ReadData_0x91_ptr->data.append(3,static_cast<char>(0xff));
                sendMsgOct645=RspsNormal_ReadData_0x91_ptr->EncodeFrame();

                ctnt+=QString("收到 0x11正常请求数据报文，进行回复    请求抄读的DI:%1 {%2};    回复的有功功率数据块的值:%3A, %4A, %5A").arg(QString(QByteArray(reinterpret_cast<const char*>(di),4).toHex())).arg(diName).arg(QString::number(actPowerA)).arg(QString::number(actPowerB)).arg(QString::number(actPowerC));
            }
            else if(meterType==0x01)
            {
                RspsNormal_ReadData_0x91_ptr->data.clear();
                uint actPower_A=bcd2hex(static_cast<uint>(actPowerA*1000));
                RspsNormal_ReadData_0x91_ptr->data.append(reinterpret_cast<const char*>(&actPower_A),3);
                uint actPower_B=bcd2hex(static_cast<uint>(actPowerB*1000));
                RspsNormal_ReadData_0x91_ptr->data.append(reinterpret_cast<const char*>(&actPower_B),3);
                uint actPower_C=bcd2hex(static_cast<uint>(actPowerC*1000));
                RspsNormal_ReadData_0x91_ptr->data.append(reinterpret_cast<const char*>(&actPower_C),3);
                sendMsgOct645=RspsNormal_ReadData_0x91_ptr->EncodeFrame();

                ctnt+=QString("收到 0x11正常请求数据报文，进行回复    请求抄读的DI:%1 {%2};    回复的有功功率数据块的值:%3A, %4A, %5A").arg(QString(QByteArray(reinterpret_cast<const char*>(di),4).toHex())).arg(diName).arg(QString::number(actPowerA)).arg(QString::number(actPowerB)).arg(QString::number(actPowerC));
            }
        }
        else if(diName=="A相相角")
        {
            uint phaseAngle_A=bcd2hex(static_cast<uint>(phaseAngleA*10));
            RspsNormal_ReadData_0x91_ptr->data.clear();
            RspsNormal_ReadData_0x91_ptr->data.append(reinterpret_cast<const char*>(&phaseAngle_A),2);
            sendMsgOct645=RspsNormal_ReadData_0x91_ptr->EncodeFrame();

            ctnt+=QString("收到 0x11正常请求数据报文，进行回复    请求抄读的DI:%1 {%2};    回复的A相相角值:%3°").arg(QString(QByteArray(reinterpret_cast<const char*>(di),4).toHex())).arg(diName).arg(QString::number(phaseAngleA));
        }
        else if(diName=="B相相角")
        {
            if(meterType==0x00)
            {
                return sendMsgOct645;
            }

            uint phaseAngle_B=bcd2hex(static_cast<uint>(phaseAngleB*10));
            RspsNormal_ReadData_0x91_ptr->data.clear();
            RspsNormal_ReadData_0x91_ptr->data.append(reinterpret_cast<const char*>(&phaseAngle_B),2);
            sendMsgOct645=RspsNormal_ReadData_0x91_ptr->EncodeFrame();

            ctnt+=QString("收到 0x11正常请求数据报文，进行回复    请求抄读的DI:%1 {%2};    回复的B相相角值:%3°").arg(QString(QByteArray(reinterpret_cast<const char*>(di),4).toHex())).arg(diName).arg(QString::number(phaseAngleB));
        }
        else if(diName=="C相相角")
        {
            if(meterType==0x00)
            {
                return sendMsgOct645;
            }

            uint phaseAngle_C=bcd2hex(static_cast<uint>(phaseAngleC*10));
            RspsNormal_ReadData_0x91_ptr->data.clear();
            RspsNormal_ReadData_0x91_ptr->data.append(reinterpret_cast<const char*>(&phaseAngle_C),2);
            sendMsgOct645=RspsNormal_ReadData_0x91_ptr->EncodeFrame();

            ctnt+=QString("收到 0x11正常请求数据报文，进行回复    请求抄读的DI:%1 {%2};    回复的C相相角值:%3°").arg(QString(QByteArray(reinterpret_cast<const char*>(di),4).toHex())).arg(diName).arg(QString::number(phaseAngleC));
        }
        else if(diName=="相角数据块")
        {
            if(meterType==0x00)
            {
                RspsNormal_ReadData_0x91_ptr->data.clear();
                uint phaseAngle_A=bcd2hex(static_cast<uint>(phaseAngleA*10));
                RspsNormal_ReadData_0x91_ptr->data.append(reinterpret_cast<const char*>(&phaseAngle_A),2);
                RspsNormal_ReadData_0x91_ptr->data.append(2,static_cast<char>(0xff));
                RspsNormal_ReadData_0x91_ptr->data.append(2,static_cast<char>(0xff));
                sendMsgOct645=RspsNormal_ReadData_0x91_ptr->EncodeFrame();

                ctnt+=QString("收到 0x11正常请求数据报文，进行回复    请求抄读的DI:%1 {%2};    回复的相角数据块的值:%3°, %4°, %5°").arg(QString(QByteArray(reinterpret_cast<const char*>(di),4).toHex())).arg(diName).arg(QString::number(phaseAngleA)).arg(QString::number(phaseAngleB)).arg(QString::number(phaseAngleC));
            }
            else if(meterType==0x01)
            {
                RspsNormal_ReadData_0x91_ptr->data.clear();
                uint phaseAngle_A=bcd2hex(static_cast<uint>(phaseAngleA*10));
                RspsNormal_ReadData_0x91_ptr->data.append(reinterpret_cast<const char*>(&phaseAngle_A),2);
                uint phaseAngle_B=bcd2hex(static_cast<uint>(phaseAngleB*10));
                RspsNormal_ReadData_0x91_ptr->data.append(reinterpret_cast<const char*>(&phaseAngle_B),2);
                uint phaseAngle_C=bcd2hex(static_cast<uint>(phaseAngleC*10));
                RspsNormal_ReadData_0x91_ptr->data.append(reinterpret_cast<const char*>(&phaseAngle_C),2);
                sendMsgOct645=RspsNormal_ReadData_0x91_ptr->EncodeFrame();

                ctnt+=QString("收到 0x11正常请求数据报文，进行回复    请求抄读的DI:%1 {%2};    回复的相角数据块的值:%3°, %4°, %5°").arg(QString(QByteArray(reinterpret_cast<const char*>(di),4).toHex())).arg(diName).arg(QString::number(phaseAngleA)).arg(QString::number(phaseAngleB)).arg(QString::number(phaseAngleC));
            }
        }
        else if(diName=="电表型号")
        {
            if(meterType==0x00)
                RspsNormal_ReadData_0x91_ptr->data=meterType_Single;
            else if(meterType==0x01)
                RspsNormal_ReadData_0x91_ptr->data=meterType_Three;

            sendMsgOct645=RspsNormal_ReadData_0x91_ptr->EncodeFrame();

            ctnt+=QString("收到 0x11正常请求数据报文，进行回复    请求抄读的DI:%1 {%2};    回复的电表型号:%3").arg(QString(QByteArray(reinterpret_cast<const char*>(di),4).toHex())).arg(diName).arg(QString(meterType));

        }
        else if(diName=="主动上报状态字")
        {
            status=QString("000000000000000000000000AAAA").toUtf8();
            RspsNormal_ReadData_0x91_ptr->data=status;
            sendMsgOct645=RspsNormal_ReadData_0x91_ptr->EncodeFrame();

            ctnt+=QString("收到 0x11正常请求数据报文，进行回复    请求抄读的DI:%1 {%2};    回复的主动上报状态字:%3").arg(QString(QByteArray(reinterpret_cast<const char*>(di),4).toHex())).arg(diName).arg(QString(status.toHex()));
        }
        else if(diName=="电表运行状态字7")
        {
            RspsNormal_ReadData_0x91_ptr->data=status;
            sendMsgOct645=RspsNormal_ReadData_0x91_ptr->EncodeFrame();

            ctnt+=QString("收到 0x11正常请求数据报文，进行回复    请求抄读的DI:%1 {%2};    回复的电表运行状态字7:%3").arg(QString(QByteArray(reinterpret_cast<const char*>(di),4).toHex())).arg(diName).arg(QString(status.toHex()));
        }
        else if(diName=="电表运行状态字数据块")
        {
            RspsNormal_ReadData_0x91_ptr->data.clear();
            for(int i=0; i<7; i++)
                RspsNormal_ReadData_0x91_ptr->data.append(status);
            sendMsgOct645=RspsNormal_ReadData_0x91_ptr->EncodeFrame();

            ctnt+=QString("收到 0x11正常请求数据报文，进行回复    请求抄读的DI:%1 {%2};    回复的电表运行状态字数据块:%3").arg(QString(QByteArray(reinterpret_cast<const char*>(di),4).toHex())).arg(diName).arg(QString(RspsNormal_ReadData_0x91_ptr->data.toHex()));
        }
        else if(diName=="日期及星期")
        {
            RspsNormal_ReadData_0x91_ptr->data=dateWeek;
            sendMsgOct645=RspsNormal_ReadData_0x91_ptr->EncodeFrame();

            ctnt+=QString("收到 0x11正常请求数据报文，进行回复    请求抄读的DI:%1 {%2};    回复的日期及星期:%3").arg(QString(QByteArray(reinterpret_cast<const char*>(di),4).toHex())).arg(diName).arg(QString(dateWeek.toHex()));
        }
        else if(diName=="时间")
        {
            RspsNormal_ReadData_0x91_ptr->data=time;
            sendMsgOct645=RspsNormal_ReadData_0x91_ptr->EncodeFrame();

            ctnt+=QString("收到 0x11正常请求数据报文，进行回复    请求抄读的DI:%1 {%2};    回复的电表时间:%3").arg(QString(QByteArray(reinterpret_cast<const char*>(di),4).toHex())).arg(diName).arg(QString(time.toHex()));
        }
        else if(diName=="开表盖总次数")
        {
            uint capOpTimes=static_cast<uint>(capOpenTimes);
            RspsNormal_ReadData_0x91_ptr->data.clear();
            RspsNormal_ReadData_0x91_ptr->data.append(reinterpret_cast<const char*>(&capOpTimes),3);
            sendMsgOct645=RspsNormal_ReadData_0x91_ptr->EncodeFrame();

            ctnt+=QString("收到 0x11正常请求数据报文，进行回复    请求抄读的DI:%1 {%2};    回复的开表盖总次数:%3次").arg(QString(QByteArray(reinterpret_cast<const char*>(di),4).toHex())).arg(diName).arg(QString::number(capOpTimes));
        }
        else if(diName=="(当前)正向有功总电能")
        {
            uint crnt_PosEneTotal=bcd2hex(static_cast<uint>(crntPosEneTotal*100));
            RspsNormal_ReadData_0x91_ptr->data.clear();
            RspsNormal_ReadData_0x91_ptr->data.append(reinterpret_cast<const char*>(&crnt_PosEneTotal),4);
            sendMsgOct645=RspsNormal_ReadData_0x91_ptr->EncodeFrame();

            ctnt+=QString("收到 0x11正常请求数据报文，进行回复    请求抄读的DI:%1 {%2};    回复的(当前)正向有功总电能:%3kWh").arg(QString(QByteArray(reinterpret_cast<const char*>(di),4).toHex())).arg(diName).arg(QString::number(crnt_PosEneTotal));
        }
        else if(diName=="(当前)反向有功总电能")
        {
            uint crnt_PosEneTotal=bcd2hex(static_cast<uint>(crntPosEneTotal*100));
            RspsNormal_ReadData_0x91_ptr->data.clear();
            RspsNormal_ReadData_0x91_ptr->data.append(reinterpret_cast<const char*>(&crnt_PosEneTotal),4);
            sendMsgOct645=RspsNormal_ReadData_0x91_ptr->EncodeFrame();

            ctnt+=QString("收到 0x11正常请求数据报文，进行回复    请求抄读的DI:%1 {%2};    回复的(当前)反向有功总电能:%3kWh").arg(QString(QByteArray(reinterpret_cast<const char*>(di),4).toHex())).arg(diName).arg(QString::number(crnt_PosEneTotal));
        }
        else if(diName=="(当前)正向有功电能数据块" || diName=="(当前)反向有功电能数据块"
                || diName=="(上1次)日冻结正向有功电能数据" || diName=="(上1次)日冻结反向有功电能数据")
        {
            uint crnt_PosEneTotal=bcd2hex(static_cast<uint>(crntPosEneTotal*100*4));
            RspsNormal_ReadData_0x91_ptr->data.clear();
            RspsNormal_ReadData_0x91_ptr->data.append(reinterpret_cast<const char*>(&crnt_PosEneTotal),4);
            QString str_info = diName + QString(":总-%1kWh").arg(QString::number(crnt_PosEneTotal));
            crnt_PosEneTotal=bcd2hex(static_cast<uint>(crntPosEneTotal*100));
            RspsNormal_ReadData_0x91_ptr->data.append(reinterpret_cast<const char*>(&crnt_PosEneTotal),4);
            str_info += QString("  费率1-%1kWh").arg(QString::number(crnt_PosEneTotal));
            RspsNormal_ReadData_0x91_ptr->data.append(reinterpret_cast<const char*>(&crnt_PosEneTotal),4);
            str_info += QString("  费率2-%1kWh").arg(QString::number(crnt_PosEneTotal));
            RspsNormal_ReadData_0x91_ptr->data.append(reinterpret_cast<const char*>(&crnt_PosEneTotal),4);
            str_info += QString("  费率3-%1kWh").arg(QString::number(crnt_PosEneTotal));
            RspsNormal_ReadData_0x91_ptr->data.append(reinterpret_cast<const char*>(&crnt_PosEneTotal),4);
            str_info += QString("  费率4-%1kWh").arg(QString::number(crnt_PosEneTotal));
            sendMsgOct645=RspsNormal_ReadData_0x91_ptr->EncodeFrame();

            ctnt+=QString("收到 0x11正常请求数据报文，进行回复    请求抄读的DI:%1 {%2};    回复的").arg(QString(QByteArray(reinterpret_cast<const char*>(di),4).toHex())).arg(diName) + str_info;
        }
        else if(diName=="(上1次)日冻结时间")
        {
            QString strLastDailyFreTime = dateStr.append("0000");
            QByteArray tmpLastDailyFreTime=QByteArray::fromHex(strLastDailyFreTime.toStdString().data());
            QByteArray LastDailyFreTime;
            for(int i=tmpLastDailyFreTime.size()-1; i>=0; i--)
                LastDailyFreTime.append(tmpLastDailyFreTime.mid(i,1));

            RspsNormal_ReadData_0x91_ptr->data=LastDailyFreTime;
            sendMsgOct645=RspsNormal_ReadData_0x91_ptr->EncodeFrame();

            ctnt+=QString("收到 0x11正常请求数据报文，进行回复    请求抄读的DI:%1 {%2};    回复的(上1次)日冻结时间:%3").arg(QString(QByteArray(reinterpret_cast<const char*>(di),4).toHex())).arg(diName).arg(QString(dateWeek.toHex()));
        }
        else if(diName=="第1类负荷记录间隔时间")
        {
            RspsNormal_ReadData_0x91_ptr->data.clear();
            RspsNormal_ReadData_0x91_ptr->data.append(reinterpret_cast<const char*>(&curvePrid),2);
            sendMsgOct645=RspsNormal_ReadData_0x91_ptr->EncodeFrame();

            ctnt+=QString("收到 0x11正常请求数据报文，进行回复    请求抄读的DI:%1 {%2};    回复的第1类负荷记录间隔时间:%3min").arg(QString(QByteArray(reinterpret_cast<const char*>(di),4).toHex())).arg(diName).arg(QString::number(curvePrid));
        }
        else if(diName=="第1类负荷最近一个记录块")
        {
            QByteArray curveData=QByteArray::fromHex(QString("A0A016000000000000000000000000000000000000000000AA00E5").toStdString().data());
            RspsNormal_ReadData_0x91_ptr->data.clear();
            RspsNormal_ReadData_0x91_ptr->data.append(curveData);
            sendMsgOct645=RspsNormal_ReadData_0x91_ptr->EncodeFrame();

            ctnt+=QString("收到 0x11正常请求数据报文，进行回复    请求抄读的DI:%1 {%2};    回复的第1类负荷最近一个记录块:%3").arg(QString(QByteArray(reinterpret_cast<const char*>(di),4).toHex())).arg(diName).arg(QString(curveData.toHex()));
        }
        else if(diName=="通信地址")
        {
            QByteArray receiveAddr=QByteArray(reinterpret_cast<char*>(msgAddr),6);
            QByteArray responseAddr=QByteArray(reinterpret_cast<char*>(assignedAddr),6);
            if(responseAddr.at(5)==receiveAddr.at(5)||receiveAddr.at(5)==char(0xAA))
            {
                memcpy(RspsNormal_ReadData_0x91_ptr->addr_,responseAddr,6);
                RspsNormal_ReadData_0x91_ptr->data.clear();
                RspsNormal_ReadData_0x91_ptr->data.append(reverseArray(responseAddr));
                sendMsgOct645=RspsNormal_ReadData_0x91_ptr->EncodeFrame();

                ctnt+=QString("收到 0x11正常请求数据报文，进行回复    请求抄读的DI:%1 {%2};    回复的通信地址:%3").arg(QString(QByteArray(reinterpret_cast<const char*>(di),4).toHex())).arg(diName).arg(QString(responseAddr.toHex()));
            }
        }
        else
        {
            ctnt+=QString("收到 0x11正常请求数据报文，未予回复    请求抄读的DI:%1 {%2}").arg(QString(QByteArray(reinterpret_cast<const char*>(di),4).toHex())).arg(diName);

            shared_ptr<dlt_645_Protocol::RspsAbNormal_ReadData_0xD1>  RspsAbNormal_ReadData_0xD1_ptr = make_shared<dlt_645_Protocol::RspsAbNormal_ReadData_0xD1>(addr,6);
            memcpy(RspsAbNormal_ReadData_0xD1_ptr->addr_,msgAddr,6);

            RspsAbNormal_ReadData_0xD1_ptr->err=0x00;
            sendMsgOct645=RspsAbNormal_ReadData_0xD1_ptr->EncodeFrame();
            sendMsgLog=QString("》》 异常应答(0xD1)：%1\n").arg(QString(sendMsgOct645.toHex()));
        }
    }
    else if(ctrlWord==dlt_645_Protocol::READ_FollowDATA)
    {
        ctnt+=QString("收到 0x12请求读后续数据报文");

        uchar addr[6];
        shared_ptr<dlt_645_Protocol::RspsAbNormal_ReadFollowData_0xD2>  RspsAbNormal_ReadFollowData_0xD2_ptr = make_shared<dlt_645_Protocol::RspsAbNormal_ReadFollowData_0xD2>(addr,6);
        memcpy(RspsAbNormal_ReadFollowData_0xD2_ptr->addr_,msgAddr,6);

        RspsAbNormal_ReadFollowData_0xD2_ptr->err=0x00;
        sendMsgOct645=RspsAbNormal_ReadFollowData_0xD2_ptr->EncodeFrame();

        sendMsgLog=QString("》》 异常应答(0xD2)：%1\n").arg(QString(sendMsgOct645.toHex()));
    }
    else if(ctrlWord==dlt_645_Protocol::WRITE_DATA)
    {
        ctnt+=QString("收到 0x14设置数据报文");

        uchar addr[6];
        shared_ptr<dlt_645_Protocol::RspsAbNormal_WriteData_0xD4>  RspsAbNormal_WriteData_0xD4_ptr = make_shared<dlt_645_Protocol::RspsAbNormal_WriteData_0xD4>(addr,6);
        memcpy(RspsAbNormal_WriteData_0xD4_ptr->addr_,msgAddr,6);

        RspsAbNormal_WriteData_0xD4_ptr->err=0x00;
        sendMsgOct645=RspsAbNormal_WriteData_0xD4_ptr->EncodeFrame();

        sendMsgLog=QString("》》 异常应答(0xD4)：%1\n").arg(QString(sendMsgOct645.toHex()));
    }
    else if(ctrlWord==dlt_645_Protocol::Froze_Cmd)
    {
        ctnt+=QString("收到 0x16冻结命令报文");

        uchar addr[6];
        shared_ptr<dlt_645_Protocol::RspsAbNormal_FrozeCmd_0xD6>  RspsAbNormal_FrozeCmd_0xD6_ptr = make_shared<dlt_645_Protocol::RspsAbNormal_FrozeCmd_0xD6>(addr,6);
        memcpy(RspsAbNormal_FrozeCmd_0xD6_ptr->addr_,msgAddr,6);

        RspsAbNormal_FrozeCmd_0xD6_ptr->err=0x00;
        sendMsgOct645=RspsAbNormal_FrozeCmd_0xD6_ptr->EncodeFrame();

        sendMsgLog=QString("》》 异常应答(0xD6)：%1\n").arg(QString(sendMsgOct645.toHex()));
    }
    else if(ctrlWord==dlt_645_Protocol::TRIP_ALARM_KEEP)
    {
        ctnt+=QString("收到 0x1C拉闸报文");

        uchar addr[6];
        shared_ptr<dlt_645_Protocol::RspsNormal_TripAlarmKeep_0x9C>  RspsNormal_TripAlarmKeep_0x9C_ptr = make_shared<dlt_645_Protocol::RspsNormal_TripAlarmKeep_0x9C>(addr,6);
        memcpy(RspsNormal_TripAlarmKeep_0x9C_ptr->addr_,msgAddr,6);
        sendMsgOct645=RspsNormal_TripAlarmKeep_0x9C_ptr->EncodeFrame();

        sendMsgLog=QString("》》 正常应答(0x9C)：%1\n").arg(QString(sendMsgOct645.toHex()));
    }
    else
    {
        ctnt+=QString("收到 其余报文    控制码:%1").arg(ctrlWord);
    }
    return sendMsgOct645;
}

QByteArray prcsOther698Msg(shared_ptr<FrameOOPBase> p_FrameOOPBase, uchar *assignedAddr, shared_ptr<QList<shared_ptr<CtrInfo>>> p_CtrInfoList)
{
    uchar meterType=0;  //0单相;1三相
    if(nullptr!=p_CtrInfoList)
    {
        for(int i=0; i<p_CtrInfoList->size(); i++)
        {
            meterType=findMeterType(assignedAddr,p_CtrInfoList->at(i));
            if(meterType!=0xff)
                break;
        }
    }

    QByteArray sendMsgOct698;

    QList<double> voltList;
    voltList.clear();
    //    QList<double> crntList;
    //    crntList.clear();
    //    QList<double> actPowerList;
    //    actPowerList.clear();
    //    QList<double> posEneList;
    //    posEneList.clear();

    if(p_FrameOOPBase->service_type_==GET_REQUEST_CLIENT && p_FrameOOPBase->service_sub_type_==uchar(GetRequestType::kGetRequestNormal))
    {
        shared_ptr<GetRequestNormal> p_GetRequestNormal = dynamic_pointer_cast<GetRequestNormal>(p_FrameOOPBase);
        if(p_GetRequestNormal->oad_.OI==ComuAddr)
        {
            shared_ptr<GetResponseNormal> p_GetResponseNormal_ReadAddr=make_shared<GetResponseNormal>();
            p_GetResponseNormal_ReadAddr->ctrl_field_.dir = 1;
            p_GetResponseNormal_ReadAddr->ctrl_field_.prm = 0;
            p_GetResponseNormal_ReadAddr->ctrl_field_.fra = 0;
            p_GetResponseNormal_ReadAddr->ctrl_field_.res = 0;
            p_GetResponseNormal_ReadAddr->ctrl_field_.sc = 0;
            p_GetResponseNormal_ReadAddr->ctrl_field_.func = 1;

            p_GetResponseNormal_ReadAddr->address_field_.sa.addr_type = 0;
            p_GetResponseNormal_ReadAddr->address_field_.sa.logic_addr = 0;
            p_GetResponseNormal_ReadAddr->address_field_.sa.addr_len = 5;
            p_GetResponseNormal_ReadAddr->address_field_.sa.address = QString((QByteArray(reinterpret_cast<char*>(assignedAddr),6).toHex()));
            p_GetResponseNormal_ReadAddr->address_field_.ca.address = 0x00;

            p_GetResponseNormal_ReadAddr->piid_acd_.serve_priority = 0;
            p_GetResponseNormal_ReadAddr->piid_acd_.request_acd = 0;
            p_GetResponseNormal_ReadAddr->piid_acd_.serve_seq = 1;

            p_GetResponseNormal_ReadAddr->a_result_normal_.oad.OI = 0x4001;
            p_GetResponseNormal_ReadAddr->a_result_normal_.oad.attribute.feature = 0;
            p_GetResponseNormal_ReadAddr->a_result_normal_.oad.attribute.seq = 2;
            p_GetResponseNormal_ReadAddr->a_result_normal_.oad.element_index = 0;

            p_GetResponseNormal_ReadAddr->a_result_normal_.get_result_ptr = std::make_shared<GetResultData>();
            std::dynamic_pointer_cast<GetResultData>(p_GetResponseNormal_ReadAddr->a_result_normal_.get_result_ptr)->value_ptr_ = std::make_shared<DataString>();
            std::dynamic_pointer_cast<DataString>(std::dynamic_pointer_cast<GetResultData>(p_GetResponseNormal_ReadAddr->a_result_normal_.get_result_ptr)->value_ptr_)->type_ = DataType::kOctet_string;
            std::dynamic_pointer_cast<DataString>(std::dynamic_pointer_cast<GetResultData>(p_GetResponseNormal_ReadAddr->a_result_normal_.get_result_ptr)->value_ptr_)->data_ = QByteArray(reinterpret_cast<char*>(assignedAddr),6).toHex();

            p_GetResponseNormal_ReadAddr->follow_report_field_.optional_ = 0;
            p_GetResponseNormal_ReadAddr->time_tag_field_.optional_ = 0;

            sendMsgOct698=p_GetResponseNormal_ReadAddr->EncodeFrame();
        }
        else if(p_GetRequestNormal->oad_.OI==DateTime_OI)
        {
            shared_ptr<GetResponseNormal> p_GetResponseNormal_DateTime=make_shared<GetResponseNormal>();
            p_GetResponseNormal_DateTime->ctrl_field_.dir = 1;
            p_GetResponseNormal_DateTime->ctrl_field_.prm = 1;
            p_GetResponseNormal_DateTime->ctrl_field_.fra = 0;
            p_GetResponseNormal_DateTime->ctrl_field_.res = 0;
            p_GetResponseNormal_DateTime->ctrl_field_.sc = 0;
            p_GetResponseNormal_DateTime->ctrl_field_.func = 3;

            p_GetResponseNormal_DateTime->address_field_.sa.addr_type = 0;
            p_GetResponseNormal_DateTime->address_field_.sa.logic_addr = 0;
            p_GetResponseNormal_DateTime->address_field_.sa.addr_len = 5;
            p_GetResponseNormal_DateTime->address_field_.sa.address = p_GetRequestNormal->address_field_.sa.address;
            p_GetResponseNormal_DateTime->address_field_.ca.address = 0x10;

            p_GetResponseNormal_DateTime->piid_acd_.serve_priority = 0;
            p_GetResponseNormal_DateTime->piid_acd_.request_acd = 0;
            p_GetResponseNormal_DateTime->piid_acd_.serve_seq = 1;

            p_GetResponseNormal_DateTime->a_result_normal_.oad.OI = DateTime_OI;
            p_GetResponseNormal_DateTime->a_result_normal_.oad.attribute.feature = 0;
            p_GetResponseNormal_DateTime->a_result_normal_.oad.attribute.seq = 2;
            p_GetResponseNormal_DateTime->a_result_normal_.oad.element_index = 0;

            p_GetResponseNormal_DateTime->a_result_normal_.get_result_ptr = std::make_shared<GetResultData>();
            std::dynamic_pointer_cast<GetResultData>(p_GetResponseNormal_DateTime->a_result_normal_.get_result_ptr)->value_ptr_ = std::make_shared<DataBasic>();
            std::dynamic_pointer_cast<DataBasic>(std::dynamic_pointer_cast<GetResultData>(p_GetResponseNormal_DateTime->a_result_normal_.get_result_ptr)->value_ptr_)->type_ = DataType::kDate_time_s;
            QString current_date_time = QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
            QString date_time_hex = QString("%1").arg(current_date_time.left(4).toInt(),4,16,QChar('0'));
            for(int i=4;i<14;i+=2)
            {
                date_time_hex += QString("%1").arg(current_date_time.mid(i,2).toInt(),2,16,QChar('0'));
            }
            QByteArray date_time=QByteArray::fromHex(date_time_hex.toStdString().data());
            std::dynamic_pointer_cast<DataBasic>(std::dynamic_pointer_cast<GetResultData>(p_GetResponseNormal_DateTime->a_result_normal_.get_result_ptr)->value_ptr_)->data_ = date_time;

            p_GetResponseNormal_DateTime->follow_report_field_.optional_ = 0;
            p_GetResponseNormal_DateTime->time_tag_field_.optional_ = 0;

            sendMsgOct698=p_GetResponseNormal_DateTime->EncodeFrame();
        }
        else if(p_GetRequestNormal->oad_.OI==PosActEne_OI)
        {
            shared_ptr<GetResponseNormal> p_GetResponseNormal_PosActEne=make_shared<GetResponseNormal>();
            p_GetResponseNormal_PosActEne->ctrl_field_.dir = 1;
            p_GetResponseNormal_PosActEne->ctrl_field_.prm = 1;
            p_GetResponseNormal_PosActEne->ctrl_field_.fra = 0;
            p_GetResponseNormal_PosActEne->ctrl_field_.res = 0;
            p_GetResponseNormal_PosActEne->ctrl_field_.sc = 0;
            p_GetResponseNormal_PosActEne->ctrl_field_.func = 3;

            p_GetResponseNormal_PosActEne->address_field_.sa.addr_type = 0;
            p_GetResponseNormal_PosActEne->address_field_.sa.logic_addr = 0;
            p_GetResponseNormal_PosActEne->address_field_.sa.addr_len = 5;
            p_GetResponseNormal_PosActEne->address_field_.sa.address = p_GetRequestNormal->address_field_.sa.address;   //QString((QByteArray(reinterpret_cast<char*>(assignedAddr),6).toHex()));
            p_GetResponseNormal_PosActEne->address_field_.ca.address = 0x10;

            p_GetResponseNormal_PosActEne->piid_acd_.serve_priority = 0;
            p_GetResponseNormal_PosActEne->piid_acd_.request_acd = 0;
            p_GetResponseNormal_PosActEne->piid_acd_.serve_seq = 1;

            p_GetResponseNormal_PosActEne->a_result_normal_.oad.OI = PosActEne_OI;
            p_GetResponseNormal_PosActEne->a_result_normal_.oad.attribute.feature = 0;
            p_GetResponseNormal_PosActEne->a_result_normal_.oad.attribute.seq = 2;
            p_GetResponseNormal_PosActEne->a_result_normal_.oad.element_index = 0;

            p_GetResponseNormal_PosActEne->a_result_normal_.get_result_ptr = std::make_shared<GetResultData>();
            std::dynamic_pointer_cast<GetResultData>(p_GetResponseNormal_PosActEne->a_result_normal_.get_result_ptr)->value_ptr_ = std::make_shared<DataList>();
            std::dynamic_pointer_cast<DataList>(std::dynamic_pointer_cast<GetResultData>(p_GetResponseNormal_PosActEne->a_result_normal_.get_result_ptr)->value_ptr_)->type_ = DataType::kArray;
            for(int i=0;i<5;i++)
            {
                shared_ptr<object_oriented_electic_data_exchange_protocol::DataBasic> eneData=std::make_shared<DataBasic>();
                eneData->type_=static_cast<DataType>(0x06);
                if(i==0||i==2)
                {
                    eneData->data_=QByteArray::fromHex("00 00 00 08");
                }
                else
                {
                    eneData->data_=QByteArray::fromHex("00 00 00 00");
                }
                std::dynamic_pointer_cast<DataList>(std::dynamic_pointer_cast<GetResultData>(p_GetResponseNormal_PosActEne->a_result_normal_.get_result_ptr)->value_ptr_)->list_data_member_.append(eneData);
            }

            p_GetResponseNormal_PosActEne->follow_report_field_.optional_ = 0;
            p_GetResponseNormal_PosActEne->time_tag_field_.optional_ = 0;

            sendMsgOct698=p_GetResponseNormal_PosActEne->EncodeFrame();
        }
        else if(p_GetRequestNormal->oad_.OI==Volt_OI)
        {

        }
        else if(p_GetRequestNormal->oad_.OI==Crnt_OI)
        {

        }
        else if(p_GetRequestNormal->oad_.OI==ActPower_OI)
        {

        }
        else if(p_GetRequestNormal->oad_.OI==ReActPower_OI)
        {

        }
        else if(p_GetRequestNormal->oad_.OI==NegActEne_OI)
        {
        }
    }
    else if(p_FrameOOPBase->service_type_==GET_REQUEST_CLIENT && p_FrameOOPBase->service_sub_type_==uchar(GetRequestType::kGetRequestNormalList))
    {
        shared_ptr<GetRequestNormalList> p_GetRequestNormalList = dynamic_pointer_cast<GetRequestNormalList>(p_FrameOOPBase);
        shared_ptr<GetResponseNormalList> p_GetResponseNormalList=make_shared<GetResponseNormalList>();
        //        OAD oad;
        //        oad.OI=PosActEne_OI;
        //        oad.attribute.feature=0;
        //        oad.attribute.seq=2;
        //        oad.element_index=1;

        p_GetResponseNormalList->ctrl_field_.dir = 1;
        p_GetResponseNormalList->ctrl_field_.prm = 1;
        p_GetResponseNormalList->ctrl_field_.fra = 0;
        p_GetResponseNormalList->ctrl_field_.res = 0;
        p_GetResponseNormalList->ctrl_field_.sc = 0;
        p_GetResponseNormalList->ctrl_field_.func = 3;

        p_GetResponseNormalList->address_field_.sa.addr_type = 0;
        p_GetResponseNormalList->address_field_.sa.logic_addr = 0;
        p_GetResponseNormalList->address_field_.sa.addr_len = 5;
        p_GetResponseNormalList->address_field_.sa.address = p_GetRequestNormalList->address_field_.sa.address;   //QString((QByteArray(reinterpret_cast<char*>(assignedAddr),6).toHex()));
        p_GetResponseNormalList->address_field_.ca.address = 0x10;

        p_GetResponseNormalList->piid_acd_.serve_priority = 0;
        p_GetResponseNormalList->piid_acd_.request_acd = 0;
        p_GetResponseNormalList->piid_acd_.serve_seq = 1;


        AResultNormal a_result_normal_;
        a_result_normal_.oad.OI = PosActEne_OI;
        a_result_normal_.oad.attribute.feature = 0;
        a_result_normal_.oad.attribute.seq = 2;
        a_result_normal_.oad.element_index = 1;

        a_result_normal_.get_result_ptr = std::make_shared<GetResultData>();
        std::dynamic_pointer_cast<GetResultData>(a_result_normal_.get_result_ptr)->value_ptr_ = std::make_shared<DataBasic>();
        std::dynamic_pointer_cast<DataBasic>(std::dynamic_pointer_cast<GetResultData>(a_result_normal_.get_result_ptr)->value_ptr_)->type_ = DataType::kDouble_long_unsigned;
        //        shared_ptr<object_oriented_electic_data_exchange_protocol::DataBasic> eneData=std::make_shared<DataBasic>();
        //        eneData->type_=static_cast<DataType>(0x06);
        //        eneData->data_=QByteArray::fromHex("00 00 00 00");
        std::dynamic_pointer_cast<DataBasic>(std::dynamic_pointer_cast<GetResultData>(a_result_normal_.get_result_ptr)->value_ptr_)->data_=QByteArray::fromHex("00 00 00 00");

        for(int i=0;i<5;i++)p_GetResponseNormalList->list_result_normal_.append(a_result_normal_);

        p_GetResponseNormalList->follow_report_field_.optional_ = 0;
        p_GetResponseNormalList->time_tag_field_.optional_ = 0;

        sendMsgOct698=p_GetResponseNormalList->EncodeFrame();

    }

    else if(p_FrameOOPBase->service_type_==ACTION_REQUEST_CLIENT && p_FrameOOPBase->service_sub_type_==uchar(ActionRequestType::kActionRequest))
    {
        //        IC_ActionRequest *p_IC_ActionRequest=(IC_ActionRequest*)p_MsgBase_698_45->p_ServiceType_698_45;

        //        if(p_IC_ActionRequest->omd.OI==TripCtrl_OI)
        //        {
        //            OMD omd;
        //            omd.OI=TripCtrl_OI;
        //            omd.methodIndex=p_IC_ActionRequest->omd.methodIndex;
        //            omd.operaMode=p_IC_ActionRequest->omd.operaMode;

        //            QByteArray dataCntnt;
        //            sendMsgOct698=MsgBase_698_45::encode_ActionResSimpleOBIS_OOP(p_MsgBase_698_45->v_AddrField_698_45.v_AddrField_SA.SA,omd,false,0x00,dataCntnt);
        //        }
    }
    else
    {
        sendMsgOct698.append(char(0x00));
        return sendMsgOct698;
    }

    return sendMsgOct698;
}



uchar findCommPrtclType(uchar *addr,CtrInfo *p_CtrInfo)
{
    uchar prtclType=0x02;
    for(int i=0; i<p_CtrInfo->p_MeterInfoForSingleNetList->values().size(); i++)
    {
        if(isArrayEqual(static_cast<uchar*>(p_CtrInfo->p_MeterInfoForSingleNetList->values().at(i)->mtrAddr),addr,6))
        {
            prtclType=p_CtrInfo->p_MeterInfoForSingleNetList->values().at(i)->prtcl;
            if(prtclType==0x00)
                prtclType=0x03;
            break;
        }
    }

    return prtclType;
}
QString calculateConsumeLen(QString start, QString end)
{
    QDateTime startTime=QDateTime::fromString(start,"yyyy-MM-dd hh:mm:ss zzz");
    QDateTime endTime=QDateTime::fromString(end,"yyyy-MM-dd hh:mm:ss zzz");
    double rmCsmTimeLen=(endTime.toMSecsSinceEpoch()-startTime.toMSecsSinceEpoch())/1000.0;

    QString consumLen=QString::number(rmCsmTimeLen,'g',3);
    uchar integerNum=0;
    uchar decimalNum=0;
    int i=0;
    for(i=0; i<consumLen.size(); i++)
    {
        if(consumLen.at(i)==".")
        {
            integerNum=uchar(i);
            decimalNum=uchar(consumLen.size()-integerNum-1);
            break;
        }
    }
    if(i==consumLen.size())
    {
        integerNum=uchar(i);
        decimalNum=0;
    }
    for(int j=0; j<3-decimalNum; j++)
        consumLen.append("0");
    for(int j=0; j<2-integerNum; j++)
        consumLen.insert(0," ");
    if(decimalNum==0)
        consumLen.insert(consumLen.size()-3,".");

    return consumLen;
}
ParallelReadMeter findDstMeterAndRemoveIt(QList<ParallelReadMeter> *parallelReadMeterList, uchar dstAddr[], bool *find)
{
    *find=false;
    ParallelReadMeter tmpParallelReadMeter;
    int tmpCnt=parallelReadMeterList->size();

    for(int i=0; i<tmpCnt; i++)
    {
        if(isArrayEqual(const_cast<uchar*>(parallelReadMeterList->at(i).addr),dstAddr,6))
        {
            *find=true;
            tmpParallelReadMeter=parallelReadMeterList->at(i);
            parallelReadMeterList->removeAt(i);
            break;
        }
    }

    return tmpParallelReadMeter;
}

uchar findIndexOfAddr(uchar *addr, CtrInfo *p_CtrInfo)
{
    uchar index=0;
    for(int i=0; i<p_CtrInfo->p_MeterInfoForSingleNetList->values().size(); i++)
    {
        if(isArrayEqual(static_cast<uchar*>(p_CtrInfo->p_MeterInfoForSingleNetList->values().at(i)->mtrAddr),addr,6))
        {
            index=uchar(i);
            break;
        }
    }

    return index;
}
//拉路由复位管脚
void pullPinReset_CCO(shared_ptr<QList<shared_ptr<CtrInfo>>> p_CtrInfoList, AbstractScriptHost *p_AbstractScriptHost)
{
    if(p_AbstractScriptHost==nullptr)
        return;

    if(p_CtrInfoList->size() == 0)
        return;
    QList<double> sendParams;
    sendParams.clear();
    QList<int> idList;
    idList.clear();
    idList = findDvcIdList(p_CtrInfoList,CCO_GW);
    p_AbstractScriptHost->controlDvc(CCO_GW,idList,CtrlCmd_ModuleRST,sendParams);//选择CtrlCmd_ModuleRST为拉复位功能
    QThread::msleep(50);
}
void pullPinReset_STA(shared_ptr<QList<shared_ptr<CtrInfo>>> p_CtrInfoList, AbstractScriptHost *p_AbstractScriptHost)
{
    if(p_AbstractScriptHost==nullptr)
        return;

    if(p_CtrInfoList->size() == 0)
        return;
    QList<double> sendParams;
    sendParams.clear();
    QList<int> idList;
    idList.clear();
    idList = findDvcIdList(p_CtrInfoList,SingleSTA);
    p_AbstractScriptHost->controlDvc(SingleSTA,idList,CtrlCmd_ModuleRST,sendParams);
    QThread::msleep(50);
}
void powerOff220V_CJQ(shared_ptr<QList<shared_ptr<CtrInfo>>> p_CtrInfoList, AbstractScriptHost *p_AbstractScriptHost)
{
    if(p_AbstractScriptHost==nullptr)
        return;

    if(p_CtrInfoList->size() == 0)
        return;
    QList<double> sendParams;
    sendParams.clear();
    QList<int> idList;
    idList.clear();
    idList = findDvcIdList(p_CtrInfoList,CJQ);
    p_AbstractScriptHost->controlDvc(CJQ,idList,CtrlCmd_PowerOff_220V,sendParams);
    QThread::msleep(50);
}
void powerOn220V_CJQ(shared_ptr<QList<shared_ptr<CtrInfo>>> p_CtrInfoList, AbstractScriptHost *p_AbstractScriptHost)
{
    if(p_AbstractScriptHost==nullptr)
        return;

    if(p_CtrInfoList->size() == 0)
        return;
    QList<double> sendParams;
    sendParams.clear();
    QList<int> idList;
    idList.clear();
    idList = findDvcIdList(p_CtrInfoList,CJQ);
    p_AbstractScriptHost->controlDvc(CJQ,idList,CtrlCmd_PowerOn_220V,sendParams);
    QThread::msleep(50);
}
void powerOff12V_STA(shared_ptr<QList<shared_ptr<CtrInfo>>> p_CtrInfoList, AbstractScriptHost *p_AbstractScriptHost)
{
    if(p_AbstractScriptHost==nullptr)
        return;

    if(p_CtrInfoList->size() == 0)
        return;
    QList<double> sendParams;
    sendParams.clear();
    QList<int> idList;
    idList.clear();
    idList = findDvcIdList(p_CtrInfoList,SingleSTA);
    p_AbstractScriptHost->controlDvc(SingleSTA,idList,CtrlCmd_PowerOff_12V,sendParams);
    QThread::msleep(50);
    idList.clear();
    idList = findDvcIdList(p_CtrInfoList,ThreeSTA);
    p_AbstractScriptHost->controlDvc(ThreeSTA,idList,CtrlCmd_PowerOff_12V,sendParams);
    QThread::msleep(50);
}

void powerOn12V_STA(shared_ptr<QList<shared_ptr<CtrInfo>>> p_CtrInfoList, AbstractScriptHost *p_AbstractScriptHost)
{
    if(p_AbstractScriptHost==nullptr)
        return;

    if(p_CtrInfoList->size() == 0)
        return;
    QList<double> sendParams;
    sendParams.clear();
    QList<int> idList;
    idList.clear();
    idList = findDvcIdList(p_CtrInfoList,SingleSTA);
    p_AbstractScriptHost->controlDvc(SingleSTA,idList,CtrlCmd_PowerOn_12V,sendParams);
    QThread::msleep(50);
    idList.clear();
    idList = findDvcIdList(p_CtrInfoList,ThreeSTA);
    p_AbstractScriptHost->controlDvc(ThreeSTA,idList,CtrlCmd_PowerOn_12V,sendParams);
    QThread::msleep(50);
}
//不执行
void powerOffAll12V(shared_ptr<QList<shared_ptr<CtrInfo>>> p_CtrInfoList, AbstractScriptHost *p_AbstractScriptHost, bool isRealPowerOff)
{
    QSettings property(QString("ScriptConfig.ini"),QSettings::IniFormat);
    property.setValue("TEST_DEVICE_PROPERTY/PowerState",!isRealPowerOff);
    if(!isRealPowerOff)//为了使脚本结束不断电
        return;
    if(p_AbstractScriptHost==nullptr)
        return;

    if(p_CtrInfoList->size() == 0)
        return;

    QList<double> sendParams;
    QList<int> idList;

    idList.clear();
    idList = findDvcIdList(p_CtrInfoList,CCO_NW);
    sendParams.clear();
    sendParams.append(9600);
    p_AbstractScriptHost->controlDvc(CCO_NW,idList,CtrlCmd_SetBaudRate,sendParams);
    QThread::msleep(50);

    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "用例结束，切断工装所有CCO的12V供电");
    sendParams.clear();
    idList.clear();
    idList = findDvcIdList(p_CtrInfoList,CCO_NW);
    p_AbstractScriptHost->controlDvc(CCO_NW,idList,CtrlCmd_PowerOff_12V,sendParams);
    QThread::msleep(50);

    idList.clear();
    idList = findDvcIdList(p_CtrInfoList,CCO_GW);
    p_AbstractScriptHost->controlDvc(CCO_GW,idList,CtrlCmd_PowerOff_12V,sendParams);
    QThread::msleep(50);


    p_AbstractScriptHost->updateProgress(ProcessState_Processing, "用例结束，切断工装所有STA的12V供电");
    idList.clear();
    idList = findDvcIdList(p_CtrInfoList,SingleSTA);
    p_AbstractScriptHost->controlDvc(SingleSTA,idList,CtrlCmd_PowerOff_12V,sendParams);
    QThread::msleep(50);

    idList.clear();
    idList = findDvcIdList(p_CtrInfoList,ThreeSTA);
    p_AbstractScriptHost->controlDvc(ThreeSTA,idList,CtrlCmd_PowerOff_12V,sendParams);
    QThread::msleep(50);

    idList.clear();
    idList = findDvcIdList(p_CtrInfoList,CJQ);
    p_AbstractScriptHost->controlDvc(CJQ,idList,CtrlCmd_PowerOff_220V,sendParams);
    QThread::msleep(50);
    //////添加电表上电
    //powerOn220V_CJQ(p_CtrInfoList, p_AbstractScriptHost);
}
QList<int> findDvcIdList(shared_ptr<QList<shared_ptr<CtrInfo> > > p_CtrInfoList, DvcType dvcType)
{
    QList<int> dvcIdList;
    dvcIdList.clear();
    switch (dvcType)
    {
    case SingleSTA:
    {
        QMap<int,MeterInfoForSingleNet*>::const_iterator const_it;
        for (int i=0;i<p_CtrInfoList->size();i++)
        {
            for (const_it = (p_CtrInfoList->at(i)->p_MeterInfoForSingleNetList)->constBegin(); const_it != (p_CtrInfoList->at(i)->p_MeterInfoForSingleNetList)->constEnd(); ++const_it)
            {
                if(const_it.value()->slotPosition == SingleSTA)
                {
                    dvcIdList.append(const_it.value()->dvcId);
                }
            }
        }

        break;
    }
    case ThreeSTA:
    {
        for (int i=0;i<p_CtrInfoList->size();i++)
        {
            QMap<int,MeterInfoForSingleNet*>::const_iterator const_it;
            for (const_it = (p_CtrInfoList->at(i)->p_MeterInfoForSingleNetList)->constBegin(); const_it != (p_CtrInfoList->at(i)->p_MeterInfoForSingleNetList)->constEnd(); ++const_it)
            {
                if(const_it.value()->slotPosition == ThreeSTA)
                {
                    dvcIdList.append(const_it.value()->dvcId);
                }
            }
        }

        break;
    }
    case CCO_GW:
    {
        for (int i=0;i<p_CtrInfoList->size();i++)
        {
            if(p_CtrInfoList->at(i)->slotPosition == CCO_GW)
            {
                dvcIdList.append(p_CtrInfoList->at(i)->dvcId);
            }
        }
        break;
    }
    case CCO_NW:
    {
        for (int i=0;i<p_CtrInfoList->size();i++)
        {
            if(p_CtrInfoList->at(i)->slotPosition == CCO_NW)
            {
                dvcIdList.append(p_CtrInfoList->at(i)->dvcId);
            }
        }
        break;
    }
    case CJQ:
    {
        for (int i=0;i<p_CtrInfoList->size();i++)
        {
            QList<int> tempIdList;
            QMap<int,MeterInfoForSingleNet*>::const_iterator const_it;
            for (const_it = (p_CtrInfoList->at(i)->p_MeterInfoForSingleNetList)->constBegin(); const_it != (p_CtrInfoList->at(i)->p_MeterInfoForSingleNetList)->constEnd(); ++const_it)
            {
                if(const_it.value()->slotPosition == CJQ)
                {
                    tempIdList.append(const_it.value()->dvcId);
                }
            }
            foreach(int id,tempIdList)
            {
                if(!dvcIdList.contains(id))
                {
                    dvcIdList.append(id);
                }
            }
        }
        break;
    }
    default:
    {
        break;
    }

    }

    if(dvcIdList.size()==0)
    {
        dvcIdList.append(p_CtrInfoList->at(0)->dvcId);
    }

    return dvcIdList;
}

uchar findMeterType(uchar *addr, shared_ptr<CtrInfo> p_CtrInfo)
{
    uchar meterType=0xff;   //0代表单相; 1代表三相; 0xff未找到
    for(int i=0; i<p_CtrInfo->p_MeterInfoForSingleNetList->values().size(); i++)
    {
        if(isArrayEqual(static_cast<uchar*>(p_CtrInfo->p_MeterInfoForSingleNetList->values().at(i)->mtrAddr),addr,6))
        {
            if(p_CtrInfo->p_MeterInfoForSingleNetList->values().at(i)->realPhase==3 ||
                    p_CtrInfo->p_MeterInfoForSingleNetList->values().at(i)->realPhase==5 ||
                    p_CtrInfo->p_MeterInfoForSingleNetList->values().at(i)->realPhase==6 ||
                    p_CtrInfo->p_MeterInfoForSingleNetList->values().at(i)->realPhase==7)
                meterType=0x01;
            else
                meterType=0x00;

            break;
        }
    }

    return meterType;
}



/************************载波协议************************/
QuerySystemInfo_Down::QuerySystemInfo_Down()
{
    memcpy(fixedHead,FIXED_HEAD,4);

    prtclVersn=1;
    msgHeadLen=20;
    reserve1=0;

    reserve2=0;
    dvcType=3;
    msgSeq=1;
    memset(srcAddr,0,6);
    memset(dstAddr,0,6);
}
QuerySystemInfo_Down::~QuerySystemInfo_Down()
{

}
QByteArray QuerySystemInfo_Down::encode_QuerySystemInfo_Down(shared_ptr<QuerySystemInfo_Down> p_QuerySystemInfo_Down)
{
    QByteArray msg_BPLC_App;

    msg_BPLC_App.append(p_QuerySystemInfo_Down->fixedHead,4);
    msg_BPLC_App.append(char((p_QuerySystemInfo_Down->msgHeadLen&0x03)<<6 | p_QuerySystemInfo_Down->prtclVersn));
    msg_BPLC_App.append(char((p_QuerySystemInfo_Down->reserve1<<4) | (p_QuerySystemInfo_Down->msgHeadLen>>2)));

    msg_BPLC_App.append(char(p_QuerySystemInfo_Down->reserve2));
    msg_BPLC_App.append(char(p_QuerySystemInfo_Down->dvcType));
    msg_BPLC_App.append(char(p_QuerySystemInfo_Down->msgSeq));
    msg_BPLC_App.append(char(p_QuerySystemInfo_Down->msgSeq>>8));
    msg_BPLC_App.append(char(p_QuerySystemInfo_Down->msgSeq>>16));
    msg_BPLC_App.append(char(p_QuerySystemInfo_Down->msgSeq>>24));
    msg_BPLC_App.append(p_QuerySystemInfo_Down->srcAddr,6);
    msg_BPLC_App.append(p_QuerySystemInfo_Down->dstAddr,6);

    return  msg_BPLC_App;
}

QuerySystemInfo_Up::QuerySystemInfo_Up()
{

}
QuerySystemInfo_Up::~QuerySystemInfo_Up()
{

}
shared_ptr<QuerySystemInfo_Up> QuerySystemInfo_Up::decode_QuerySystemInfo_Up(QByteArray * data)
{
    shared_ptr<QuerySystemInfo_Up> p_QuerySystemInfo_Up=make_shared<QuerySystemInfo_Up>();

    memcpy(p_QuerySystemInfo_Up->fixedHead,data->data(),4);
    p_QuerySystemInfo_Up->prtclVersn=0x3f&data->at(4);
    p_QuerySystemInfo_Up->msgHeadLen=((0x0f&data->at(5))*4)+((0xC0&data->at(4))>>6);
    p_QuerySystemInfo_Up->ansState=(0xf0&data->at(5))>>4;

    p_QuerySystemInfo_Up->reserve=0xff&data->at(6);
    p_QuerySystemInfo_Up->dvcType=0xff&data->at(7);
    p_QuerySystemInfo_Up->msgSeq=(0xff&data->at(11))*256*256*256+(0xff&data->at(10))*256*256+(0xff&data->at(9))*256+(0xff&data->at(8));
    memcpy(p_QuerySystemInfo_Up->srcAddr,data->data()+12,6);
    memcpy(p_QuerySystemInfo_Up->dstAddr,data->data()+18,6);
    memcpy(p_QuerySystemInfo_Up->mnfctrCode,data->data()+24,2);
    memcpy(p_QuerySystemInfo_Up->chipCode,data->data()+26,2);
    p_QuerySystemInfo_Up->bootVersn=data->at(28);
    memcpy(p_QuerySystemInfo_Up->softVersn,data->data()+29,4);
    memcpy(p_QuerySystemInfo_Up->softVersnTime,data->data()+33,7);
    p_QuerySystemInfo_Up->versnCSFlashVal=(0xff&data->at(43))*256*256*256+(0xff&data->at(42))*256*256+(0xff&data->at(41))*256+(0xff&data->at(40));
    p_QuerySystemInfo_Up->versnCSCaculValue=(0xff&data->at(47))*256*256*256+(0xff&data->at(46))*256*256+(0xff&data->at(45))*256+(0xff&data->at(44));
    memcpy(p_QuerySystemInfo_Up->nodeAddr,data->data()+48,6);

    return p_QuerySystemInfo_Up;
}

SetParam_Down::SetParam_Down()
{
    memcpy(fixedHead,SET_PARAM,4);

    prtclVersn=1;
    msgHeadLen=20;
    reserve1=0;

    idCnt=1;
    msgSeq=1;
    memset(srcAddr,0,6);
    memset(dstAddr,0,6);

    paramInfoList.clear();
}
SetParam_Down::~SetParam_Down()
{

}
QByteArray SetParam_Down::encode_SetParam_Down(shared_ptr<SetParam_Down> p_SetParam_Down)
{
    QByteArray msg_BPLC_App;

    msg_BPLC_App.append(p_SetParam_Down->fixedHead,4);
    msg_BPLC_App.append(char((p_SetParam_Down->msgHeadLen&0x03)<<6 | p_SetParam_Down->prtclVersn));
    msg_BPLC_App.append(char((p_SetParam_Down->reserve1<<4) | (p_SetParam_Down->msgHeadLen>>2)));

    msg_BPLC_App.append(char(p_SetParam_Down->idCnt&0xff));
    msg_BPLC_App.append(char((p_SetParam_Down->idCnt>>8)&0xff));
    msg_BPLC_App.append(char(p_SetParam_Down->msgSeq&0xff));
    msg_BPLC_App.append(char((p_SetParam_Down->msgSeq>>8)&0xff));
    msg_BPLC_App.append(char(p_SetParam_Down->msgSeq>>16&0xff));
    msg_BPLC_App.append(char((p_SetParam_Down->msgSeq>>24)&0xff));
    msg_BPLC_App.append(p_SetParam_Down->srcAddr,6);
    msg_BPLC_App.append(p_SetParam_Down->dstAddr,6);

    for(int i=0; i<p_SetParam_Down->paramInfoList.size(); i++)
    {
        msg_BPLC_App.append(char(p_SetParam_Down->paramInfoList.at(i).id&0xff));
        msg_BPLC_App.append(char((p_SetParam_Down->paramInfoList.at(i).id>>8)&0xff));
        msg_BPLC_App.append(char(p_SetParam_Down->paramInfoList.at(i).idLen&0xff));
        msg_BPLC_App.append(char((p_SetParam_Down->paramInfoList.at(i).idLen>>8)&0xff));
        msg_BPLC_App.append(p_SetParam_Down->paramInfoList.at(i).idCntnt);
    }

    return  msg_BPLC_App;
}

SetParam_Up::SetParam_Up()
{

}
SetParam_Up::~SetParam_Up()
{

}
shared_ptr<SetParam_Up> SetParam_Up::decode_SetParam_Up(QByteArray * data)
{
    shared_ptr<SetParam_Up> p_SetParam_Up=make_shared<SetParam_Up>();

    memcpy(p_SetParam_Up->fixedHead,data->data(),4);
    p_SetParam_Up->prtclVersn=0x3f&data->at(4);
    p_SetParam_Up->msgHeadLen=((0x0f&data->at(5))*4)+((0xC0&data->at(4))>>6);
    p_SetParam_Up->resStatus=(0xf0&data->at(5))>>4;

    p_SetParam_Up->idCnt=(0xff&data->at(7))*256+(0xff&data->at(6));
    p_SetParam_Up->msgSeq=(0xff&data->at(11))*256*256*256+(0xff&data->at(10))*256*256+(0xff&data->at(9))*256+(0xff&data->at(8));
    memcpy(p_SetParam_Up->srcAddr,data->data()+12,6);
    memcpy(p_SetParam_Up->dstAddr,data->data()+18,6);

    p_SetParam_Up->paramSetRlstInfoList.clear();
    for(int i=0; i<p_SetParam_Up->idCnt; i++)
    {
        ParamSetRlstInfo paramSetRlstInfo;

        paramSetRlstInfo.id=(0xff&data->at(25+i*3))*256+(0xff&data->at(24+i*3));
        paramSetRlstInfo.setRes=0xff&data->at(26+i*3);

        p_SetParam_Up->paramSetRlstInfoList.append(paramSetRlstInfo);
    }

    return p_SetParam_Up;
}

ChkParam_Down::ChkParam_Down()
{
    memcpy(fixedHead,CHK_PARAM,4);

    prtclVersn=1;
    msgHeadLen=20;
    reserve1=0;

    idCnt=1;
    msgSeq=1;
    memset(srcAddr,0,6);
    memset(dstAddr,0,6);

    idList.clear();
}
ChkParam_Down::~ChkParam_Down()
{

}
QByteArray ChkParam_Down::encode_ChkParam_Down(shared_ptr<ChkParam_Down> p_ChkParam_Down)
{
    QByteArray msg_BPLC_App;

    msg_BPLC_App.append(p_ChkParam_Down->fixedHead,4);
    msg_BPLC_App.append(char((p_ChkParam_Down->msgHeadLen&0x03)<<6 | p_ChkParam_Down->prtclVersn));
    msg_BPLC_App.append(char((p_ChkParam_Down->reserve1<<4) | (p_ChkParam_Down->msgHeadLen>>2)));

    p_ChkParam_Down->idCnt=ushort(p_ChkParam_Down->idList.size());
    msg_BPLC_App.append(char(p_ChkParam_Down->idCnt&0xff));
    msg_BPLC_App.append(char((p_ChkParam_Down->idCnt>>8)&0xff));
    msg_BPLC_App.append(char(p_ChkParam_Down->msgSeq&0xff));
    msg_BPLC_App.append(char((p_ChkParam_Down->msgSeq>>8)&0xff));
    msg_BPLC_App.append(char((p_ChkParam_Down->msgSeq>>16)&0xff));
    msg_BPLC_App.append(char((p_ChkParam_Down->msgSeq>>24)&0xff));
    msg_BPLC_App.append(p_ChkParam_Down->srcAddr,6);
    msg_BPLC_App.append(p_ChkParam_Down->dstAddr,6);

    for(int i=0; i<p_ChkParam_Down->idList.size(); i++)
    {
        msg_BPLC_App.append(char(p_ChkParam_Down->idList.at(i)&0xff));
        msg_BPLC_App.append(char((p_ChkParam_Down->idList.at(i)>>8)&0xff));
    }

    return  msg_BPLC_App;
}

ChkParam_Up::ChkParam_Up()
{

}
ChkParam_Up::~ChkParam_Up()
{

}
shared_ptr<ChkParam_Up> ChkParam_Up::decode_ChkParam_Up(QByteArray * data)
{
    shared_ptr<ChkParam_Up> p_ChkParam_Up=make_shared<ChkParam_Up>();

    memcpy(p_ChkParam_Up->fixedHead,data->data(),4);
    p_ChkParam_Up->prtclVersn=0x3f&data->at(4);
    p_ChkParam_Up->msgHeadLen=((0x0f&data->at(5))*4)+((0xC0&data->at(4))>>6);
    p_ChkParam_Up->resStatus=(0xf0&data->at(5))>>4;

    p_ChkParam_Up->idCnt=(0xff&data->at(7))*256+(0xff&data->at(6));
    p_ChkParam_Up->msgSeq=(0xff&data->at(11))*256*256*256+(0xff&data->at(10))*256*256+(0xff&data->at(9))*256+(0xff&data->at(8));
    memcpy(p_ChkParam_Up->srcAddr,data->data()+12,6);
    memcpy(p_ChkParam_Up->dstAddr,data->data()+18,6);

    int index=0;
    p_ChkParam_Up->paramInfoList.clear();
    for(int i=0; i<p_ChkParam_Up->idCnt; i++)
    {
        ParamInfo paramInfo;

        paramInfo.id=(0xff&data->at(25+index))*256+(0xff&data->at(24+index));
        paramInfo.idLen=(0xff&data->at(27+index))*256+(0xff&data->at(26+index));
        paramInfo.idCntnt=data->mid(28+index,paramInfo.idLen);

        index+=(4+paramInfo.idLen);

        p_ChkParam_Up->paramInfoList.append(paramInfo);
    }

    return p_ChkParam_Up;
}



