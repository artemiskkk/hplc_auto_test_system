#include "core/Scheduler.h"
#include "host/ScriptHost.h"
#include "core/ConfigLoader.h"
#include "common/Logger.h"
#include <QDir>
#include <QDateTime>
#include <QFileInfo>

Scheduler::Scheduler(QObject *parent) : QObject(parent) {}

void Scheduler::setHost(ScriptHost *h)
{
    m_host = h;
    if (m_host)
        connect(m_host, &ScriptHost::progress,
                this, &Scheduler::onHostProgress, Qt::QueuedConnection);
}

bool Scheduler::runCase(int tcID, QString &err)
{
    if (!m_db || !m_host) { err = "调度未就绪"; return false; }
    if (m_running) { err = "已有用例在运行"; return false; }

    const TestCaseInfo *tc = nullptr;
    for (const auto &c : m_db->cases) if (c.tcID == tcID) { tc = &c; break; }
    if (!tc) { err = QString("未找到用例 tcID=%1").arg(tcID); return false; }

    m_cur = *tc;
    m_runIndex = 1;
    m_running = true;
    return assembleAndStart(err);
}

bool Scheduler::assembleAndStart(QString &err)
{
    // 取方案：用 schmIDs 第一个；缺方案则回退为 首个集中器 + 全部电表
    QList<ConcentratorInfo> conc;
    QList<MeterInfo> meter;
    QList<SchemeCfgInfo> scheme;

    int schmID = m_cur.schmIDs.isEmpty() ? -1 : *m_cur.schmIDs.begin();
    const SchemeCfgInfo *s = (schmID >= 0) ? m_db->schemeById(schmID) : nullptr;
    int matchedMeterCount = 0;
    if (s) {
        scheme << *s;
        if (const ConcentratorInfo *c = m_db->concById(s->ctrID)) conc << *c;
        for (int mid : s->mtrIDs) {
            if (const MeterInfo *m = m_db->meterById(mid)) {
                meter << *m;
                ++matchedMeterCount;
            }
        }
    }
    if (conc.isEmpty() && !m_db->concentrators.isEmpty()) conc << m_db->concentrators.first();
    if (meter.isEmpty()) meter = m_db->meters;   // 回退：全部电表

    if (conc.isEmpty()) { err = "无可用集中器档案"; m_running = false; return false; }

    // 关键：脚本 comnAddAddrsInfo 按 scheme.mtrIDs 组装“单网表列表”，只认方案里列出的电表ID。
    // 若方案缺失或一个电表都没匹配上，即便上面回退了全部电表，脚本仍拿到 0 块表（“没检测到表档案”，
    // 组网探测里表现为“总规模应为1”）。故此处用最终 meter 列表的全部电表ID补全 scheme.mtrIDs，
    // 并把 ctrID 对齐到传入集中器，保证 comnAddAddrsInfo 能建出 CtrInfo。
    if (matchedMeterCount == 0 && !meter.isEmpty()) {
        if (scheme.isEmpty()) scheme << SchemeCfgInfo();
        scheme[0].schmID = (schmID > 0 ? schmID : 1);
        scheme[0].ctrID  = conc.first().ctrID;
        scheme[0].mtrIDs.clear();
        for (const MeterInfo &m : meter) scheme[0].mtrIDs << m.mtrID;
        Logger::instance().info(QString("方案未匹配到电表，已用全部已加载电表(%1 块)补全方案，避免脚本读不到表档案")
                                .arg(meter.size()));
    }

    if (meter.isEmpty())
        Logger::instance().error("电表档案为空：请检查 DataBase/MeterInfo.csv 是否存在且已正确加载");

    // 参数文件
    QMap<QString,QString> para;
    if (!m_cur.paramFileName.isEmpty()) {
        QString pf = QDir(m_paraDir).filePath(m_cur.paramFileName);
        if (!QFileInfo::exists(pf) && !pf.endsWith(".txt", Qt::CaseInsensitive))
            pf += ".txt";
        para = ConfigLoader::loadParaFile(pf);
        if (para.isEmpty())
            Logger::instance().error("脚本参数文件为空或读取失败: " + pf);
    }

    const QString cls = ModelUtil::scriptClassName(m_cur.tcDLLName);
    m_cur.startTime = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    emit caseStarted(m_cur.tcID, m_cur.tcName, m_runIndex, m_totalTimes);
    Logger::instance().info(QString("==== 用例[%1] %2  第 %3/%4 次 ====")
                            .arg(m_cur.tcID).arg(m_cur.tcName).arg(m_runIndex).arg(m_totalTimes));

    if (!m_host->runCase(cls, conc, meter, scheme, m_freq, para, err)) {
        m_running = false;
        return false;
    }
    return true;
}

void Scheduler::onHostProgress(int state, const QString &desc)
{
    if (!m_running) return;
    emit caseProgress(m_cur.tcID, state, desc);

    int result = -1;
    if (state == ProcessState_Success) result = TestCaseResult_PASS;
    else if (state == ProcessState_Failed) result = TestCaseResult_FAIL;
    else if (state == ProcessState_Error)  result = TestCaseResult_ERROR;
    if (result < 0) return;   // 过程态，不判定

    m_cur.endTime = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    m_cur.result = static_cast<TestCaseResult>(result);
    m_cur.resultInfo = desc;
    emit caseFinished(m_cur.tcID, result, m_runIndex);

    CaseResult cr;
    cr.tcID = m_cur.tcID;
    cr.name = m_cur.tcName;
    cr.catalogue = m_cur.catalogueName;
    cr.scriptClass = ModelUtil::scriptClassName(m_cur.tcDLLName);
    cr.startTime = m_cur.startTime;
    cr.endTime = m_cur.endTime;
    cr.result = result;
    cr.runIndex = m_runIndex;
    cr.totalTimes = m_totalTimes;
    cr.info = desc;
    emit caseResult(cr);
    Logger::instance().info(QString("==== 用例[%1] 第 %2 次结果：%3 ====")
                            .arg(m_cur.tcID).arg(m_runIndex)
                            .arg(result == TestCaseResult_PASS ? "PASS"
                                 : result == TestCaseResult_FAIL ? "FAIL" : "ERROR"));

    // 重复测试
    if (m_runIndex < m_totalTimes) {
        m_runIndex++;
        QString err;
        if (!assembleAndStart(err)) {
            Logger::instance().error(QString("重复测试启动失败：%1").arg(err));
            m_running = false;
        }
    } else {
        m_running = false;
    }
}

void Scheduler::stop()
{
    if (m_host) m_host->stopCase();
    m_running = false;
}
