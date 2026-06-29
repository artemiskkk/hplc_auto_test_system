#-------------------------------------------------
# 国网 HPLC 自动化测试系统 — 串口直连 CCO 上位机
# 详细设计 V1.1 对应实现
#
# 说明：本程序通过 QPluginLoader 加载既有测试脚本插件 SgHplcTestScript.dll，
#       实现 AbstractScriptHost，与插件零改动集成。
#       插件接口头与插件工程共享（保证 ABI 一致），见 PLUGIN_DIR。
#-------------------------------------------------

QT       += core gui widgets serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG   += c++11
TARGET    = sgcc_auto_test
TEMPLATE  = app

DEFINES  += QT_DEPRECATED_WARNINGS

# === 测试脚本插件源码目录（提供共享接口头：AbstractScript / AbstractScriptHost /
#     AbstractPluginScript / commdatatype / logitem）。请按实际路径调整。 ===
PLUGIN_DIR = D:/QT/SGCC_auto/sgcc_test_script/sgcc_test_script
INCLUDEPATH += $$PLUGIN_DIR
INCLUDEPATH += $$PWD/src

SOURCES += \
    src/main.cpp \
    src/core/ConfigLoader.cpp \
    src/comm/SerialComm.cpp \
    src/device/DeviceControl.cpp \
    src/device/VirtualMeter.cpp \
    src/host/ScriptHost.cpp \
    src/core/Scheduler.cpp \
    src/core/Reporter.cpp \
    src/ui/RingChart.cpp \
    src/ui/ParamConfigDialog.cpp \
    src/ui/MainWindow.cpp \
    src/ui/MainWindowLogReport.cpp

HEADERS += \
    src/common/Model.h \
    src/common/Logger.h \
    src/config/AppConfig.h \
    src/core/ConfigLoader.h \
    src/comm/SerialComm.h \
    src/device/DeviceControl.h \
    src/device/VirtualMeter.h \
    src/host/ScriptHost.h \
    src/core/Scheduler.h \
    src/core/Reporter.h \
    src/ui/Theme.h \
    src/ui/TitleBar.h \
    src/ui/RingChart.h \
    src/ui/ParamConfigDialog.h \
    src/ui/MainWindow.h

RESOURCES += \
    src/ui/assets/ui_assets.qrc

# 可执行文件图标（Windows 资源管理器/任务栏的 .exe 图标）
RC_ICONS = src/ui/assets/app.ico
