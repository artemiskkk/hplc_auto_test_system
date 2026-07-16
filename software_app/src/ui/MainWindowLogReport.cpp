// ============================================================================
// MainWindow 的「运行日志」与「测试报告」相关实现（从 MainWindow.cpp 拆出）。
//   - 运行日志：UI 日志表追加、逐用例日志文件（句柄常开）、清空。
//   - 测试报告：CSV 生成（test_report/）、查看、导出。
// 均为 MainWindow 的成员函数，分散到本翻译单元，不影响类定义。
// ============================================================================
#include "ui/MainWindow.h"
#include "ui/Theme.h"
#include "core/Reporter.h"
#include "common/Logger.h"

#include <QtWidgets>
#include <QCoreApplication>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QFileInfo>
#include <QDate>
#include <QDateTime>
#include <QProcess>
#include <QDesktopServices>
#include <QUrl>
#include <QRegExp>
#include <QStringList>

// ===================== 文件内小工具（路径/文件名） =====================
namespace {

QString safeFileName(const QString &name)
{
    QString v = name.trimmed();
    if (v.isEmpty())
        v = "未命名测试项";

    const QString invalid = "\\/:*?\"<>|";
    for (int i = 0; i < v.size(); ++i) {
        const QChar ch = v.at(i);
        if (invalid.contains(ch) || ch.unicode() < 32)
            v[i] = '_';
    }

    while (!v.isEmpty() && (v.endsWith('.') || v.endsWith(' ')))
        v.chop(1);
    if (v.isEmpty())
        v = "未命名测试项";
    return v.left(80);
}

QStringList logCategoryDirs(const QString &catalogueName)
{
    QString cat = catalogueName.trimmed();
    if (cat.isEmpty())
        cat = "未分类";

    QStringList dirs;
    const QStringList parts = cat.split(QRegExp("\\s*[-－—]+\\s*"), QString::SkipEmptyParts);
    for (const QString &part : parts) {
        const QString dir = safeFileName(part);
        if (!dir.isEmpty())
            dirs << dir;
    }
    if (dirs.isEmpty())
        dirs << "未分类";
    return dirs;
}

QString uniqueFilePath(const QDir &dir, const QString &baseName, const QString &suffix)
{
    QString path = dir.filePath(baseName + suffix);
    if (!QFileInfo::exists(path))
        return path;

    const QString stamp = QDateTime::currentDateTime().toString("HHmmss");
    path = dir.filePath(baseName + "_" + stamp + suffix);
    int idx = 2;
    while (QFileInfo::exists(path)) {
        path = dir.filePath(QString("%1_%2_%3%4").arg(baseName).arg(stamp).arg(idx).arg(suffix));
        ++idx;
    }
    return path;
}

// 返回 test_report/ 下的报告基路径（不含扩展名），html/csv 共用同一基名
QString reportBasePath(bool unique)
{
    QDir appDir(QCoreApplication::applicationDirPath());
    appDir.mkpath("test_report");
    QDir reportDir(appDir.filePath("test_report"));
    const QString baseName = "hplc_检测报告_" + QDate::currentDate().toString("yyyy-MM-dd");
    if (!unique)
        return reportDir.filePath(baseName);
    QString p = uniqueFilePath(reportDir, baseName, ".html");   // 以 .html 判重
    p.chop(5);                                                  // 去掉 ".html"，返回基路径
    return p;
}

QString caseLogPath(const QString &catalogueName, const QString &caseName, const QString &stamp)
{
    QDir appDir(QCoreApplication::applicationDirPath());
    QString logDirPath = "logs";
    for (const QString &dir : logCategoryDirs(catalogueName))
        logDirPath += "/" + dir;

    const QString safeName = safeFileName(caseName);
    appDir.mkpath(logDirPath + "/" + safeName);
    QDir caseDir(appDir.filePath(logDirPath + "/" + safeName));
    // 文件名带运行时间戳，避免每次运行覆盖历史日志（同一批次内多轮仍写同一文件以便追加）
    const QString suffix = stamp.isEmpty()
                           ? QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") : stamp;
    return caseDir.filePath(safeName + "_" + suffix + ".log");
}

QString normalizeLogText(QString text)
{
    text.replace("\\r\\n", " ");
    text.replace("\\r", " ");
    text.replace("\\n", " ");
    text.replace("\r\n", " ");
    text.replace('\r', ' ');
    text.replace('\n', ' ');
    return text.trimmed();
}

} // namespace

// ===================== 运行日志：文件句柄 =====================

// 打开本用例运行日志文件并保持句柄常开（重复轮次追加）。由 onCaseStarted 调用。
void MainWindow::openCaseLog(int tcID, const QString &name, const QString &catalogue, int runIdx, int total)
{
    const QString path = caseLogPath(catalogue, name, m_runStamp);
    const bool appendRun = runIdx > 1 && QFileInfo::exists(path);
    closeCaseLog();                       // 关掉上一个用例的句柄
    m_logFilePath = path;
    m_logFile = new QFile(m_logFilePath);
    QIODevice::OpenMode mode = QIODevice::WriteOnly | QIODevice::Text
                             | (appendRun ? QIODevice::Append : QIODevice::Truncate);
    if (m_logFile->open(mode)) {
        QTextStream out(m_logFile);
        out.setCodec("UTF-8");
        out.setGenerateByteOrderMark(!appendRun);
        if (appendRun) out << '\n';
        out << QString::fromUtf8("测试项: ") << name << '\n';
        out << QString::fromUtf8("所属目录: ")
            << (catalogue.trimmed().isEmpty() ? QString::fromUtf8("未分类") : catalogue.trimmed()) << '\n';
        out << QString::fromUtf8("用例编号: ") << QString("TC_%1").arg(tcID, 5, 10, QChar('0')) << '\n';
        out << QString::fromUtf8("开始时间: ") << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << '\n';
        out << QString::fromUtf8("执行轮次: ") << runIdx << "/" << total << '\n';
        out << "----------------------------------------\n";
        out.flush();
        m_logFile->flush();
        m_lastLogPath = m_logFilePath;    // 关闭后仍可"查看日志"定位到它
    } else {
        Logger::instance().error("运行日志文件创建失败: " + m_logFilePath);
        delete m_logFile; m_logFile = nullptr; m_logFilePath.clear();
    }
    if (m_logFile)
        Logger::instance().info("运行日志文件: " + m_logFilePath);
}

void MainWindow::closeCaseLog()
{
    if (m_logFile) {
        if (m_logFile->isOpen()) m_logFile->close();
        delete m_logFile;
        m_logFile = nullptr;
    }
    m_logFilePath.clear();
}

void MainWindow::appendLogFile(const QString &ts, const QString &level, const QString &source, const QString &msg)
{
    if (!m_logFile || !m_logFile->isOpen())
        return;

    const QString line = normalizeLogText(msg);

    QTextStream out(m_logFile);
    out.setCodec("UTF-8");
    out << "[" << ts << "] "
        << "[" << level << "] "
        << "[" << source << "] "
        << line << '\n';
    out.flush();
    m_logFile->flush();               // 落盘，避免崩溃丢日志（仍远比每行 open/close 轻）
}

// ===================== 运行日志：UI 表 =====================
void MainWindow::appendLog(const QString &ts, int module, int /*catalogue*/, const QString &msg)
{
    const QString cleanMsg = normalizeLogText(msg);
    QString level = "INFO"; QColor color("#1F2A37");
    if (msg.startsWith(">>") || msg.contains("发送")) { level = "TX"; color = QColor(Theme::GREEN); }
    else if (msg.startsWith("<<") || msg.contains("接收")) { level = "RX"; color = QColor(Theme::BLUE_RX); }
    else if (msg.contains("[controlDvc") || msg.contains("[断路器")) { level = "CTRL"; color = QColor(Theme::RED); }
    else if (msg.contains("失败") || msg.contains("错误") || msg.contains("超时")) { level = "ERROR"; color = QColor(Theme::RED); }
    else if (msg.contains("警告")) { level = "WARN"; color = QColor(Theme::ORANGE); }

    const char *src[] = {"其他","系统","脚本","CCO","STA","CJQ"};
    const QString source = (module >= 0 && module < 6) ? src[module] : "系统";
    appendLogFile(ts, level, source, cleanMsg);

    QString shortTs = ts;
    if (shortTs.size() >= 19 && shortTs.at(10) == ' ')
        shortTs = shortTs.mid(11, 8);

    int row = m_logTable->rowCount();
    m_logTable->insertRow(row);
    QTableWidgetItem *i0 = new QTableWidgetItem(shortTs);
    QTableWidgetItem *i1 = new QTableWidgetItem(cleanMsg);
    i0->setForeground(QColor("#8A97A6"));
    i0->setToolTip(QString("%1   级别:%2  来源:%3").arg(ts, level, source));
    i1->setForeground(color);
    i1->setToolTip(QString("[%1/%2] %3").arg(level, source, cleanMsg));
    i1->setData(Qt::UserRole, level);
    m_logTable->setItem(row, 0, i0);
    m_logTable->setItem(row, 1, i1);
    // 限行：超过上限时一次性批量裁掉一批（而非每行 removeRow(0)，那样每行都要把后面所有行上移 O(n)，
    // 高频日志下很拖慢）。批量删一次，约每 256 行才触发一次。
    const int kMaxLogRows = 2000;
    if (m_logTable->rowCount() > kMaxLogRows)
        m_logTable->model()->removeRows(0, m_logTable->rowCount() - kMaxLogRows + 256);
    m_logTable->scrollToBottom();

    m_logCount++;
    m_sbLogCount->setText(QString("日志: %1 条").arg(m_logCount));
}

void MainWindow::onClearLog()
{
    m_logTable->setRowCount(0);
    m_logCount = 0;
    m_sbLogCount->setText("日志: 0 条");
}

// 打开运行日志存储目录：优先在资源管理器中定位到当前/最近的日志文件，
// 否则打开 logs 根目录（exe 同级）。
void MainWindow::onViewLog()
{
    QDir appDir(QCoreApplication::applicationDirPath());
    const QString logsRoot = appDir.filePath("logs");
    QDir().mkpath(logsRoot);

    QString target = (!m_logFilePath.isEmpty() && QFileInfo::exists(m_logFilePath))
                     ? m_logFilePath : m_lastLogPath;

    if (!target.isEmpty() && QFileInfo::exists(target)) {
        const QString nativePath = QDir::toNativeSeparators(target);
        if (QProcess::startDetached("explorer.exe", QStringList() << ("/select," + nativePath)))
            return;
    }
    // 没有具体文件就打开 logs 目录
    if (!QProcess::startDetached("explorer.exe", QStringList() << QDir::toNativeSeparators(logsRoot)))
        QDesktopServices::openUrl(QUrl::fromLocalFile(logsRoot));
}

// ===================== 测试报告 =====================

// 测试完成后在 exe 同级 test_report 目录生成 CSV 测试结果（可在 Excel 打开）
void MainWindow::writeAutoReport()
{
    if (m_reporter->count() == 0) return;
    const QString base = reportBasePath(true);

    QString err;
    if (m_reporter->exportCsv(base + ".csv", err)) {
        m_lastReportPath = base + ".csv";
        Logger::instance().info("测试报告已生成: " + m_lastReportPath);
    } else {
        Logger::instance().error("测试报告生成失败: " + err);
    }
}

void MainWindow::onBrowseReport()
{
    if (m_reporter->count() == 0) { QMessageBox::information(this, "查看报告", "暂无测试结果"); return; }

    if (m_lastReportPath.isEmpty() || !QFileInfo::exists(m_lastReportPath))
        writeAutoReport();

    // 在资源管理器中打开报告目录，并选中最新生成的 CSV
    QDir appDir(QCoreApplication::applicationDirPath());
    const QString reportDir = appDir.filePath("test_report");
    QDir().mkpath(reportDir);

    if (!m_lastReportPath.isEmpty() && QFileInfo::exists(m_lastReportPath)) {
        const QString nativePath = QDir::toNativeSeparators(m_lastReportPath);
        if (QProcess::startDetached("explorer.exe", QStringList() << ("/select," + nativePath)))
            return;
    }
    if (!QProcess::startDetached("explorer.exe", QStringList() << QDir::toNativeSeparators(reportDir)))
        QDesktopServices::openUrl(QUrl::fromLocalFile(reportDir));
}

void MainWindow::onExportReport()
{
    if (m_reporter->count() == 0) { QMessageBox::information(this, "导出报告", "暂无测试结果"); return; }
    QString path = QFileDialog::getSaveFileName(this, "导出报告",
        reportBasePath(false) + ".csv", "CSV 数据 (*.csv)");
    if (path.isEmpty()) return;

    if (!path.endsWith(".csv", Qt::CaseInsensitive)) path += ".csv";
    QString err;
    if (m_reporter->exportCsv(path, err)) QMessageBox::information(this, "导出报告", "已导出:\n" + path);
    else QMessageBox::warning(this, "导出报告", err);
}
