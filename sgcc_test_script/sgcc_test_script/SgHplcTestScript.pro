#-------------------------------------------------
#
# Project created by QtCreator 2020-10-29 T14:59:23
#
#-------------------------------------------------

QT       += core
QT       += network
QT       += serialport
QT       += widgets
#greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SgHplcTestScript
TEMPLATE = lib
CONFIG += c++11

#-------------------------------------------------
#使用预编译头
#PRECOMPILED_HEADER = stable.h
#编译器使用并行编译
#QMAKE_CXXFLAGS += /MP
#-------------------------------------------------

#INCLUDEPATH += C:\Boost\include\boost-1_67

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
    CommonModule/BuildNetwork.cpp \
    CommonModule/BuildNetworkDetect.cpp \
    PublicDataStruct/commdatatype.cpp \
    SgHplcTestScript.cpp \
    TestCase/20Standard/BaudConsult/BaudConsult.cpp \
    TestCase/20Standard/BaudConsult/BaudConsultPrepare.cpp \
    TestCase/20Standard/BaudConsult/Script_20Standard_BaudConsult_Abnormal_CCO.cpp \
    TestCase/20Standard/BaudConsult/Script_20Standard_BaudConsult_ConfirmDelay_CCO.cpp \
    TestCase/20Standard/BaudConsult/Script_20Standard_BaudConsult_Confirm_CCO.cpp \
    TestCase/20Standard/BaudConsult/Script_20Standard_BaudConsult_Confirm_CKQ.cpp \
    TestCase/20Standard/BaudConsult/Script_20Standard_BaudConsult_DefaultRate_CCO.cpp \
    TestCase/20Standard/BaudConsult/Script_20Standard_BaudConsult_Deny_CCO.cpp \
    TestCase/20Standard/BaudConsult/Script_20Standard_BaudConsult_NotSearchMeterRate_CCO.cpp \
    TestCase/20Standard/EventReport/EventReportPrepare.cpp \
    TestCase/20Standard/EventReport/Script_20Standard_EventReport_InNet.cpp \
    TestCase/20Standard/EventReport/Script_20Standard_EventReport_NotInNet.cpp \
    TestCase/AfnTest/Script_AfnTest.cpp \
    TestCase/AreaIdentity/Script_AreaIdentity.cpp \
    TestCase/AreaTopologyIndentity/Script_TopoIndentity_Com_645.cpp \
    TestCase/AreaTopologyIndentity/Script_TopoIndentity_Com_Factory645.cpp \
    TestCase/AreaTopologyIndentity/Script_TopoIndentity_Controller_645.cpp \
    TestCase/AreaTopologyIndentity/Script_TopoIndentity_Controller_Factory645.cpp \
    TestCase/AreaTopologyIndentity/Script_TopoIndentity_Controller_FactoryOOP.cpp \
    TestCase/AreaTopologyIndentity/Script_TopoIndentity_Controller_OOP.cpp \
    TestCase/AreaTopologyIndentity/Script_TopoIndentity_Router_645.cpp \
    TestCase/AreaTopologyIndentity/Script_TopoIndentity_Router_OOP.cpp \
    TestCase/AreaTopologyIndentity/Script_TopoIndentity_SetSwitch.cpp \
    TestCase/AreaTopologyIndentity/Script_TopoIndentity_Timing_645.cpp \
    TestCase/AreaTopologyIndentity/Script_TopoIndentity_Timing_OOP.cpp \
    TestCase/BroadcastTime/Script_BroadcastTime.cpp \
    TestCase/BroadcastTime/Script_BroadcastTime_DelayRelated.cpp \
    TestCase/BroadcastTime/Script_BroadcastTime_ExceedMaxTime.cpp \
    TestCase/BroadcastTime/Script_BroadcastTime_LocalProtocolTest.cpp \
    TestCase/BroadcastTime/Script_BroadcastTime_NotBuildNet.cpp \
    TestCase/BroadcastTime/Script_BroadcastTime_ZeroTime.cpp \
    TestCase/BuildMultiNetwork_GW.cpp \
    TestCase/BuildNetwork/Script_BuildNetwork_GW.cpp \
    TestCase/BuildNetwork/Script_BuildNetwork_GW_AddNode.cpp \
    TestCase/BuildNetwork/Script_BuildNetwork_GW_AddNode_V3.cpp \
    TestCase/BuildNetwork/Script_BuildNetwork_GW_DeleteNode.cpp \
    TestCase/BuildNetwork/Script_BuildNetwork_GW_HardReset.cpp \
    TestCase/BuildNetwork/Script_BuildNetwork_GW_MasterNodeAddress.cpp \
    TestCase/BuildNetwork/Script_BuildNetwork_GW_MasterNodeAddress_V3.cpp \
    TestCase/BuildNetwork/Script_BuildNetwork_GW_NetFlag.cpp \
    TestCase/BuildNetwork/Script_BuildNetwork_GW_ParameterInit.cpp \
    TestCase/BuildNetwork/Script_SearchMeterWithOutNodes_GW_NetFlag.cpp \
    TestCase/BuildNetwork_GW.cpp \
    TestCase/CCO_AddrManage/Script_CCOAddrManage_LengthAbnormal.cpp \
    TestCase/CCO_AddrManage/Script_CCO_AddrManage_normal.cpp \
    TestCase/CommAddrRequest/Script_CommAddrRequest_1.cpp \
    TestCase/CommAddrRequest/Script_CommAddrRequest_2.cpp \
    TestCase/CommAddrRequest/Script_CommAddrRequest_3.cpp \
    TestCase/CommAddrRequest/Script_CommAddrRequest_4.cpp \
    TestCase/CommonData/CommonDataType_TestCase.cpp \
    TestCase/EventReport/Script_EventReport.cpp \
    TestCase/EventReport/Script_PowerOffCjqReport.cpp \
    TestCase/EventReport/Script_PowerOffStaReport.cpp \
    TestCase/FlashTest/EraseWriteProgram.cpp \
    TestCase/FlashTest/Script_EraseWriteProgram.cpp \
    TestCase/FlashTest/Script_FlashTest.cpp \
    TestCase/FlashTest/Script_FlashTest_V3.cpp \
    TestCase/FlashTest/Script_ReadStaRfParameter.cpp \
    TestCase/FreqManage/Script_FreqManage_FreqAndFreqDivideSwitch.cpp \
    TestCase/FreqManage/Script_FreqManage_FreqDivideTest1.cpp \
    TestCase/FreqManage/Script_FreqManage_FreqDivideTest2.cpp \
    TestCase/FreqManage/Script_FreqManage_RecordSwitching.cpp \
    TestCase/FreqManage/Script_FreqManage_SetFreq0To0.cpp \
    TestCase/FreqManage/Script_FreqManage_SetFreq0To1.cpp \
    TestCase/FreqManage/Script_FreqManage_SetFreq0To2.cpp \
    TestCase/FreqManage/Script_FreqManage_SetFreq0To3.cpp \
    TestCase/FreqManage/Script_FreqManage_SetFreq1To0.cpp \
    TestCase/FreqManage/Script_FreqManage_SetFreq1To1.cpp \
    TestCase/FreqManage/Script_FreqManage_SetFreq1To2.cpp \
    TestCase/FreqManage/Script_FreqManage_SetFreq1To2_V2.cpp \
    TestCase/FreqManage/Script_FreqManage_SetFreq1To2_V3.cpp \
    TestCase/FreqManage/Script_FreqManage_SetFreq1To3.cpp \
    TestCase/FreqManage/Script_FreqManage_SetFreq2To0.cpp \
    TestCase/FreqManage/Script_FreqManage_SetFreq2To1.cpp \
    TestCase/FreqManage/Script_FreqManage_SetFreq2To2.cpp \
    TestCase/FreqManage/Script_FreqManage_SetFreq2To3.cpp \
    TestCase/FreqManage/Script_FreqManage_SetFreq3To0.cpp \
    TestCase/FreqManage/Script_FreqManage_SetFreq3To1.cpp \
    TestCase/FreqManage/Script_FreqManage_SetFreq3To2.cpp \
    TestCase/FreqManage/Script_FreqManage_SetFreq3To3.cpp \
    TestCase/FreqManage/Script_FreqManage_SetFreqNetRestart.cpp \
    TestCase/FreqManage/Script_FreqManage_SetFreqNoPara.cpp \
    TestCase/FreqManage/Script_FreqManage_SetFreqNoPara_V3.cpp \
    TestCase/FreqManage/Script_FreqManage_SetFreqReset.cpp \
    TestCase/FreqManage/Script_FreqManage_SetFreqTwice.cpp \
    TestCase/FreqManage/Script_FreqManage_SetSameFreq.cpp \
    TestCase/HighPower/Script_CcoHighPowerConfig.cpp \
    TestCase/HighPower/Script_CcoHighPowerConfig_AbnormalPara.cpp \
    TestCase/HighPower/Script_STAHighPowerConfig_AbnormalParaConfig_02F1.cpp \
    TestCase/HighPower/Script_STAHighPowerConfig_AbnormalParaConfig_F0F21.cpp \
    TestCase/HighPower/Script_STAHighPowerConfig_NormalParaConfig_02F1.cpp \
    TestCase/HighPower/Script_STAHighPowerConfig_NormalParaConfig_F0F21.cpp \
    TestCase/HighPower/Script_STAHighPowerConfig_NotNetConfig_02F1.cpp \
    TestCase/HighPower/Script_STAHighPowerConfig_NotNetConfig_F0F21.cpp \
    TestCase/HighPower/Script_STAHighPowerConfig_TurnOffHighPowerConfig.cpp \
    TestCase/IdSnManage/Script_ChipIdManage_CCO.cpp \
    TestCase/IdSnManage/Script_ChipIdManage_CCO_V3Key.cpp \
    TestCase/IdSnManage/Script_ChipIdManage_STA.cpp \
    TestCase/IdSnManage/Script_ChipIdManage_STA_V3Key.cpp \
    TestCase/IdSnManage/Script_IdManage_CCO.cpp \
    TestCase/IdSnManage/Script_IdManage_STA.cpp \
    TestCase/IdSnManage/Script_IdSnManage_STA_Serial.cpp \
    TestCase/IdSnManage/Script_Module_Chip_Sn_WipeFlash.cpp \
    TestCase/IdSnManage/Script_SnManage_CCO.cpp \
    TestCase/IdSnManage/Script_SnManage_STA.cpp \
    TestCase/IdSnManage/Script_StaChipID.cpp \
    TestCase/IdSnManage/Script_StaChipIdAndKey.cpp \
    TestCase/IdSnManage/Script_StaIDPressurTest_Module_Chip_Sn.cpp \
    TestCase/IdSnManage/Script_StaModuleID.cpp \
    TestCase/IdSnManage/Script_StaSn.cpp \
    TestCase/InitializeTest/Script_CcoHardWareInit_FrequentReset_1.cpp \
    TestCase/InitializeTest/Script_CcoHardWareInit_FrequentReset_2.cpp \
    TestCase/InitializeTest/Script_CcoHardWareInit_NetworkingFinish_1.cpp \
    TestCase/InitializeTest/Script_CcoHardWareInit_NetworkingFinish_2.cpp \
    TestCase/InitializeTest/Script_CcoHardWareInit_NotBuildFinish_1.cpp \
    TestCase/InitializeTest/Script_CcoHardWareInit_NotBuildFinish_2.cpp \
    TestCase/InitializeTest/Script_CcoHardWareInit_ReadMeter.cpp \
    TestCase/InitializeTest/Script_CcoHardWareInit_SearchMeter.cpp \
    TestCase/InitializeTest/Script_CcoHardWareInit_StaUpdate.cpp \
    TestCase/InitializeTest/Script_CcoParaInit_NetworkingFinish.cpp \
    TestCase/InitializeTest/Script_CcoParaInit_ReadMeter.cpp \
    TestCase/InitializeTest/Script_CcoParaInit_RouteUpgrade.cpp \
    TestCase/InitializeTest/Script_CcoParaInit_SearchMeter.cpp \
    TestCase/InitializeTest/Script_CcoParaInit_StaUpdate.cpp \
    TestCase/InitializeTest/Script_InitializeTest.cpp \
    TestCase/InitializeTest/Script_InitializeTest_NotBuildNet.cpp \
    TestCase/InitializeTest/Script_InitializeTest_ReadMeter.cpp \
    TestCase/InitializeTest/Script_InitializeTest_SearchMeter.cpp \
    TestCase/InitializeTest/Script_InitializeTest_Temp.cpp \
    TestCase/InitializeTest/Script_PullPinReset_100.cpp \
    TestCase/InterOperate_GW/Script_ActiveRegisterFlow.cpp \
    TestCase/InterOperate_GW/Script_BroadcastTimeFlow.cpp \
    TestCase/InterOperate_GW/Script_ConcentratorActiveReadMeterFlow.cpp \
    TestCase/InterOperate_GW/Script_HplcSupplementFlow.cpp \
    TestCase/InterOperate_GW/Script_IdentityFlow.cpp \
    TestCase/InterOperate_GW/Script_ParameterSynchronousFlow.cpp \
    TestCase/InterOperate_GW/Script_ReadMeter13F1Flow.cpp \
    TestCase/InterOperate_GW/Script_ReadMeterTestFlow.cpp \
    TestCase/InterOperate_GW/Script_RouterActiveReadMeterFlow.cpp \
    TestCase/InterOperate_GW_STA/Script_EventActiveReport_645.cpp \
    TestCase/InterOperate_GW_STA/Script_EventActiveReport_OOP.cpp \
    TestCase/InterOperate_GW_STA/Script_IdentityFlow_STA.cpp \
    TestCase/InterOperate_GW_STA/Script_ReadMeterTestFlow_STA.cpp \
    TestCase/InteroperateBase_GW.cpp \
    TestCase/InteroperateInit_GW.cpp \
    TestCase/LN_Inverse/Script_LN_Inverse.cpp \
    TestCase/LN_Inverse/Script_LN_Inverse_AddNodeRepeat.cpp \
    TestCase/LN_Inverse/Script_LN_Inverse_RouterReset.cpp \
    TestCase/LN_Inverse/Script_LN_Inverse_V3.cpp \
    TestCase/MultiNetwork/Script_BuildMultiNetwork_GW.cpp \
    TestCase/MultiNetwork/Script_MultiNetwork_ReadMeter.cpp \
    TestCase/MultiTask_V3/Script_BroadcastTime_MultiTask.cpp \
    TestCase/MultiTask_V3/Script_BuildNetwork_MultiTask.cpp \
    TestCase/MultiTask_V3/Script_ReadMeter_14F1_13F1_F1F1.cpp \
    TestCase/MultiTask_V3/Script_ReadMeter_14F1_13F1_F1F1_RestartPauseRecover.cpp \
    TestCase/MultiTask_V3/Script_ReadMeter_SearchMeter_Upgrade_MultiTask.cpp \
    TestCase/MultiTask_V3/Script_Upgrade_STA_normal_2_MultiTask.cpp \
    TestCase/NodeSurvive/Script_NodeSurvive_Partial.cpp \
    TestCase/NodeSurvive/Script_NodeSurvive_Whole.cpp \
    TestCase/ParameterManage/Script_ParameterManage_DeleteNotInRouter.cpp \
    TestCase/ParameterManage/Script_ParameterManage_EraseFlash.cpp \
    TestCase/ParameterManage/Script_ParameterManage_ExceedMaxNum.cpp \
    TestCase/ParameterManage/Script_ParameterManage_ExceedMaxNum_V3.cpp \
    TestCase/ParameterManage/Script_ParameterManage_HardReset.cpp \
    TestCase/ParameterManage/Script_ParameterManage_HardResetHaveParameter.cpp \
    TestCase/ParameterManage/Script_ParameterManage_InitState.cpp \
    TestCase/ParameterManage/Script_ParameterManage_LetterParameter.cpp \
    TestCase/ParameterManage/Script_ParameterManage_NormalNum.cpp \
    TestCase/ParameterManage/Script_ParameterManage_ParameterInit.cpp \
    TestCase/ParameterManage/Script_ParameterManage_RepeatParameter.cpp \
    TestCase/ParameterManage/Script_ParameterManage_RepeatParameter_V3.cpp \
    TestCase/PhaseIdentity/Script_Demo_PhaseIdentity_10F31.cpp \
    TestCase/PhaseIdentity/Script_PhaseIdentity.cpp \
    TestCase/PhaseIdentity/Script_PhaseIdentity_V3.cpp \
    TestCase/ProxyChange/Script_ProxyChange.cpp \
    TestCase/ProxyChange/Script_ProxyChange_DeletePco.cpp \
    TestCase/ReadMeter/ReadMeter_13F1/Script_ReadMeter_13F1.cpp \
    TestCase/ReadMeter/ReadMeter_13F1/Script_ReadMeter_13F1_AddDeleteNode.cpp \
    TestCase/ReadMeter/ReadMeter_13F1/Script_ReadMeter_13F1_AddrAll9AndAllA.cpp \
    TestCase/ReadMeter/ReadMeter_13F1/Script_ReadMeter_13F1_ContinueReadSameNetNode.cpp \
    TestCase/ReadMeter/ReadMeter_13F1/Script_ReadMeter_13F1_ContrlArea.cpp \
    TestCase/ReadMeter/ReadMeter_13F1/Script_ReadMeter_13F1_DelayRelated.cpp \
    TestCase/ReadMeter/ReadMeter_13F1/Script_ReadMeter_13F1_DelayRelatedAbandon.cpp \
    TestCase/ReadMeter/ReadMeter_13F1/Script_ReadMeter_13F1_InfoAreaAddrAreaLogicalMatch.cpp \
    TestCase/ReadMeter/ReadMeter_13F1/Script_ReadMeter_13F1_MeterAnswerLogicTest.cpp \
    TestCase/ReadMeter/ReadMeter_13F1/Script_ReadMeter_13F1_NetAccessAndLeave.cpp \
    TestCase/ReadMeter/ReadMeter_13F1/Script_ReadMeter_13F1_NoConnectedToTheNet_NoArchives.cpp \
    TestCase/ReadMeter/ReadMeter_13F1/Script_ReadMeter_13F1_NoRouterPause.cpp \
    TestCase/ReadMeter/ReadMeter_13F1/Script_ReadMeter_13F1_NodeNotInRouter.cpp \
    TestCase/ReadMeter/ReadMeter_13F1/Script_ReadMeter_13F1_NotBuildNet.cpp \
    TestCase/ReadMeter/ReadMeter_13F1/Script_ReadMeter_13F1_ProtocolConformance.cpp \
    TestCase/ReadMeter/ReadMeter_13F1/Script_ReadMeter_13F1_SubsidiaryNode.cpp \
    TestCase/ReadMeter/ReadMeter_14F1/Script_ReadMeter_14F1.cpp \
    TestCase/ReadMeter/ReadMeter_14F1/Script_ReadMeter_14F1_10F5QueryReadFailedNodeInfo.cpp \
    TestCase/ReadMeter/ReadMeter_14F1/Script_ReadMeter_14F1_3762InfoAreaFrameSeqNotMatch.cpp \
    TestCase/ReadMeter/ReadMeter_14F1/Script_ReadMeter_14F1_AddDeleteNode.cpp \
    TestCase/ReadMeter/ReadMeter_14F1/Script_ReadMeter_14F1_ArchivesTypeNotLikeMeterProtocolType.cpp \
    TestCase/ReadMeter/ReadMeter_14F1/Script_ReadMeter_14F1_ChangeMeter.cpp \
    TestCase/ReadMeter/ReadMeter_14F1/Script_ReadMeter_14F1_Delay14F3DownNoResponse.cpp \
    TestCase/ReadMeter/ReadMeter_14F1/Script_ReadMeter_14F1_DelayRelated.cpp \
    TestCase/ReadMeter/ReadMeter_14F1/Script_ReadMeter_14F1_MeterReadParaInit.cpp \
    TestCase/ReadMeter/ReadMeter_14F1/Script_ReadMeter_14F1_NotBuildNet.cpp \
    TestCase/ReadMeter/ReadMeter_14F1/Script_ReadMeter_14F1_ReadSignNot02HDelayRelated.cpp \
    TestCase/ReadMeter/ReadMeter_14F1/Script_ReadMeter_14F1_RelayAddress.cpp \
    TestCase/ReadMeter/ReadMeter_14F1/Script_ReadMeter_14F1_RestartPauseRocover1.cpp \
    TestCase/ReadMeter/ReadMeter_14F1/Script_ReadMeter_14F1_RestartPauseRocover2.cpp \
    TestCase/ReadMeter/ReadMeter_14F1/Script_ReadMeter_14F1_SearchMeter.cpp \
    TestCase/ReadMeter/ReadMeter_14F1/Script_ReadMeter_14F1_SubsidiaryNode.cpp \
    TestCase/ReadMeter/ReadMeter_14F1/Script_ReadMeter_14F1_SuccessFailFlag.cpp \
    TestCase/ReadMeter/ReadMeter_14F1/Script_ReadMeter_14F1_VariousDataID.cpp \
    TestCase/ReadMeter/ReadMeter_F1F1/Script_ReadMeter_Curve.cpp \
    TestCase/ReadMeter/ReadMeter_F1F1/Script_ReadMeter_F1F1.cpp \
    TestCase/ReadMeter/ReadMeter_F1F1/Script_ReadMeter_F1F1_AddDeleteNode.cpp \
    TestCase/ReadMeter/ReadMeter_F1F1/Script_ReadMeter_F1F1_ExceedMaxFrameNum.cpp \
    TestCase/ReadMeter/ReadMeter_F1F1/Script_ReadMeter_F1F1_ExceedMaxParallelNum.cpp \
    TestCase/ReadMeter/ReadMeter_F1F1/Script_ReadMeter_F1F1_MeterAddrInconsistent.cpp \
    TestCase/ReadMeter/ReadMeter_F1F1/Script_ReadMeter_F1F1_MultipleFrames.cpp \
    TestCase/ReadMeter/ReadMeter_F1F1/Script_ReadMeter_F1F1_MultipleFramesMixedWithError.cpp \
    TestCase/ReadMeter/ReadMeter_F1F1/Script_ReadMeter_F1F1_NodeNotInRouter.cpp \
    TestCase/ReadMeter/ReadMeter_F1F1/Script_ReadMeter_F1F1_NotBuildNet.cpp \
    TestCase/ReadMeter/ReadMeter_F1F1/Script_ReadMeter_F1F1_NotPauseRouter.cpp \
    TestCase/ReadMeter/ReadMeter_F1F1/Script_ReadMeter_F1F1_ProtocolAbnormal.cpp \
    TestCase/ReadMeter/ReadMeter_F1F1/Script_ReadMeter_F1F1_QueryRunState.cpp \
    TestCase/ReadMeter/ReadMeter_F1F1/Script_ReadMeter_F1F1_RelayAddress.cpp \
    TestCase/ReadMeter/ReadMeter_F1F1/Script_ReadMeter_F1F1_SameMeterNewFrame.cpp \
    TestCase/ReadMeter/ReadMeter_F1F1/Script_ReadMeter_F1F1_SearchMeter.cpp \
    TestCase/ReadMeter/ReadMeter_F1F1/Script_ReadMeter_F1F1_SubsidiaryNode.cpp \
    TestCase/RefuseListReport/Script_RefuseListReport.cpp \
    TestCase/RefuseListReport/Script_RefuseListReport_DefaultClose.cpp \
    TestCase/RefuseListReport/Script_RefuseListReport_ResetPoweroff.cpp \
    TestCase/Script_Template.cpp \
    TestCase/SearchMeter/Script_CcoSearchMeter_PastTime.cpp \
    TestCase/SearchMeter/Script_CcoSearchMeter_RouteWorking.cpp \
    TestCase/SearchMeter/Script_SearchMeter_Delay2Mins.cpp \
    TestCase/SearchMeter/Script_SearchMeter_NonePara_Normal.cpp \
    TestCase/SearchMeter/Script_SearchMeter_NonePara_Stop.cpp \
    TestCase/SearchMeter/Script_SearchMeter_PartialPara_0Min.cpp \
    TestCase/SearchMeter/Script_SearchMeter_PartialPara_0Min_V2.cpp \
    TestCase/SearchMeter/Script_SearchMeter_PartialPara_DeleteInfo.cpp \
    TestCase/SearchMeter/Script_SearchMeter_PartialPara_Normal.cpp \
    TestCase/SearchMeter/Script_SearchMeter_PartialPara_Offline.cpp \
    TestCase/SearchMeter/Script_SearchMeter_PartialPara_SendTwice.cpp \
    TestCase/SearchMeter/Script_SearchMeter_PartialPara_Stop.cpp \
    TestCase/SearchMeter/Script_SearchMeter_RouterReset.cpp \
    TestCase/SearchMeter/Script_SearchMeter_WholePara_Normal.cpp \
    TestCase/SearchMeter/Script_SearchMeter_WholePara_Stop.cpp \
#    TestCase/SuperpowerTest/Script_SuperpowerTest.cpp \
#    TestCase/SuperpowerTest_Shanghai/Script_AbnormalAndClose_SuperpowerParameterConfig.cpp \
#    TestCase/SuperpowerTest_Shanghai/Script_AbnormalSuperpowerParameterConfig.cpp \
#    TestCase/SuperpowerTest_Shanghai/Script_SuperpowerParameterConfig_CCO.cpp \
#    TestCase/SuperpowerTest_Shanghai/Script_SuperpowerParameterConfig_STA.cpp \
    TestCase/TestCase_BeiJing/AreaIdentity/Script_AreaIdentity_Beijing.cpp \
    TestCase/TestCase_BeiJing/CarrierSignalQuality/Script_CarrierSignalQuality_Beijing.cpp \
    TestCase/TestCase_BeiJing/NodeInfo/Script_NodeInfo_Beijing.cpp \
    TestCase/TestCase_BeiJing/NodeNetLock/Script_SingleNodeNetLock_Beijing.cpp \
    TestCase/TestCase_BeiJing/NodeNetLock/Script_SingleNodeNetUnlock_Beijing.cpp \
    TestCase/TestCase_BeiJing/NodeNetLock/Script_WholeNodeNetLock_Beijing.cpp \
    TestCase/TestCase_BeiJing/NodeNetLock/Script_WholeNodeNetUnlock_Beijing.cpp \
    TestCase/TestCase_BeiJing/RouterState/Script_RouterState_ReadMeter_Beijing.cpp \
    TestCase/TestCase_BeiJing/RouterState/Script_RouterState_SearchMeter_Upgrade_Beijing.cpp \
    TestCase/TestCase_BeiJing/WhiteListTest/Script_WhiteListTest_Beijing.cpp \
    TestCase/TestCase_BeiJing/WhiteListTest/Script_WhiteListTest_Normal_Beijing.cpp \
    TestCase/TestCase_GanSu/Script_RouteRunningStatus_Gansu.cpp \
    TestCase/TestCase_HuNan/BroadcastTime/Script_BroadcastTime_Hunan.cpp \
    TestCase/TestCase_HuNan/CurveStorage/Script_CurveRead_F1F100_Hunan.cpp \
    TestCase/TestCase_HuNan/CurveStorage/Script_TaskConfig_Disable_Hunan.cpp \
    TestCase/TestCase_HuNan/CurveStorage/Script_TaskConfig_Enable_Hunan.cpp \
    TestCase/TestCase_HuNan/CurveStorage/Script_TaskConfig_Normal_Hunan.cpp \
    TestCase/TestCase_HuNan/PhaseIdentity/Script_PhaseIdentity_Hunan.cpp \
    TestCase/TestCase_HuNan/Script_ClockOverUnderReport_Hunan.cpp \
    TestCase/TestCase_HuNan/Script_MinuteFreeze_NormalConfigRead_Hunan.cpp \
    TestCase/TestCase_HuNan/Script_MinuteFreeze_Reset_Hunan.cpp \
    TestCase/TestCase_HuNan/Script_MinuteFreeze_StorageRequire_Hunan.cpp \
    TestCase/TestCase_HuNan/Script_ProgramComparison_CCO_Hunan.cpp \
    TestCase/TestCase_HuNan/Script_ProgramComparison_STA_Hunan.cpp \
    TestCase/TestCase_HuNan/Script_RouterTimePeriod_Hunan.cpp \
    TestCase/TestCase_HuNan/SoftwareRecord/Script_SoftwareRecord_CCO_Hunan.cpp \
    TestCase/TestCase_HuNan/SoftwareRecord/Script_SoftwareRecord_STA_13F1_Hunan.cpp \
    TestCase/TestCase_HuNan/SoftwareRecord/Script_SoftwareRecord_STA_Hunan.cpp \
    TestCase/TestCase_HuNan/StaAttest/Script_StaAttest_Controller_Hunan.cpp \
    TestCase/TestCase_HuNan/StaAttest/Script_StaAttest_Hunan.cpp \
    TestCase/TestCase_JiangSu/Script_BaudRateQueryAndSetting.cpp \
    TestCase/TestCase_JiangSu/Script_MinuteCollect_CalibrationTime.cpp \
    TestCase/TestCase_JiangSu/Script_MinuteCollect_NoCalibrationTime.cpp \
    TestCase/TestCase_JiangSu/Script_NodeSupportMinuteCollection.cpp \
    TestCase/TestCase_JiangSu/Script_NodeSupportMinuteCollection_PartialNetwork.cpp \
    TestCase/TestCase_JiangSu/Script_ReadMeter_F1F1_DelayTime20s.cpp \
    TestCase/TestCase_JiangSu/Script_SlaveNodeActiveRegister.cpp \
    TestCase/TestCase_ShanXi/Script_AllowReportingFromNodeStatusChanges_Shanxi.cpp \
    TestCase/TestCase_ShanXi/Script_NodeStatusReport_Shanxi.cpp \
    TestCase/TestCase_ShanXi/Script_ProhibitReportingFromNodeStatusChanges_Shanxi.cpp \
    TestCase/TestCase_ShanXi/Script_QueryBasicNetworkInfo_Shanxi.cpp \
    TestCase/TestCase_ShanXi/Script_QueryEnableFlagReportingFromNodeStatusChange_Shanxi.cpp \
    TestCase/TestCase_ShanXi/Script_QueryNetworkNodeInfo_F0F102_Shanxi.cpp \
    TestCase/TestCase_ShanXi/Script_QueryNetworkNodeInfo_F0F103_Shanxi.cpp \
    TestCase/TestCase_ShannXi/AreaIdentity/Script_AreaIdentity_Shaanxi.cpp \
    TestCase/TestCase_ShannXi/ModuleInfoManage/Script_ModuleInfoManage_Shaanxi.cpp \
    TestCase/TestCase_ShannXi/PhaseIdentity/Script_PhaseIdentity_Shaanxi.cpp \
    TestCase/TestCase_ShannXi/WholeNetPercept/Script_WholeNetPercept_Shaanxi.cpp \
    TestCase/TestCase_ZheJiang/BuildNetwork/BuildNetwork_Zhejiang.cpp \
    TestCase/TestCase_ZheJiang/CollectImmediately/Script_CollectImmediately_Zhejiang.cpp \
    TestCase/TestCase_ZheJiang/CollectImmediately/Script_RouterParameterComplare_Zhejiang.cpp \
    TestCase/TestCase_ZheJiang/NetTopology/Script_NetTopology_Zhejiang.cpp \
    TestCase/TestCase_ZheJiang/NetworkOptimize/Script_NetworkOptimize_Zhejiang.cpp \
    TestCase/TestCase_ZheJiang/Script_ActivelyReport06F4.cpp \
    TestCase/TestCase_ZheJiang/Script_CcoOneMinuteNoBeacon.cpp \
    TestCase/TestCase_ZheJiang/Script_CcoOneMinuteNoBeacon_Retiming.cpp \
    TestCase/TestCase_ZheJiang/Script_CollectImmediately.cpp \
    TestCase/TestCase_ZheJiang/Script_OffGridLock.cpp \
    TestCase/TestCase_ZheJiang/Script_QueryCcoInternalMeterInfo_10F150.cpp \
    TestCase/TestCase_ZheJiang/Script_SetQueryStaDeviceAddress.cpp \
    TestCase/TransparentTrans/Script_TransparentTrans.cpp \
    TestCase/TransparentTrans/Script_TransparentTrans_All99Addr.cpp \
    TestCase/TransparentTrans/Script_TransparentTrans_NotBuildNet.cpp \
    TestCase/TransparentTrans/Script_TransparentTrans_ProtocolTest.cpp \
    TestCase/TransparentTrans/Script_TransparentTrans_RepeatRead.cpp \
    TestCase/Upgrade/FileTransfer.cpp \
    TestCase/Upgrade/QueryVersion.cpp \
    TestCase/Upgrade/Upgrade_CCO/Script_CCOUpgrade_BaudTest.cpp \
    TestCase/Upgrade/Upgrade_CCO/Script_CCOUpgrade_FormatTest.cpp \
    TestCase/Upgrade/Upgrade_CCO/Script_CCOUpgrade_LengthTest.cpp \
    TestCase/Upgrade/Upgrade_CCO/Script_CCOUpgrade_MissFileTest.cpp \
    TestCase/Upgrade/Upgrade_CCO/Script_CCOUpgrade_RouterResetTest.cpp \
    TestCase/Upgrade/Upgrade_CCO/Script_Upgrade_CCO_changepacketlength.cpp \
    TestCase/Upgrade/Upgrade_CCO/Script_Upgrade_CCO_changepacketorder.cpp \
    TestCase/Upgrade/Upgrade_CCO/Script_Upgrade_CCO_normal.cpp \
    TestCase/Upgrade/Upgrade_CCO/Script_Upgrade_CCO_normal_2.cpp \
    TestCase/Upgrade/Upgrade_CCO/Script_Upgrade_CCO_over512K.cpp \
    TestCase/Upgrade/Upgrade_CCO/Script_Upgrade_CCO_over768K_V3.cpp \
    TestCase/Upgrade/Upgrade_CCO/Script_Upgrade_CCO_resendpacket.cpp \
    TestCase/Upgrade/Upgrade_CCO/Script_Upgrade_CCO_wrongbin.cpp \
    TestCase/Upgrade/Upgrade_STA/Script_STAUpgrade_WrongFile.cpp \
    TestCase/Upgrade/Upgrade_STA/Script_Upgrade_STA.cpp \
    TestCase/Upgrade/Upgrade_STA/Script_Upgrade_STA_normal.cpp \
    TestCase/Upgrade/Upgrade_STA/Script_Upgrade_STA_normal_2.cpp \
    TestCase/Upgrade/Upgrade_STA_ReadController/Script_STAUpgrade_CKQ_WrongFile.cpp \
    TestCase/Upgrade/Upgrade_STA_ReadController/Script_Upgrade_STA_ReadController.cpp \
    TestCase/Upgrade/Upgrade_STA_Serial/Script_Upgrade_STA_Serial_normal.cpp \
    TestCase/VersionHead/Script_VersionHead_CcoFail.cpp \
    TestCase/VersionHead/Script_VersionHead_CcoNormal.cpp \
    TestCase/VersionHead/Script_VersionHead_ControllerSta.cpp \
    TestCase/VersionHead/Script_VersionHead_QueryRecordCco.cpp \
    TestCase/VersionHead/Script_VersionHead_QueryRecordSta.cpp \
    TestCase/VersionHead/Script_VersionHead_SerialSta.cpp \
    TestCase/WorkModeManage/Script_WorkModeManage.cpp \
    TestCase/WorkModeManage/Script_WorkModeManage_CloseToOpen.cpp \
    TestCase/WorkModeManage/Script_WorkModeManage_EraseFlash.cpp \
    TestCase/WorkModeManage/Script_WorkModeManage_OpenToClose.cpp \
    TestCase/WrongMessage/Script_WrongMessage.cpp


HEADERS += \
    CommonModule/BuildNetwork.h \
    CommonModule/BuildNetworkDetect.h \
    DynamicCreate.h \
    PublicDataStruct/AbstractPluginScript.h \
    PublicDataStruct/abstractscript.h \
    PublicDataStruct/abstractscripthost.h \
    PublicDataStruct/commdatatype.h \
    PublicDataStruct/logitem.h \
    ReflectFactory.h \
    SgHplcTestScript.h \
    TestCase/20Standard/BaudConsult/BaudConsult.h \
    TestCase/20Standard/BaudConsult/BaudConsultPrepare.h \
    TestCase/20Standard/BaudConsult/Script_20Standard_BaudConsult_Abnormal_CCO.h \
    TestCase/20Standard/BaudConsult/Script_20Standard_BaudConsult_ConfirmDelay_CCO.h \
    TestCase/20Standard/BaudConsult/Script_20Standard_BaudConsult_Confirm_CCO.h \
    TestCase/20Standard/BaudConsult/Script_20Standard_BaudConsult_Confirm_CKQ.h \
    TestCase/20Standard/BaudConsult/Script_20Standard_BaudConsult_DefaultRate_CCO.h \
    TestCase/20Standard/BaudConsult/Script_20Standard_BaudConsult_Deny_CCO.h \
    TestCase/20Standard/BaudConsult/Script_20Standard_BaudConsult_NotSearchMeterRate_CCO.h \
    TestCase/20Standard/EventReport/EventReportPrepare.h \
    TestCase/20Standard/EventReport/Script_20Standard_EventReport_InNet.h \
    TestCase/20Standard/EventReport/Script_20Standard_EventReport_NotInNet.h \
    TestCase/AfnTest/Script_AfnTest.h \
    TestCase/AreaIdentity/Script_AreaIdentity.h \
    TestCase/AreaTopologyIndentity/Script_TopoIndentity_Com_645.h \
    TestCase/AreaTopologyIndentity/Script_TopoIndentity_Com_Factory645.h \
    TestCase/AreaTopologyIndentity/Script_TopoIndentity_Controller_645.h \
    TestCase/AreaTopologyIndentity/Script_TopoIndentity_Controller_Factory645.h \
    TestCase/AreaTopologyIndentity/Script_TopoIndentity_Controller_FactoryOOP.h \
    TestCase/AreaTopologyIndentity/Script_TopoIndentity_Controller_OOP.h \
    TestCase/AreaTopologyIndentity/Script_TopoIndentity_Router_645.h \
    TestCase/AreaTopologyIndentity/Script_TopoIndentity_Router_OOP.h \
    TestCase/AreaTopologyIndentity/Script_TopoIndentity_SetSwitch.h \
    TestCase/AreaTopologyIndentity/Script_TopoIndentity_Timing_645.h \
    TestCase/AreaTopologyIndentity/Script_TopoIndentity_Timing_OOP.h \
    TestCase/BroadcastTime/Script_BroadcastTime.h \
    TestCase/BroadcastTime/Script_BroadcastTime_DelayRelated.h \
    TestCase/BroadcastTime/Script_BroadcastTime_ExceedMaxTime.h \
    TestCase/BroadcastTime/Script_BroadcastTime_LocalProtocolTest.h \
    TestCase/BroadcastTime/Script_BroadcastTime_NotBuildNet.h \
    TestCase/BroadcastTime/Script_BroadcastTime_ZeroTime.h \
    TestCase/BuildMultiNetwork_GW.h \
    TestCase/BuildNetwork/Script_BuildNetwork_GW.h \
    TestCase/BuildNetwork/Script_BuildNetwork_GW_AddNode.h \
    TestCase/BuildNetwork/Script_BuildNetwork_GW_AddNode_V3.h \
    TestCase/BuildNetwork/Script_BuildNetwork_GW_DeleteNode.h \
    TestCase/BuildNetwork/Script_BuildNetwork_GW_HardReset.h \
    TestCase/BuildNetwork/Script_BuildNetwork_GW_MasterNodeAddress.h \
    TestCase/BuildNetwork/Script_BuildNetwork_GW_MasterNodeAddress_V3.h \
    TestCase/BuildNetwork/Script_BuildNetwork_GW_NetFlag.h \
    TestCase/BuildNetwork/Script_BuildNetwork_GW_ParameterInit.h \
    TestCase/BuildNetwork/Script_SearchMeterWithOutNodes_GW_NetFlag.h \
    TestCase/BuildNetwork_GW.h \
    TestCase/CCO_AddrManage/Script_CCOAddrManage_LengthAbnormal.h \
    TestCase/CCO_AddrManage/Script_CCO_AddrManage_normal.h \
    TestCase/CommAddrRequest/Script_CommAddrRequest_1.h \
    TestCase/CommAddrRequest/Script_CommAddrRequest_2.h \
    TestCase/CommAddrRequest/Script_CommAddrRequest_3.h \
    TestCase/CommAddrRequest/Script_CommAddrRequest_4.h \
    TestCase/CommonData/CommonDataType_TestCase.h \
    TestCase/EventReport/Script_EventReport.h \
    TestCase/EventReport/Script_PowerOffCjqReport.h \
    TestCase/EventReport/Script_PowerOffStaReport.h \
    TestCase/FlashTest/EraseWriteProgram.h \
    TestCase/FlashTest/Script_EraseWriteProgram.h \
    TestCase/FlashTest/Script_FlashTest.h \
    TestCase/FlashTest/Script_FlashTest_V3.h \
    TestCase/FlashTest/Script_ReadStaRfParameter.h \
    TestCase/FreqManage/Script_FreqManage_FreqAndFreqDivideSwitch.h \
    TestCase/FreqManage/Script_FreqManage_FreqDivideTest1.h \
    TestCase/FreqManage/Script_FreqManage_FreqDivideTest2.h \
    TestCase/FreqManage/Script_FreqManage_RecordSwitching.h \
    TestCase/FreqManage/Script_FreqManage_SetFreq0To0.h \
    TestCase/FreqManage/Script_FreqManage_SetFreq0To1.h \
    TestCase/FreqManage/Script_FreqManage_SetFreq0To2.h \
    TestCase/FreqManage/Script_FreqManage_SetFreq0To3.h \
    TestCase/FreqManage/Script_FreqManage_SetFreq1To0.h \
    TestCase/FreqManage/Script_FreqManage_SetFreq1To1.h \
    TestCase/FreqManage/Script_FreqManage_SetFreq1To2.h \
    TestCase/FreqManage/Script_FreqManage_SetFreq1To2_V2.h \
    TestCase/FreqManage/Script_FreqManage_SetFreq1To2_V3.h \
    TestCase/FreqManage/Script_FreqManage_SetFreq1To3.h \
    TestCase/FreqManage/Script_FreqManage_SetFreq2To0.h \
    TestCase/FreqManage/Script_FreqManage_SetFreq2To1.h \
    TestCase/FreqManage/Script_FreqManage_SetFreq2To2.h \
    TestCase/FreqManage/Script_FreqManage_SetFreq2To3.h \
    TestCase/FreqManage/Script_FreqManage_SetFreq3To0.h \
    TestCase/FreqManage/Script_FreqManage_SetFreq3To1.h \
    TestCase/FreqManage/Script_FreqManage_SetFreq3To2.h \
    TestCase/FreqManage/Script_FreqManage_SetFreq3To3.h \
    TestCase/FreqManage/Script_FreqManage_SetFreqNetRestart.h \
    TestCase/FreqManage/Script_FreqManage_SetFreqNoPara.h \
    TestCase/FreqManage/Script_FreqManage_SetFreqNoPara_V3.h \
    TestCase/FreqManage/Script_FreqManage_SetFreqReset.h \
    TestCase/FreqManage/Script_FreqManage_SetFreqTwice.h \
    TestCase/FreqManage/Script_FreqManage_SetSameFreq.h \
    TestCase/HighPower/Script_CcoHighPowerConfig.h \
    TestCase/HighPower/Script_CcoHighPowerConfig_AbnormalPara.h \
    TestCase/HighPower/Script_STAHighPowerConfig_AbnormalParaConfig_02F1.h \
    TestCase/HighPower/Script_STAHighPowerConfig_AbnormalParaConfig_F0F21.h \
    TestCase/HighPower/Script_STAHighPowerConfig_NormalParaConfig_02F1.h \
    TestCase/HighPower/Script_STAHighPowerConfig_NormalParaConfig_F0F21.h \
    TestCase/HighPower/Script_STAHighPowerConfig_NotNetConfig_02F1.h \
    TestCase/HighPower/Script_STAHighPowerConfig_NotNetConfig_F0F21.h \
    TestCase/HighPower/Script_STAHighPowerConfig_TurnOffHighPowerConfig.h \
    TestCase/IdSnManage/Script_ChipIdManage_CCO.h \
    TestCase/IdSnManage/Script_ChipIdManage_CCO_V3Key.h \
    TestCase/IdSnManage/Script_ChipIdManage_STA.h \
    TestCase/IdSnManage/Script_ChipIdManage_STA_V3Key.h \
    TestCase/IdSnManage/Script_IdManage_CCO.h \
    TestCase/IdSnManage/Script_IdManage_STA.h \
    TestCase/IdSnManage/Script_IdSnManage_STA_Serial.h \
    TestCase/IdSnManage/Script_Module_Chip_Sn_WipeFlash.h \
    TestCase/IdSnManage/Script_SnManage_CCO.h \
    TestCase/IdSnManage/Script_SnManage_STA.h \
    TestCase/IdSnManage/Script_StaChipID.h \
    TestCase/IdSnManage/Script_StaChipIdAndKey.h \
    TestCase/IdSnManage/Script_StaIDPressurTest_Module_Chip_Sn.h \
    TestCase/IdSnManage/Script_StaModuleID.h \
    TestCase/IdSnManage/Script_StaSn.h \
    TestCase/InitializeTest/Script_CcoHardWareInit_FrequentReset_1.h \
    TestCase/InitializeTest/Script_CcoHardWareInit_FrequentReset_2.h \
    TestCase/InitializeTest/Script_CcoHardWareInit_NetworkingFinish_1.h \
    TestCase/InitializeTest/Script_CcoHardWareInit_NetworkingFinish_2.h \
    TestCase/InitializeTest/Script_CcoHardWareInit_NotBuildFinish_1.h \
    TestCase/InitializeTest/Script_CcoHardWareInit_NotBuildFinish_2.h \
    TestCase/InitializeTest/Script_CcoHardWareInit_ReadMeter.h \
    TestCase/InitializeTest/Script_CcoHardWareInit_SearchMeter.h \
    TestCase/InitializeTest/Script_CcoHardWareInit_StaUpdate.h \
    TestCase/InitializeTest/Script_CcoParaInit_NetworkingFinish.h \
    TestCase/InitializeTest/Script_CcoParaInit_ReadMeter.h \
    TestCase/InitializeTest/Script_CcoParaInit_RouteUpgrade.h \
    TestCase/InitializeTest/Script_CcoParaInit_SearchMeter.h \
    TestCase/InitializeTest/Script_CcoParaInit_StaUpdate.h \
    TestCase/InitializeTest/Script_InitializeTest.h \
    TestCase/InitializeTest/Script_InitializeTest_NotBuildNet.h \
    TestCase/InitializeTest/Script_InitializeTest_ReadMeter.h \
    TestCase/InitializeTest/Script_InitializeTest_SearchMeter.h \
    TestCase/InitializeTest/Script_InitializeTest_Temp.h \
    TestCase/InitializeTest/Script_PullPinReset_100.h \
    TestCase/InterOperate_GW/Script_ActiveRegisterFlow.h \
    TestCase/InterOperate_GW/Script_BroadcastTimeFlow.h \
    TestCase/InterOperate_GW/Script_ConcentratorActiveReadMeterFlow.h \
    TestCase/InterOperate_GW/Script_HplcSupplementFlow.h \
    TestCase/InterOperate_GW/Script_IdentityFlow.h \
    TestCase/InterOperate_GW/Script_ParameterSynchronousFlow.h \
    TestCase/InterOperate_GW/Script_ReadMeter13F1Flow.h \
    TestCase/InterOperate_GW/Script_ReadMeterTestFlow.h \
    TestCase/InterOperate_GW/Script_RouterActiveReadMeterFlow.h \
    TestCase/InterOperate_GW_STA/Script_EventActiveReport_645.h \
    TestCase/InterOperate_GW_STA/Script_EventActiveReport_OOP.h \
    TestCase/InterOperate_GW_STA/Script_IdentityFlow_STA.h \
    TestCase/InterOperate_GW_STA/Script_ReadMeterTestFlow_STA.h \
    TestCase/InteroperateBase_GW.h \
    TestCase/InteroperateInit_GW.h \
    TestCase/LN_Inverse/Script_LN_Inverse.h \
    TestCase/LN_Inverse/Script_LN_Inverse_AddNodeRepeat.h \
    TestCase/LN_Inverse/Script_LN_Inverse_RouterReset.h \
    TestCase/LN_Inverse/Script_LN_Inverse_V3.h \
    TestCase/MultiNetwork/Script_BuildMultiNetwork_GW.h \
    TestCase/MultiNetwork/Script_MultiNetwork_ReadMeter.h \
    TestCase/MultiTask_V3/Script_BroadcastTime_MultiTask.h \
    TestCase/MultiTask_V3/Script_BuildNetwork_MultiTask.h \
    TestCase/MultiTask_V3/Script_ReadMeter_14F1_13F1_F1F1.h \
    TestCase/MultiTask_V3/Script_ReadMeter_14F1_13F1_F1F1_RestartPauseRecover.h \
    TestCase/MultiTask_V3/Script_ReadMeter_SearchMeter_Upgrade_MultiTask.h \
    TestCase/MultiTask_V3/Script_Upgrade_STA_normal_2_MultiTask.h \
    TestCase/NodeSurvive/Script_NodeSurvive_Partial.h \
    TestCase/NodeSurvive/Script_NodeSurvive_Whole.h \
    TestCase/ParameterManage/Script_ParameterManage_DeleteNotInRouter.h \
    TestCase/ParameterManage/Script_ParameterManage_EraseFlash.h \
    TestCase/ParameterManage/Script_ParameterManage_ExceedMaxNum.h \
    TestCase/ParameterManage/Script_ParameterManage_ExceedMaxNum_V3.h \
    TestCase/ParameterManage/Script_ParameterManage_HardReset.h \
    TestCase/ParameterManage/Script_ParameterManage_HardResetHaveParameter.h \
    TestCase/ParameterManage/Script_ParameterManage_InitState.h \
    TestCase/ParameterManage/Script_ParameterManage_LetterParameter.h \
    TestCase/ParameterManage/Script_ParameterManage_NormalNum.h \
    TestCase/ParameterManage/Script_ParameterManage_ParameterInit.h \
    TestCase/ParameterManage/Script_ParameterManage_RepeatParameter.h \
    TestCase/ParameterManage/Script_ParameterManage_RepeatParameter_V3.h \
    TestCase/PhaseIdentity/Script_Demo_PhaseIdentity_10F31.h \
    TestCase/PhaseIdentity/Script_PhaseIdentity.h \
    TestCase/PhaseIdentity/Script_PhaseIdentity_V3.h \
    TestCase/ProxyChange/Script_ProxyChange.h \
    TestCase/ProxyChange/Script_ProxyChange_DeletePco.h \
    TestCase/ReadMeter/ReadMeter_13F1/Script_ReadMeter_13F1.h \
    TestCase/ReadMeter/ReadMeter_13F1/Script_ReadMeter_13F1_AddDeleteNode.h \
    TestCase/ReadMeter/ReadMeter_13F1/Script_ReadMeter_13F1_AddrAll9AndAllA.h \
    TestCase/ReadMeter/ReadMeter_13F1/Script_ReadMeter_13F1_ContinueReadSameNetNode.h \
    TestCase/ReadMeter/ReadMeter_13F1/Script_ReadMeter_13F1_ContrlArea.h \
    TestCase/ReadMeter/ReadMeter_13F1/Script_ReadMeter_13F1_DelayRelated.h \
    TestCase/ReadMeter/ReadMeter_13F1/Script_ReadMeter_13F1_DelayRelatedAbandon.h \
    TestCase/ReadMeter/ReadMeter_13F1/Script_ReadMeter_13F1_InfoAreaAddrAreaLogicalMatch.h \
    TestCase/ReadMeter/ReadMeter_13F1/Script_ReadMeter_13F1_MeterAnswerLogicTest.h \
    TestCase/ReadMeter/ReadMeter_13F1/Script_ReadMeter_13F1_NetAccessAndLeave.h \
    TestCase/ReadMeter/ReadMeter_13F1/Script_ReadMeter_13F1_NoConnectedToTheNet_NoArchives.h \
    TestCase/ReadMeter/ReadMeter_13F1/Script_ReadMeter_13F1_NoRouterPause.h \
    TestCase/ReadMeter/ReadMeter_13F1/Script_ReadMeter_13F1_NodeNotInRouter.h \
    TestCase/ReadMeter/ReadMeter_13F1/Script_ReadMeter_13F1_NotBuildNet.h \
    TestCase/ReadMeter/ReadMeter_13F1/Script_ReadMeter_13F1_ProtocolConformance.h \
    TestCase/ReadMeter/ReadMeter_13F1/Script_ReadMeter_13F1_SubsidiaryNode.h \
    TestCase/ReadMeter/ReadMeter_14F1/Script_ReadMeter_14F1.h \
    TestCase/ReadMeter/ReadMeter_14F1/Script_ReadMeter_14F1_10F5QueryReadFailedNodeInfo.h \
    TestCase/ReadMeter/ReadMeter_14F1/Script_ReadMeter_14F1_3762InfoAreaFrameSeqNotMatch.h \
    TestCase/ReadMeter/ReadMeter_14F1/Script_ReadMeter_14F1_AddDeleteNode.h \
    TestCase/ReadMeter/ReadMeter_14F1/Script_ReadMeter_14F1_ArchivesTypeNotLikeMeterProtocolType.h \
    TestCase/ReadMeter/ReadMeter_14F1/Script_ReadMeter_14F1_ChangeMeter.h \
    TestCase/ReadMeter/ReadMeter_14F1/Script_ReadMeter_14F1_Delay14F3DownNoResponse.h \
    TestCase/ReadMeter/ReadMeter_14F1/Script_ReadMeter_14F1_DelayRelated.h \
    TestCase/ReadMeter/ReadMeter_14F1/Script_ReadMeter_14F1_MeterReadParaInit.h \
    TestCase/ReadMeter/ReadMeter_14F1/Script_ReadMeter_14F1_NotBuildNet.h \
    TestCase/ReadMeter/ReadMeter_14F1/Script_ReadMeter_14F1_ReadSignNot02HDelayRelated.h \
    TestCase/ReadMeter/ReadMeter_14F1/Script_ReadMeter_14F1_RelayAddress.h \
    TestCase/ReadMeter/ReadMeter_14F1/Script_ReadMeter_14F1_RestartPauseRocover1.h \
    TestCase/ReadMeter/ReadMeter_14F1/Script_ReadMeter_14F1_RestartPauseRocover2.h \
    TestCase/ReadMeter/ReadMeter_14F1/Script_ReadMeter_14F1_SearchMeter.h \
    TestCase/ReadMeter/ReadMeter_14F1/Script_ReadMeter_14F1_SubsidiaryNode.h \
    TestCase/ReadMeter/ReadMeter_14F1/Script_ReadMeter_14F1_SuccessFailFlag.h \
    TestCase/ReadMeter/ReadMeter_14F1/Script_ReadMeter_14F1_VariousDataID.h \
    TestCase/ReadMeter/ReadMeter_F1F1/Script_ReadMeter_Curve.h \
    TestCase/ReadMeter/ReadMeter_F1F1/Script_ReadMeter_F1F1.h \
    TestCase/ReadMeter/ReadMeter_F1F1/Script_ReadMeter_F1F1_AddDeleteNode.h \
    TestCase/ReadMeter/ReadMeter_F1F1/Script_ReadMeter_F1F1_ExceedMaxFrameNum.h \
    TestCase/ReadMeter/ReadMeter_F1F1/Script_ReadMeter_F1F1_ExceedMaxParallelNum.h \
    TestCase/ReadMeter/ReadMeter_F1F1/Script_ReadMeter_F1F1_MeterAddrInconsistent.h \
    TestCase/ReadMeter/ReadMeter_F1F1/Script_ReadMeter_F1F1_MultipleFrames.h \
    TestCase/ReadMeter/ReadMeter_F1F1/Script_ReadMeter_F1F1_MultipleFramesMixedWithError.h \
    TestCase/ReadMeter/ReadMeter_F1F1/Script_ReadMeter_F1F1_NodeNotInRouter.h \
    TestCase/ReadMeter/ReadMeter_F1F1/Script_ReadMeter_F1F1_NotBuildNet.h \
    TestCase/ReadMeter/ReadMeter_F1F1/Script_ReadMeter_F1F1_NotPauseRouter.h \
    TestCase/ReadMeter/ReadMeter_F1F1/Script_ReadMeter_F1F1_ProtocolAbnormal.h \
    TestCase/ReadMeter/ReadMeter_F1F1/Script_ReadMeter_F1F1_QueryRunState.h \
    TestCase/ReadMeter/ReadMeter_F1F1/Script_ReadMeter_F1F1_RelayAddress.h \
    TestCase/ReadMeter/ReadMeter_F1F1/Script_ReadMeter_F1F1_SameMeterNewFrame.h \
    TestCase/ReadMeter/ReadMeter_F1F1/Script_ReadMeter_F1F1_SearchMeter.h \
    TestCase/ReadMeter/ReadMeter_F1F1/Script_ReadMeter_F1F1_SubsidiaryNode.h \
    TestCase/RefuseListReport/Script_RefuseListReport.h \
    TestCase/RefuseListReport/Script_RefuseListReport_DefaultClose.h \
    TestCase/RefuseListReport/Script_RefuseListReport_ResetPoweroff.h \
    TestCase/Script_Template.h \
    TestCase/SearchMeter/Script_CcoSearchMeter_PastTime.h \
    TestCase/SearchMeter/Script_CcoSearchMeter_RouteWorking.h \
    TestCase/SearchMeter/Script_SearchMeter_Delay2Mins.h \
    TestCase/SearchMeter/Script_SearchMeter_NonePara_Normal.h \
    TestCase/SearchMeter/Script_SearchMeter_NonePara_Stop.h \
    TestCase/SearchMeter/Script_SearchMeter_PartialPara_ SendTwice.h \
    TestCase/SearchMeter/Script_SearchMeter_PartialPara_0Min.h \
    TestCase/SearchMeter/Script_SearchMeter_PartialPara_0Min_V2.h \
    TestCase/SearchMeter/Script_SearchMeter_PartialPara_DeleteInfo.h \
    TestCase/SearchMeter/Script_SearchMeter_PartialPara_Normal.h \
    TestCase/SearchMeter/Script_SearchMeter_PartialPara_Offline.h \
    TestCase/SearchMeter/Script_SearchMeter_PartialPara_SendTwice.h \
    TestCase/SearchMeter/Script_SearchMeter_PartialPara_Stop.h \
    TestCase/SearchMeter/Script_SearchMeter_RouterReset.h \
    TestCase/SearchMeter/Script_SearchMeter_WholePara_Normal.h \
    TestCase/SearchMeter/Script_SearchMeter_WholePara_Stop.h \
#    TestCase/SuperpowerTest/Script_SuperpowerTest.h \
#    TestCase/SuperpowerTest_Shanghai/Script_AbnormalAndClose_SuperpowerParameterConfig.h \
#    TestCase/SuperpowerTest_Shanghai/Script_AbnormalSuperpowerParameterConfig.h \
#    TestCase/SuperpowerTest_Shanghai/Script_SuperpowerParameterConfig_CCO.h \
#    TestCase/SuperpowerTest_Shanghai/Script_SuperpowerParameterConfig_STA.h \
    TestCase/TestCase_BeiJing/AreaIdentity/Script_AreaIdentity_Beijing.h \
    TestCase/TestCase_BeiJing/CarrierSignalQuality/Script_CarrierSignalQuality_Beijing.h \
    TestCase/TestCase_BeiJing/NodeInfo/Script_NodeInfo_Beijing.h \
    TestCase/TestCase_BeiJing/NodeNetLock/Script_SingleNodeNetLock_Beijing.h \
    TestCase/TestCase_BeiJing/NodeNetLock/Script_SingleNodeNetUnlock_Beijing.h \
    TestCase/TestCase_BeiJing/NodeNetLock/Script_WholeNodeNetLock_Beijing.h \
    TestCase/TestCase_BeiJing/NodeNetLock/Script_WholeNodeNetUnlock_Beijing.h \
    TestCase/TestCase_BeiJing/RouterState/Script_RouterState_ReadMeter_Beijing.h \
    TestCase/TestCase_BeiJing/RouterState/Script_RouterState_SearchMeter_Upgrade_Beijing.h \
    TestCase/TestCase_BeiJing/WhiteListTest/Script_WhiteListTest_Beijing.h \
    TestCase/TestCase_BeiJing/WhiteListTest/Script_WhiteListTest_Normal_Beijing.h \
    TestCase/TestCase_GanSu/Script_RouteRunningStatus_Gansu.h \
    TestCase/TestCase_HuNan/BroadcastTime/Script_BroadcastTime_Hunan.h \
    TestCase/TestCase_HuNan/CurveStorage/Script_CurveRead_F1F100_Hunan.h \
    TestCase/TestCase_HuNan/CurveStorage/Script_TaskConfig_Disable_Hunan.h \
    TestCase/TestCase_HuNan/CurveStorage/Script_TaskConfig_Enable_Hunan.h \
    TestCase/TestCase_HuNan/CurveStorage/Script_TaskConfig_Normal_Hunan.h \
    TestCase/TestCase_HuNan/PhaseIdentity/Script_PhaseIdentity_Hunan.h \
    TestCase/TestCase_HuNan/Script_ClockOverUnderReport_Hunan.h \
    TestCase/TestCase_HuNan/Script_MinuteFreeze_NormalConfigRead_Hunan.h \
    TestCase/TestCase_HuNan/Script_MinuteFreeze_Reset_Hunan.h \
    TestCase/TestCase_HuNan/Script_MinuteFreeze_StorageRequire_Hunan.h \
    TestCase/TestCase_HuNan/Script_ProgramComparison_CCO_Hunan.h \
    TestCase/TestCase_HuNan/Script_ProgramComparison_STA_Hunan.h \
    TestCase/TestCase_HuNan/Script_RouterTimePeriod_Hunan.h \
    TestCase/TestCase_HuNan/SoftwareRecord/Script_SoftwareRecord_CCO_Hunan.h \
    TestCase/TestCase_HuNan/SoftwareRecord/Script_SoftwareRecord_STA_13F1_Hunan.h \
    TestCase/TestCase_HuNan/SoftwareRecord/Script_SoftwareRecord_STA_Hunan.h \
    TestCase/TestCase_HuNan/StaAttest/Script_StaAttest_Controller_Hunan.h \
    TestCase/TestCase_HuNan/StaAttest/Script_StaAttest_Hunan.h \
    TestCase/TestCase_JiangSu/Script_BaudRateQueryAndSetting.h \
    TestCase/TestCase_JiangSu/Script_MinuteCollect_CalibrationTime.h \
    TestCase/TestCase_JiangSu/Script_MinuteCollect_NoCalibrationTime.h \
    TestCase/TestCase_JiangSu/Script_NodeSupportMinuteCollection.h \
    TestCase/TestCase_JiangSu/Script_NodeSupportMinuteCollection_PartialNetwork.h \
    TestCase/TestCase_JiangSu/Script_ReadMeter_F1F1_DelayTime20s.h \
    TestCase/TestCase_JiangSu/Script_SlaveNodeActiveRegister.h \
    TestCase/TestCase_ShanXi/Script_AllowReportingFromNodeStatusChanges_Shanxi.h \
    TestCase/TestCase_ShanXi/Script_NodeStatusReport_Shanxi.h \
    TestCase/TestCase_ShanXi/Script_ProhibitReportingFromNodeStatusChanges_Shanxi.h \
    TestCase/TestCase_ShanXi/Script_QueryBasicNetworkInfo_Shanxi.h \
    TestCase/TestCase_ShanXi/Script_QueryEnableFlagReportingFromNodeStatusChange_Shanxi.h \
    TestCase/TestCase_ShanXi/Script_QueryNetworkNodeInfo_F0F102_Shanxi.h \
    TestCase/TestCase_ShanXi/Script_QueryNetworkNodeInfo_F0F103_Shanxi.h \
    TestCase/TestCase_ShannXi/AreaIdentity/Script_AreaIdentity_Shaanxi.h \
    TestCase/TestCase_ShannXi/ModuleInfoManage/Script_ModuleInfoManage_Shaanxi.h \
    TestCase/TestCase_ShannXi/PhaseIdentity/Script_PhaseIdentity_Shaanxi.h \
    TestCase/TestCase_ShannXi/WholeNetPercept/Script_WholeNetPercept_Shaanxi.h \
    TestCase/TestCase_ZheJiang/BuildNetwork/BuildNetwork_Zhejiang.h \
    TestCase/TestCase_ZheJiang/CollectImmediately/Script_CollectImmediately_Zhejiang.h \
    TestCase/TestCase_ZheJiang/CollectImmediately/Script_RouterParameterComplare_Zhejiang.h \
    TestCase/TestCase_ZheJiang/NetTopology/Script_NetTopology_Zhejiang.h \
    TestCase/TestCase_ZheJiang/NetworkOptimize/Script_NetworkOptimize_Zhejiang.h \
    TestCase/TestCase_ZheJiang/Script_ActivelyReport06F4.h \
    TestCase/TestCase_ZheJiang/Script_CcoOneMinuteNoBeacon.h \
    TestCase/TestCase_ZheJiang/Script_CcoOneMinuteNoBeacon_Retiming.h \
    TestCase/TestCase_ZheJiang/Script_CollectImmediately.h \
    TestCase/TestCase_ZheJiang/Script_OffGridLock.h \
    TestCase/TestCase_ZheJiang/Script_QueryCcoInternalMeterInfo_10F150.h \
    TestCase/TestCase_ZheJiang/Script_SetQueryStaDeviceAddress.h \
    TestCase/TransparentTrans/Script_TransparentTrans.h \
    TestCase/TransparentTrans/Script_TransparentTrans_All99Addr.h \
    TestCase/TransparentTrans/Script_TransparentTrans_NotBuildNet.h \
    TestCase/TransparentTrans/Script_TransparentTrans_ProtocolTest.h \
    TestCase/TransparentTrans/Script_TransparentTrans_RepeatRead.h \
    TestCase/Upgrade/FileTransfer.h \
    TestCase/Upgrade/QueryVersion.h \
    TestCase/Upgrade/Upgrade_CCO/Script_CCOUpgrade_BaudTest.h \
    TestCase/Upgrade/Upgrade_CCO/Script_CCOUpgrade_FormatTest.h \
    TestCase/Upgrade/Upgrade_CCO/Script_CCOUpgrade_LengthTest.h \
    TestCase/Upgrade/Upgrade_CCO/Script_CCOUpgrade_MissFileTest.h \
    TestCase/Upgrade/Upgrade_CCO/Script_CCOUpgrade_RouterResetTest.h \
    TestCase/Upgrade/Upgrade_CCO/Script_Upgrade_CCO_changepacketlength.h \
    TestCase/Upgrade/Upgrade_CCO/Script_Upgrade_CCO_changepacketorder.h \
    TestCase/Upgrade/Upgrade_CCO/Script_Upgrade_CCO_normal.h \
    TestCase/Upgrade/Upgrade_CCO/Script_Upgrade_CCO_normal_2.h \
    TestCase/Upgrade/Upgrade_CCO/Script_Upgrade_CCO_over512K.h \
    TestCase/Upgrade/Upgrade_CCO/Script_Upgrade_CCO_over768K_V3.h \
    TestCase/Upgrade/Upgrade_CCO/Script_Upgrade_CCO_resendpacket.h \
    TestCase/Upgrade/Upgrade_CCO/Script_Upgrade_CCO_wrongbin.h \
    TestCase/Upgrade/Upgrade_STA/Script_STAUpgrade_WrongFile.h \
    TestCase/Upgrade/Upgrade_STA/Script_Upgrade_STA.h \
    TestCase/Upgrade/Upgrade_STA/Script_Upgrade_STA_normal.h \
    TestCase/Upgrade/Upgrade_STA/Script_Upgrade_STA_normal_2.h \
    TestCase/Upgrade/Upgrade_STA_ReadController/Script_STAUpgrade_CKQ_WrongFile.h \
    TestCase/Upgrade/Upgrade_STA_ReadController/Script_Upgrade_STA_ReadController.h \
    TestCase/Upgrade/Upgrade_STA_Serial/Script_Upgrade_STA_Serial_normal.h \
    TestCase/VersionHead/Script_VersionHead_CcoFail.h \
    TestCase/VersionHead/Script_VersionHead_CcoNormal.h \
    TestCase/VersionHead/Script_VersionHead_ControllerSta.h \
    TestCase/VersionHead/Script_VersionHead_QueryRecordCco.h \
    TestCase/VersionHead/Script_VersionHead_QueryRecordSta.h \
    TestCase/VersionHead/Script_VersionHead_SerialSta.h \
    TestCase/WorkModeManage/Script_WorkModeManage.h \
    TestCase/WorkModeManage/Script_WorkModeManage_CloseToOpen.h \
    TestCase/WorkModeManage/Script_WorkModeManage_EraseFlash.h \
    TestCase/WorkModeManage/Script_WorkModeManage_OpenToClose.h \
    TestCase/WrongMessage/Script_WrongMessage.h

FORMS +=



win32:CONFIG(release, debug|release): LIBS += -L$$PWD/3rdparty/StaticLib/DLT_645_Protocol_Lib/lib/ -lDLT_645_Protocol_Lib
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/3rdparty/StaticLib/DLT_645_Protocol_Lib/lib/ -lDLT_645_Protocol_Libd

INCLUDEPATH += $$PWD/3rdparty/StaticLib/DLT_645_Protocol_Lib/include
DEPENDPATH += $$PWD/3rdparty/StaticLib/DLT_645_Protocol_Lib/include

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/3rdparty/StaticLib/DLT_645_Protocol_Lib/lib/libDLT_645_Protocol_Lib.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/3rdparty/StaticLib/DLT_645_Protocol_Lib/lib/libDLT_645_Protocol_Libd.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/3rdparty/StaticLib/DLT_645_Protocol_Lib/lib/DLT_645_Protocol_Lib.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/3rdparty/StaticLib/DLT_645_Protocol_Lib/lib/DLT_645_Protocol_Libd.lib




win32:CONFIG(release, debug|release): LIBS += -L$$PWD/3rdparty/StaticLib/OOP_Lib/lib/ -lObject_Oriented_Electic_Data_Exchange_Protocol_Lib
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/3rdparty/StaticLib/OOP_Lib/lib/ -lObject_Oriented_Electic_Data_Exchange_Protocol_Libd
else:unix: LIBS += -L$$PWD/3rdparty/StaticLib/OOP_Lib/lib/ -lObject_Oriented_Electic_Data_Exchange_Protocol_Lib

INCLUDEPATH += $$PWD/3rdparty/StaticLib/OOP_Lib/include
DEPENDPATH += $$PWD/3rdparty/StaticLib/OOP_Lib/include

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/3rdparty/StaticLib/OOP_Lib/lib/libObject_Oriented_Electic_Data_Exchange_Protocol_Lib.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/3rdparty/StaticLib/OOP_Lib/lib/libObject_Oriented_Electic_Data_Exchange_Protocol_Libd.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/3rdparty/StaticLib/OOP_Lib/lib/Object_Oriented_Electic_Data_Exchange_Protocol_Lib.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/3rdparty/StaticLib/OOP_Lib/lib/Object_Oriented_Electic_Data_Exchange_Protocol_Libd.lib
else:unix: PRE_TARGETDEPS += $$PWD/3rdparty/StaticLib/OOP_Lib/lib/libObject_Oriented_Electic_Data_Exchange_Protocol_Lib.a


win32:CONFIG(release, debug|release): LIBS += -L$$PWD/3rdparty/StaticLib/QGDW_376_2_Protocol_Lib/lib/ -lQGDW_376_2_Lib
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/3rdparty/StaticLib/QGDW_376_2_Protocol_Lib/lib/ -lQGDW_376_2_Libd
else:unix: LIBS += -L$$PWD/3rdparty/StaticLib/QGDW_376_2_Protocol_Lib/lib/ -lQGDW_376_2_Lib

INCLUDEPATH += $$PWD/3rdparty/StaticLib/QGDW_376_2_Protocol_Lib/include
DEPENDPATH += $$PWD/3rdparty/StaticLib/QGDW_376_2_Protocol_Lib/include

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/3rdparty/StaticLib/QGDW_376_2_Protocol_Lib/lib/libQGDW_376_2_Lib.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/3rdparty/StaticLib/QGDW_376_2_Protocol_Lib/lib/libQGDW_376_2_Libd.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/3rdparty/StaticLib/QGDW_376_2_Protocol_Lib/lib/QGDW_376_2_Lib.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/3rdparty/StaticLib/QGDW_376_2_Protocol_Lib/lib/QGDW_376_2_Libd.lib
else:unix: PRE_TARGETDEPS += $$PWD/3rdparty/StaticLib/QGDW_376_2_Protocol_Lib/lib/libQGDW_376_2_Lib.a
