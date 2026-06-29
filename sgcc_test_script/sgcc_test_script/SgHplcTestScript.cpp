#include "SgHplcTestScript.h"

SgHplcTestScript::SgHplcTestScript(QObject *parent) : QObject(parent)
{
    p_AbstractScriptHost = nullptr;

    AbstractScript_ptr = nullptr;

    p_ConcentratorInfoList = nullptr;
    p_MeterInfoList = nullptr;
    p_SchemeCfgInfoList = nullptr;
    freq = 2;

    scriptName = QString("");
    paraDic = nullptr;
}

SgHplcTestScript::~SgHplcTestScript()
{
    AbstractScript_ptr = nullptr;
}

void  SgHplcTestScript::execute()
{
    getCurrentScript(this->scriptName);

    if((AbstractScript_ptr == nullptr) || (p_AbstractScriptHost == nullptr))
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Error, QString("脚本加载失败：未找到执行类"));
        return;
    }

    AbstractScript_ptr->setHost(p_AbstractScriptHost);
    AbstractScript_ptr->addAddrsInfo(p_ConcentratorInfoList,p_MeterInfoList,p_SchemeCfgInfoList,freq);
    AbstractScript_ptr->config(paraDic);

    AbstractScript_ptr->execute();
}

void  SgHplcTestScript::stop()
{
    if((AbstractScript_ptr == nullptr) || (p_AbstractScriptHost == nullptr))
    {
        p_AbstractScriptHost->updateProgress(ProcessState_Error, QString("脚本中止失败：未找到执行类"));
        return;
    }

    AbstractScript_ptr->stop();


    AbstractScript_ptr = nullptr;
}

void SgHplcTestScript::addAddrsInfo(const QList<ConcentratorInfo> *p_ConcentratorInfoList, const QList<MeterInfo> *p_MeterInfoList, const QList<SchemeCfgInfo> *p_SchemeCfgInfoList, const int freq)
{
    this->p_ConcentratorInfoList = p_ConcentratorInfoList;
    this->p_MeterInfoList = p_MeterInfoList;
    this->p_SchemeCfgInfoList = p_SchemeCfgInfoList;
    this->freq = freq;
}

void  SgHplcTestScript::setHost(AbstractScriptHost *host)
{
    this->p_AbstractScriptHost=host;

}

bool  SgHplcTestScript::config(const QMap<QString,QString> *paraDic)
{
    this->paraDic = paraDic;

    return true;
}

void SgHplcTestScript::processMsg(DvcType dvcType, int id, uchar *data, int datalen)
{
    if(AbstractScript_ptr == nullptr)
        return;

    AbstractScript_ptr->processMsg(dvcType, id, data, datalen);
}

void SgHplcTestScript::processCtrlDvcRes(DvcType dvcType, QList<int> idList, CtrlCmdType ctrlCmdType, bool isSucs, QList<double> params)
{
    if(AbstractScript_ptr == nullptr)
        return;

    AbstractScript_ptr->processCtrlDvcRes(dvcType, idList, ctrlCmdType, isSucs, params);
}

void SgHplcTestScript::setScript(const QString scriptName)
{
    this->scriptName = scriptName;
}

void SgHplcTestScript::unLoadScript()
{
    AbstractScript_ptr = nullptr;
}

void SgHplcTestScript::getCurrentScript(QString scriptName)
{
    AbstractScript_ptr = nullptr;

    //AbstractScript* p_AbstractScript =  reinterpret_cast<AbstractScript*>(ReflectFactory::createObject(scriptName.toLatin1()));
    AbstractScript_ptr =  ReflectFactory::newInstance(scriptName.toLatin1());

}
