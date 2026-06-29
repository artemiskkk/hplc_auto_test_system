#ifndef ABSTRACTSCRIPTHOST_H
#define ABSTRACTSCRIPTHOST_H
#include <QObject>
#include "PublicDataStruct/commdatatype.h"
#include "PublicDataStruct/logitem.h"


Q_DECLARE_METATYPE(ProcessState)

class AbstractScriptHost
{
public:
   virtual void sendMsg2Dvc(DvcType dvcType,int id,uchar *data,int datalen)=0;
   virtual void updateProgress(ProcessState state,QString desc)=0;
   virtual void recordLog(LogItem *log)=0;
   virtual void controlDvc(DvcType dvcType,QList<int> idList,CtrlCmdType cmd,QList<double> params)=0;

};

#endif // ABSTRACTSCRIPTHOST_H
