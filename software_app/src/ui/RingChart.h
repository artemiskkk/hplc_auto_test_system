#ifndef RINGCHART_H
#define RINGCHART_H
//
// 自绘环形图：可作进度环（单值）或多段统计环。不依赖 QtCharts。
//
#include <QWidget>
#include <QColor>
#include <QVector>

class RingChart : public QWidget
{
    Q_OBJECT
public:
    struct Seg { QColor color; double value; };

    explicit RingChart(QWidget *parent = nullptr);

    void setProgress(double percent, const QString &caption = "进度");
    void setSegments(const QVector<Seg> &segs, const QString &centerBig, const QString &centerSmall);
    void setThickness(int t) { m_thick = t; update(); }

    QSize sizeHint() const override { return QSize(150, 150); }

protected:
    void paintEvent(QPaintEvent *) override;

private:
    bool    m_progressMode = true;
    double  m_percent = 0.0;
    QString m_caption = "进度";
    QVector<Seg> m_segs;
    QString m_big, m_small;
    int     m_thick = 16;
};

#endif // RINGCHART_H
