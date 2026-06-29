#ifndef CONFIGLOADER_H
#define CONFIGLOADER_H
//
// DataBase CSV 解析（GBK 编码），构建内存模型 Database。
//
#include <QString>
#include "common/Model.h"

class ConfigLoader
{
public:
    // 加载 dbDir 下全部配置；失败返回 false 并填 err。
    static bool load(const QString &dbDir, Database &out, QString &err);

    static bool parseConcentrators(const QString &csv, QList<ConcentratorInfo> &out, QString &err);
    static bool parseMeters       (const QString &csv, QList<MeterInfo>        &out, QString &err);
    static bool parseSchemes      (const QString &csv, QList<SchemeCfgInfo>    &out, QString &err);
    static bool parseTestCases    (const QString &csv, QList<TestCaseInfo>     &out, QString &err);
    static bool parseDvcSerials   (const QString &csv, QList<DvcSerial>        &out, QString &err);

    // 读取 ScriptPara 参数文件为键值字典（键=值，或 CSV 两列）。
    static QMap<QString,QString> loadParaFile(const QString &filePath);

private:
    // 读 GBK 文本并按行/逗号切分（跳过表头），返回每行字段列表。
    static QList<QStringList> readCsv(const QString &path, bool skipHeader, QString &err);
};

#endif // CONFIGLOADER_H
