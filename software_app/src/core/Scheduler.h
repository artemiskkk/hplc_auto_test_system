#ifndef SCHEDULER_H
#define SCHEDULER_H
//
// 调度：方案/参数装配、启动用例、重复测试、结果判定。
//
#include <QObject>
#include "common/Model.h"

class ScriptHost;

class Scheduler : public QObject
{
    Q_OBJECT
public:
    explicit Scheduler(QObject *parent = nullptr);

    void setDatabase(Database *db)   { m_db = db; }
    void setHost(ScriptHost *h);
    void setScriptParaDir(const QString &d) { m_paraDir = d; }
    void setFreqEncoded(int f)       { m_freq = f; }
    void setTotalTimes(int n)        { m_totalTimes = qMax(1, n); }

    // 启动指定用例（按 tcID）。
    bool runCase(int tcID, QString &err);
    void stop();
    bool isRunning() const { return m_running; }

signals:
    void caseStarted(int tcID, const QString &name, int runIndex, int totalTimes);
    void caseProgress(int tcID, int state, const QString &desc);
    void caseFinished(int tcID, int result /*TestCaseResult*/, int runIndex);
    void caseResult(const CaseResult &r);   // 完整结果（供报告/回显）

private slots:
    void onHostProgress(int state, const QString &desc);

private:
    bool assembleAndStart(QString &err);

    Database   *m_db = nullptr;
    ScriptHost *m_host = nullptr;
    QString     m_paraDir;
    int         m_freq = 2;
    int         m_totalTimes = 1;

    bool        m_running = false;
    TestCaseInfo m_cur;
    int          m_runIndex = 0;
};

#endif // SCHEDULER_H
