# 国网 HPLC 自动化测试系统

面向 **HPLC（高速电力线载波）通信单元（CCO/STA）** 的自动化测试上位机。
通过**串口直连 CCO 本地 UART**（抄控器底座 + 排针接触）下发 376.2 / 645 / 698 报文，
驱动一套不修改的测试脚本插件，自动执行组网、抄表、升级、广播校时、断路器控制等测试项并出报告。

> 早期方案经"工装板 + 网络"控制；当前为**串口直连 CCO**，上位机以主站角色直接与 CCO 本地口通信。

---

## 系统架构

```
┌─────────────────────────────┐      串口(9600-8-E-1)      ┌─────────┐
│  上位机 software_app (Qt)   │ ───── 376.2/645/698 ─────▶ │   CCO   │
│  ┌───────────────────────┐  │                            │ (被测)  │
│  │ ScriptHost (QPluginLoader) │ ◀── 加载 ──┐             └────┬────┘
│  └───────────────────────┘  │            │                   │ PLC
│  ┌───────────────────────┐  │     SgHplcTestScript.dll        │
│  │ SerialComm / DeviceCtrl│  │     (测试脚本插件，不改)        ▼
│  │ VirtualMeter(虚拟表)   │  │                              STA / 电表
│  └───────────────────────┘  │
└─────────────────────────────┘
```

- **software_app**：上位机程序（UI、串口、调度、报告、虚拟表、断路器控制）。
- **sgcc_test_script**：测试脚本插件源码，编译成 `SgHplcTestScript.dll`，由上位机经 `QPluginLoader` 加载。
- **直连 CCO 模式（directCcoMode）**：无工装板，STA 上下电/复位等设备控制由上位机虚拟应答；
  `SetBaudRate` 针对 CCO 时会真改本机串口波特率以跟随。

---

## 目录结构

```
sgcc_auto_test/
├── software_app/                 上位机源码（Qt 工程 software_app.pro）
│   └── src/
│       ├── ui/                   主界面、参数配置对话框、日志/报告
│       ├── comm/                 SerialComm（串口 + 376.2 拆帧）
│       ├── host/                 ScriptHost（插件宿主，回灌 processMsg）
│       ├── device/               DeviceControl / BreakerCtrl / VirtualMeter
│       ├── core/                 Scheduler / Reporter / ConfigLoader
│       └── config/               AppConfig
├── sgcc_test_script/
│   └── sgcc_test_script/         插件源码（SgHplcTestScript.pro）
│       ├── TestCase/             各测试用例脚本（抄表/组网/升级/广播校时/20规范…）
│       ├── CommonModule/         BuildNetwork / BuildNetworkDetect 等公共模块
│       └── 3rdparty/StaticLib/   QGDW_376_2 / DLT_645 / OOP / QCSG 协议库（含头与 release .a）
├── DataBase/                     运行档案与配置（见下）
└── .gitignore
```

> 注：编译产物（`build-*/`、部署 DLL、debug 静态库、固件 `*.bin`、`logs/`、`test_report/`）已被 `.gitignore` 排除，不入库。

---

## 编译构建

- **工具链**：Qt **5.9.9 MinGW 32-bit**（与 CCO 端工具链一致）。
- **插件与上位机分别编译**，且都用 **Release**（插件与宿主的 Debug/Release 必须一致，否则插件加载不上）。

1. **插件**：用 Qt Creator 打开 `sgcc_test_script/sgcc_test_script/SgHplcTestScript.pro` → MinGW 32-bit Release → 构建，
   产出 `SgHplcTestScript.dll`，放到上位机运行目录的 `DataBase/Scripts/` 下。
2. **上位机**：打开 `software_app/software_app.pro` → 同套件 Release → 构建。

常见坑：
- **改了工程位置后第一次编译报 `frame645helper.h` 找不到**：删掉旧的影子构建目录、重跑 qmake 再构建。
- **路径不要太深**（Windows 260 长度限制），建议放在较短路径下。

---

## 配置说明（DataBase）

运行目录（exe 同级）下的 `DataBase/`：

| 文件 | 说明 |
|---|---|
| `ConcentratorInfo.csv` | 集中器(CCO)档案：`ConcentratorId,CcoAddress,SlotPosition,DvcId` |
| `MeterInfo.csv` | 电表档案：`MeterId,MeterAddress,SlotPosition,Phase,PhaseSeq,Protocol,CJQ_Address,DvcId` |
| `SchemeInfo.csv` | 方案：`SchemeId,ConcentratorId,MeterId`（MeterId 支持区间，如 `1-256`） |
| `TestCase.csv` | 用例清单：`CaseId,CaseName,SchemeId,ScriptDllName,PARA_FILE_NAME,CATALOGUE` |
| `ScriptPara/*.txt` | 各用例的脚本参数（needBuildNet、波特率门限、ccoVendorChipCode 等） |
| `Upgrade/` | 升级固件（不入库） |

运行目录根部还有：

- **`PropertyConfig.ini`**（芯片/厂商信息，脚本据此判定）：
  ```ini
  [SYSTEM_PROPERTY]
  ChipType=GY        ; 芯片类型（本平台 GY，等价旧 V2 判定）
  CCOVendorCoder=G1  ; 厂商代码
  CCOChipCode=GY     ; 芯片代码（升级后校验 厂商芯片代码=G1GY）
  ```
- `ScriptConfig.ini`：`[TEST_DEVICE_PROPERTY] PowerState=true` 等。

> 串口固定 **9600-8-E-1**（与 CSV 无关）；测试过程中脚本会按需切换波特率，上位机跟随。
> CSV 编码 UTF-8 / GBK 均可（加载时自动识别）。

---

## 主要功能

- **测试用例**：抄表（13F1/14F1/F1F1）、组网、升级（CCO/STA/15F1 系列）、广播校时、20规范、多功能测试等，
  支持勾选、批量顺序执行、进度/通过率统计、自动出报告。
- **虚拟表**：参数配置里可设虚拟表地址与 645/698 应答项、波特率协商行为，模拟表端应答。
- **断路器控制**：面板可下发断路器地址（11F1 添加从节点入网）、断电/上电（376.2 02F1 转发内层 698 帧）。
- **日志与报告**：每用例独立日志（按运行时间命名、不覆盖历史）；测试完在 `test_report/` 生成 CSV，
  "查看报告"打开目录并定位最新结果。

---

## 备注

- 本仓库只含**源码 + 配置**；固件、编译产物、部署运行库不入库。
- 这是内部测试系统，仓库**私有**保管。
