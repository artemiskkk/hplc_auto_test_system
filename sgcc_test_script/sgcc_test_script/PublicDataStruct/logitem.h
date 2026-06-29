#ifndef LOGITEM_H
#define LOGITEM_H

#include <QString>

enum LogCatalogue
{
    Log_Debug,
    Log_Trace,
    Log_Info,
    Log_Error
};

enum LogModule
{
    Module_OTHER,
    Module_SYSTEM,
    Module_SCRIPT,
    Module_CCO,
    Module_STA,
    Module_CJQ
};

enum LogType
{
    Log_SystemType,
    Log_ComuMsgType
};

class LogItem
{
public:
    LogItem() {}
    LogCatalogue Catalogue;
    LogModule Module;
    QString LogMsg;
    QString Header;
    QString TimeStamp;
};

#endif // LOGITEM_H
