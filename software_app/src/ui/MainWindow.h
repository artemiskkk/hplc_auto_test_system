#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>
#include <QSet>
#include <QList>
#include <QElapsedTimer>
#include <QDateTime>
#include "common/Model.h"

class QTreeWidget;
class QTreeWidgetItem;
class QTableWidget;
class QPlainTextEdit;
class QComboBox;
class QLabel;
class QProgressBar;
class QTimer;
class QLineEdit;
class QCheckBox;
class QFile;
class QPushButton;
class QResizeEvent;
class QShowEvent;
class QSplitter;

class SerialComm;
class DeviceControl;
class ScriptHost;
class Scheduler;
class Reporter;
class RingChart;
class VirtualMeter;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void showEvent(QShowEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    // 工具栏 / 操作
    void onLoadDataBase();
    void onLoadPlugin();
    void onConnectCco();
    void onStart();
    void onStop();
    void onPause();
    void onViewLog();                  // 打开运行日志存储目录
    void onClearLog();
    void onUpgradeFile();
    void onBrowseReport();
    void onParamConfig();
    void onExportReport();
    void onBreaker(int op);            // 0=断电 1=上电 2=复位
    void onBreakerAddNode();           // 下发 11F1 添加从节点（断路器入网）

    // 调度回调
    void onCaseStarted(int tcID, const QString &name, int runIdx, int total);
    void onCaseProgress(int tcID, int state, const QString &desc);
    void onCaseFinished(int tcID, int result, int runIdx);
    void onCaseResult(const CaseResult &r);

    // 日志 / 计时
    void appendLog(const QString &ts, int module, int catalogue, const QString &msg);
    void onTick();
    void onCaseSelected();

    // 勾选 / 批量
    void onTreeItemChanged(QTreeWidgetItem *item, int column);
    void onSelectAll(bool on);

private:
    void buildUi();
    QWidget* buildToolbar();
    QWidget* buildStatusInfo();
    QWidget* buildLeft();
    QWidget* buildCenter();
    QWidget* buildRight();
    QWidget* buildBottom();
    void buildStatusBar();
    void adjustMainSplit();
    void applyUiScale();
    double currentScreenScale() const;
    int spx(int value) const;
    void buildCaseTree(const QString &filter = QString());
    void autoLoadDataBase();
    bool loadDataBaseFromDir(const QString &dir, bool showError);
    void tryLoadDefaultPlugin();
    int  selectedCaseId() const;
    void refreshStats();
    void setCaseInfo(const TestCaseInfo *tc);
    void setCcoConnectedUi(bool connected, const QString &serialText = QString());
    void writeAutoReport();             // 测试完成后在 test_report 目录生成 CSV 报告
    void appendLogFile(const QString &ts, const QString &level, const QString &source, const QString &msg);
    void openCaseLog(int tcID, const QString &name, int runIdx, int total);  // 打开用例运行日志文件
    void closeCaseLog();                // 关闭当前用例运行日志文件句柄
    QList<int> checkedCaseIds() const;  // 勾选的用例（按档案顺序）
    void updateSelCount();              // 刷新"已选 N 项"与全选框状态
    void startNextInQueue();            // 取队列下一个用例执行
    void resetRunResults();             // 开始新一轮检测前清空上轮结果

    // ===== backend =====
    Database       m_db;
    SerialComm    *m_serial   = nullptr;
    DeviceControl *m_devctrl  = nullptr;
    ScriptHost    *m_host     = nullptr;
    Scheduler     *m_sched    = nullptr;
    Reporter      *m_reporter = nullptr;
    VirtualMeter  *m_virtualMeter = nullptr;
    QMap<int, QTreeWidgetItem*> m_itemById;

    // ===== 状态信息条 =====
    QComboBox   *m_freqCombo  = nullptr;
    QLabel      *m_sysState   = nullptr;
    QLabel      *m_comPortVal = nullptr;
    QLabel      *m_ccoVal     = nullptr;
    QLabel      *m_staVal     = nullptr;
    QProgressBar*m_progBar    = nullptr;
    QLabel      *m_progPct    = nullptr;
    QLabel      *m_curCaseVal = nullptr;
    QLabel      *m_runTimeVal = nullptr;
    QLabel      *m_totalVal   = nullptr;
    QLabel      *m_passRateVal= nullptr;
    QComboBox   *m_portCombo  = nullptr;
    QComboBox   *m_baudCombo  = nullptr;
    QPushButton *m_connectCcoBtn = nullptr;

    // ===== 左：用例树 =====
    QTreeWidget *m_tree     = nullptr;
    QLineEdit   *m_search   = nullptr;
    QCheckBox   *m_chkAll   = nullptr;
    QLabel      *m_selCount = nullptr;
    QSet<int>    m_checkedIds;            // 已勾选的用例 ID（跨过滤保持）
    bool         m_updatingChecks = false;

    // ===== 中：执行 =====
    QMap<QString,QLabel*> m_info;       // 用例/方案 KV
    QTableWidget *m_stepTable = nullptr;
    QSplitter    *m_mainSplit = nullptr;
    bool          m_screenHooked = false;
    double        m_uiScale = 1.0;
    QWidget      *m_toolbarWidget = nullptr;
    QWidget      *m_appStatusWidget = nullptr;
    QWidget      *m_leftPane = nullptr;
    QWidget      *m_rightPane = nullptr;
    QWidget      *m_bottomPane = nullptr;

    // ===== 底：断路器 / 统计 =====
    QLabel  *m_breakerState = nullptr, *m_breakerMode = nullptr;
    QLineEdit *m_breakerAddr = nullptr;   // 断路器地址输入（可编辑，默认 111111111111）
    uchar    m_breakerSeq = 0;            // 11F1 下发报文序号（自增）

    // ===== 右：运行日志 =====
    QTableWidget *m_logTable = nullptr;
    RingChart    *m_statRing = nullptr;
    QLabel       *m_statLegend = nullptr;

    // ===== 底部状态栏 =====
    QLabel *m_sbLogCount = nullptr, *m_sbCpu = nullptr,
           *m_sbMem = nullptr, *m_sbTime = nullptr, *m_sbStatus = nullptr;

    // ===== 运行态 =====
    QTimer      *m_clock = nullptr;
    QElapsedTimer m_runElapsed;
    QDateTime    m_stepStartTime;        // 当前步骤起始时刻（用于结束时间/耗时）
    int          m_stepSeq = 0;          // 步骤累计序号（步骤表限行后仍保持真实编号）
    bool         m_running = false;
    int          m_logCount = 0;
    QMap<int,int> m_caseResult;          // tcID -> 最近一次结果（0/1/2），统计去重
    QString      m_lastReportPath;       // 最近一次生成的报告路径
    QString      m_logFilePath;          // 当前测试项运行日志路径
    QString      m_lastLogPath;          // 最近一次用例日志文件（关闭后仍保留，供查看）
    QString      m_runStamp;             // 本次"开始检测"的时间戳，拼进各用例日志文件名，避免覆盖历史
    QFile       *m_logFile = nullptr;     // 常开句柄，避免每行 open/close

    // ===== 批量执行队列 =====
    QList<int>   m_runQueue;             // 待执行用例队列
    int          m_batchTotal = 0;       // 本轮批次用例数（进度/统计以此为基数）
    bool         m_queueActive = false;  // 队列是否在跑
};

#endif // MAINWINDOW_H
