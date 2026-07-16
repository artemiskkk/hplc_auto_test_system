#ifndef THEME_H
#define THEME_H
//
// 工业上位机主题（QSS）。白底 + 深蓝标题栏 + 工业蓝主色，
// 绿=在线/通过，红=失败/停止，橙=暂停/警告。卡片清晰、圆角适中。
//
#include <QString>
#include <QRegularExpression>
#include <QtGlobal>

namespace Theme
{
    // 主色板
    static const char *PRIMARY  = "#1769B0"; // 工业蓝
    static const char *GREEN    = "#1E8E3E"; // 在线/通过
    static const char *RED      = "#D14343"; // 失败/停止
    static const char *ORANGE   = "#E08A1E"; // 暂停/警告
    static const char *BLUE_RX  = "#1769B0";

    inline QString scalePxValues(const QString &style, double scale)
    {
        if (scale <= 1.01)
            return style;

        QRegularExpression re("(\\d+)px");
        QRegularExpressionMatchIterator it = re.globalMatch(style);
        QString out;
        int last = 0;
        while (it.hasNext()) {
            QRegularExpressionMatch m = it.next();
            out += style.mid(last, m.capturedStart() - last);
            const int px = m.captured(1).toInt();
            out += QString::number(qMax(1, qRound(px * scale))) + "px";
            last = m.capturedEnd();
        }
        out += style.mid(last);
        return out;
    }

    inline QString qss(double scale = 1.0)
    {
        QString style = QString(R"(
* { font-family: "Microsoft YaHei","SimHei",sans-serif; font-size: 12px; color: #1F2A37; }
QMainWindow, #rootBg { background: #EEF2F7; }

QGroupBox {
    background: #FFFFFF; border: 1px solid #CFD9E6; border-radius: 6px;
    margin-top: 16px; padding: 10px 10px 10px 10px;
}
QGroupBox::title {
    subcontrol-origin: margin; subcontrol-position: top left;
    left: 10px; top: 2px; padding: 0 6px;
    color: #14395E; font-weight: bold; font-size: 12px;
}
QGroupBox QGroupBox {
    background: #FAFCFF; border: 1px solid #D8E1EC; border-radius: 6px;
}

#titleBar { background: #123A5A; border-bottom: 1px solid #0C2B45; }
#titleName { color: #FFFFFF; font-size: 15px; font-weight: bold; }
#titleVer  { color: #BBD2E8; font-size: 11px; padding-top: 2px; }

/* Secondary windows: raised surface over a dimmed main window. */
#dlgRoot { background: transparent; border: none; }
#dialogSurface {
    background: #F6F9FC;
    border: 1px solid #8FA6BA;
    border-radius: 8px;
}
#dialogSurface #titleBar {
    border-top-left-radius: 7px;
    border-top-right-radius: 7px;
}
#dlgBody {
    background: #F6F9FC;
    border-bottom-left-radius: 7px;
    border-bottom-right-radius: 7px;
}
#dlgBody > QLabel { color: #44505F; }
#winBtn, #winClose {
    background: transparent; color: #DCE7F2; border: none;
    min-width: 42px; min-height: 30px; font-size: 14px;
}
#winBtn:hover { background: rgba(255,255,255,0.14); }
#winClose:hover { background: #D14343; color: #fff; }

#commandBar {
    background: #FFFFFF;
    border-bottom: 1px solid #CFD9E6;
}
#commandButton, #commandPrimary, #commandSuccess, #commandDanger {
    min-width: 86px;
    min-height: 34px;
    max-height: 34px;
    padding: 0 16px;
    border-radius: 5px;
    font-size: 12px;
    font-weight: bold;
}
#commandButton {
    background: #F7FAFD;
    border: 1px solid #C5D1DF;
    color: #233244;
}
#commandButton:hover {
    background: #EAF2FA;
    border-color: #AFC3D8;
    color: #14395E;
}
#commandButton:pressed {
    background: #DCEAF7;
    border-color: #9DB6CF;
}
#commandPrimary { background: #1769B0; color: #FFFFFF; border: none; }
#commandPrimary:hover { background: #125A99; }
#commandSuccess { background: #1E8E3E; color: #FFFFFF; border: none; }
#commandSuccess:hover { background: #177632; }
#commandDanger { background: #D14343; color: #FFFFFF; border: none; }
#commandDanger:hover { background: #B73434; }
#commandDivider {
    background: #DCE4EE;
    border: none;
    min-width: 1px;
    max-width: 1px;
}

#statusBarTop { background: #FFFFFF; border: 1px solid #CFD9E6; border-radius: 6px; }
#metricChip {
    background: #F7FAFD; border: 1px solid #DCE5EF; border-radius: 5px;
}
#metricKey { color: #647386; font-size: 11px; }
#metricVal { color: #17263A; font-size: 13px; font-weight: bold; }
#metricVal[state="good"] { color: #1E8E3E; }
#metricVal[state="bad"] { color: #D14343; }

#kvRow { background: transparent; }
#kvKey { color: #647386; font-size: 12px; }
#kvVal { color: #17263A; font-size: 12px; font-weight: 500; }

QPushButton {
    background: #FFFFFF; border: 1px solid #C5D1DF; border-radius: 5px;
    padding: 5px 12px; color: #233244;
}
QPushButton:hover { background: #F0F6FC; border-color: #AFC3D8; }
QPushButton#primary { background: #1769B0; color: #fff; border: none; }
QPushButton#primary:hover { background: #125A99; }
QPushButton#success { background: #1E8E3E; color: #fff; border: none; }
QPushButton#danger  { background: #D14343; color: #fff; border: none; }
QPushButton#warn    { background: #E08A1E; color: #fff; border: none; }
QPushButton:disabled { background: #EEF1F5; color: #A7B0BC; border: 1px solid #E2E8F0; }

QLineEdit, QComboBox {
    background: #FFFFFF; border: 1px solid #C5D1DF; border-radius: 5px;
    padding: 4px 34px 4px 8px; min-height: 24px;
}
QLineEdit:focus, QComboBox:focus { border: 1px solid #1769B0; background: #FBFDFF; }
QLineEdit::placeholder { color: #8997A8; }
QComboBox::drop-down {
    subcontrol-origin: padding;
    subcontrol-position: top right;
    width: 28px;
    border-left: 1px solid #C5D1DF;
    border-top-right-radius: 5px;
    border-bottom-right-radius: 5px;
    background: #EAF2FA;
}
QComboBox::drop-down:hover { background: #DCEAF7; }
QComboBox::down-arrow {
    image: url(:/ui/icons/combo_down.xpm);
    width: 10px;
    height: 6px;
}
QComboBox QAbstractItemView {
    background: #FFFFFF;
    border: 1px solid #AFC3D8;
    selection-background-color: #DCEAF7;
    selection-color: #14395E;
    outline: 0;
}

QTreeWidget, QTableWidget, QPlainTextEdit {
    background: #FFFFFF; border: 1px solid #D2DCE8; border-radius: 5px;
    selection-background-color: #DCEAF7; selection-color: #1F2A37; outline: 0;
}
QTreeWidget::item { height: 25px; padding-left: 4px; }
QTreeWidget::item:selected { background: #DCEAF7; color: #14395E; }
QHeaderView::section {
    background: #EAF0F7; color: #44505F; border: none; border-right: 1px solid #DCE4EE;
    border-bottom: 1px solid #CFD9E6; padding: 6px 8px; font-weight: bold;
}
QTableWidget { gridline-color: #EAEFF5; }
QTableWidget::item { padding: 4px 7px; }

QTabBar::tab {
    background: #EDF2F8; color: #647386; border: 1px solid #DCE5EF; border-bottom: none;
    padding: 6px 16px; border-top-left-radius: 5px; border-top-right-radius: 5px; margin-right: 2px;
}
QTabBar::tab:selected { background: #FFFFFF; color: #14395E; font-weight: bold; }
QTabWidget::pane { border: 1px solid #D6DEE8; border-radius: 0 6px 6px 6px; top: -1px; }

QProgressBar {
    background: #E5ECF4; border: none; border-radius: 6px; height: 12px;
    text-align: center; color: #14395E; font-size: 10px;
}
QProgressBar::chunk { background: #1769B0; border-radius: 6px; }

#appStatusBar { background: #123A5A; color: #CFE0F2; border-top: 1px solid #0C2B45; }
#appStatusBar QLabel { color: #CFE0F2; font-size: 11px; }

QScrollBar:vertical { background: #F0F3F8; width: 10px; margin: 0; }
QScrollBar::handle:vertical { background: #C2CEDD; border-radius: 5px; min-height: 24px; }
QScrollBar::add-line, QScrollBar::sub-line { height: 0; }
)");
        return scalePxValues(style, scale);
    }
}

#endif // THEME_H
