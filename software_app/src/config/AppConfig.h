#ifndef APPCONFIG_H
#define APPCONFIG_H
//
// 应用级配置：插件路径、DataBase 目录、频段、重复测试次数、超时等。
//
#include <QString>

struct AppConfig
{
    // 测试脚本插件 DLL（QPluginLoader 加载）
    QString pluginPath  = "SgHplcTestScript.dll";
    // DataBase 目录（含 TestCase.csv / ConcentratorInfo.csv / MeterInfo.csv /
    // SchemeInfo.csv / DvcSerialInfo.csv 及 ScriptPara/）
    QString dataBaseDir = "DataBase";
    QString scriptParaDir = "DataBase/ScriptPara";
    QString upgradeDir    = "DataBase/Upgrade";

    // 频段（addAddrsInfo 的 freq；低4位=频段0-3，高4位=协议选择，0x03=OOP）
    int  freqBand   = 2;
    int  protocolHi = 0;          // 高4位协议选择（0/3）
    int  freqEncoded() const { return ((protocolHi & 0x0f) << 4) | (freqBand & 0x0f); }

    // 重复测试次数（SysIniInfo.TOTAL_TEST_TIMES）
    int  totalTestTimes = 1;

    // 控制命令异步回执延时(ms)，模拟真实控制时延、避免同栈重入
    int  ctrlAckDelayMs = 50;

    // 串口直连 CCO 模式。
    bool directCcoMode = true;
    // Open a second serial endpoint for the software virtual meter.
    // When enabled, legacy script sends to table-side devices are ignored.
    bool virtualMeterSerialEnabled = true;
    // 是否强制关闭脚本组网(needBuildNet=false)。
    //   false：保留脚本组网检测——needBuildNet=true 的用例会先发 10F21 探测拓扑
    //          （纯 CCO 串口、零 controlDvc），在网数与档案一致才进入正题，否则重新组网。
    //          这正是"测试前确认组网完成"所需，故默认开启（=false）。
    bool skipScriptBuildNetwork = false;
    // 是否强制 needPowerOff=false：保持开启，避免脚本结束时断 STA 电，
    //   使网络在批量用例间保持在线、后续用例探测即过、无需反复重建。
    bool skipScriptPowerOff = true;

    static AppConfig& instance() { static AppConfig c; return c; }
};

#endif // APPCONFIG_H
