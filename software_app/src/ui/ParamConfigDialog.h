#ifndef PARAMCONFIGDIALOG_H
#define PARAMCONFIGDIALOG_H

#include <QDialog>
#include <QByteArray>
#include <QList>
#include <QSet>
#include <QtGlobal>
#include "PublicDataStruct/commdatatype.h"

class QLineEdit;
class QTabWidget;
class QTreeWidget;
class QTreeWidgetItem;
class QComboBox;
class QPlainTextEdit;
class QLabel;
class VirtualMeter;
class SerialComm;

class ParamConfigDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ParamConfigDialog(VirtualMeter *meter, SerialComm *serial,
                               DvcType meterDvcType, int meterDvcId,
                               QWidget *parent = nullptr);

protected:
    void accept() override;

private:
    void buildUi();
    void populateTrees();
    void populate645Tree();
    void populate698Tree();
    void setCurrentItemsChecked(bool checked);
    void refreshSendItems();
    void refreshFramePreview();
    void sendPreviewFrame();
    QSet<QString> collect645Items() const;
    QSet<quint16> collect698Items() const;

    VirtualMeter *m_meter = nullptr;
    SerialComm *m_serial = nullptr;
    DvcType m_meterDvcType = SingleMeter;
    int m_meterDvcId = 1;
    QLineEdit *m_addrEdit = nullptr;
    QTabWidget *m_modeTabs = nullptr;
    QTabWidget *m_tabs = nullptr;
    QTreeWidget *m_tree645 = nullptr;
    QTreeWidget *m_tree698 = nullptr;
    QList<QTreeWidgetItem*> m_item645List;
    QList<QTreeWidgetItem*> m_item698List;
    QComboBox *m_sendProtocolCombo = nullptr;
    QComboBox *m_sendItemCombo = nullptr;
    QPlainTextEdit *m_framePreview = nullptr;
    QLabel *m_sendPortStatus = nullptr;
    QByteArray m_manualFrame;
};

#endif // PARAMCONFIGDIALOG_H
