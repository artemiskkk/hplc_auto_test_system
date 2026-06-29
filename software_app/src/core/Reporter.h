#ifndef REPORTER_H
#define REPORTER_H
//
// 测试报告：累积用例结果，导出 CSV（原始数据）/ HTML（带样式、便于查看）。
//
#include <QObject>
#include <QList>
#include "common/Model.h"

class Reporter : public QObject
{
    Q_OBJECT
public:
    explicit Reporter(QObject *parent = nullptr) : QObject(parent) {}

    void clear()                  { m_rows.clear(); }
    int  count() const            { return m_rows.size(); }
    const QList<CaseResult>& rows() const { return m_rows; }

    bool exportCsv (const QString &path, QString &err) const;
    bool exportHtml(const QString &path, QString &err) const;   // 列宽固定、UTF-8、结果着色

public slots:
    void add(const CaseResult &r) { m_rows << r; }

private:
    QList<CaseResult> m_rows;
};

#endif // REPORTER_H
