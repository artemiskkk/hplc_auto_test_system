#include "ui/MainWindow.h"
#include "ui/Theme.h"
#include "ui/TitleBar.h"
#include "ui/RingChart.h"
#include "ui/ParamConfigDialog.h"
#include "comm/SerialComm.h"
#include "device/DeviceControl.h"
#include "device/VirtualMeter.h"
#include "host/ScriptHost.h"
#include "core/Scheduler.h"
#include "core/Reporter.h"
#include "core/ConfigLoader.h"
#include "config/AppConfig.h"
#include "common/Logger.h"

#include <QtWidgets>
#include <QCoreApplication>
#include <QEvent>
#include <QMouseEvent>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QDateTime>
#include <QTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QProcess>
#include <QGuiApplication>
#include <QScreen>
#include <QWindow>
#include <algorithm>

// ===================== 文件内小工具 =====================
namespace {

QLabel* lbl(const QString &t, int pt, bool bold, const QString &color)
{
    QLabel *l = new QLabel(t);
    QFont f = l->font(); f.setPointSize(pt); f.setBold(bold); l->setFont(f);
    l->setStyleSheet(QString("color:%1;").arg(color));
    return l;
}

// 状态信息条的小卡片（key 在上 / value 在下）
QFrame* chip(const QString &key, QLabel **valOut, const QString &initVal = "-")
{
    QFrame *f = new QFrame;
    f->setObjectName("metricChip");
    f->setMinimumWidth(78);
    f->setMaximumHeight(58);
    QVBoxLayout *v = new QVBoxLayout(f);
    v->setContentsMargins(10, 6, 10, 6); v->setSpacing(2);
    QLabel *k = lbl(key, 8, false, "#647386");
    k->setObjectName("metricKey");
    v->addWidget(k);
    QLabel *val = lbl(initVal, 10, true, "#1F2A37");
    val->setObjectName("metricVal");
    val->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    *valOut = val;
    v->addWidget(val);
    return f;
}

// 信息面板的一行 key : value
QWidget* kvRow(const QString &key, QLabel **valOut)
{
    QWidget *w = new QWidget;
    w->setObjectName("kvRow");
    w->setMinimumHeight(25);
    QHBoxLayout *h = new QHBoxLayout(w);
    h->setContentsMargins(0, 2, 0, 2); h->setSpacing(10);
    QLabel *k = lbl(key, 9, false, "#647386");
    k->setObjectName("kvKey");
    k->setMinimumWidth(70);
    QLabel *val = lbl("-", 9, false, "#1F2A37");
    val->setObjectName("kvVal");
    val->setWordWrap(true);
    val->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    *valOut = val;
    h->addWidget(k); h->addWidget(val, 1);
    return w;
}

QGroupBox* card(const QString &title, QVBoxLayout **layOut)
{
    QGroupBox *g = new QGroupBox(title);
    QVBoxLayout *v = new QVBoxLayout(g);
    v->setContentsMargins(12, 10, 12, 10); v->setSpacing(8);
    *layOut = v;
    return g;
}

QString normalizePath(const QString &path)
{
    QFileInfo fi(path);
    return fi.exists() ? fi.canonicalFilePath() : QDir::cleanPath(path);
}

bool caseUnsupported(int tcID)
{
    static const QSet<int> s = QSet<int>()
        << 117 << 118 << 119 << 120
        << 128 << 130
        << 175
        << 183 << 185 << 186 << 187
        << 188 << 189 << 190 << 191;
    return s.contains(tcID);
}
const char *kUnsupportedTip = "该用例依赖事件触发、主动上报或完整表端仿真，暂不可执行";

bool looksLikeDataBaseDir(const QString &path)
{
    QDir d(path);
    return d.exists("TestCase.csv") &&
           d.exists("ConcentratorInfo.csv") &&
           d.exists("MeterInfo.csv");
}

void addDataBaseCandidates(QStringList &out, const QString &startPath)
{
    QDir d(startPath);
    for (int i = 0; i < 6; ++i) {
        const QString candidate = normalizePath(d.filePath("DataBase"));
        if (!out.contains(candidate))
            out << candidate;
        if (!d.cdUp())
            break;
    }
}

void setComboByText(QComboBox *box, const QString &text)
{
    const int idx = box->findText(text);
    if (idx >= 0)
        box->setCurrentIndex(idx);
    else if (box->isEditable())
        box->setEditText(text);
}

void setComboByData(QComboBox *box, int data)
{
    const int idx = box->findData(data);
    if (idx >= 0)
        box->setCurrentIndex(idx);
}

bool isValidSerialPortName(const QString &raw)
{
    const QString s = raw.trimmed();
    if (s.isEmpty())
        return false;
    if (s.contains(',') || s.contains(';') || s.contains(' '))
        return false;
    if (s.toUInt() > 0)
        return false;

    QRegExp winCom("^COM\\d+$", Qt::CaseInsensitive);
    if (winCom.exactMatch(s))
        return true;

    return s.startsWith("/dev/") || s.contains(QRegExp("[A-Za-z]"));
}

int serialPortSortKey(const QString &raw)
{
    QRegExp winCom("^COM(\\d+)$", Qt::CaseInsensitive);
    if (winCom.exactMatch(raw.trimmed()))
        return winCom.cap(1).toInt();
    return 1000000;
}

QString parityCode(QSerialPort::Parity parity)
{
    switch (parity) {
    case QSerialPort::OddParity: return "O";
    case QSerialPort::EvenParity: return "E";
    case QSerialPort::MarkParity: return "M";
    case QSerialPort::SpaceParity: return "S";
    default: return "N";
    }
}

QString stopBitsCode(QSerialPort::StopBits stopBits)
{
    switch (stopBits) {
    case QSerialPort::OneAndHalfStop: return "1.5";
    case QSerialPort::TwoStop: return "2";
    default: return "1";
    }
}

QString serialFormatText(const SerialEndpoint &ep, const QString &role)
{
    return QString("%1 %2  %3-%4-%5-%6")
        .arg(role)
        .arg(ep.portName)
        .arg(ep.baudRate)
        .arg(static_cast<int>(ep.dataBits))
        .arg(parityCode(ep.parity))
        .arg(stopBitsCode(ep.stopBits));
}

class DragMover : public QObject
{
public:
    explicit DragMover(QWidget *target) : QObject(target), m_target(target) {}
protected:
    bool eventFilter(QObject *o, QEvent *e) override
    {
        if (e->type() == QEvent::MouseButtonPress) {
            QMouseEvent *me = static_cast<QMouseEvent*>(e);
            if (me->button() == Qt::LeftButton) {
                m_pressed = true;
                m_offset = me->globalPos() - m_target->frameGeometry().topLeft();
            }
        } else if (e->type() == QEvent::MouseMove && m_pressed) {
            m_target->move(static_cast<QMouseEvent*>(e)->globalPos() - m_offset);
        } else if (e->type() == QEvent::MouseButtonRelease) {
            m_pressed = false;
        }
        return QObject::eventFilter(o, e);
    }
private:
    QWidget *m_target;
    QPoint   m_offset;
    bool     m_pressed = false;
};

} // namespace

// ===================== 构造 =====================
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    setMinimumSize(1360, 800);
    resize(1480, 900);
    setStyleSheet(Theme::qss(m_uiScale));

    // backend
    m_serial   = new SerialComm(this);
    m_devctrl  = new DeviceControl(this);
    m_host     = new ScriptHost(this);
    m_sched    = new Scheduler(this);
    m_reporter = new Reporter(this);
    m_virtualMeter = new VirtualMeter(this);
    m_devctrl->setSerial(m_serial);
    m_devctrl->setAckDelayMs(AppConfig::instance().ctrlAckDelayMs);
    m_host->setSerial(m_serial);
    m_host->setDeviceControl(m_devctrl);
    m_virtualMeter->setSerial(m_serial);
    m_sched->setHost(m_host);

    connect(m_serial, &SerialComm::bytesReceived, this, [this](DvcType dvcType, int dvcId, const QByteArray &data) {
        if (m_virtualMeter)
            m_virtualMeter->handleBytes(dvcType, dvcId, data);
        m_host->onBytesReceived(dvcType, dvcId, data);
    });
    connect(m_serial, &SerialComm::portError, this, [this](DvcType dvcType, int dvcId, const QString &msg) {
        Logger::instance().error(QString("串口异常 type=%1,dvcId=%2: %3").arg(int(dvcType)).arg(dvcId).arg(msg));
        if (!m_db.concentrators.isEmpty()
            && dvcId == m_db.concentrators.first().dvcId
            && dvcType == m_db.concentrators.first().slotPosition
            && !m_serial->isOpen(dvcType, dvcId)) {
            setCcoConnectedUi(false);
        }
    });
    connect(&Logger::instance(), &Logger::lineLogged, this, &MainWindow::appendLog);
    connect(m_sched, &Scheduler::caseStarted,  this, &MainWindow::onCaseStarted);
    connect(m_sched, &Scheduler::caseProgress, this, &MainWindow::onCaseProgress);
    connect(m_sched, &Scheduler::caseFinished, this, &MainWindow::onCaseFinished);
    connect(m_sched, &Scheduler::caseResult,   m_reporter, &Reporter::add);
    connect(m_sched, &Scheduler::caseResult,   this, &MainWindow::onCaseResult);

    buildUi();

    m_clock = new QTimer(this);
    connect(m_clock, &QTimer::timeout, this, &MainWindow::onTick);
    m_clock->start(1000);
    onTick();
    autoLoadDataBase();
    refreshStats();
}

MainWindow::~MainWindow()
{
    closeCaseLog();
}

int MainWindow::spx(int value) const
{
    return qMax(1, qRound(value * m_uiScale));
}

double MainWindow::currentScreenScale() const
{
    QScreen *screen = windowHandle() ? windowHandle()->screen() : QGuiApplication::primaryScreen();
    if (!screen)
        return 1.0;

    QScreen *primary = QGuiApplication::primaryScreen();
    const QRect g = screen->geometry();
    if (screen == primary)
        return 1.0;

    double scale = 1.0;

    const double primaryDpi = primary ? primary->logicalDotsPerInch() : 96.0;
    const double dpiScale = primaryDpi > 1.0 ? screen->logicalDotsPerInch() / primaryDpi : 1.0;
    if (dpiScale > 1.05)
        scale = qMax(scale, dpiScale);

    if (g.width() >= 2500 || g.height() >= 1400)
        scale = qMax(scale, 1.12);
    if (g.width() >= 3300 || g.height() >= 1900)
        scale = qMax(scale, 1.18);

    return qBound(1.0, scale, 1.2);
}

void MainWindow::applyUiScale()
{
    const double nextScale = currentScreenScale();
    const double diff = nextScale > m_uiScale ? nextScale - m_uiScale : m_uiScale - nextScale;
    if (diff < 0.02 && !styleSheet().isEmpty())
        return;

    m_uiScale = nextScale;
    setStyleSheet(Theme::qss(m_uiScale));
    QSize minSize(spx(1360), spx(800));
    QScreen *screen = windowHandle() ? windowHandle()->screen() : QGuiApplication::primaryScreen();
    if (screen) {
        const QRect available = screen->availableGeometry();
        minSize.setWidth(qMin(minSize.width(), qMax(1360, available.width() - 40)));
        minSize.setHeight(qMin(minSize.height(), qMax(800, available.height() - 40)));
    }
    setMinimumSize(minSize);

    QWidget *title = findChild<QWidget*>("titleBar");
    if (title) title->setFixedHeight(spx(46));
    if (m_toolbarWidget) m_toolbarWidget->setFixedHeight(spx(54));
    if (m_appStatusWidget) m_appStatusWidget->setFixedHeight(spx(28));
    if (m_bottomPane) m_bottomPane->setMaximumHeight(spx(132));

    if (m_leftPane) {
        m_leftPane->setMinimumWidth(spx(360));
        m_leftPane->setMaximumWidth(spx(760));
    }
    if (m_rightPane) {
        m_rightPane->setMinimumWidth(spx(340));
        m_rightPane->setMaximumWidth(spx(760));
    }

    if (m_progBar) m_progBar->setFixedWidth(spx(150));
    if (m_tree) {
        m_tree->setColumnWidth(0, spx(62));
        m_tree->header()->setDefaultSectionSize(spx(90));
    }
    if (m_stepTable) {
        m_stepTable->verticalHeader()->setDefaultSectionSize(spx(28));
        m_stepTable->horizontalHeader()->setDefaultSectionSize(spx(90));
        m_stepTable->setColumnWidth(0, spx(48));
        m_stepTable->setColumnWidth(2, spx(78));
        m_stepTable->setColumnWidth(3, spx(92));
        m_stepTable->setColumnWidth(4, spx(92));
        m_stepTable->setColumnWidth(5, spx(72));
    }
    if (m_logTable) {
        m_logTable->verticalHeader()->setDefaultSectionSize(spx(28));
        m_logTable->setColumnWidth(0, spx(64));
    }

    for (QFrame *chip : findChildren<QFrame*>("metricChip"))
        chip->setMaximumHeight(spx(58));

    adjustMainSplit();
}

void MainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);

    if (!m_screenHooked && windowHandle()) {
        m_screenHooked = true;
        connect(windowHandle(), &QWindow::screenChanged, this, [this](QScreen *) {
            QTimer::singleShot(0, this, [this]() { applyUiScale(); });
        });
    }

    QTimer::singleShot(0, this, [this]() { applyUiScale(); });
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    QTimer::singleShot(0, this, [this]() { adjustMainSplit(); });
}

void MainWindow::adjustMainSplit()
{
    if (!m_mainSplit || m_mainSplit->count() < 3)
        return;

    const int total = m_mainSplit->width();
    if (total <= 0)
        return;

    const bool wideScreen = total >= spx(1700);
    const int left = wideScreen
        ? qBound(spx(440), total * 24 / 100, spx(760))
        : qBound(spx(360), total * 20 / 100, spx(620));
    const int right = wideScreen
        ? qBound(spx(420), total * 24 / 100, spx(760))
        : qBound(spx(340), total * 22 / 100, spx(680));
    const int center = qMax(spx(650), total - left - right);
    m_mainSplit->setSizes(QList<int>() << left << center << right);
}

// ===================== 总装 =====================
void MainWindow::buildUi()
{
    QWidget *root = new QWidget;
    root->setObjectName("rootBg");
    QVBoxLayout *outer = new QVBoxLayout(root);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->setSpacing(0);

    outer->addWidget(new TitleBar(this,
        "国网 HPLC 自动化测试系统", "V1.0.0"));
    outer->addWidget(buildToolbar());

    QWidget *workArea = new QWidget;
    QVBoxLayout *work = new QVBoxLayout(workArea);
    work->setContentsMargins(8, 8, 8, 8); work->setSpacing(8);

    // 3 状态信息条
    work->addWidget(buildStatusInfo());

    m_mainSplit = new QSplitter(Qt::Horizontal);
    m_mainSplit->setHandleWidth(6);
    m_mainSplit->setChildrenCollapsible(false);
    m_mainSplit->addWidget(buildLeft());

    QWidget *centerColumn = new QWidget;
    QVBoxLayout *centerLay = new QVBoxLayout(centerColumn);
    centerLay->setContentsMargins(0, 0, 0, 0);
    centerLay->setSpacing(10);
    centerLay->addWidget(buildCenter(), 1);
    centerLay->addWidget(buildBottom(), 0);
    m_mainSplit->addWidget(centerColumn);

    m_mainSplit->addWidget(buildRight());
    m_mainSplit->setStretchFactor(0, 3);
    m_mainSplit->setStretchFactor(1, 9);
    m_mainSplit->setStretchFactor(2, 4);
    m_mainSplit->setSizes(QList<int>() << 380 << 760 << 420);
    work->addWidget(m_mainSplit, 1);

    outer->addWidget(workArea, 1);

    // 9 底部状态栏
    QWidget *sb = new QWidget;
    m_appStatusWidget = sb;
    sb->setObjectName("appStatusBar");
    sb->setFixedHeight(28);
    QHBoxLayout *sbl = new QHBoxLayout(sb);
    sbl->setContentsMargins(14, 0, 6, 0); sbl->setSpacing(18);
    auto sbItem = [&](const QString &t)->QLabel*{ QLabel *l = new QLabel(t); sbl->addWidget(l); return l; };
    sbItem("项目: sgcc_test_project");
    sbl->addStretch();
    m_sbLogCount = sbItem("日志: 0 条");
    m_sbCpu      = sbItem("CPU: -");
    m_sbMem      = sbItem("内存: -");
    m_sbTime     = sbItem("--:--:--");
    m_sbStatus   = sbItem("系统运行正常");
    m_sbStatus->setStyleSheet("color:#7CE0A0;");
    QSizeGrip *grip = new QSizeGrip(sb);
    sbl->addWidget(grip, 0, Qt::AlignBottom | Qt::AlignRight);
    outer->addWidget(sb);

    setCentralWidget(root);
}

// ===================== 2 工具栏 =====================
QWidget* MainWindow::buildToolbar()
{
    QFrame *bar = new QFrame;
    m_toolbarWidget = bar;
    bar->setObjectName("commandBar");
    bar->setFixedHeight(54);
    QHBoxLayout *h = new QHBoxLayout(bar);
    h->setContentsMargins(12, 8, 12, 8);
    h->setSpacing(8);

    auto addButton = [&](const QString &text, const QString &tip, void (MainWindow::*slot)(), const QString &role = QString()) {
        QPushButton *b = new QPushButton(text);
        b->setObjectName(role.isEmpty() ? "commandButton" : role);
        b->setToolTip(tip);
        b->setCursor(Qt::PointingHandCursor);
        b->setMinimumSize(86, 34);
        b->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        connect(b, &QPushButton::clicked, this, slot);
        h->addWidget(b);
        return b;
    };
    auto addDivider = [&]() {
        QFrame *line = new QFrame;
        line->setObjectName("commandDivider");
        line->setFrameShape(QFrame::VLine);
        line->setFixedHeight(24);
        h->addWidget(line);
    };

    m_connectCcoBtn = addButton("串口配置", "配置并连接 CCO 串口", &MainWindow::onConnectCco, "commandPrimary");
    addDivider();
    addButton("开始检测", "开始执行当前测试用例", &MainWindow::onStart, "commandSuccess");
    addButton("中止检测", "停止当前测试任务", &MainWindow::onStop, "commandDanger");
    addDivider();
    addButton("查看日志", "打开运行日志存储目录", &MainWindow::onViewLog);
    addButton("清空日志", "清空右侧运行日志", &MainWindow::onClearLog);
    addDivider();
    addButton("升级文件", "选择升级文件", &MainWindow::onUpgradeFile);
    addButton("查看报告", "打开自动保存的报告目录", &MainWindow::onBrowseReport);
    addButton("参数配置", "配置虚拟表地址和 645/698 应答项", &MainWindow::onParamConfig);
    h->addStretch();
    return bar;
}

// ===================== 3 状态信息条 =====================
QWidget* MainWindow::buildStatusInfo()
{
    QFrame *bar = new QFrame;
    bar->setObjectName("statusBarTop");
    QHBoxLayout *h = new QHBoxLayout(bar);
    h->setContentsMargins(12, 8, 12, 8); h->setSpacing(8);

    // 频段选择
    QFrame *fq = new QFrame;
    fq->setObjectName("metricChip");
    fq->setMinimumWidth(168);
    fq->setMaximumHeight(58);
    QVBoxLayout *fv = new QVBoxLayout(fq);
    fv->setContentsMargins(10, 5, 10, 6); fv->setSpacing(2);
    QLabel *freqKey = lbl("频段选择", 8, false, "#647386");
    freqKey->setObjectName("metricKey");
    fv->addWidget(freqKey);
    m_freqCombo = new QComboBox;
    m_freqCombo->addItems(QStringList() << "频段0" << "频段1" << "频段2" << "频段3");
    m_freqCombo->setCurrentIndex(AppConfig::instance().freqBand);
    connect(m_freqCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(onCaseSelected())); // 占位
    fv->addWidget(m_freqCombo);
    h->addWidget(fq);

    h->addWidget(chip("系统状态",  &m_sysState,   "未就绪"));
    h->addWidget(chip("COM 端口",  &m_comPortVal, "未连接"));
    h->addWidget(chip("CCO 设备",  &m_ccoVal,     "离线"));
    h->addWidget(chip("STA 设备",  &m_staVal,     "离线"));
    m_sysState->setStyleSheet("color:#D14343;font-weight:bold;");
    m_ccoVal->setStyleSheet("color:#D14343;font-weight:bold;");
    m_staVal->setStyleSheet("color:#D14343;font-weight:bold;");

    // 进度
    QFrame *pf = new QFrame;
    pf->setObjectName("metricChip");
    pf->setMinimumWidth(190);
    pf->setMaximumHeight(58);
    QVBoxLayout *pv = new QVBoxLayout(pf);
    pv->setContentsMargins(10, 5, 10, 6); pv->setSpacing(3);
    QHBoxLayout *ph = new QHBoxLayout(); ph->setSpacing(6);
    QLabel *progressKey = lbl("测试进度", 8, false, "#647386");
    progressKey->setObjectName("metricKey");
    ph->addWidget(progressKey);
    m_progPct = lbl("0%", 9, true, "#14395E"); ph->addStretch(); ph->addWidget(m_progPct);
    pv->addLayout(ph);
    m_progBar = new QProgressBar; m_progBar->setRange(0, 100); m_progBar->setValue(0);
    m_progBar->setTextVisible(false); m_progBar->setFixedWidth(150);
    pv->addWidget(m_progBar);
    h->addWidget(pf);

    h->addWidget(chip("当前用例", &m_curCaseVal, "-"));
    h->addWidget(chip("运行时间", &m_runTimeVal, "00:00:00"));
    h->addWidget(chip("用例总数", &m_totalVal,   "0"));
    h->addWidget(chip("通过率",   &m_passRateVal,"0.0%"));
    h->addStretch();

    m_portCombo = new QComboBox; m_portCombo->setFixedWidth(0); m_portCombo->hide();
    for (const QSerialPortInfo &info : QSerialPortInfo::availablePorts())
        m_portCombo->addItem(info.portName());
    m_baudCombo = new QComboBox; m_baudCombo->hide();
    m_baudCombo->addItems(QStringList() << "115200" << "57600" << "38400" << "19200" << "9600" << "2400");
    return bar;
}

// ===================== 4 左：用例树 =====================
QWidget* MainWindow::buildLeft()
{
    QVBoxLayout *v; QGroupBox *g = card("测试用例", &v);
    m_leftPane = g;
    g->setMinimumWidth(360);
    g->setMaximumWidth(760);
    m_search = new QLineEdit; m_search->setPlaceholderText("搜索用例名称 / ID");
    m_search->setMinimumHeight(34);
    m_search->setClearButtonEnabled(true);
    connect(m_search, &QLineEdit::textChanged, this, [this](const QString &s){ buildCaseTree(s); });
    v->addWidget(m_search);

    // 全选 + 已选计数（勾选哪些就按顺序批量执行）
    QHBoxLayout *selRow = new QHBoxLayout();
    selRow->setContentsMargins(2, 0, 2, 0); selRow->setSpacing(8);
    m_chkAll = new QCheckBox("全选");
    m_chkAll->setCursor(Qt::PointingHandCursor);
    connect(m_chkAll, &QCheckBox::toggled, this, &MainWindow::onSelectAll);
    m_selCount = lbl("已选 0 项", 9, false, "#647386");
    selRow->addWidget(m_chkAll);
    selRow->addStretch();
    selRow->addWidget(m_selCount);
    v->addLayout(selRow);

    m_tree = new QTreeWidget;
    m_tree->setHeaderLabels(QStringList() << "编号" << "用例 / 分类");
    m_tree->setColumnWidth(0, 62);
    m_tree->header()->setSectionResizeMode(0, QHeaderView::Fixed);
    m_tree->header()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_tree->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_tree->setAlternatingRowColors(true);
    connect(m_tree, &QTreeWidget::itemSelectionChanged, this, &MainWindow::onCaseSelected);
    connect(m_tree, &QTreeWidget::itemChanged, this, &MainWindow::onTreeItemChanged);
    v->addWidget(m_tree, 1);
    return g;
}

// ===================== 5 中：执行 =====================
QWidget* MainWindow::buildCenter()
{
    QVBoxLayout *v; QGroupBox *g = card("当前用例执行", &v);
    g->setMinimumWidth(650);

    auto infoBox = [&](const QString &title, const QStringList &keys)->QWidget*{
        QVBoxLayout *bv; QGroupBox *b = card(title, &bv);
        b->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
        for (const QString &k : keys) { QLabel *val; bv->addWidget(kvRow(k, &val)); m_info.insert(k, val); }
        return b;
    };
    v->addWidget(infoBox("用例信息", QStringList() << "用例ID" << "用例名称" << "所属目录" << "参数文件"));

    // 下：执行步骤（表格直接置顶，余下空间全部给表格）
    QVBoxLayout *sv; QGroupBox *stepBox = card("执行步骤", &sv);
    stepBox->setMinimumHeight(160);   // 仅保底，实际随窗口拉伸，避免最小高度过大撑出可视区

    m_stepTable = new QTableWidget(0, 6);
    m_stepTable->setHorizontalHeaderLabels(QStringList() << "步骤" << "步骤描述" << "状态" << "开始时间" << "结束时间" << "耗时");
    m_stepTable->verticalHeader()->setVisible(false);
    m_stepTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_stepTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_stepTable->setAlternatingRowColors(true);
    m_stepTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_stepTable->setColumnWidth(0, 48);
    m_stepTable->setColumnWidth(2, 78);
    m_stepTable->setColumnWidth(3, 92);
    m_stepTable->setColumnWidth(4, 92);
    m_stepTable->setColumnWidth(5, 72);
    sv->addWidget(m_stepTable);
    v->addWidget(stepBox, 1);
    return g;
}

// ===================== 6 右：运行日志 =====================
QWidget* MainWindow::buildRight()
{
    QVBoxLayout *lv; QGroupBox *logBox = card("运行日志", &lv);
    m_rightPane = logBox;
    logBox->setMinimumWidth(340);
    logBox->setMaximumWidth(760);

    // 右栏较窄：只保留 时间 + 日志内容，内容列拉满；级别用颜色区分、来源进 tooltip
    m_logTable = new QTableWidget(0, 2);
    m_logTable->setHorizontalHeaderLabels(QStringList() << "时间" << "日志内容");
    m_logTable->verticalHeader()->setVisible(false);
    m_logTable->verticalHeader()->setDefaultSectionSize(28);
    m_logTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_logTable->setAlternatingRowColors(true);
    m_logTable->setWordWrap(false);
    m_logTable->setTextElideMode(Qt::ElideRight);
    m_logTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    m_logTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_logTable->setColumnWidth(0, 64);
    lv->addWidget(m_logTable, 1);
    return logBox;
}

// ===================== 7/8 底部：断路器 + 统计 =====================
QWidget* MainWindow::buildBottom()
{
    QSplitter *sp = new QSplitter(Qt::Horizontal);
    m_bottomPane = sp;
    sp->setHandleWidth(8);
    sp->setMaximumHeight(132);

    QVBoxLayout *bv; QGroupBox *bk = card("断路器状态", &bv);
    bk->setMinimumWidth(280);
    bv->setSpacing(6);

    QLabel *v1; bv->addWidget(kvRow("STA 电源", &v1)); m_breakerState = v1; m_breakerState->setText("未知");
    QLabel *v2; bv->addWidget(kvRow("控制方式", &v2)); m_breakerMode = v2; m_breakerMode->setText("CCO 控制");

    // 断路器地址：可编辑（默认 111111111111），下发 = 11F1 添加从节点（入网，不进测试档案）
    QHBoxLayout *addrRow = new QHBoxLayout();
    addrRow->setSpacing(8);
    QLabel *addrKey = new QLabel("断路器地址");
    addrKey->setStyleSheet("color:#5B6675;");
    m_breakerAddr = new QLineEdit("111111111111");
    m_breakerAddr->setMaxLength(12);
    m_breakerAddr->setToolTip("断路器固定地址：12 位十六进制 / 6 字节。下发 = 11F1 添加从节点，使其入网。");
    QPushButton *bAdd = new QPushButton("下发");
    bAdd->setMinimumHeight(26);
    bAdd->setCursor(Qt::PointingHandCursor);
    connect(bAdd, &QPushButton::clicked, this, &MainWindow::onBreakerAddNode);
    addrRow->addWidget(addrKey);
    addrRow->addWidget(m_breakerAddr, 1);
    addrRow->addWidget(bAdd);
    bv->addLayout(addrRow);

    bv->addStretch();

    QHBoxLayout *bh = new QHBoxLayout();
    bh->setSpacing(8);
    QPushButton *bOff = new QPushButton("断电"); bOff->setObjectName("danger");
    QPushButton *bOn  = new QPushButton("上电"); bOn->setObjectName("success");
    for (QPushButton *b : {bOff, bOn}) {
        b->setMinimumHeight(28);
        b->setCursor(Qt::PointingHandCursor);
    }
    connect(bOff, &QPushButton::clicked, this, [this]{ onBreaker(0); });
    connect(bOn,  &QPushButton::clicked, this, [this]{ onBreaker(1); });
    bh->addWidget(bOff, 1);
    bh->addWidget(bOn, 1);
    bv->addLayout(bh);
    sp->addWidget(bk);

    // 8 结果统计
    QVBoxLayout *rv; QGroupBox *statBox = card("测试结果统计", &rv);
    statBox->setMinimumWidth(340);
    QHBoxLayout *rh = new QHBoxLayout();
    rh->setContentsMargins(0, 0, 0, 0);
    rh->setSpacing(14);
    m_statRing = new RingChart; m_statRing->setMinimumSize(88, 88);
    m_statRing->setMaximumSize(112, 112);
    rh->addWidget(m_statRing, 1);
    m_statLegend = new QLabel("-"); m_statLegend->setTextFormat(Qt::RichText);
    m_statLegend->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    rh->addWidget(m_statLegend, 1);
    rv->addLayout(rh);
    sp->addWidget(statBox);
    sp->setStretchFactor(0, 3); sp->setStretchFactor(1, 4);
    return sp;
}

void MainWindow::buildStatusBar() {}

// ===================== 用例树 =====================
void MainWindow::buildCaseTree(const QString &filter)
{
    m_updatingChecks = true;             // 填充期间抑制 itemChanged 处理
    m_tree->clear(); m_itemById.clear();
    QMap<QString, QTreeWidgetItem*> groups;
    int shown = 0;
    for (const TestCaseInfo &tc : m_db.cases) {
        if (!filter.isEmpty()) {
            const QString f = filter.trimmed();
            if (!tc.tcName.contains(f, Qt::CaseInsensitive) &&
                !QString::number(tc.tcID).contains(f) &&
                !tc.tcDLLName.contains(f, Qt::CaseInsensitive)) continue;
        }
        const QString cat = tc.catalogueName.isEmpty() ? "未分类" : tc.catalogueName;
        QTreeWidgetItem *gr = groups.value(cat, nullptr);
        if (!gr) {
            gr = new QTreeWidgetItem(m_tree, QStringList() << "" << cat);
            gr->setExpanded(true);
            QFont bf = gr->font(1); bf.setBold(true); gr->setFont(1, bf);
            gr->setToolTip(1, cat);
            gr->setFlags(gr->flags() | Qt::ItemIsUserCheckable);
            gr->setCheckState(0, Qt::Unchecked);
            groups.insert(cat, gr);
        }
        QTreeWidgetItem *it = new QTreeWidgetItem(gr, QStringList() << QString::number(tc.tcID) << tc.tcName);
        it->setData(0, Qt::UserRole, tc.tcID);
        it->setData(1, Qt::UserRole, tc.tcName);
        it->setToolTip(0, QString::number(tc.tcID));
        if (caseUnsupported(tc.tcID)) {
            it->setText(1, tc.tcName + "（暂不支持）");
            it->setForeground(0, QColor("#AAB3BF"));
            it->setForeground(1, QColor("#AAB3BF"));
            it->setToolTip(1, kUnsupportedTip);
        } else {
            it->setToolTip(1, tc.tcName);
            it->setFlags(it->flags() | Qt::ItemIsUserCheckable);
            it->setCheckState(0, m_checkedIds.contains(tc.tcID) ? Qt::Checked : Qt::Unchecked);
        }
        m_itemById.insert(tc.tcID, it);
        ++shown;
    }
    for (auto it = groups.begin(); it != groups.end(); ++it) {
        QTreeWidgetItem *group = it.value();
        group->setText(1, QString("%1 (%2)").arg(it.key()).arg(group->childCount()));
        group->setToolTip(1, it.key());
        int checkable = 0, checked = 0;
        for (int i = 0; i < group->childCount(); ++i) {
            QTreeWidgetItem *ch = group->child(i);
            if (!ch->flags().testFlag(Qt::ItemIsUserCheckable)) continue;
            ++checkable;
            if (ch->checkState(0) == Qt::Checked) ++checked;
        }
        if (checkable == 0) {
            group->setFlags(group->flags() & ~Qt::ItemIsUserCheckable);
            group->setData(0, Qt::CheckStateRole, QVariant());   // 移除勾选框
            group->setForeground(1, QColor("#AAB3BF"));
        } else {
            group->setCheckState(0, checked == 0 ? Qt::Unchecked
                                    : checked == checkable ? Qt::Checked
                                    : Qt::PartiallyChecked);
        }
    }
    m_updatingChecks = false;
    updateSelCount();
}

// 勾选变化：分类与子项联动，并维护 m_checkedIds
void MainWindow::onTreeItemChanged(QTreeWidgetItem *item, int column)
{
    if (column != 0 || m_updatingChecks || !item) return;
    m_updatingChecks = true;

    if (item->childCount() > 0) {
        // 分类节点：全选 / 全不选其可勾选子项（跳过暂不支持的灰项）
        const Qt::CheckState st = item->checkState(0);
        if (st != Qt::PartiallyChecked) {
            for (int i = 0; i < item->childCount(); ++i) {
                QTreeWidgetItem *ch = item->child(i);
                if (!ch->flags().testFlag(Qt::ItemIsUserCheckable)) continue;
                ch->setCheckState(0, st);
                const int id = ch->data(0, Qt::UserRole).toInt();
                if (st == Qt::Checked) m_checkedIds.insert(id); else m_checkedIds.remove(id);
            }
        }
    } else {
        const int id = item->data(0, Qt::UserRole).toInt();
        if (item->checkState(0) == Qt::Checked) m_checkedIds.insert(id); else m_checkedIds.remove(id);
        if (QTreeWidgetItem *p = item->parent()) {
            int checkable = 0, checked = 0;
            for (int i = 0; i < p->childCount(); ++i) {
                QTreeWidgetItem *ch = p->child(i);
                if (!ch->flags().testFlag(Qt::ItemIsUserCheckable)) continue;
                ++checkable;
                if (ch->checkState(0) == Qt::Checked) ++checked;
            }
            if (checkable > 0)
                p->setCheckState(0, checked == 0 ? Qt::Unchecked
                                   : checked == checkable ? Qt::Checked
                                   : Qt::PartiallyChecked);
        }
    }

    m_updatingChecks = false;
    updateSelCount();
}

void MainWindow::onSelectAll(bool on)
{
    if (m_updatingChecks) return;        // updateSelCount 回写时不重入
    m_updatingChecks = true;
    m_checkedIds.clear();
    if (on)
        for (const TestCaseInfo &tc : m_db.cases)
            if (!caseUnsupported(tc.tcID)) m_checkedIds.insert(tc.tcID);
    const Qt::CheckState st = on ? Qt::Checked : Qt::Unchecked;
    for (int i = 0; i < m_tree->topLevelItemCount(); ++i) {
        QTreeWidgetItem *cat = m_tree->topLevelItem(i);
        if (cat->flags().testFlag(Qt::ItemIsUserCheckable)) cat->setCheckState(0, st);
        for (int j = 0; j < cat->childCount(); ++j) {
            QTreeWidgetItem *ch = cat->child(j);
            if (ch->flags().testFlag(Qt::ItemIsUserCheckable)) ch->setCheckState(0, st);
        }
    }
    m_updatingChecks = false;
    updateSelCount();
}

void MainWindow::updateSelCount()
{
    const int n = m_checkedIds.size();
    int runnable = 0;
    for (const TestCaseInfo &tc : m_db.cases)
        if (!caseUnsupported(tc.tcID)) ++runnable;
    if (m_selCount) m_selCount->setText(QString("已选 %1 项").arg(n));
    if (m_chkAll) {
        const bool block = m_chkAll->blockSignals(true);
        m_chkAll->setChecked(runnable > 0 && n >= runnable);   // 全选满可执行项
        m_chkAll->blockSignals(block);
    }
}

QList<int> MainWindow::checkedCaseIds() const
{
    QList<int> ids;
    for (const TestCaseInfo &tc : m_db.cases)
        if (!caseUnsupported(tc.tcID) && m_checkedIds.contains(tc.tcID)) ids << tc.tcID;
    return ids;
}

int MainWindow::selectedCaseId() const
{
    QTreeWidgetItem *it = m_tree->currentItem();
    if (!it) return -1;
    QVariant v = it->data(0, Qt::UserRole);
    return v.isValid() ? v.toInt() : -1;
}

void MainWindow::setCaseInfo(const TestCaseInfo *tc)
{
    m_info["用例ID"]->setText(QString("TC_%1").arg(tc->tcID, 5, 10, QChar('0')));
    m_info["用例名称"]->setText(tc->tcName);
    m_info["所属目录"]->setText(tc->catalogueName);
    m_info["参数文件"]->setText(tc->paramFileName.isEmpty() ? "-" : tc->paramFileName);
}

void MainWindow::setCcoConnectedUi(bool connected, const QString &serialText)
{
    if (connected) {
        if (!serialText.isEmpty())
            m_comPortVal->setText(serialText);
        m_comPortVal->setToolTip(serialText);
        m_ccoVal->setText("在线");
        m_ccoVal->setStyleSheet("color:#1E8E3E;font-weight:bold;");
        m_staVal->setText("虚拟表在线");
        m_staVal->setStyleSheet("color:#1E8E3E;font-weight:bold;");
        m_breakerState->setText("已上电");
        if (m_connectCcoBtn) {
            m_connectCcoBtn->setText("断开串口");
            m_connectCcoBtn->setToolTip("断开当前 CCO 串口连接");
            m_connectCcoBtn->setObjectName("commandDanger");
        }
    } else {
        m_comPortVal->setText("未连接");
        m_comPortVal->setToolTip(QString());
        m_ccoVal->setText("离线");
        m_ccoVal->setStyleSheet("color:#D14343;font-weight:bold;");
        m_staVal->setText(m_db.meters.isEmpty() ? "离线" : QString("档案 %1").arg(m_db.meters.size()));
        m_staVal->setStyleSheet("color:#D14343;font-weight:bold;");
        m_breakerState->setText("未知");
        m_breakerState->setStyleSheet(QString());
        if (m_connectCcoBtn) {
            m_connectCcoBtn->setText("串口配置");
            m_connectCcoBtn->setToolTip("配置并连接 CCO 串口");
            m_connectCcoBtn->setObjectName("commandPrimary");
        }
    }

    if (m_connectCcoBtn) {
        m_connectCcoBtn->style()->unpolish(m_connectCcoBtn);
        m_connectCcoBtn->style()->polish(m_connectCcoBtn);
        m_connectCcoBtn->update();
    }
}

void MainWindow::onCaseSelected()
{
    int id = selectedCaseId();
    if (id < 0) return;
    for (const TestCaseInfo &tc : m_db.cases) if (tc.tcID == id) { setCaseInfo(&tc); break; }
}

// ===================== 工具栏动作 =====================
void MainWindow::autoLoadDataBase()
{
    QStringList candidates;
    if (!AppConfig::instance().dataBaseDir.trimmed().isEmpty()) {
        const QString configured = normalizePath(AppConfig::instance().dataBaseDir);
        if (!candidates.contains(configured))
            candidates << configured;
    }

    addDataBaseCandidates(candidates, QCoreApplication::applicationDirPath());
    addDataBaseCandidates(candidates, QDir::currentPath());

    for (const QString &dir : candidates) {
        if (!looksLikeDataBaseDir(dir))
            continue;
        if (loadDataBaseFromDir(dir, false))
            return;
    }

    Logger::instance().error(QString("未找到可用 DataBase，已搜索：%1").arg(candidates.join(" | ")));
}

bool MainWindow::loadDataBaseFromDir(const QString &dir, bool showError)
{
    const QString dbDir = normalizePath(dir);
    QString err;
    Database db;
    if (!ConfigLoader::load(dbDir, db, err)) {
        Logger::instance().error(QString("DataBase 加载失败：%1").arg(err));
        if (showError)
            QMessageBox::warning(this, "加载失败", err);
        return false;
    }

    QDir base(dbDir);
    m_db = db;
    AppConfig::instance().dataBaseDir = dbDir;
    AppConfig::instance().scriptParaDir = base.filePath("ScriptPara");
    AppConfig::instance().upgradeDir = base.filePath("Upgrade");
    const QString workRoot = QFileInfo(dbDir).absolutePath();
    if (!workRoot.isEmpty())
        QDir::setCurrent(workRoot);

    buildCaseTree();
    m_totalVal->setText(QString::number(m_db.cases.size()));
    // STA 设备数取自档案（电表/节点数），不主动向 CCO 查询
    m_staVal->setText(QString("档案 %1").arg(m_db.meters.size()));
    Logger::instance().info(QString("DataBase 加载完成：用例 %1，集中器 %2，电表 %3，方案 %4")
        .arg(m_db.cases.size()).arg(m_db.concentrators.size()).arg(m_db.meters.size()).arg(m_db.schemes.size()));
    if (m_db.meters.isEmpty())
        Logger::instance().error(QString("电表档案为空：%1 下的 MeterInfo.csv 未读到任何电表，请确认该目录下的 MeterInfo.csv 内容/格式")
                                 .arg(base.filePath("MeterInfo.csv")));
    Logger::instance().info("脚本工作目录已切换为: " + QDir::currentPath());
    m_batchTotal = 0;
    refreshStats();
    tryLoadDefaultPlugin();
    return true;
}

void MainWindow::tryLoadDefaultPlugin()
{
    if (m_host->isPluginLoaded())
        return;

    QStringList candidates;
    auto addPlugin = [&](const QString &path) {
        const QString normalized = normalizePath(path);
        if (!candidates.contains(normalized))
            candidates << normalized;
    };

    addPlugin(AppConfig::instance().pluginPath);
    addPlugin(QDir(QCoreApplication::applicationDirPath()).filePath("SgHplcTestScript.dll"));
    addPlugin(QDir(AppConfig::instance().dataBaseDir).filePath("Scripts/SgHplcTestScript.dll"));

    bool found = false;
    for (const QString &path : candidates) {
        if (!QFileInfo::exists(path))
            continue;
        found = true;
        QString err;
        if (m_host->loadPlugin(path, err)) {
            AppConfig::instance().pluginPath = path;
            Logger::instance().info("脚本插件自动加载成功");
            return;
        }
        Logger::instance().error(err);
    }

    if (!found)
        Logger::instance().error(QString("未找到脚本插件，已搜索：%1").arg(candidates.join(" | ")));
}

void MainWindow::onLoadDataBase()
{
    QString dir = QFileDialog::getExistingDirectory(this, "选择 DataBase 目录", AppConfig::instance().dataBaseDir);
    if (dir.isEmpty()) return;
    loadDataBaseFromDir(dir, true);
}

void MainWindow::onLoadPlugin()
{
    QString path = AppConfig::instance().pluginPath;
    if (!QFileInfo::exists(path))
        path = QFileDialog::getOpenFileName(this, "选择脚本插件", QString(), "插件 (*.dll *.so)");
    if (path.isEmpty()) return;
    QString err;
    if (!m_host->loadPlugin(path, err)) { QMessageBox::warning(this, "插件加载失败", err); return; }
    AppConfig::instance().pluginPath = path;
    Logger::instance().info("脚本插件加载成功");
}

void MainWindow::onConnectCco()
{
    if (m_db.concentrators.isEmpty()) { QMessageBox::warning(this, "串口配置", "请先加载 DataBase"); return; }

    const ConcentratorInfo &cco = m_db.concentrators.first();
    if (m_serial->isOpen(cco.slotPosition, cco.dvcId)) {
        if (m_queueActive || m_running) {
            QMessageBox::information(this, "断开串口", "当前正在执行检测，请先中止检测后再断开串口");
            return;
        }
        m_serial->closePorts();
        setCcoConnectedUi(false);
        Logger::instance().info("CCO/STA 串口已断开");
        return;
    }

    QStringList ports;
    auto addPortCandidate = [&ports](const QString &port, bool prepend = false) {
        const QString normalized = port.trimmed();
        if (!isValidSerialPortName(normalized) || ports.contains(normalized, Qt::CaseInsensitive))
            return;
        if (prepend)
            ports.prepend(normalized);
        else
            ports << normalized;
    };
    for (const QSerialPortInfo &info : QSerialPortInfo::availablePorts())
        addPortCandidate(info.portName());
    std::sort(ports.begin(), ports.end(), [](const QString &a, const QString &b) {
        const int ak = serialPortSortKey(a);
        const int bk = serialPortSortKey(b);
        if (ak != bk)
            return ak < bk;
        return QString::compare(a, b, Qt::CaseInsensitive) < 0;
    });
    if (ports.isEmpty()) {
        ports << "COM3" << "COM4";
    }

    SerialEndpoint ccoEp;
    ccoEp.dvcId    = cco.dvcId;
    ccoEp.dvcType  = cco.slotPosition;
    ccoEp.baudRate = 9600;
    ccoEp.dataBits = QSerialPort::Data8;
    ccoEp.parity   = QSerialPort::EvenParity;
    ccoEp.stopBits = QSerialPort::OneStop;

    if (const DvcSerial *dbSerial = m_db.serialByDvcId(cco.dvcId)) {
        if (isValidSerialPortName(dbSerial->portName)) {
            ccoEp.portName = dbSerial->portName.trimmed();
            addPortCandidate(ccoEp.portName, true);
        }
        if (dbSerial->baudRate > 0)
            ccoEp.baudRate = dbSerial->baudRate;
        ccoEp.dataBits = dbSerial->dataBits;
        ccoEp.parity   = dbSerial->parity;
        ccoEp.stopBits = dbSerial->stopBits;
    }
    if (ccoEp.portName.isEmpty())
        ccoEp.portName = ports.value(0, "COM3");

    SerialEndpoint staEp;
    staEp.dvcId    = 1;
    staEp.dvcType  = SingleSTA;
    staEp.baudRate = 9600;
    staEp.dataBits = QSerialPort::Data8;
    staEp.parity   = QSerialPort::EvenParity;
    staEp.stopBits = QSerialPort::OneStop;
    for (const MeterInfo &meter : m_db.meters) {
        if (meter.slotPosition == SingleSTA || meter.slotPosition == ThreeSTA) {
            staEp.dvcId = meter.dvcId;
            staEp.dvcType = meter.slotPosition;
            break;
        }
    }
    for (const QString &p : ports) {
        if (p.compare(ccoEp.portName, Qt::CaseInsensitive) != 0) {
            staEp.portName = p;
            break;
        }
    }
    if (staEp.portName.isEmpty()) {
        staEp.portName = (ccoEp.portName.compare("COM3", Qt::CaseInsensitive) == 0) ? "COM4" : "COM3";
        addPortCandidate(staEp.portName);
    }

    QDialog dlg(this);
    dlg.setObjectName("dlgRoot");
    dlg.setWindowTitle("串口配置");
    dlg.setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    dlg.setMinimumWidth(620);

    QVBoxLayout *outer = new QVBoxLayout(&dlg);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->setSpacing(0);

    // 与主界面一致的深蓝标题条（可拖动）
    QFrame *dlgHdr = new QFrame; dlgHdr->setObjectName("titleBar");
    dlgHdr->setFixedHeight(40);
    dlgHdr->installEventFilter(new DragMover(&dlg));
    QHBoxLayout *hdrLay = new QHBoxLayout(dlgHdr);
    hdrLay->setContentsMargins(14, 0, 6, 0);
    QLabel *dlgTitle = new QLabel("串口配置"); dlgTitle->setObjectName("titleName");
    QPushButton *dlgClose = new QPushButton(QString(QChar(0x00D7)));  // ×
    dlgClose->setObjectName("winClose");
    dlgClose->setCursor(Qt::PointingHandCursor);
    connect(dlgClose, &QPushButton::clicked, &dlg, &QDialog::reject);
    hdrLay->addWidget(dlgTitle); hdrLay->addStretch(); hdrLay->addWidget(dlgClose);
    outer->addWidget(dlgHdr);

    QWidget *dlgBody = new QWidget; dlgBody->setObjectName("dlgBody");
    QVBoxLayout *root = new QVBoxLayout(dlgBody);
    root->setContentsMargins(18, 16, 18, 14);
    root->setSpacing(12);
    outer->addWidget(dlgBody);

    struct SerialForm {
        QComboBox *port = nullptr;
        QComboBox *baud = nullptr;
        QComboBox *dataBits = nullptr;
        QComboBox *stopBits = nullptr;
        QComboBox *parity = nullptr;
    };

    auto makeSerialGroup = [&](const QString &title, const SerialEndpoint &ep, SerialForm *out) -> QGroupBox* {
        QGroupBox *group = new QGroupBox(title);
        group->setObjectName("serialGroup");
        QFormLayout *form = new QFormLayout(group);
        form->setLabelAlignment(Qt::AlignRight);
        form->setFormAlignment(Qt::AlignLeft | Qt::AlignTop);
        form->setHorizontalSpacing(12);
        form->setVerticalSpacing(10);

        out->port = new QComboBox;
        out->port->setEditable(true);
        out->port->setMinimumHeight(34);
        out->port->addItems(ports);
        if (isValidSerialPortName(ep.portName))
            setComboByText(out->port, ep.portName);
        else if (!ports.isEmpty())
            out->port->setCurrentIndex(0);
        form->addRow("串口:", out->port);

        out->baud = new QComboBox;
        out->baud->setEditable(true);
        out->baud->setMinimumHeight(34);
        out->baud->addItems(QStringList() << "1200" << "2400" << "4800" << "9600"
                                          << "19200" << "38400" << "57600" << "115200"
                                          << "230400" << "460800" << "921600");
        setComboByText(out->baud, QString::number(ep.baudRate));
        form->addRow("波特率:", out->baud);

        out->dataBits = new QComboBox;
        out->dataBits->setMinimumHeight(34);
        out->dataBits->addItem("5", int(QSerialPort::Data5));
        out->dataBits->addItem("6", int(QSerialPort::Data6));
        out->dataBits->addItem("7", int(QSerialPort::Data7));
        out->dataBits->addItem("8", int(QSerialPort::Data8));
        setComboByData(out->dataBits, ep.dataBits);
        form->addRow("数据位:", out->dataBits);

        out->stopBits = new QComboBox;
        out->stopBits->setMinimumHeight(34);
        out->stopBits->addItem("1", int(QSerialPort::OneStop));
        out->stopBits->addItem("1.5", int(QSerialPort::OneAndHalfStop));
        out->stopBits->addItem("2", int(QSerialPort::TwoStop));
        setComboByData(out->stopBits, ep.stopBits);
        form->addRow("停止位:", out->stopBits);

        out->parity = new QComboBox;
        out->parity->setMinimumHeight(34);
        out->parity->addItem("无校验(None)", int(QSerialPort::NoParity));
        out->parity->addItem("奇校验(Odd)", int(QSerialPort::OddParity));
        out->parity->addItem("偶校验(Even)", int(QSerialPort::EvenParity));
        out->parity->addItem("Mark", int(QSerialPort::MarkParity));
        out->parity->addItem("Space", int(QSerialPort::SpaceParity));
        setComboByData(out->parity, ep.parity);
        form->addRow("校验位:", out->parity);
        return group;
    };

    QHBoxLayout *serialLay = new QHBoxLayout;
    serialLay->setSpacing(12);
    SerialForm ccoForm, staForm;
    serialLay->addWidget(makeSerialGroup("CCO 串口", ccoEp, &ccoForm));
    serialLay->addWidget(makeSerialGroup("STA/虚拟表串口", staEp, &staForm));
    root->addLayout(serialLay);

    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    if (QPushButton *ok = buttons->button(QDialogButtonBox::Ok)) {
        ok->setObjectName("primary"); ok->setText("连接"); ok->setMinimumHeight(32);
        ok->setCursor(Qt::PointingHandCursor);
    }
    if (QPushButton *cancel = buttons->button(QDialogButtonBox::Cancel)) {
        cancel->setText("取消"); cancel->setMinimumHeight(32);
        cancel->setCursor(Qt::PointingHandCursor);
    }
    connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    root->addWidget(buttons);

    if (dlg.exec() != QDialog::Accepted)
        return;

    auto readForm = [](const SerialForm &form, SerialEndpoint *ep) {
        ep->portName = form.port->currentText().trimmed();
        ep->baudRate = form.baud->currentText().trimmed().toUInt();
        ep->dataBits = static_cast<QSerialPort::DataBits>(form.dataBits->currentData().toInt());
        ep->stopBits = static_cast<QSerialPort::StopBits>(form.stopBits->currentData().toInt());
        ep->parity = static_cast<QSerialPort::Parity>(form.parity->currentData().toInt());
    };
    readForm(ccoForm, &ccoEp);
    readForm(staForm, &staEp);

    const bool directCco = AppConfig::instance().directCcoMode;
    if (!isValidSerialPortName(ccoEp.portName) || ccoEp.baudRate == 0
        || (!directCco && (!isValidSerialPortName(staEp.portName) || staEp.baudRate == 0))) {
        QMessageBox::warning(this, "串口配置", "请检查 CCO/STA 串口和波特率配置，串口名不能填写波特率");
        return;
    }
    if (!directCco && ccoEp.portName.compare(staEp.portName, Qt::CaseInsensitive) == 0) {
        QMessageBox::warning(this, "串口配置", "CCO 与 STA/虚拟表不能选择同一个串口");
        return;
    }

    QString err;
    QList<SerialEndpoint> endpoints;
    endpoints << ccoEp;
    if (!directCco)
        endpoints << staEp;
    if (!m_serial->openPorts(endpoints, err)) {
        setCcoConnectedUi(false);
        QMessageBox::warning(this, "连接失败", err); return;
    }
    m_devctrl->setCcoDvcId(cco.dvcId);
    const QString ccoText = serialFormatText(ccoEp, "CCO");
    const QString staText = serialFormatText(staEp, "STA");
    const QString serialText = directCco ? ccoText : QString("%1 | %2").arg(ccoText, staText);
    setCcoConnectedUi(true, serialText);
    Logger::instance().info(QString("串口已连接：%1；虚拟表地址 %2")
                            .arg(serialText, m_virtualMeter ? m_virtualMeter->meterAddress() : QString("112233445566")));
}

void MainWindow::onParamConfig()
{
    if (!m_virtualMeter)
        return;

    ParamConfigDialog dlg(m_virtualMeter, this);
    if (dlg.exec() != QDialog::Accepted)
        return;

    QString baudMode = "确认";
    switch (m_virtualMeter->baudConsultMode()) {
    case VirtualMeter::BaudConsultDeny: baudMode = "否认"; break;
    case VirtualMeter::BaudConsultNoResponse: baudMode = "不响应"; break;
    case VirtualMeter::BaudConsultWrongResponse: baudMode = "异常回复"; break;
    case VirtualMeter::BaudConsultConfirm:
    default: break;
    }
    Logger::instance().info(QString("虚拟表参数已保存：表地址=%1，645项=%2，698项=%3，波特率协商=%4，跟随切换=%5")
                            .arg(m_virtualMeter->meterAddress())
                            .arg(m_virtualMeter->enabled645Items().size())
                            .arg(m_virtualMeter->enabled698Items().size())
                            .arg(baudMode)
                            .arg(m_virtualMeter->baudConsultSwitchSerial() ? "是" : "否"));
}

void MainWindow::onStart()
{
    if (!m_host->isPluginLoaded()) { QMessageBox::warning(this, "开始", "请先加载脚本插件"); return; }
    if (m_queueActive) { QMessageBox::information(this, "开始", "正在执行中，请先中止"); return; }

    QList<int> queue = checkedCaseIds();
    if (queue.isEmpty()) {
        int sel = selectedCaseId();
        if (sel >= 0) {
            if (caseUnsupported(sel)) {
                QMessageBox::information(this, "开始", QString("该用例") + kUnsupportedTip + "。");
                return;
            }
            queue << sel;
        }
    }
    if (queue.isEmpty()) {
        QMessageBox::information(this, "开始", "请勾选要执行的用例（或在左侧选中一个）");
        return;
    }

    AppConfig::instance().freqBand = m_freqCombo->currentIndex();
    m_sched->setDatabase(&m_db);
    m_sched->setScriptParaDir(AppConfig::instance().scriptParaDir);
    m_sched->setFreqEncoded(AppConfig::instance().freqEncoded());
    m_sched->setTotalTimes(AppConfig::instance().totalTestTimes);

    resetRunResults();              // 清空上一轮结果（统计/树标记/报告/步骤表）

    m_runQueue = queue;
    m_batchTotal = queue.size();    // 进度/统计以本轮勾选执行的用例数为基数
    m_runStamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");  // 本批次日志时间戳
    m_queueActive = true;
    m_running = true;
    m_runElapsed.restart();
    m_sysState->setText("运行中"); m_sysState->setStyleSheet("color:#1E8E3E;font-weight:bold;");
    Logger::instance().info(QString("开始批量检测：共 %1 个用例").arg(m_runQueue.size()));
    startNextInQueue();
}

void MainWindow::resetRunResults()
{
    m_caseResult.clear();
    if (m_reporter) m_reporter->clear();
    m_lastReportPath.clear();
    m_stepTable->setRowCount(0);
    m_stepSeq = 0;

    // 清掉用例树上的【PASS/FAIL/ERROR】标记与结果底色
    m_updatingChecks = true;
    for (auto it = m_itemById.constBegin(); it != m_itemById.constEnd(); ++it) {
        QTreeWidgetItem *item = it.value();
        const QString base = item->data(1, Qt::UserRole).toString();
        item->setText(1, caseUnsupported(it.key()) ? base + "（暂不支持）" : base);
        item->setBackground(0, QBrush());
        item->setBackground(1, QBrush());
    }
    m_updatingChecks = false;

    refreshStats();
}

// 取队列下一个用例执行；队列空则整体收尾并出报告
void MainWindow::startNextInQueue()
{
    if (!m_queueActive) return;

    if (m_runQueue.isEmpty()) {
        closeCaseLog();
        m_queueActive = false;
        m_running = false;
        m_sysState->setText("已完成"); m_sysState->setStyleSheet("color:#1E8E3E;font-weight:bold;");
        Logger::instance().info("全部用例执行完成");
        writeAutoReport();
        return;
    }

    const int tcID = m_runQueue.takeFirst();
    if (caseUnsupported(tcID)) {
        Logger::instance().info(QString("用例 %1 暂不支持，自动跳过").arg(tcID));
        QTimer::singleShot(0, this, [this]{ startNextInQueue(); });
        return;
    }
    if (QTreeWidgetItem *it = m_itemById.value(tcID, nullptr)) {
        m_tree->setCurrentItem(it);
        m_tree->scrollToItem(it);
    }
    for (const TestCaseInfo &tc : m_db.cases) if (tc.tcID == tcID) { setCaseInfo(&tc); break; }
    m_stepTable->setRowCount(0);
    m_stepSeq = 0;
    m_stepStartTime = QDateTime();

    QString err;
    if (!m_sched->runCase(tcID, err)) {
        Logger::instance().error(QString("用例 %1 启动失败：%2，跳过").arg(tcID).arg(err));
        QTimer::singleShot(0, this, [this]{ startNextInQueue(); });
    }
}

void MainWindow::onStop()
{
    m_runQueue.clear();
    m_queueActive = false;
    m_sched->stop();
    m_running = false;
    m_sysState->setText("已停止"); m_sysState->setStyleSheet("color:#D14343;font-weight:bold;");
    Logger::instance().info("用户中止检测");
    closeCaseLog();
}

void MainWindow::onPause()
{
    Logger::instance().log(Module_SYSTEM, Log_Info, "暂停：脚本无暂停接口，当前不支持（可中止后重跑）");
}

void MainWindow::onUpgradeFile()
{
    QString f = QFileDialog::getOpenFileName(this, "选择升级文件", AppConfig::instance().upgradeDir, "升级文件 (*.bin *.*)");
    if (!f.isEmpty()) Logger::instance().info("已选择升级文件: " + f);
}

// 运行日志（appendLog / appendLogFile / openCaseLog / closeCaseLog / onClearLog）
// 与测试报告（writeAutoReport / onBrowseReport / onExportReport）实现见 MainWindowLogReport.cpp
void MainWindow::onBreaker(int op)
{
    // op: 0=断电 1=上电（命令 = 3762 02F1 转发，内层为现场解析过的固定 698 帧）
    static const uchar off698[] = {
        0x68,0x27,0x00,0x43,0x05,0x11,0x11,0x11,0x11,0x11,0x11,0x00,0x24,0xb5,0x07,0x01,
        0x03,0x80,0x00,0x81,0x00,0x01,0x01,0x02,0x04,0x51,0xf2,0x05,0x02,0x01,0x11,0x00,
        0x12,0x00,0x01,0x03,0x01,0x00,0x64,0x1b,0x16
    };
    static const uchar on698[] = {
        0x68,0x22,0x00,0x43,0x05,0x11,0x11,0x11,0x11,0x11,0x11,0x00,0xe0,0xbe,0x07,0x01,
        0x03,0x80,0x00,0x82,0x00,0x01,0x01,0x02,0x02,0x51,0xf2,0x05,0x02,0x01,0x16,0x00,
        0x00,0x62,0x52,0x16
    };
    const QByteArray inner = (op == 0)
        ? QByteArray(reinterpret_cast<const char*>(off698), int(sizeof(off698)))
        : QByteArray(reinterpret_cast<const char*>(on698),  int(sizeof(on698)));

    if (m_db.concentrators.isEmpty()) {
        QMessageBox::warning(this, "断路器", "未配置 CCO 档案"); return;
    }
    const ConcentratorInfo &cco = m_db.concentrators.first();
    if (!m_serial->isOpen(cco.slotPosition, cco.dvcId)) {
        QMessageBox::warning(this, "断路器", "请先连接 CCO"); return;
    }

    const QString bhex = m_breakerAddr->text().trimmed().remove(' ');
    const QByteArray dst6 = QByteArray::fromHex(bhex.toLatin1());
    if (dst6.size() != 6) {
        QMessageBox::warning(this, "断路器", "断路器地址需要 12 位十六进制（6 字节）"); return;
    }
    const QByteArray src6(reinterpret_cast<const char*>(cco.ccoAddr), 6);

    const QByteArray frame = BreakerCtrl::build02F1(src6, dst6, inner, m_breakerSeq++, 0x03);
    m_serial->send(cco.slotPosition, cco.dvcId, frame);

    const char *name[] = {"断电", "上电"};
    Logger::instance().log(Module_SYSTEM, Log_Info,
        QString("[断路器] %1（02F1+698）：%2").arg(name[op]).arg(QString(frame.toHex(' ').toUpper())));
    if (op == 1) { m_breakerState->setText("已上电"); m_breakerState->setStyleSheet("color:#1E8E3E;"); }
    else        { m_breakerState->setText("已断电"); m_breakerState->setStyleSheet("color:#D14343;"); }
}

void MainWindow::onBreakerAddNode()
{
    if (m_db.concentrators.isEmpty()) {
        QMessageBox::warning(this, "下发", "未配置 CCO 档案"); return;
    }
    const ConcentratorInfo &cco = m_db.concentrators.first();
    const int ccoId = cco.dvcId;
    if (!m_serial->isOpen(cco.slotPosition, ccoId)) {
        QMessageBox::warning(this, "下发", "请先连接 CCO"); return;
    }

    QString hex = m_breakerAddr->text().trimmed().remove(' ').toUpper();
    static const QString kHexSet = "0123456789ABCDEF";
    bool okHex = (hex.size() == 12);
    for (int i = 0; okHex && i < hex.size(); ++i)
        if (!kHexSet.contains(hex.at(i))) okHex = false;
    if (!okHex) {
        QMessageBox::warning(this, "下发", "断路器地址需要 12 位十六进制（6 字节），例如 111111111111");
        return;
    }

    const QByteArray addr6 = QByteArray::fromHex(hex.toLatin1());
    const QByteArray frame = BreakerCtrl::buildAddNode11F1(addr6, m_breakerSeq++, 0x03);
    m_serial->send(cco.slotPosition, ccoId, frame);
    Logger::instance().log(Module_SYSTEM, Log_Info,
        QString("[断路器] 下发添加从节点(11F1) 地址=%1：%2")
        .arg(hex).arg(QString(frame.toHex(' ').toUpper())));
}

// ===================== 调度回调 =====================
void MainWindow::onCaseStarted(int tcID, const QString &name, int runIdx, int total)
{
    m_curCaseVal->setText(QString("TC_%1 - %2").arg(tcID, 5, 10, QChar('0')).arg(name));
    openCaseLog(tcID, name, runIdx, total);
    m_stepStartTime = QDateTime();
    for (const TestCaseInfo &tc : m_db.cases) if (tc.tcID == tcID) { setCaseInfo(&tc); break; }
}

void MainWindow::onCaseProgress(int /*tcID*/, int /*state*/, const QString &desc)
{
    const QDateTime now = QDateTime::currentDateTime();
    int row = m_stepTable->rowCount();
    // 上一步收尾：标记完成，补全结束时间与耗时
    if (row > 0 && m_stepStartTime.isValid()) {
        m_stepTable->item(row - 1, 2)->setText("完成");
        m_stepTable->setItem(row - 1, 4, new QTableWidgetItem(now.toString("HH:mm:ss")));
        const double secs = m_stepStartTime.msecsTo(now) / 1000.0;
        m_stepTable->setItem(row - 1, 5, new QTableWidgetItem(QString::number(secs, 'f', 1) + " s"));
    }
    // 把脚本实际上报的进度作为执行步骤逐条记录
    m_stepTable->insertRow(row);
    m_stepTable->setItem(row, 0, new QTableWidgetItem(QString::number(++m_stepSeq)));
    m_stepTable->setItem(row, 1, new QTableWidgetItem(desc));
    m_stepTable->setItem(row, 2, new QTableWidgetItem("进行中"));
    m_stepTable->setItem(row, 3, new QTableWidgetItem(now.toString("HH:mm:ss")));
    // 限制步骤表行数：长时间运行(尤其大批量抄表)会堆到上万行，表格膨胀导致卡顿。
    // 只保留最近若干条（步骤编号用 m_stepSeq 持续累计，仍为真实序号）。
    const int kMaxStepRows = 400;
    while (m_stepTable->rowCount() > kMaxStepRows)
        m_stepTable->removeRow(0);
    m_stepTable->scrollToBottom();
    m_stepStartTime = now;
}

void MainWindow::onCaseFinished(int /*tcID*/, int result, int /*runIdx*/)
{
    Q_UNUSED(result);
    int rc = m_stepTable->rowCount();
    if (rc > 0) {
        const QDateTime now = QDateTime::currentDateTime();
        m_stepTable->item(rc - 1, 2)->setText("完成");
        if (!m_stepTable->item(rc - 1, 4)) {
            m_stepTable->setItem(rc - 1, 4, new QTableWidgetItem(now.toString("HH:mm:ss")));
            if (m_stepStartTime.isValid()) {
                const double secs = m_stepStartTime.msecsTo(now) / 1000.0;
                m_stepTable->setItem(rc - 1, 5, new QTableWidgetItem(QString::number(secs, 'f', 1) + " s"));
            }
        }
    }
}

void MainWindow::onCaseResult(const CaseResult &r)
{
    QTreeWidgetItem *it = m_itemById.value(r.tcID, nullptr);
    if (it) {
        m_updatingChecks = true;
        const QString base = it->data(1, Qt::UserRole).toString();
        it->setText(1, QString("%1  [%2]").arg(base, r.resultText()));
        QColor col = (r.result == 0) ? QColor(0xE6,0xF4,0xEA) : (r.result == 1) ? QColor(0xFB,0xE6,0xE6) : QColor(0xFD,0xF1,0xDD);
        it->setBackground(0, col); it->setBackground(1, col);
        it->setToolTip(1, r.info);
        m_updatingChecks = false;
    }
    // 同一用例重复执行只按最近一次结果计入统计，避免重复计数
    m_caseResult[r.tcID] = r.result;
    refreshStats();

    if (r.runIndex >= r.totalTimes && m_queueActive)
        QTimer::singleShot(0, this, [this]{ startNextInQueue(); });
}

// ===================== 统计 =====================
void MainWindow::refreshStats()
{
    const int total = m_db.cases.size();
    int unsupported = 0;
    for (const TestCaseInfo &tc : m_db.cases)
        if (caseUnsupported(tc.tcID)) ++unsupported;
    const int runnable = qMax(0, total - unsupported);

    int pass = 0, fail = 0, error = 0;
    for (auto it = m_caseResult.constBegin(); it != m_caseResult.constEnd(); ++it) {
        if (it.value() == 0) ++pass;
        else if (it.value() == 1) ++fail;
        else ++error;
    }
    const int done = m_caseResult.size();
    const int base = (m_batchTotal > 0) ? m_batchTotal : runnable;
    const int notRun = qMax(0, base - done);

    QVector<RingChart::Seg> segs;
    segs << RingChart::Seg{ QColor(Theme::GREEN),  double(pass) }
         << RingChart::Seg{ QColor(Theme::RED),    double(fail) }
         << RingChart::Seg{ QColor(Theme::ORANGE), double(error) }
         << RingChart::Seg{ QColor("#C2CEDD"),     double(notRun) };
    m_statRing->setSegments(segs, QString::number(done), "已完成");

    auto pct = [&](int n){ return base > 0 ? n * 100.0 / base : 0.0; };
    QString legend = QString(
        "<div style='line-height:170%'>"
        "<span style='color:#1E8E3E'>通过</span> %1 (%2%)<br>"
        "<span style='color:#D14343'>失败</span> %3 (%4%)<br>"
        "<span style='color:#E08A1E'>错误</span> %5 (%6%)<br>"
        "<span style='color:#9AA7B6'>未执行</span> %7 (%8%)</div>")
        .arg(pass).arg(QString::number(pct(pass),'f',1))
        .arg(fail).arg(QString::number(pct(fail),'f',1))
        .arg(error).arg(QString::number(pct(error),'f',1))
        .arg(notRun).arg(QString::number(pct(notRun),'f',1));
    if (unsupported > 0)
        legend += QString("<div style='color:#AAB3BF;font-size:11px'>另有 %1 项暂不支持，暂不可执行</div>")
                  .arg(unsupported);
    m_statLegend->setText(legend);

    double rate = done > 0 ? pass * 100.0 / done : 0.0;
    m_passRateVal->setText(QString::number(rate, 'f', 1) + "%");
    int op = base > 0 ? done * 100 / base : 0;
    m_progBar->setValue(op); m_progPct->setText(QString::number(op) + "%");
    m_totalVal->setText(QString::number(total));
}

// 日志（appendLog / appendLogFile）实现见 MainWindowLogReport.cpp

// ===================== 计时 =====================
void MainWindow::onTick()
{
    m_sbTime->setText(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"));
    if (m_running) {
        qint64 s = m_runElapsed.elapsed() / 1000;
        m_runTimeVal->setText(QString("%1:%2:%3")
            .arg(s / 3600, 2, 10, QChar('0')).arg((s % 3600) / 60, 2, 10, QChar('0')).arg(s % 60, 2, 10, QChar('0')));
    }
}
