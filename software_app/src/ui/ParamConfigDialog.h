#ifndef PARAMCONFIGDIALOG_H
#define PARAMCONFIGDIALOG_H

#include <QDialog>
#include <QList>
#include <QSet>
#include <QtGlobal>

class QLineEdit;
class QTabWidget;
class QTreeWidget;
class QTreeWidgetItem;
class QComboBox;
class QCheckBox;
class VirtualMeter;

class ParamConfigDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ParamConfigDialog(VirtualMeter *meter, QWidget *parent = nullptr);

protected:
    void accept() override;

private:
    void buildUi();
    void populateTrees();
    void populate645Tree();
    void populate698Tree();
    void setCurrentItemsChecked(bool checked);
    QSet<QString> collect645Items() const;
    QSet<quint16> collect698Items() const;

    VirtualMeter *m_meter = nullptr;
    QLineEdit *m_addrEdit = nullptr;
    QComboBox *m_baudModeCombo = nullptr;
    QComboBox *m_baudFallbackCombo = nullptr;
    QCheckBox *m_baudSwitchCheck = nullptr;
    QTabWidget *m_tabs = nullptr;
    QTreeWidget *m_tree645 = nullptr;
    QTreeWidget *m_tree698 = nullptr;
    QList<QTreeWidgetItem*> m_item645List;
    QList<QTreeWidgetItem*> m_item698List;
};

#endif // PARAMCONFIGDIALOG_H
