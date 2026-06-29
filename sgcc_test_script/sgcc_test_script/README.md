# SGCC HPLC 自动化测试系统

面向国网低压电力线宽带载波（HPLC）通信模块的自动化一致性测试平台，依据 QGDW 376.2 等国标协议对 CCO（中央协调器）及从节点设备进行功能验证。

---

## 系统架构

```
上位机测试脚本（Qt/C++）
       ↕ 控制命令 / 报文收发
  硬件接口层（串口/以太网）
       ↕
   CCO 被测设备
       ↕ HPLC 载波
   从节点电表（DL/T 645 / OOP 698）
```

---

## 主要功能

- **自动组网**：驱动 CCO 完成全网组网，监控入网率达到阈值后进入测试流程
- **协议报文收发**：构造、发送 QGDW 376.2 帧，解析 CCO 应答（AFN 10/11/12/13/14 等）
- **状态机驱动**：各测试用例以状态机方式运行，支持超时重试
- **多轮测试**：支持基础测试、重复添加从节点、硬件复位后三轮验证
- **结果判决**：对比 CCO 上报数据与档案预设值，自动统计识别错误电表并输出日志

---

## 测试用例目录

| 目录 | 测试内容 |
|------|---------|
| `LN_Inverse` | 单相零火互逆 / 三相相序识别（10F31 + 13F1 双重验证）|
| `PhaseIdentity` | 相位识别 |
| `BuildNetwork` | 组网流程（添加/删除节点、硬件复位、主节点地址等）|
| `ReadMeter` | 抄表（13F1 单播、14F1 广播、F1F1 并发）|
| `SearchMeter` | 搜表 |
| `Upgrade` | CCO / STA 固件升级 |
| `FreqManage` | 频段管理 |
| `IdSnManage` | 芯片 ID / 序列号管理 |
| `InitializeTest` | 初始化测试 |
| `TransparentTrans` | 透明传输 |
| `MultiTask_V3` | 多任务并发 |
| `20Standard` | 20 标准协议一致性 |

---

## 依赖

- Qt 5.x
- `3rdparty/StaticLib/QGDW_376_2_Protocol_Lib` — QGDW 376.2 协议解析库
- `3rdparty/StaticLib/DLT_645_Protocol_Lib` — DL/T 645 协议解析库
- `3rdparty/StaticLib/QCSG_Local_Module_Protocol_Lib` — 本地模块协议库

---

## 构建

使用 Qt Creator 打开 `SgHplcTestScript.pro` 编译即可。
