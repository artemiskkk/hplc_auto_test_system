#ifndef DATATYPEDEF_645_H
#define DATATYPEDEF_645_H

#include "QtGlobal"
#include <QList>

using namespace std;

namespace  dlt_645_Protocol {




/******起始字符68H******/
const uchar START_FLAG_645 = 0x68;          //起始字符


/*******控制码C*******/
const uchar READ_DATA                = 0x11;
const uchar NORMAL_RESP              = 0x91;
const uchar NORMAL_RESP_HaveFlwData  = 0xB1;
const uchar AbNORMAL_RESP_ReadData   = 0xD1;

const uchar READ_FollowDATA          = 0x12;
const uchar NORMAL_RESP_FollowData   = 0x92;
const uchar NORMAL_RESP_FollowData_HaveFlwData  = 0xB2;
const uchar AbNORMAL_RESP_ReadFollowData   = 0xD2;

const uchar WRITE_DATA               = 0x14;
const uchar NORMAL_RESP_WRITE_DATA   = 0x94;
const uchar AbNORMAL_RESP_WRITE_DATA = 0xD4;

const uchar READ_ADDR                = 0x13;
const uchar RESP_ADDR                = 0x93;

const uchar WRITE_ADDR               = 0x15;
const uchar RESP_WRITE_ADDR          = 0x95;

const uchar BROADCAST                = 0x08;

const uchar Froze_Cmd               = 0x16;
const uchar RESP_Froze_Cmd          = 0x96;
const uchar AbNORMAL_RESP_Froze_Cmd          = 0xD6;

const uchar EXCHANGE_RATE            = 0x17;
const uchar NORMAL_RESP_EXCHANGE_RATE   = 0x97;
const uchar AbNORMAL_RESP_EXCHANGE_RATE = 0xD7;

const uchar EXCHANGE_PASSWORD        = 0x18;
const uchar NORMAL_RESP_EXCHANGE_PASSWORD   = 0x98;
const uchar AbNORMAL_RESP_EXCHANGE_PASSWORD = 0xD8;

const uchar CLEAR_MAX_DEMAND         = 0x19;
const uchar NORMAL_RESP_CLEAR_MAX_DEMAND   = 0x99;
const uchar AbNORMAL_RESP_CLEAR_MAX_DEMAND = 0xD9;

const uchar CLEAR_METER              = 0x1A;
const uchar NORMAL_RESP_CLEAR_METER   = 0x9A;
const uchar AbNORMAL_RESP_CLEAR_METER = 0xDA;

const uchar CLEAR_EVNT               = 0x1B;
const uchar NORMAL_RESP_CLEAR_EVNT   = 0x9B;
const uchar AbNORMAL_RESP_CLEAR_EVNT = 0xDB;

const uchar TRIP_ALARM_KEEP          = 0x1C;
const uchar NORMAL_RESP_TRIP_ALARM_KEEP   = 0x9C;
const uchar AbNORMAL_RESP_TRIP_ALARM_KEEP = 0xDC;

const uchar MultiFuncTermPtptCtrlCmd          = 0x1D;
const uchar NORMAL_RESP_MultiFuncTermPtptCtrlCmd   = 0x9D;
const uchar AbNORMAL_RESP_MultiFuncTermPtptCtrlCmd = 0xDD;

const uchar SecurityAuthenticCmd          = 0x03;
const uchar NORMAL_RESP_SecurityAuthenticCmd   = 0x83;
const uchar AbNORMAL_RESP_SecurityAuthenticCmd = 0xC3;







const uchar SNODE_ANS_NORMAL         = 0x9C;
const uchar POWER_FUJIAN             = 0x81;


const uchar READ_DATA_ANHUI          = 0xFF;

const uchar CHK_BOX_FLAG             = 0x1E;
const uchar PLAT_IDENT_OR_BOX_FLAG   = 0x9E;

const uchar CHK_ML_ID = 0x1F;
const uchar RES_ML_ID = 0x9F;
const uchar ClkErrRpt_HuNan = 0x9F;


/******结束字符16H******/
const uchar END_FLAG_645 = 0x16;          //结束字符

const uchar ComAddr[4]={0x01,0x04,0x00,0x04};
//DI0-DI1-DI2-DI3
const uchar Volt_A[4]={0x00,0x01,0x01,0x02};
const uchar Volt_B[4]={0x00,0x02,0x01,0x02};
const uchar Volt_C[4]={0x00,0x03,0x01,0x02};
const uchar Volt_Blck[4]={0x00,0xFF,0x01,0x02};

const uchar Crnt_A[4]={0x00,0x01,0x02,0x02};
const uchar Crnt_B[4]={0x00,0x02,0x02,0x02};
const uchar Crnt_C[4]={0x00,0x03,0x02,0x02};
const uchar Crnt_Blck[4]={0x00,0xFF,0x02,0x02};
const uchar Crnt_ZeroLine[4]={0x01,0x00,0x80,0x02};

const uchar PhaseAngle_A[4]={0x00,0x01,0x07,0x02};
const uchar PhaseAngle_B[4]={0x00,0x02,0x07,0x02};
const uchar PhaseAngle_C[4]={0x00,0x03,0x07,0x02};
const uchar PhaseAngle_Blck[4]={0x00,0xFF,0x07,0x02};

const uchar ActPower_Total[4]={0x00,0x00,0x03,0x02};
const uchar ActPower_A[4]={0x00,0x01,0x03,0x02};
const uchar ActPower_B[4]={0x00,0x02,0x03,0x02};
const uchar ActPower_C[4]={0x00,0x03,0x03,0x02};
const uchar ActPower_Blck[4]={0x00,0xFF,0x03,0x02};

const uchar ReActPower_Total[4]={0x00,0x00,0x04,0x02};
const uchar ReActPower_A[4]={0x00,0x01,0x04,0x02};
const uchar ReActPower_B[4]={0x00,0x02,0x04,0x02};
const uchar ReActPower_C[4]={0x00,0x03,0x04,0x02};
const uchar ReActPower_Blck[4]={0x00,0xFF,0x04,0x02};

const uchar PowerFactor_Total[4]={0x00,0x00,0x06,0x02};
const uchar PowerFactor_A[4]={0x00,0x01,0x06,0x02};
const uchar PowerFactor_B[4]={0x00,0x02,0x06,0x02};
const uchar PowerFactor_C[4]={0x00,0x03,0x06,0x02};
const uchar PowerFactor_Blck[4]={0x00,0xFF,0x06,0x02};

const uchar CombActEne_Total[4]={0x00,0x00,0x00,0x00};
const uchar CombActEne_Blck[4]={0x00,0xFF,0x00,0x00};

const uchar PosActEne_Total[4]={0x00,0x00,0x01,0x00};
const uchar PosActEne_Blck[4]={0x00,0xFF,0x01,0x00};
const uchar readVendorInfo_Blck[4]={0x01,0x00,0x81,0x04};

const uchar NegActEne_Total[4]={0x00,0x00,0x02,0x00};
const uchar NegActEne_Blck[4]={0x00,0xFF,0x02,0x00};

const uchar CombReActEne1_Total[4]={0x00,0x00,0x03,0x00};

const uchar CombReActEne2_Total[4]={0x00,0x00,0x04,0x00};

const uchar ReActEneQuadrant1_Total[4]={0x00,0x00,0x05,0x00};

const uchar ReActEneQuadrant2_Total[4]={0x00,0x00,0x06,0x00};

const uchar ReActEneQuadrant3_Total[4]={0x00,0x00,0x07,0x00};

const uchar ReActEneQuadrant4_Total[4]={0x00,0x00,0x08,0x00};

const uchar ActDemand[4]={0x04,0x00,0x80,0x02};

const uchar ReActDemand[4]={0x05,0x00,0x80,0x02};



const uchar LineFreq[4]={0x02,0x00,0x80,0x02};


const uchar MeterType[4]={0x0B,0x04,0x00,0x04};
const uchar ActRptStatusWord[4]={0x01,0x15,0x00,0x04};
const uchar MeterRunStatusWord7[4]={0x07,0x05,0x00,0x04};
const uchar MeterRunStatusWordBlck[4]={0xFF,0x05,0x00,0x04};

const uchar DateAndWeek[4]={0x01,0x01,0x00,0x04};
const uchar Time645[4]={0x02,0x01,0x00,0x04};


const uchar CapOpenTimes[4]={0x00,0x0D,0x30,0x03};
const uchar PosActTotalMaxDemandList[4]={0xFF,0x00,0x01,0x01};

const uchar LoopDpdnc[4]={0x01,0x00,0xC3,0x03};

const uchar Last_DailyFre_Time[4]={0x01,0x00,0x06,0x05};
const uchar Last_DailyFre_ActEne[4]={0x01,0x01,0x06,0x05};
const uchar Last_DailyFre_NegActEne[4]={0x01,0x02,0x06,0x05};

const uchar CurvePrid_Class1[4]={0x02,0x0A,0x00,0x04};
const uchar EarliestCurveBlc_Total[4]={0x00,0x00,0x00,0x06};
const uchar GivenTimeCurveBlc_Total[4]={0x01,0x00,0x00,0x06};
const uchar LastCurveBlc_Total[4]={0x02,0x00,0x00,0x06};
const uchar EarliestCurveBlc_Class1[4]={0x00,0x00,0x01,0x06};
const uchar GivenTimeCurveBlc_Class1[4]={0x01,0x00,0x01,0x06};
const uchar LastCurveBlc_Class1[4]={0x02,0x00,0x01,0x06};

//2014版协议
const uchar GivenTimeCurveBlc_VoltA[4]={0x01,0x01,0x10,0x06};


struct DataDI
{
    uchar item[4];
};

struct AddrAndDataDI
{
    uchar addr[6];
    QList<DataDI> diList;
};



}


#endif // DATATYPEDEF_645_H
