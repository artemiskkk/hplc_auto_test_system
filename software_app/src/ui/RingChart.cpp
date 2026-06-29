#include "ui/RingChart.h"
#include <QPainter>
#include <QPaintEvent>

RingChart::RingChart(QWidget *parent) : QWidget(parent)
{
    setMinimumSize(120, 120);
}

void RingChart::setProgress(double percent, const QString &caption)
{
    m_progressMode = true;
    m_percent = qBound(0.0, percent, 100.0);
    m_caption = caption;
    update();
}

void RingChart::setSegments(const QVector<Seg> &segs, const QString &centerBig, const QString &centerSmall)
{
    m_progressMode = false;
    m_segs = segs;
    m_big = centerBig;
    m_small = centerSmall;
    update();
}

void RingChart::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const int side = qMin(width(), height());
    const int margin = m_thick / 2 + 4;
    QRectF rect((width() - side) / 2.0 + margin, (height() - side) / 2.0 + margin,
                side - 2 * margin, side - 2 * margin);

    QPen track(QColor("#EAEFF5"));
    track.setWidth(m_thick);
    track.setCapStyle(Qt::FlatCap);

    // 底环
    p.setPen(track);
    p.drawArc(rect, 0, 360 * 16);

    if (m_progressMode) {
        QPen arc(QColor("#1769B0"));
        arc.setWidth(m_thick);
        arc.setCapStyle(Qt::RoundCap);
        p.setPen(arc);
        const int span = static_cast<int>(-m_percent / 100.0 * 360 * 16);
        p.drawArc(rect, 90 * 16, span);   // 从 12 点顺时针

        p.setPen(QColor("#14395E"));
        QFont f = font(); f.setPointSize(18); f.setBold(true); p.setFont(f);
        p.drawText(rect, Qt::AlignCenter, QString::number(qRound(m_percent)) + "%");
        QFont f2 = font(); f2.setPointSize(9); f2.setBold(false); p.setFont(f2);
        p.setPen(QColor("#5B6675"));
        QRectF capRect = rect.adjusted(0, rect.height() * 0.62, 0, 0);
        p.drawText(capRect, Qt::AlignHCenter | Qt::AlignTop, m_caption);
    } else {
        double total = 0;
        for (const Seg &s : m_segs) total += s.value;
        if (total <= 0) total = 1;
        double start = 90.0; // 12 点
        for (const Seg &s : m_segs) {
            if (s.value <= 0) continue;
            double sweep = s.value / total * 360.0;
            QPen seg(s.color);
            seg.setWidth(m_thick);
            seg.setCapStyle(Qt::FlatCap);
            p.setPen(seg);
            p.drawArc(rect, static_cast<int>(start * 16), static_cast<int>(-sweep * 16));
            start -= sweep;
        }
        p.setPen(QColor("#14395E"));
        QFont f = font(); f.setPointSize(16); f.setBold(true); p.setFont(f);
        QRectF top(rect.left(), rect.center().y() - 22, rect.width(), 24);
        p.drawText(top, Qt::AlignCenter, m_big);

        p.setPen(QColor("#5B6675"));
        QFont f2 = font(); f2.setPointSize(8); f2.setBold(false); p.setFont(f2);
        QRectF bot(rect.left(), rect.center().y() + 2, rect.width(), 20);
        p.drawText(bot, Qt::AlignHCenter | Qt::AlignTop, m_small);
    }
}
