#include "ui/ParamConfigDialog.h"
#include "device/VirtualMeter.h"

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
        << ConfigRow{"功率因数", "00000602", "总功率因数"}
        << ConfigRow{"功率因数", "00010602", "A 相功率因数"}
        << ConfigRow{"功率因数", "00020602", "B 相功率因数"}
        << ConfigRow{"功率因数", "00030602", "C 相功率因数"}
        << ConfigRow{"功率因数", "00FF0602", "功率因数块"}
        << ConfigRow{"电能", "00000000", "组合有功电能"}
        << ConfigRow{"电能", "00000100", "正向有功电能"}
        << ConfigRow{"电能", "00000200", "反向有功电能"}
        << ConfigRow{"电能", "00FF0000", "组合有功电能块"}
        << ConfigRow{"电能", "00FF0100", "正向有功电能块"}
        << ConfigRow{"电能", "00FF0200", "反向有功电能块"}
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
        << ConfigRow{"波特率协商", "F209", "波特率协商"};
}

} // namespace

ParamConfigDialog::ParamConfigDialog(VirtualMeter *meter, QWidget *parent)
    : QDialog(parent),
      m_meter(meter)
{
    buildUi();
    populateTrees();
}

void ParamConfigDialog::buildUi()
{
    setObjectName("dlgRoot");
    setWindowTitle("参数配置");
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setMinimumSize(760, 600);

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
    )");

    QVBoxLayout *outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->setSpacing(0);

    QFrame *dlgHdr = new QFrame;
    dlgHdr->setObjectName("titleBar");
    dlgHdr->setFixedHeight(40);
    dlgHdr->installEventFilter(new DialogDragMover(this));

    QHBoxLayout *hdrLay = new QHBoxLayout(dlgHdr);
    hdrLay->setContentsMargins(14, 0, 6, 0);
    QLabel *dlgTitle = new QLabel("参数配置");
    dlgTitle->setObjectName("titleName");
    QPushButton *dlgClose = new QPushButton(QString(QChar(0x00D7)));
    dlgClose->setObjectName("winClose");
    dlgClose->setCursor(Qt::PointingHandCursor);
    connect(dlgClose, &QPushButton::clicked, this, &QDialog::reject);
    hdrLay->addWidget(dlgTitle);
    hdrLay->addStretch();
    hdrLay->addWidget(dlgClose);
    outer->addWidget(dlgHdr);

    QWidget *dlgBody = new QWidget;
    dlgBody->setObjectName("dlgBody");
    QVBoxLayout *root = new QVBoxLayout(dlgBody);
    root->setContentsMargins(18, 16, 18, 14);
    root->setSpacing(12);
    outer->addWidget(dlgBody);

    QGroupBox *addrGroup = new QGroupBox("虚拟表");
    QGridLayout *g = new QGridLayout(addrGroup);
    g->setContentsMargins(12, 8, 12, 12);
    g->setHorizontalSpacing(12);
    g->setVerticalSpacing(10);
    g->setColumnStretch(1, 1);
    g->setColumnStretch(3, 1);

    m_addrEdit = new QLineEdit(m_meter ? m_meter->meterAddress() : QString("112233445566"));
    m_addrEdit->setMaxLength(12);
    m_addrEdit->setMinimumHeight(34);
    m_addrEdit->setPlaceholderText("112233445566（12 位十六进制）");
    g->addWidget(new QLabel("表地址"), 0, 0);
    g->addWidget(m_addrEdit, 0, 1, 1, 3);

    m_baudModeCombo = new QComboBox;
    m_baudModeCombo->setMinimumHeight(34);
    m_baudModeCombo->addItem("确认并切换波特率", int(VirtualMeter::BaudConsultConfirm));
    m_baudModeCombo->addItem("否认协商", int(VirtualMeter::BaudConsultDeny));
    m_baudModeCombo->addItem("不响应", int(VirtualMeter::BaudConsultNoResponse));
    m_baudModeCombo->addItem("异常回复", int(VirtualMeter::BaudConsultWrongResponse));
    const int modeIndex = m_baudModeCombo->findData(int(m_meter ? m_meter->baudConsultMode() : VirtualMeter::BaudConsultConfirm));
    if (modeIndex >= 0)
        m_baudModeCombo->setCurrentIndex(modeIndex);

    m_baudFallbackCombo = new QComboBox;
    m_baudFallbackCombo->setMinimumHeight(34);
    m_baudFallbackCombo->addItems(QStringList() << "1200" << "2400" << "4800" << "9600"
                                                << "19200" << "38400" << "57600" << "115200");
    const int fallbackIndex = m_baudFallbackCombo->findText(QString::number(m_meter ? m_meter->baudConsultFallbackBaud() : 9600));
    if (fallbackIndex >= 0)
        m_baudFallbackCombo->setCurrentIndex(fallbackIndex);

    g->addWidget(new QLabel("波特率协商"), 1, 0);
    g->addWidget(m_baudModeCombo, 1, 1);
    g->addWidget(new QLabel("默认波特率"), 1, 2);
    g->addWidget(m_baudFallbackCombo, 1, 3);

    m_baudSwitchCheck = new QCheckBox("确认后同步切换 STA 串口波特率");
    m_baudSwitchCheck->setChecked(!m_meter || m_meter->baudConsultSwitchSerial());
    g->addWidget(m_baudSwitchCheck, 2, 1, 1, 3);
    root->addWidget(addrGroup);

    m_tabs = new QTabWidget;
    m_tabs->setDocumentMode(false);
    root->addWidget(m_tabs, 1);

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
    root->addLayout(quick);

    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    if (QPushButton *ok = buttons->button(QDialogButtonBox::Ok)) {
        ok->setObjectName("primary");
        ok->setText("保存");
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
        QMessageBox::warning(this, "参数配置", "至少勾选一个 645 或 698 应答项");
        return;
    }

    const QString addr = m_addrEdit->text().trimmed().remove(' ').toUpper();
    if (!m_meter->setMeterAddress(addr)) {
        QMessageBox::warning(this, "参数配置", "表地址必须是 12 位十六进制，例如 112233445566");
        return;
    }

    m_meter->setEnabled645Items(enabled645);
    m_meter->setEnabled698Items(enabled698);
    m_meter->setBaudConsultMode(static_cast<VirtualMeter::BaudConsultMode>(m_baudModeCombo->currentData().toInt()));
    m_meter->setBaudConsultFallbackBaud(m_baudFallbackCombo->currentText().toInt());
    m_meter->setBaudConsultSwitchSerial(m_baudSwitchCheck->isChecked());
    QDialog::accept();
}
