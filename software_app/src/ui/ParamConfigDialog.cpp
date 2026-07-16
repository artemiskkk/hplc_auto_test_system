#include "ui/ParamConfigDialog.h"
#include "device/VirtualMeter.h"
#include "comm/SerialComm.h"
#include "common/Logger.h"

#include <QtWidgets>
#include <QEvent>
#include <QMouseEvent>

namespace {

struct ConfigRow
{
    QString group;
    QString key;
    QString name;
};

class DialogDragMover : public QObject
{
public:
    explicit DialogDragMover(QWidget *target) : QObject(target), m_target(target) {}

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
    QWidget *m_target = nullptr;
    QPoint m_offset;
    bool m_pressed = false;
};

QTreeWidget *makeTree(const QStringList &headers)
{
    QTreeWidget *tree = new QTreeWidget;
    tree->setObjectName("cfgTree");
    tree->setColumnCount(headers.size());
    tree->setHeaderLabels(headers);
    tree->setRootIsDecorated(true);
    tree->setAlternatingRowColors(true);
    tree->setMinimumHeight(360);
    tree->setIndentation(16);
    tree->setUniformRowHeights(true);
    tree->setSelectionMode(QAbstractItemView::NoSelection);
    tree->setFocusPolicy(Qt::NoFocus);
    tree->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    tree->header()->setStretchLastSection(false);
    for (int i = 1; i < headers.size(); ++i) {
        tree->header()->setSectionResizeMode(i, QHeaderView::Fixed);
        tree->header()->resizeSection(i, 150);
    }
    return tree;
}

void addRows(QTreeWidget *tree, const QVector<ConfigRow> &rows,
             const QSet<QString> &checked, QList<QTreeWidgetItem*> *items, bool is698)
{
    QFont mono("Consolas");
    mono.setStyleHint(QFont::Monospace);
    const QBrush bandBg(QColor("#E7EEF7"));
    const QColor groupFg("#14395E");
    const QColor codeFg("#6B7886");

    QMap<QString, QTreeWidgetItem*> groups;
    for (const ConfigRow &row : rows) {
        QTreeWidgetItem *group = groups.value(row.group, nullptr);
        if (!group) {
            group = new QTreeWidgetItem(tree, QStringList() << row.group);
            QFont f = group->font(0);
            f.setBold(true);
            group->setFont(0, f);
            group->setForeground(0, groupFg);
            group->setBackground(0, bandBg);
            group->setBackground(1, bandBg);
            group->setFirstColumnSpanned(true);
            group->setFlags(Qt::ItemIsEnabled);   // 分组行：不可选、不可勾
            group->setExpanded(true);
            groups.insert(row.group, group);
        }

        const QString code = row.key == "READ_ADDR" ? "控制码 13H" : row.key;
        QTreeWidgetItem *item = new QTreeWidgetItem(group, QStringList() << row.name << code);
        item->setData(0, Qt::UserRole, row.key);
        item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
        item->setCheckState(0, checked.contains(is698 ? row.key.toUpper() : row.key) ? Qt::Checked : Qt::Unchecked);
        item->setFont(1, mono);
        item->setForeground(1, codeFg);
        item->setTextAlignment(1, Qt::AlignLeft | Qt::AlignVCenter);
        items->append(item);
    }
}

QVector<ConfigRow> default645Rows()
{
    return QVector<ConfigRow>()
        << ConfigRow{"基础", "READ_ADDR", "搜表 / 读通信地址"}
        << ConfigRow{"基础", "01040004", "通信地址"}
        << ConfigRow{"基础", "01008104", "厂商信息"}
        << ConfigRow{"基础", "0C010004", "通信速率特征字"}
        << ConfigRow{"基础", "01150004", "主动上报状态字"}
        << ConfigRow{"基础", "0B040004", "电表型号"}
        << ConfigRow{"基础", "07050004", "运行状态字 7"}
        << ConfigRow{"基础", "FF050004", "运行状态字块"}
        << ConfigRow{"基础", "01010004", "日期及星期"}
        << ConfigRow{"基础", "02010004", "时间"}
        << ConfigRow{"基础", "000D3003", "开盖次数"}
        << ConfigRow{"电压", "00010102", "A 相电压"}
        << ConfigRow{"电压", "00020102", "B 相电压"}
        << ConfigRow{"电压", "00030102", "C 相电压"}
        << ConfigRow{"电压", "00FF0102", "电压块"}
        << ConfigRow{"电流", "00010202", "A 相电流"}
        << ConfigRow{"电流", "00020202", "B 相电流"}
        << ConfigRow{"电流", "00030202", "C 相电流"}
        << ConfigRow{"电流", "00FF0202", "电流块"}
        << ConfigRow{"电流", "01008002", "零线电流"}
        << ConfigRow{"有功功率", "00000302", "总有功功率"}
        << ConfigRow{"有功功率", "00010302", "A 相有功功率"}
        << ConfigRow{"有功功率", "00020302", "B 相有功功率"}
        << ConfigRow{"有功功率", "00030302", "C 相有功功率"}
        << ConfigRow{"有功功率", "00FF0302", "有功功率块"}
        << ConfigRow{"无功功率", "00000402", "总无功功率"}
        << ConfigRow{"无功功率", "00010402", "A 相无功功率"}
        << ConfigRow{"无功功率", "00020402", "B 相无功功率"}
        << ConfigRow{"无功功率", "00030402", "C 相无功功率"}
        << ConfigRow{"无功功率", "00FF0402", "无功功率块"}
        << ConfigRow{"功率因数", "00000602", "总功率因数"}
        << ConfigRow{"功率因数", "00010602", "A 相功率因数"}
        << ConfigRow{"功率因数", "00020602", "B 相功率因数"}
        << ConfigRow{"功率因数", "00030602", "C 相功率因数"}
        << ConfigRow{"功率因数", "00FF0602", "功率因数块"}
        << ConfigRow{"相角", "00010702", "A 相相角"}
        << ConfigRow{"相角", "00020702", "B 相相角"}
        << ConfigRow{"相角", "00030702", "C 相相角"}
        << ConfigRow{"相角", "00FF0702", "相角块"}
        << ConfigRow{"电能", "00000000", "组合有功电能"}
        << ConfigRow{"电能", "00000100", "正向有功电能"}
        << ConfigRow{"电能", "00000200", "反向有功电能"}
        << ConfigRow{"电能", "00FF0000", "组合有功电能块"}
        << ConfigRow{"电能", "00FF0100", "正向有功电能块"}
        << ConfigRow{"电能", "00FF0200", "反向有功电能块"}
        << ConfigRow{"冻结", "01000605", "上日冻结时间"}
        << ConfigRow{"冻结", "01010605", "上日正向有功电能"}
        << ConfigRow{"冻结", "01020605", "上日反向有功电能"}
        << ConfigRow{"曲线", "020A0004", "一类曲线周期"}
        << ConfigRow{"曲线", "02000106", "上一块一类曲线"}
        << ConfigRow{"温度", "00008002", "表内温度"}
        << ConfigRow{"温度", "02008002", "模块温度"}
        << ConfigRow{"温度", "04008002", "端子温度 1"}
        << ConfigRow{"温度", "05008002", "端子温度 2"};
}

QVector<ConfigRow> default698Rows()
{
    return QVector<ConfigRow>()
        << ConfigRow{"基础", "4001", "通信地址"}
        << ConfigRow{"基础", "4000", "日期时间"}
        << ConfigRow{"电能", "0000", "组合有功电能"}
        << ConfigRow{"电能", "0010", "正向有功电能"}
        << ConfigRow{"电能", "0020", "反向有功电能"}
        << ConfigRow{"电压电流", "2000", "电压"}
        << ConfigRow{"电压电流", "2001", "电流"}
        << ConfigRow{"功率", "2004", "有功功率"}
        << ConfigRow{"功率", "2005", "无功功率"}
        << ConfigRow{"功率", "200A", "功率因数"}
        << ConfigRow{"温度", "1010", "表内温度"}
        << ConfigRow{"温度", "1030", "模块温度"}
        << ConfigRow{"温度", "200C", "采集温度"}
        << ConfigRow{"温度", "2010", "环境温度"}
        << ConfigRow{"温度", "4300", "扩展温度"}
        << ConfigRow{"波特率协商", "F209", "波特率协商确认"};
}

} // namespace

ParamConfigDialog::ParamConfigDialog(VirtualMeter *meter, SerialComm *serial,
                                     DvcType meterDvcType, int meterDvcId,
                                     QWidget *parent)
    : QDialog(parent),
      m_meter(meter),
      m_serial(serial),
      m_meterDvcType(meterDvcType),
      m_meterDvcId(meterDvcId)
{
    buildUi();
    populateTrees();
}

void ParamConfigDialog::buildUi()
{
    setObjectName("dlgRoot");
    setWindowTitle("虚拟表功能");
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_TranslucentBackground);
    setMinimumSize(820, 680);

    // 本对话框作用域样式：精致勾选框 + 行高 + 柔和斑马纹 + 分组色带
    setStyleSheet(R"(
        QTreeWidget::indicator, QCheckBox::indicator { width:18px; height:18px; }
        QTreeWidget::indicator:unchecked, QCheckBox::indicator:unchecked { image:url(:/ui/icons/check_off.png); }
        QTreeWidget::indicator:checked,   QCheckBox::indicator:checked   { image:url(:/ui/icons/check_on.png); }
        #cfgTree { border:1px solid #D6DEEA; border-radius:6px; background:#FFFFFF;
                   alternate-background-color:#F7FAFD; outline:0; }
        #cfgTree::item { height:30px; padding-left:2px; border:none; }
        #cfgTree::item:selected { background:#DCEAF7; color:#14395E; }
        #cfgHint { color:#7C8A9C; font-size:11px; }
        #framePreview { background:#17263A; color:#D7E7F5; border:1px solid #0F1C2B;
                        border-radius:6px; padding:10px; font-family:Consolas; font-size:12px; }
    )");

    QVBoxLayout *outer = new QVBoxLayout(this);
    outer->setContentsMargins(18, 18, 18, 18);
    outer->setSpacing(0);

    QFrame *surface = new QFrame;
    surface->setObjectName("dialogSurface");
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(surface);
    shadow->setBlurRadius(30);
    shadow->setOffset(0, 8);
    shadow->setColor(QColor(12, 36, 56, 105));
    surface->setGraphicsEffect(shadow);
    QVBoxLayout *surfaceLayout = new QVBoxLayout(surface);
    surfaceLayout->setContentsMargins(0, 0, 0, 0);
    surfaceLayout->setSpacing(0);
    outer->addWidget(surface);

    QFrame *dlgHdr = new QFrame;
    dlgHdr->setObjectName("titleBar");
    dlgHdr->setFixedHeight(40);
    dlgHdr->installEventFilter(new DialogDragMover(this));

    QHBoxLayout *hdrLay = new QHBoxLayout(dlgHdr);
    hdrLay->setContentsMargins(14, 0, 6, 0);
    QLabel *dlgTitle = new QLabel("虚拟表功能");
    dlgTitle->setObjectName("titleName");
    QPushButton *dlgClose = new QPushButton(QString(QChar(0x00D7)));
    dlgClose->setObjectName("winClose");
    dlgClose->setCursor(Qt::PointingHandCursor);
    connect(dlgClose, &QPushButton::clicked, this, &QDialog::reject);
    hdrLay->addWidget(dlgTitle);
    hdrLay->addStretch();
    hdrLay->addWidget(dlgClose);
    surfaceLayout->addWidget(dlgHdr);

    QWidget *dlgBody = new QWidget;
    dlgBody->setObjectName("dlgBody");
    QVBoxLayout *root = new QVBoxLayout(dlgBody);
    root->setContentsMargins(18, 16, 18, 14);
    root->setSpacing(12);
    surfaceLayout->addWidget(dlgBody, 1);

    QGroupBox *addrGroup = new QGroupBox("虚拟表基础参数");
    QFormLayout *addressForm = new QFormLayout(addrGroup);
    addressForm->setContentsMargins(12, 8, 12, 12);
    addressForm->setHorizontalSpacing(12);
    addressForm->setLabelAlignment(Qt::AlignRight);

    m_addrEdit = new QLineEdit(m_meter ? m_meter->meterAddress() : QString("112233445566"));
    m_addrEdit->setMaxLength(12);
    m_addrEdit->setMinimumHeight(34);
    m_addrEdit->setPlaceholderText("112233445566（12 位十六进制）");
    addressForm->addRow("表地址:", m_addrEdit);
    root->addWidget(addrGroup);

    m_modeTabs = new QTabWidget;
    m_modeTabs->setDocumentMode(false);
    root->addWidget(m_modeTabs, 1);

    QWidget *autoPage = new QWidget;
    QVBoxLayout *autoLayout = new QVBoxLayout(autoPage);
    autoLayout->setContentsMargins(0, 8, 0, 0);
    autoLayout->setSpacing(10);

    m_tabs = new QTabWidget;
    m_tabs->setDocumentMode(false);
    autoLayout->addWidget(m_tabs, 1);

    m_tree645 = makeTree(QStringList() << "645 项目" << "DI");
    m_tree698 = makeTree(QStringList() << "698 项目" << "OI");
    m_tabs->addTab(m_tree645, "DL/T645");
    m_tabs->addTab(m_tree698, "698 / OOP");

    QHBoxLayout *quick = new QHBoxLayout;
    quick->setSpacing(8);
    QPushButton *selectAll = new QPushButton("全选");
    QPushButton *selectNone = new QPushButton("全不选");
    for (QPushButton *button : {selectAll, selectNone}) {
        button->setMinimumHeight(30);
        button->setCursor(Qt::PointingHandCursor);
    }
    connect(selectAll, &QPushButton::clicked, this, [this]{ setCurrentItemsChecked(true); });
    connect(selectNone, &QPushButton::clicked, this, [this]{ setCurrentItemsChecked(false); });
    quick->addWidget(selectAll);
    quick->addWidget(selectNone);
    quick->addStretch();
    QLabel *hint = new QLabel("勾选项 = 虚拟表收到对应抄读时予以应答");
    hint->setObjectName("cfgHint");
    quick->addWidget(hint);
    autoLayout->addLayout(quick);
    m_modeTabs->addTab(autoPage, "自动应答配置");

    QWidget *sendPage = new QWidget;
    QVBoxLayout *sendLayout = new QVBoxLayout(sendPage);
    sendLayout->setContentsMargins(10, 14, 10, 10);
    sendLayout->setSpacing(12);

    QGroupBox *sendOptions = new QGroupBox("表端报文生成");
    QFormLayout *sendForm = new QFormLayout(sendOptions);
    sendForm->setLabelAlignment(Qt::AlignRight);
    sendForm->setHorizontalSpacing(12);
    sendForm->setVerticalSpacing(10);
    m_sendProtocolCombo = new QComboBox;
    m_sendProtocolCombo->addItem("DL/T645", 645);
    m_sendProtocolCombo->addItem("DL/T698 / OOP", 698);
    m_sendProtocolCombo->setMinimumHeight(34);
    m_sendItemCombo = new QComboBox;
    m_sendItemCombo->setMinimumHeight(34);
    sendForm->addRow("通信协议:", m_sendProtocolCombo);
    sendForm->addRow("数据项:", m_sendItemCombo);
    QLabel *direction = new QLabel("发送方向：上位机虚拟表 → STA 表端串口（表端应答帧）");
    direction->setObjectName("cfgHint");
    sendForm->addRow(QString(), direction);
    sendLayout->addWidget(sendOptions);

    QHBoxLayout *portStateLayout = new QHBoxLayout;
    portStateLayout->addWidget(new QLabel("虚拟表串口状态:"));
    m_sendPortStatus = new QLabel;
    const bool meterPortOpen = m_serial && m_serial->isOpen(m_meterDvcType, m_meterDvcId);
    m_sendPortStatus->setText(meterPortOpen ? "已连接" : "未连接（仍可生成报文）");
    m_sendPortStatus->setStyleSheet(meterPortOpen
        ? "color:#1E8E3E;font-weight:bold;"
        : "color:#D14343;font-weight:bold;");
    portStateLayout->addWidget(m_sendPortStatus);
    portStateLayout->addStretch();
    sendLayout->addLayout(portStateLayout);

    QLabel *previewTitle = new QLabel("生成报文");
    QFont previewTitleFont = previewTitle->font();
    previewTitleFont.setBold(true);
    previewTitle->setFont(previewTitleFont);
    sendLayout->addWidget(previewTitle);
    m_framePreview = new QPlainTextEdit;
    m_framePreview->setObjectName("framePreview");
    m_framePreview->setReadOnly(true);
    m_framePreview->setLineWrapMode(QPlainTextEdit::WidgetWidth);
    m_framePreview->setMinimumHeight(150);
    sendLayout->addWidget(m_framePreview, 1);

    QHBoxLayout *sendActions = new QHBoxLayout;
    QPushButton *regenerate = new QPushButton("重新生成");
    QPushButton *sendFrame = new QPushButton("发送报文");
    sendFrame->setObjectName("primary");
    for (QPushButton *button : {regenerate, sendFrame}) {
        button->setMinimumHeight(34);
        button->setCursor(Qt::PointingHandCursor);
    }
    sendActions->addStretch();
    sendActions->addWidget(regenerate);
    sendActions->addWidget(sendFrame);
    sendLayout->addLayout(sendActions);
    m_modeTabs->addTab(sendPage, "报文发送");

    connect(m_sendProtocolCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int){ refreshSendItems(); });
    connect(m_sendItemCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int){ refreshFramePreview(); });
    connect(m_addrEdit, &QLineEdit::textChanged, this, [this](const QString &){ refreshFramePreview(); });
    connect(regenerate, &QPushButton::clicked, this, [this]{ refreshFramePreview(); });
    connect(sendFrame, &QPushButton::clicked, this, [this]{ sendPreviewFrame(); });
    refreshSendItems();

    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    if (QPushButton *ok = buttons->button(QDialogButtonBox::Ok)) {
        ok->setObjectName("primary");
        ok->setText("保存配置");
        ok->setMinimumHeight(32);
        ok->setCursor(Qt::PointingHandCursor);
    }
    if (QPushButton *cancel = buttons->button(QDialogButtonBox::Cancel)) {
        cancel->setText("取消");
        cancel->setMinimumHeight(32);
        cancel->setCursor(Qt::PointingHandCursor);
    }
    connect(buttons, &QDialogButtonBox::accepted, this, &ParamConfigDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    root->addWidget(buttons);
}

void ParamConfigDialog::populateTrees()
{
    populate645Tree();
    populate698Tree();
}

void ParamConfigDialog::populate645Tree()
{
    if (!m_tree645 || !m_meter)
        return;

    m_item645List.clear();
    addRows(m_tree645, default645Rows(), m_meter->enabled645Items(), &m_item645List, false);
}

void ParamConfigDialog::populate698Tree()
{
    if (!m_tree698 || !m_meter)
        return;

    QSet<QString> enabled698Text;
    for (quint16 oi : m_meter->enabled698Items())
        enabled698Text.insert(QString("%1").arg(oi, 4, 16, QChar('0')).toUpper());

    m_item698List.clear();
    addRows(m_tree698, default698Rows(), enabled698Text, &m_item698List, true);
}

void ParamConfigDialog::setCurrentItemsChecked(bool checked)
{
    const QList<QTreeWidgetItem*> &items = (m_tabs && m_tabs->currentWidget() == m_tree645) ? m_item645List : m_item698List;
    for (QTreeWidgetItem *item : items)
        item->setCheckState(0, checked ? Qt::Checked : Qt::Unchecked);
}

void ParamConfigDialog::refreshSendItems()
{
    if (!m_sendProtocolCombo || !m_sendItemCombo || !m_meter)
        return;

    const bool is645 = m_sendProtocolCombo->currentData().toInt() == 645;
    const QVector<ConfigRow> rows = is645 ? default645Rows() : default698Rows();
    m_sendItemCombo->blockSignals(true);
    m_sendItemCombo->clear();
    for (const ConfigRow &row : rows) {
        QByteArray frame;
        if (is645) {
            frame = m_meter->buildManual645Response(row.key);
        } else {
            bool ok = false;
            const quint16 oi = row.key.toUShort(&ok, 16);
            if (ok)
                frame = m_meter->buildManual698Response(oi);
        }
        if (!frame.isEmpty())
            m_sendItemCombo->addItem(QString("%1  [%2]").arg(row.name, row.key), row.key);
    }
    m_sendItemCombo->blockSignals(false);
    refreshFramePreview();
}

void ParamConfigDialog::refreshFramePreview()
{
    m_manualFrame.clear();
    if (!m_meter || !m_sendProtocolCombo || !m_sendItemCombo || !m_framePreview)
        return;

    const QString editedAddr = m_addrEdit ? m_addrEdit->text().trimmed().remove(' ').toUpper() : QString();
    const QString originalAddr = m_meter->meterAddress();
    const bool addressChanged = editedAddr != originalAddr;
    if (addressChanged && !m_meter->setMeterAddress(editedAddr)) {
        m_framePreview->setPlainText("表地址无效：请输入 12 位十六进制地址");
        return;
    }

    const QString key = m_sendItemCombo->currentData().toString();
    if (m_sendProtocolCombo->currentData().toInt() == 645) {
        m_manualFrame = m_meter->buildManual645Response(key);
    } else {
        bool ok = false;
        const quint16 oi = key.toUShort(&ok, 16);
        if (ok)
            m_manualFrame = m_meter->buildManual698Response(oi);
    }

    if (addressChanged)
        m_meter->setMeterAddress(originalAddr);

    if (m_manualFrame.isEmpty()) {
        m_framePreview->setPlainText("当前数据项无法生成报文");
        return;
    }

    const QString hex = QString(m_manualFrame.toHex(' ').toUpper());
    QStringList lines;
    const int bytesPerLine = 16;
    for (int byteOffset = 0; byteOffset < m_manualFrame.size(); byteOffset += bytesPerLine)
        lines << hex.mid(byteOffset * 3, qMin(bytesPerLine, m_manualFrame.size() - byteOffset) * 3).trimmed();
    m_framePreview->setPlainText(lines.join('\n'));
}

void ParamConfigDialog::sendPreviewFrame()
{
    refreshFramePreview();
    if (m_manualFrame.isEmpty()) {
        QMessageBox::warning(this, "发送报文", "当前没有可发送的有效报文");
        return;
    }
    if (!m_serial || !m_serial->isOpen(m_meterDvcType, m_meterDvcId)) {
        QMessageBox::warning(this, "发送报文", "虚拟表串口未连接，请先在串口配置中连接虚拟表串口");
        return;
    }

    m_serial->send(m_meterDvcType, m_meterDvcId, m_manualFrame);
    if (m_sendPortStatus) {
        m_sendPortStatus->setText("报文已发送");
        m_sendPortStatus->setStyleSheet("color:#1E8E3E;font-weight:bold;");
    }
    Logger::instance().info(QString("[虚拟表功能] 手动发送 %1 报文：%2")
                            .arg(m_sendProtocolCombo->currentText(),
                                 QString(m_manualFrame.toHex(' ').toUpper())));
}

QSet<QString> ParamConfigDialog::collect645Items() const
{
    QSet<QString> items;
    for (QTreeWidgetItem *item : m_item645List) {
        if (item->checkState(0) == Qt::Checked)
            items.insert(item->data(0, Qt::UserRole).toString().toUpper());
    }
    return items;
}

QSet<quint16> ParamConfigDialog::collect698Items() const
{
    QSet<quint16> items;
    for (QTreeWidgetItem *item : m_item698List) {
        if (item->checkState(0) != Qt::Checked)
            continue;

        bool ok = false;
        const quint16 oi = item->data(0, Qt::UserRole).toString().toUShort(&ok, 16);
        if (ok)
            items.insert(oi);
    }
    return items;
}

void ParamConfigDialog::accept()
{
    if (!m_meter)
        return;

    QSet<QString> enabled645 = collect645Items();
    QSet<quint16> enabled698 = collect698Items();
    if (enabled645.isEmpty() && enabled698.isEmpty()) {
        QMessageBox::warning(this, "虚拟表功能", "至少勾选一个 645 或 698 应答项");
        return;
    }

    const QString addr = m_addrEdit->text().trimmed().remove(' ').toUpper();
    if (!m_meter->setMeterAddress(addr)) {
        QMessageBox::warning(this, "虚拟表功能", "表地址必须是 12 位十六进制，例如 112233445566");
        return;
    }

    m_meter->setEnabled645Items(enabled645);
    m_meter->setEnabled698Items(enabled698);
    QDialog::accept();
}
