#include "core/Reporter.h"
#include <QFile>
#include <QTextStream>
#include <QDateTime>

static QString csvEscape(const QString &s)
{
    QString v = s;
    if (v.contains(',') || v.contains('"') || v.contains('\n') || v.contains('\r')) {
        v.replace('"', "\"\"");
        v = '"' + v + '"';
    }
    return v;
}

static QString htmlEscape(const QString &s)
{
    QString v = s;
    v.replace('&', "&amp;").replace('<', "&lt;").replace('>', "&gt;").replace('"', "&quot;");
    return v;
}

bool Reporter::exportCsv(const QString &path, QString &err) const
{
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        err = QString("无法写入：%1").arg(path);
        return false;
    }
    QTextStream ts(&f);
    ts.setCodec("UTF-8");
    ts.setGenerateByteOrderMark(true);   // UTF-8 BOM，便于 Excel 识别中文
    // 注意：QTextStream << const char* 会按 Latin1 处理中文导致乱码，表头必须用 QString 写出。
    ts << QString::fromUtf8("用例编号,用例名称,分类,脚本类,开始时间,结束时间,结果,执行轮次,说明\n");
    for (const CaseResult &r : m_rows) {
        ts << r.tcID << ','
           << csvEscape(r.name) << ','
           << csvEscape(r.catalogue) << ','
           << csvEscape(r.scriptClass) << ','
           << csvEscape(r.startTime) << ','
           << csvEscape(r.endTime) << ','
           << r.resultText() << ','
           // "第N次/共M次"：带中文，避免 Excel 把 "1/1" 误识别成日期
           << QString::fromUtf8("第%1/%2次").arg(r.runIndex).arg(r.totalTimes) << ','
           << csvEscape(r.info) << '\n';
    }
    f.close();
    return true;
}

bool Reporter::exportHtml(const QString &path, QString &err) const
{
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        err = QString("无法写入：%1").arg(path);
        return false;
    }
    int total = m_rows.size(), pass = 0, fail = 0, error = 0;
    for (const CaseResult &r : m_rows) {
        if (r.result == 0) ++pass; else if (r.result == 1) ++fail; else if (r.result == 2) ++error;
    }
    const double rate = total > 0 ? pass * 100.0 / total : 0.0;

    QTextStream ts(&f);
    ts.setCodec("UTF-8");
    // HTML 用 meta 声明编码即可，不写 BOM；含中文的字面量统一走 QString 避免 Latin1 乱码。

    ts << "<!DOCTYPE html>\n<html lang=\"zh-CN\"><head><meta charset=\"UTF-8\">\n";
    ts << QString::fromUtf8("<title>国网 HPLC 自动化检测报告</title>\n");
    ts << "<style>\n"
          "body{font-family:'Microsoft YaHei','SimHei',sans-serif;font-size:13px;color:#1f2a37;margin:20px;}\n"
          "h2{color:#14395e;margin:0 0 6px;}\n"
          ".sum{margin:8px 0 16px;font-size:14px;color:#44505f;}\n"
          ".sum b{font-size:18px;color:#14395e;}\n"
          "table{border-collapse:collapse;width:100%;table-layout:fixed;}\n"
          "th,td{border:1px solid #d2dce8;padding:6px 10px;text-align:left;vertical-align:top;"
          "word-break:break-all;white-space:pre-wrap;}\n"
          "th{background:#eef2f7;color:#14395e;white-space:nowrap;}\n"
          "tr:nth-child(even) td{background:#fafcff;}\n"
          "td.pass{color:#1e8e3e;font-weight:bold;}td.fail{color:#d14343;font-weight:bold;}"
          "td.err{color:#e08a1e;font-weight:bold;}\n"
          "</style></head><body>\n";

    ts << QString::fromUtf8("<h2>国网 HPLC 自动化检测报告</h2>\n");
    ts << QString::fromUtf8(
              "<div class=\"sum\">用例总数 <b>%1</b> &nbsp;|&nbsp; "
              "<span style='color:#1e8e3e'>通过 %2</span> &nbsp; "
              "<span style='color:#d14343'>失败 %3</span> &nbsp; "
              "<span style='color:#e08a1e'>错误 %4</span> &nbsp;|&nbsp; "
              "通过率 <b>%5%</b> &nbsp;|&nbsp; 生成时间 %6</div>\n")
          .arg(total).arg(pass).arg(fail).arg(error)
          .arg(QString::number(rate, 'f', 1))
          .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"));

    // 固定各列宽度，长说明自动换行
    ts << "<table>\n<colgroup>"
          "<col style='width:56px'><col style='width:200px'><col style='width:110px'>"
          "<col style='width:200px'><col style='width:150px'><col style='width:150px'>"
          "<col style='width:64px'><col style='width:84px'><col>"
          "</colgroup>\n";
    ts << QString::fromUtf8("<tr><th>编号</th><th>用例名称</th><th>分类</th><th>脚本类</th>"
                           "<th>开始时间</th><th>结束时间</th><th>结果</th><th>执行轮次</th><th>说明</th></tr>\n");
    for (const CaseResult &r : m_rows) {
        const char *cls = (r.result == 0) ? "pass" : (r.result == 1) ? "fail" : "err";
        ts << "<tr>"
           << "<td>" << r.tcID << "</td>"
           << "<td>" << htmlEscape(r.name) << "</td>"
           << "<td>" << htmlEscape(r.catalogue) << "</td>"
           << "<td>" << htmlEscape(r.scriptClass) << "</td>"
           << "<td>" << htmlEscape(r.startTime) << "</td>"
           << "<td>" << htmlEscape(r.endTime) << "</td>"
           << "<td class='" << cls << "'>" << r.resultText() << "</td>"
           << "<td>" << r.runIndex << " / " << r.totalTimes << "</td>"
           << "<td>" << htmlEscape(r.info) << "</td>"
           << "</tr>\n";
    }
    ts << "</table>\n</body></html>\n";
    f.close();
    return true;
}
