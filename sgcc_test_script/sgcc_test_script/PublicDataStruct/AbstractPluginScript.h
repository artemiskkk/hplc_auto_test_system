#ifndef ABSTRACTPLUGINSCRIPT_H
#define ABSTRACTPLUGINSCRIPT_H

#include <QObject>
#include <QList>
#include <QMap>
#include "PublicDataStruct/abstractscript.h"

class AbstractPluginScript :public AbstractScript
{
public:
    virtual void  setScript(const QString scriptName)=0;
    virtual void  unLoadScript()=0;
};

Q_DECLARE_INTERFACE(AbstractPluginScript, "com.topscomm.AbstractPluginScript")

#endif // ABSTRACTPLUGINSCRIPT_H
