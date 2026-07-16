#include "core/ConfigLoader.h"
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QTextStream>
#include <QTextCodec>

namespace {

QSerialPort::DataBits parseDataBits(const QString &raw)
{
    const int value = raw.trimmed().toInt();
    switch (value) {
    case 5: return QSerialPort::Data5;
    case 6: return QSerialPort::Data6;
    case 7: return QSerialPort::Data7;
    case 8: return QSerialPort::Data8;
    default: return QSerialPort::Data8;
    }
}

QSerialPort::StopBits parseStopBits(const QString &raw)
{
    const QString value = raw.trimmed().toLower();
    if (value == "1.5" || value == "oneandhalf")
        return QSerialPort::OneAndHalfStop;
    if (value == "2" || value == "two")
        return QSerialPort::TwoStop;
    return QSerialPort::OneStop;
}

QSerialPort::Parity parseParity(const QString &raw)
{
    const QString value = raw.trimmed().toLower();
    if (value == "odd" || value == "o" || value == "1")
        return QSerialPort::OddParity;
    if (value == "even" || value == "e" || value == "2")
        return QSerialPort::EvenParity;
    if (value == "mark" || value == "m" || value == "3")
        return QSerialPort::MarkParity;
    if (value == "space" || value == "s" || value == "4")
        return QSerialPort::SpaceParity;
    return QSerialPort::NoParity;
}

uchar parsePhaseCode(const QString &raw)
{
    QString phase = raw.trimmed().toUpper();
    if (phase.isEmpty())
        return 0;

    phase.remove(QRegExp("[\\s,;/|_\\-]+"));
    phase.remove(QChar(0x76F8));

    bool ok = false;
    const int numeric = phase.toInt(&ok);
    if (ok)
        return (numeric >= 0 && numeric <= 12) ? static_cast<uchar>(numeric) : 0;

    if (phase == "A")   return 1;
    if (phase == "B")   return 2;
    if (phase == "AB")  return 3;
    if (phase == "C")   return 4;
    if (phase == "AC")  return 5;
    if (phase == "BC")  return 6;
    if (phase == "ABC") return 7;
    if (phase == "ACB") return 8;
    if (phase == "BAC") return 9;
    if (phase == "BCA") return 10;
    if (phase == "CAB") return 11;
    if (phase == "CBA") return 12;

    return 0;
}

} // namespace

QList<QStringList> ConfigLoader::readCsv(const QString &path, bool skipHeader, QString &err)
{
    QList<QStringList> rows;
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        err = QString("无法打开文件：%1").arg(path);
        return rows;
    }
    QByteArray raw = f.readAll();
    f.close();
    // 自动识别编码：DataBase CSV 多数为 GBK，但个别文件(如另存的 MeterInfo.csv)是 UTF-8。
    // 若强行按 GBK 解 UTF-8，Qt 的 GBK 解码器会把中文后面的逗号当作汉字尾字节吃掉，
    // 导致整行切不出 8 列被全部跳过(表现为“电表 0、没检测到表档案”)。故先验 UTF-8，不合法再回退 GBK。
    QString text;
    if (QTextCodec *utf8 = QTextCodec::codecForName("UTF-8")) {
        QTextCodec::ConverterState st;
        const QString u = utf8->toUnicode(raw.constData(), raw.size(), &st);
        if (st.invalidChars == 0)
            text = u;                       // 合法 UTF-8（含纯 ASCII）
    }
    if (text.isNull()) {                    // 非 UTF-8 → 按 GBK
        QTextCodec *gbk = QTextCodec::codecForName("GBK");
        text = gbk ? gbk->toUnicode(raw) : QString::fromLocal8Bit(raw);
    }
    if (text.startsWith(QChar(0xFEFF)))     // 去掉可能的 BOM
        text.remove(0, 1);

    const QStringList lines = text.split(QRegExp("[\r\n]+"), QString::SkipEmptyParts);
    for (int i = 0; i < lines.size(); ++i) {
        if (skipHeader && i == 0) continue;
        rows << lines.at(i).split(',');
    }
    return rows;
}

bool ConfigLoader::parseConcentrators(const QString &csv, QList<ConcentratorInfo> &out, QString &err)
{
    // 列：CtrId, CcoAddress, SlotPosition, DvcId
    const auto rows = readCsv(csv, true, err);
    if (!err.isEmpty()) return false;
    out.clear();
    for (const auto &c : rows) {
        if (c.size() < 4) continue;
        ConcentratorInfo info;
        info.ctrID = c.at(0).trimmed().toInt();
        ModelUtil::parseBcdAddr(c.at(1), info.ccoAddr);
        info.slotPosition = ModelUtil::slotToDvcType(c.at(2));
        info.dvcId = c.at(3).trimmed().toInt();
        out << info;
    }
    return true;
}

bool ConfigLoader::parseMeters(const QString &csv, QList<MeterInfo> &out, QString &err)
{
    // 列：MeterId, MeterAddress, SlotPosition, Phase, PhaseSeq, Protocol, CJQ_Address, DvcId
    const auto rows = readCsv(csv, true, err);
    if (!err.isEmpty()) return false;
    out.clear();
    for (const auto &c : rows) {
        if (c.size() < 8) continue;
        MeterInfo m;
        m.mtrID = c.at(0).trimmed().toInt();
        ModelUtil::parseBcdAddr(c.at(1), m.mtrAddr);
        m.slotPosition = ModelUtil::slotToDvcType(c.at(2));
        m.realPhase = parsePhaseCode(c.at(3));
        m.phaseSeq  = static_cast<uchar>(c.at(4).trimmed().toInt());
        m.prtcl     = ModelUtil::protocolCode(c.at(5));
        ModelUtil::parseBcdAddr(c.at(6), m.CJQAddr);
        m.dvcId = c.at(7).trimmed().toInt();
        out << m;
    }
    return true;
}

bool ConfigLoader::parseSchemes(const QString &csv, QList<SchemeCfgInfo> &out, QString &err)
{
    // 列：SchemeId, ConcentratorId, MeterId
    // MeterId 采用区间语法，如 "1-256"、"1-2"、"1"（亦兼容 "1-2|5,7" 多段）。
    const auto rows = readCsv(csv, true, err);
    if (!err.isEmpty()) return false;
    out.clear();
    for (const auto &c : rows) {
        if (c.size() < 2) continue;
        SchemeCfgInfo s;
        s.schmID = c.at(0).trimmed().toInt();
        s.ctrID  = c.at(1).trimmed().toInt();
        if (c.size() >= 3) {
            // 合并第 3 列及之后（防止区间含逗号被 CSV 拆分），再统一展开
            const QString spec = QStringList(c.mid(2)).join(",");
            s.mtrIDs = ModelUtil::expandMeterIds(spec);
        }
        out << s;
    }
    return true;
}

bool ConfigLoader::parseTestCases(const QString &csv, QList<TestCaseInfo> &out, QString &err)
{
    // 列：CaseId, CaseName, SchemeId, ScriptDllName, PARA_FILE_NAME, CATALOGUE
    const auto rows = readCsv(csv, true, err);
    if (!err.isEmpty()) return false;
    out.clear();
    for (const auto &c : rows) {
        if (c.size() < 6) continue;
        TestCaseInfo tc;
        tc.tcID = c.at(0).trimmed().toInt();
        tc.tcName = c.at(1).trimmed();
        tc.schmIDs.clear();
        for (const auto &id : c.at(2).split(QRegExp("[|;]"), QString::SkipEmptyParts))
            tc.schmIDs.insert(id.trimmed().toInt());
        tc.tcDLLName = c.at(3).trimmed();
        tc.paramFileName = c.at(4).trimmed();
        tc.catalogueName = c.at(5).trimmed();
        tc.result = TestCaseResult_UNKNOW;
        out << tc;
    }
    return true;
}

bool ConfigLoader::parseDvcSerials(const QString &csv, QList<DvcSerial> &out, QString &err)
{
    // 列：DvcId, DvcName, PortName, BaudRate[, DataBits, StopBits, Parity]
    const auto rows = readCsv(csv, true, err);
    if (!err.isEmpty()) return false;
    out.clear();
    for (const auto &c : rows) {
        if (c.size() < 4) continue;
        DvcSerial d;
        d.dvcId = c.at(0).trimmed().toInt();
        d.dvcName = c.at(1).trimmed();
        d.portName.clear();
        d.baudRate = 9600;
        d.dataBits = QSerialPort::Data8;
        d.stopBits = QSerialPort::OneStop;
        d.parity   = QSerialPort::EvenParity;
        bool thirdIsBaud = false;
        const QString third = c.at(2).trimmed();
        const uint thirdBaud = third.toUInt(&thirdIsBaud);
        if (thirdIsBaud) {
            // Legacy layout: DvcId,DvcName,BaudRate,DataBits,StopBits,Parity
            d.baudRate = thirdBaud;
            if (c.size() >= 4 && !c.at(3).trimmed().isEmpty())
                d.dataBits = parseDataBits(c.at(3));
            if (c.size() >= 5 && !c.at(4).trimmed().isEmpty())
                d.stopBits = parseStopBits(c.at(4));
            if (c.size() >= 6 && !c.at(5).trimmed().isEmpty())
                d.parity = parseParity(c.at(5));
        } else {
            // Current layout: DvcId,DvcName,PortName,BaudRate,DataBits,StopBits,Parity
            d.portName = third;
            d.baudRate = c.at(3).trimmed().toUInt();
            if (d.baudRate == 0)
                d.baudRate = 9600;
            if (c.size() >= 5 && !c.at(4).trimmed().isEmpty())
                d.dataBits = parseDataBits(c.at(4));
            if (c.size() >= 6 && !c.at(5).trimmed().isEmpty())
                d.stopBits = parseStopBits(c.at(5));
            if (c.size() >= 7 && !c.at(6).trimmed().isEmpty())
                d.parity = parseParity(c.at(6));
        }
        out << d;
    }
    return true;
}

QMap<QString,QString> ConfigLoader::loadParaFile(const QString &filePath)
{
    QMap<QString,QString> dic;
    QFile f(filePath);
    if (!f.open(QIODevice::ReadOnly)) return dic;
    QByteArray raw = f.readAll(); f.close();
    QTextCodec *codec = QTextCodec::codecForName("GBK");
    QString text = codec ? codec->toUnicode(raw) : QString::fromLocal8Bit(raw);
    const QStringList lines = text.split(QRegExp("[\r\n]+"), QString::SkipEmptyParts);
    for (const QString &ln : lines) {
        QString line = ln.trimmed();
        if (line.startsWith('#') || line.startsWith("//")) continue;
        int p = line.indexOf('=');
        if (p < 0) p = line.indexOf(',');
        if (p <= 0) continue;
        dic.insert(line.left(p).trimmed(), line.mid(p + 1).trimmed());
    }
    return dic;
}

bool ConfigLoader::load(const QString &dbDir, Database &out, QString &err)
{
    QDir d(dbDir);
    if (!d.exists()) { err = QString("DataBase 目录不存在：%1").arg(dbDir); return false; }

    auto path = [&](const QString &n){ return d.filePath(n); };

    if (!parseTestCases   (path("TestCase.csv"),        out.cases,        err)) return false;
    if (!parseConcentrators(path("ConcentratorInfo.csv"), out.concentrators, err)) return false;
    if (!parseMeters      (path("MeterInfo.csv"),       out.meters,       err)) return false;
    // 方案/串口为可选（缺失不致命）
    QString e2;
    parseSchemes   (path("SchemeInfo.csv"),    out.schemes, e2);
    parseDvcSerials(path("DvcSerialInfo.csv"), out.serials, e2);
    return true;
}
