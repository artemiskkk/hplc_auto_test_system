#ifndef LOGGER_H
#define LOGGER_H
//
// 简易日志器：运行日志 / 报文日志，单例，线程内使用。
//
#include <QObject>
#include <QString>
#include <QDateTime>
#include "PublicDataStruct/logitem.h"

class Logger : public QObject
{
    Q_OBJECT
public:
    static Logger& instance() { static Logger l; return l; }

    void log(LogModule mod, LogCatalogue cat, const QString &msg)
    {
        const QString ts = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz");
        emit lineLogged(ts, mod, cat, normalizeMessage(msg));
    }
    void info(const QString &m)  { log(Module_SYSTEM, Log_Info,  m); }
    void error(const QString &m) { log(Module_SYSTEM, Log_Error, m); }
    void debug(const QString &m) { log(Module_SYSTEM, Log_Debug, m); }
    void msg(const QString &m)   { log(Module_CCO,    Log_Trace, m); } // 报文

    void record(LogItem *item)
    {
        if (!item) return;
        const QString ts = item->TimeStamp.isEmpty()
            ? QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz")
            : item->TimeStamp;
        emit lineLogged(ts, item->Module, item->Catalogue,
                        normalizeMessage(item->Header + item->LogMsg));
    }

signals:
    void lineLogged(const QString &ts, int module, int catalogue, const QString &msg);

private:
    Logger() {}

    static QString normalizeMessage(QString msg)
    {
        msg.replace("\\r\\n", " ");
        msg.replace("\\r", " ");
        msg.replace("\\n", " ");
        msg.replace("\r\n", " ");
        msg.replace('\r', ' ');
        msg.replace('\n', ' ');
        return msg.trimmed();
    }
};

#endif // LOGGER_H
