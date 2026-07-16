#ifndef SCRIPTHOST_H
#define SCRIPTHOST_H
//
// 脚本宿主：实现 AbstractScriptHost，经 QPluginLoader 加载脚本插件
// （AbstractPluginScript），管理脚本生命周期并回灌 processMsg / processCtrlDvcRes。
//
#include <QObject>
#include <QPluginLoader>
#include <QList>
#include <QMap>
#include "PublicDataStruct/abstractscripthost.h"
#include "PublicDataStruct/AbstractPluginScript.h"
#include "PublicDataStruct/commdatatype.h"

class SerialComm;
class DeviceControl;
class VirtualMeter;

class ScriptHost : public QObject, public AbstractScriptHost
{
    Q_OBJECT
public:
    explicit ScriptHost(QObject *parent = nullptr);
    ~ScriptHost();

    void setSerial(SerialComm *s)        { m_serial = s; }
    void setDeviceControl(DeviceControl *d);
    void setVirtualMeter(VirtualMeter *meter) { m_virtualMeter = meter; }

    bool loadPlugin(const QString &path, QString &err);
    bool isPluginLoaded() const { return m_plugin != nullptr; }

    // 启动一个用例（档案/方案/参数已装配好）。conc/meter/scheme 在运行期保持有效。
    bool runCase(const QString &className,
                 const QList<ConcentratorInfo> &conc,
                 const QList<MeterInfo> &meter,
                 const QList<SchemeCfgInfo> &scheme,
                 int freqEncoded,
                 const QMap<QString,QString> &para,
                 QString &err);
    void stopCase();

    // —— AbstractScriptHost 实现 ——
    void sendMsg2Dvc(DvcType dvcType, int id, uchar *data, int datalen) override;
    void controlDvc(DvcType dvcType, QList<int> idList, CtrlCmdType cmd, QList<double> params) override;
    void updateProgress(ProcessState state, QString desc) override;
    void recordLog(LogItem *log) override;

signals:
    void progress(int state, const QString &desc);   // 透出给 UI/调度

public slots:
    void onBytesReceived(DvcType dvcType, int id, QByteArray data);
    void onCtrlResult(DvcType dvcType, QList<int> idList, CtrlCmdType cmd, bool isSucs, QList<double> params);

private:
    QPluginLoader        m_loader;
    AbstractPluginScript *m_plugin = nullptr;
    SerialComm           *m_serial = nullptr;
    DeviceControl        *m_devctrl = nullptr;
    VirtualMeter         *m_virtualMeter = nullptr;
    bool                  m_scriptFaulted = false;

    // 运行期档案（addAddrsInfo 取 const 指针，须在执行期间保持有效）
    QList<ConcentratorInfo> m_conc;
    QList<MeterInfo>        m_meter;
    QList<SchemeCfgInfo>    m_scheme;
    QMap<QString,QString>   m_para;
};

#endif // SCRIPTHOST_H
