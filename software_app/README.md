# 国网 HPLC 自动化测试系统 — 串口直连 CCO 上位机

按《串口直连 CCO 详细设计》实现的上位机。通过 `QPluginLoader` 加载既有测试脚本插件
`SgHplcTestScript.dll`，实现 `AbstractScriptHost`，与插件零改动集成；串口直连 CCO。

## 目录结构

```
software_app/
  software_app.pro
  src/
    main.cpp
    common/   Model.h(内存模型/工具)  Logger.h
    config/   AppConfig.h
    core/     ConfigLoader(CSV解析)  Scheduler(装配/调度/重复/结果)
    comm/     SerialComm(串口，原样转发，不在串口层断帧)
    device/   DeviceControl(controlDvc适配) + BreakerCtrl(断路器)
    host/     ScriptHost(实现 AbstractScriptHost，加载插件，回灌)
    ui/       MainWindow
```

## 依赖与构建

- Qt 5（与插件一致），模块：core gui widgets serialport，C++11。
- 共享接口头：`software_app.pro` 中 `PLUGIN_DIR` 指向脚本插件源码目录
  （默认 `D:/QT/SGCC_auto/sgcc_test_script/sgcc_test_script`，提供
  `PublicDataStruct/abstractscript.h`、`abstractscripthost.h`、
  `AbstractPluginScript.h`、`commdatatype.h`、`logitem.h`）。
  **须与插件使用同一套接口头以保证 ABI 一致。**

```
qmake software_app.pro
make            # 或在 Qt Creator 打开 .pro 构建
```

运行目录需能找到 `SgHplcTestScript.dll`（及其依赖的协议库 DLL，如有）。

## 集成约定（与插件对接）

- 调用顺序：`setScript(className) → setHost → addAddrsInfo(conc,meter,scheme,freq) → config(para) → execute()`。
- 报文内存所有权（详细设计 §4.1，须严格遵守）：
  - 下行 `sendMsg2Dvc(uchar* data)`：脚本 `new[]`，宿主发送后 `delete[]`。
  - 上行 `processMsg(uchar* data)`：宿主 `new[]`，脚本处理后 `delete[]`。
- 串口层不做协议断帧；收到原始字节按 CCO 的 (DvcType,dvcId) 回灌 `processMsg`，
  断帧由脚本完成。回灌的 DvcType/dvcId 必须等于该 CCO 档案的 `slotPosition/dvcId`。
- `controlDvc` 异步回 `processCtrlDvcRes(isSucs=true)`（QTimer::singleShot），避免同栈重入。

## controlDvc 落地（DeviceControl）

| 命令 / 目标 | 动作 |
|---|---|
| SetBaudRate | 设本机串口波特率（params[0]） |
| PowerOn(CCO) | 空操作（CCO 已自组网） |
| PowerOn/Off、ModuleRST(STA) | 断路器控 STA 电（经 CCO 发帧；复位=断电再上电） |
| PowerOff/ModuleRST(CCO) | 断路器控不了 CCO：裁剪或改协议复位(01F1/12F1) |
| EventPin(STA) | 掉电事件建议以断 STA 电触发（待确认） |

## 待确认（与详细设计一致）

- `controlDvc` 完整 params/idList 编码（对照 BuildNetwork_GW 调用点定稿）。
- 断路器寻址与控电命令帧格式（`BreakerCtrl::powerSta` 内 TODO）。
- DataBase 的 `ConcentratorInfo.csv / SchemeInfo.csv / DvcSerialInfo.csv` 列格式
  （本实现按常见列假定，见 `ConfigLoader.cpp`，需与实际文件对齐）。

## 当前完成度

已实现可运行骨架：DataBase 加载、用例树展示、串口连接、插件加载、单用例执行
（含组网前置经 controlDvc 即时成功 + 真实 3762 帧）、重复测试、日志。
报告导出、参数文件精确格式、断路器协议为后续迭代项。
