#ifndef ABSTRACTSCRIPT_H
#define ABSTRACTSCRIPT_H

#include <QObject>
#include <QList>
#include <QMap>
#include "PublicDataStruct/commdatatype.h"
#include "PublicDataStruct/abstractscripthost.h"

class AbstractScript
{
public:
    virtual void  execute()=0;
    virtual void  stop()=0;
    virtual void  addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq)=0;
    virtual void  setHost(AbstractScriptHost *host)=0;
    virtual bool  config(const QMap<QString,QString> *paraDic)=0;
    virtual void  processMsg(DvcType dvcType,int id,uchar* data,int datalen) = 0;
    virtual void  processCtrlDvcRes(DvcType dvcType,QList<int> idList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params) = 0;
};

Q_DECLARE_INTERFACE(AbstractScript, "com.topscomm.AbstractScript")

#endif // ABSTRACTSCRIPT_H
