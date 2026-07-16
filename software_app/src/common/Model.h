#ifndef MODEL_H
#define MODEL_H
//
// 内存数据模型与公共工具。
// 直接复用插件接口头中的结构体定义（ConcentratorInfo / MeterInfo / SchemeCfgInfo /
// TestCaseInfo / DvcSerial / DvcType 等），保证与脚本插件 ABI 一致。
//
#include <QList>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QByteArray>
#include <QMetaType>
#include <QRegExp>
#include "PublicDataStruct/commdatatype.h"

// 单次用例执行结果（用于报告与界面回显）
struct CaseResult
{
    int     tcID = 0;
    QString name;
    QString catalogue;
    QString scriptClass;
    QString startTime;
    QString endTime;
    int     result = 3;        // TestCaseResult：0=PASS 1=FAIL 2=ERROR 3=UNKNOW
    int     runIndex = 1;
    int     totalTimes = 1;
    QString info;

    QString resultText() const {
        switch (result) {
        case 0: return "PASS";
        case 1: return "FAIL";
        case 2: return "ERROR";
        default: return "UNKNOWN";
        }
    }
};

Q_DECLARE_METATYPE(CaseResult)

// 全部档案/配置/用例的内存模型
struct Database
{
    QList<ConcentratorInfo> concentrators;   // 集中器(CCO)档案
    QList<MeterInfo>        meters;           // 电表档案
    QList<SchemeCfgInfo>    schemes;          // 方案
    QList<TestCaseInfo>     cases;            // 用例
    QList<DvcSerial>        serials;          // 设备串口配置

    const SchemeCfgInfo* schemeById(int id) const {
        for (const auto &s : schemes) if (s.schmID == id) return &s;
        return nullptr;
    }
    const ConcentratorInfo* concById(int ctrID) const {
        for (const auto &c : concentrators) if (c.ctrID == ctrID) return &c;
        return nullptr;
    }
    const MeterInfo* meterById(int mtrID) const {
        for (const auto &m : meters) if (m.mtrID == mtrID) return &m;
        return nullptr;
    }
    const DvcSerial* serialByDvcId(int dvcId) const {
        for (const auto &d : serials) if (d.dvcId == dvcId) return &d;
        return nullptr;
    }
};

namespace ModelUtil
{
    // 槽位中文 → DvcType（依据 DvcType 枚举顺序与旧系统 dvcList 约定）
    inline DvcType slotToDvcType(const QString &slotRaw)
    {
        const QString s = slotRaw.trimmed();
        if (s == "单通")   return SingleSTA;
        if (s == "三通")   return ThreeSTA;
        if (s == "国网" || s == "国网路由") return CCO_GW;
        if (s == "南网" || s == "南网路由") return CCO_NW;
        if (s == "采集器")  return CJQ;
        if (s == "单相表")  return SingleMeter;
        if (s == "三相表")  return ThreeMeter;
        if (s == "集中器表" || s == "采集器表") return CJQMeter;
        // 兼容直接写枚举数字
        bool ok = false; int v = s.toInt(&ok);
        if (ok) return static_cast<DvcType>(v);
        return CCO_GW;
    }

    // "000000002234"(12位BCD字符串) → 6字节地址。不足/超长按低位对齐。
    inline void parseBcdAddr(const QString &hex12, uchar out[6])
    {
        QByteArray b = QByteArray::fromHex(hex12.trimmed().toLatin1());
        for (int i = 0; i < 6; ++i) out[i] = 0;
        int n = qMin(6, b.size());
        // 右对齐拷贝
        for (int i = 0; i < n; ++i)
            out[6 - n + i] = static_cast<uchar>(b.at(b.size() - n + i));
    }

    // 协议码：2=DL/T645，3=OOP/698
    inline uchar protocolCode(const QString &s)
    {
        bool ok = false; int v = s.trimmed().toInt(&ok);
        return ok ? static_cast<uchar>(v) : 2;
    }

    // 从 tcDLLName ("SgHplcTestScript.dll::Script_X") 取脚本类名
    inline QString scriptClassName(const QString &tcDLLName)
    {
        int idx = tcDLLName.lastIndexOf("::");
        return idx >= 0 ? tcDLLName.mid(idx + 2).trimmed() : tcDLLName.trimmed();
    }

    // 电表编号规格展开：支持 "1-256"(区间)、"1"(单个)、"1-2|5;7-8"(多段)。
    inline QList<int> expandMeterIds(const QString &spec)
    {
        QList<int> ids;
        const QStringList toks = spec.split(QRegExp("[|;,]"), QString::SkipEmptyParts);
        for (const QString &raw : toks) {
            const QString t = raw.trimmed();
            if (t.isEmpty()) continue;
            int dash = t.indexOf('-');
            if (dash > 0) {
                int a = t.left(dash).toInt();
                int b = t.mid(dash + 1).toInt();
                if (a > b) qSwap(a, b);
                for (int i = a; i <= b; ++i) ids << i;
            } else {
                ids << t.toInt();
            }
        }
        return ids;
    }
}

#endif // MODEL_H
