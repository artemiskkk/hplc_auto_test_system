#include <QApplication>
#include <QIcon>
#include "ui/MainWindow.h"

#ifdef Q_OS_WIN
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

// 让整个程序使用“系统级 DPI 感知”(单一缩放 = 主屏)。
// 这样副屏由 Windows 直接位图拉伸，Qt 不会因显示器 DPI 不同而按屏重排，
// 跨屏后界面布局与主屏保持一致（副屏可能略糊，但不再错乱）。
// 必须在 QApplication 构造之前调用：进程 DPI 感知只能在 Qt 锁定它之前设置一次。
static void forceSystemDpiAwareness()
{
    // Win10 1607+：SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_SYSTEM_AWARE == (HANDLE)-2)
    typedef BOOL (WINAPI *SetCtxFn)(HANDLE);
    if (HMODULE user32 = GetModuleHandleW(L"user32.dll")) {
        if (auto setCtx = reinterpret_cast<SetCtxFn>(GetProcAddress(user32, "SetProcessDpiAwarenessContext"))) {
            if (setCtx(reinterpret_cast<HANDLE>(-2)))
                return;
        }
    }
    // Win8.1+：SetProcessDpiAwareness(PROCESS_SYSTEM_DPI_AWARE == 1)
    typedef HRESULT (WINAPI *SetAwarenessFn)(int);
    if (HMODULE shcore = LoadLibraryW(L"Shcore.dll")) {
        auto setAwareness = reinterpret_cast<SetAwarenessFn>(GetProcAddress(shcore, "SetProcessDpiAwareness"));
        if (setAwareness) {
            setAwareness(1);   // PROCESS_SYSTEM_DPI_AWARE
            FreeLibrary(shcore);
            return;
        }
        FreeLibrary(shcore);
    }
    // 旧系统兜底：SetProcessDPIAware()（动态取，避免头文件因 _WIN32_WINNT 未声明它）
    typedef BOOL (WINAPI *SetDpiAwareFn)(void);
    if (HMODULE user32b = GetModuleHandleW(L"user32.dll")) {
        if (auto setDpiAware = reinterpret_cast<SetDpiAwareFn>(GetProcAddress(user32b, "SetProcessDPIAware")))
            setDpiAware();
    }
}
#endif

int main(int argc, char *argv[])
{
#ifdef Q_OS_WIN
    forceSystemDpiAwareness();
#endif

    QApplication app(argc, argv);
    app.setApplicationName("sgcc_auto_test");
    app.setOrganizationName("topscomm");
    app.setWindowIcon(QIcon(":/ui/icons/app.ico"));   // 标题栏/任务栏图标

    MainWindow w;
    w.show();
    return app.exec();
}
