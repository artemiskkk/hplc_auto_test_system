#ifndef TITLEBAR_H
#define TITLEBAR_H
//
// 深蓝自定义标题栏（配合无边框窗口）：系统名 + 版本 + 最小化/最大化/关闭，可拖动。
//
#include <QWidget>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QPushButton>
#include <QMouseEvent>
#include <QRect>

class TitleBar : public QWidget
{
public:
    explicit TitleBar(QWidget *win, const QString &name, const QString &ver, QWidget *parent = nullptr)
        : QWidget(parent), m_win(win)
    {
        setObjectName("titleBar");
        setFixedHeight(46);
        auto *lay = new QHBoxLayout(this);
        lay->setContentsMargins(16, 0, 6, 0);
        lay->setSpacing(10);

        auto *iconLbl = new QLabel(this);
        iconLbl->setObjectName("titleIcon");
        iconLbl->setFixedSize(22, 22);
        iconLbl->setPixmap(QIcon(":/ui/icons/app.ico").pixmap(22, 22));
        iconLbl->setScaledContents(true);
        lay->addWidget(iconLbl);

        auto *nameLbl = new QLabel(name, this);
        nameLbl->setObjectName("titleName");
        auto *verLbl = new QLabel(ver, this);
        verLbl->setObjectName("titleVer");
        verLbl->setAlignment(Qt::AlignVCenter);
        lay->addWidget(nameLbl);
        lay->addWidget(verLbl);
        lay->addStretch();

        auto mkBtn = [&](const QString &t, const QString &obj) {
            auto *b = new QPushButton(t, this);
            b->setObjectName(obj);
            b->setCursor(Qt::PointingHandCursor);
            lay->addWidget(b);
            return b;
        };
        QPushButton *btnMin = mkBtn("—", "winBtn");
        m_btnMax = mkBtn("□", "winBtn");
        QPushButton *btnClose = mkBtn("✕", "winClose"); // #winClose hover 变红

        connect(btnMin,   &QPushButton::clicked, this, [this]{ m_win->showMinimized(); });
        connect(m_btnMax, &QPushButton::clicked, this, [this]{ toggleMax(); });
        connect(btnClose, &QPushButton::clicked, this, [this]{ m_win->close(); });
    }

protected:
    void mousePressEvent(QMouseEvent *e) override {
        if (e->button() == Qt::LeftButton) {
            m_drag = true;
            m_restoreOnDrag = m_win->isMaximized();
            m_pressRatio = width() > 0 ? double(e->pos().x()) / double(width()) : 0.5;
            if (!m_restoreOnDrag)
                m_off = e->globalPos() - m_win->frameGeometry().topLeft();
        }
    }
    void mouseMoveEvent(QMouseEvent *e) override {
        if (!m_drag || !(e->buttons() & Qt::LeftButton))
            return;

        if (m_restoreOnDrag) {
            const QRect normal = m_win->normalGeometry();
            const QSize size = normal.isValid() ? normal.size() : QSize(1480, 900);
            m_win->showNormal();
            if (m_btnMax) m_btnMax->setText("□");
            m_win->resize(size);
            const int xOff = qBound(0, int(size.width() * m_pressRatio), qMax(0, size.width() - 1));
            m_off = QPoint(xOff, qMin(e->pos().y(), qMax(0, size.height() - 1)));
            m_restoreOnDrag = false;
        }

        if (m_drag)
            m_win->move(e->globalPos() - m_off);
    }
    void mouseReleaseEvent(QMouseEvent *) override { m_drag = false; m_restoreOnDrag = false; }
    void mouseDoubleClickEvent(QMouseEvent *) override { toggleMax(); }

private:
    void toggleMax() {
        if (m_win->isMaximized()) { m_win->showNormal(); m_btnMax->setText("□"); }
        else                      { m_win->showMaximized(); m_btnMax->setText("❐"); }
    }
    QWidget    *m_win;
    QPushButton*m_btnMax = nullptr;
    bool        m_drag = false;
    bool        m_restoreOnDrag = false;
    double      m_pressRatio = 0.5;
    QPoint      m_off;
};

#endif // TITLEBAR_H
