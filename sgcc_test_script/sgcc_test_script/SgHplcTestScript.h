#ifndef SG_HPLC_TEST_SCRIPT_GW_H
#define SG_HPLC_TEST_SCRIPT_GW_H

#include <QObject>
#include <QDebug>
#include <QtCore/qglobal.h>

#include "PublicDataStruct/AbstractPluginScript.h"
//#include "PublicDataStruct/abstractscript.h"
#include "PublicDataStruct/abstractscripthost.h"
#include "PublicDataStruct/commdatatype.h"

#include "ReflectFactory.h"


class SgHplcTestScript : public QObject, public AbstractPluginScript
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.topscomm.AbstractPluginScript")
    Q_INTERFACES(AbstractPluginScript)
public:
    explicit SgHplcTestScript(QObject *parent = nullptr);
    ~SgHplcTestScript();

    AbstractScriptHost *p_AbstractScriptHost;

    std::shared_ptr<AbstractScript> AbstractScript_ptr;
    //AbstractScript *AbstractScript_ptr;

    const QList<ConcentratorInfo> *p_ConcentratorInfoList;
    const QList<MeterInfo> *p_MeterInfoList;
    const QList<SchemeCfgInfo> *p_SchemeCfgInfoList;
    int freq;
    QString scriptName;
    const QMap<QString,QString> *paraDic;

public:
    void  execute();
    void  stop();
    void  addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq);
    void  setHost(AbstractScriptHost *host);
    bool  config(const QMap<QString,QString> *paraDic);
    void  processMsg(DvcType dvcType,int id,uchar* data,int datalen);
    void  processCtrlDvcRes(DvcType dvcType,QList<int> idList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params);
    void  setScript(const QString scriptName);
    void  unLoadScript();

private:
    void getCurrentScript(QString scriptName);

};

#endif // SG_HPLC_TEST_SCRIPT_GW_H
