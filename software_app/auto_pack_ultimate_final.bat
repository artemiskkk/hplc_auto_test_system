@echo off
setlocal enabledelayedexpansion

rem ==========================================
rem        SGCC Auto Test Single-EXE Packer
rem ==========================================
set "SCRIPT_ROOT=%~dp0"
if "%SCRIPT_ROOT:~-1%"=="\" set "SCRIPT_ROOT=%SCRIPT_ROOT:~0,-1%"

set "SOURCE_EXE_DIR=D:\source_code\auto_testing_sys\sgcc_auto_test\build-software_app-Desktop_Qt_5_9_9_MinGW_32bit-Release\release"
set "SOURCE_EXE_PATH=%SOURCE_EXE_DIR%\sgcc_auto_test.exe"
set "QT_BIN_DIR=D:\soft_ware\qt\qt_old_version_install\5.9.9\mingw53_32\bin"
set "ENIGMA_CONSOLE=D:\soft_ware\Qt_package\Enigma Virtual Box\enigmavbconsole.exe"
set "EVB_GENERATOR=%SCRIPT_ROOT%\generate_enigma_config.ps1"

set "BUILD_EXE_DIR=%SCRIPT_ROOT%\build_exe"
set "PACKED_DIR=%SCRIPT_ROOT%\packed"
set "EVB_FILE=%SCRIPT_ROOT%\final_pack.evb"
set "FINAL_EXE_NAME=sgcc_auto_test.exe"

echo ==========================================
echo       SGCC Auto Test Auto Packer
echo ==========================================
echo.

echo [1/4] Preparing environment...
taskkill /F /IM "%FINAL_EXE_NAME%" >nul 2>&1

if exist "%BUILD_EXE_DIR%" rd /s /q "%BUILD_EXE_DIR%" 2>nul
mkdir "%BUILD_EXE_DIR%"
if not exist "%PACKED_DIR%" mkdir "%PACKED_DIR%"
if exist "%PACKED_DIR%\%FINAL_EXE_NAME%" del /f /q "%PACKED_DIR%\%FINAL_EXE_NAME%" >nul 2>&1
if exist "%EVB_FILE%" del /f /q "%EVB_FILE%" >nul 2>&1

if not exist "%SOURCE_EXE_PATH%" (
    echo [ERROR] Source EXE not found.
    echo Please build Release in Qt Creator first.
    echo Expected: %SOURCE_EXE_PATH%
    if not defined NO_PAUSE pause
    exit /b 1
)

if not exist "%QT_BIN_DIR%\windeployqt.exe" (
    echo [ERROR] windeployqt.exe not found.
    echo Expected: %QT_BIN_DIR%\windeployqt.exe
    if not defined NO_PAUSE pause
    exit /b 1
)

if not exist "%ENIGMA_CONSOLE%" (
    echo [ERROR] Enigma console not found.
    echo Expected: %ENIGMA_CONSOLE%
    if not defined NO_PAUSE pause
    exit /b 1
)

if not exist "%EVB_GENERATOR%" (
    echo [ERROR] EVB generator script not found.
    echo Expected: %EVB_GENERATOR%
    if not defined NO_PAUSE pause
    exit /b 1
)

echo OK

echo [2/4] Deploying Qt dependencies...
copy /y "%SOURCE_EXE_PATH%" "%BUILD_EXE_DIR%\" >nul

set "PATH=%QT_BIN_DIR%;%PATH%"
pushd "%BUILD_EXE_DIR%"
"%QT_BIN_DIR%\windeployqt.exe" "%FINAL_EXE_NAME%" --release --no-translations >nul 2>&1
if errorlevel 1 (
    popd
    echo [ERROR] windeployqt failed.
    if not defined NO_PAUSE pause
    exit /b 1
)
popd

echo        - Patching MinGW runtime DLLs...
if exist "%QT_BIN_DIR%\libstdc++-6.dll" copy /y "%QT_BIN_DIR%\libstdc++-6.dll" "%BUILD_EXE_DIR%\" >nul
if exist "%QT_BIN_DIR%\libgcc_s_dw2-1.dll" copy /y "%QT_BIN_DIR%\libgcc_s_dw2-1.dll" "%BUILD_EXE_DIR%\" >nul
if exist "%QT_BIN_DIR%\libgcc_s_sjlj-1.dll" copy /y "%QT_BIN_DIR%\libgcc_s_sjlj-1.dll" "%BUILD_EXE_DIR%\" >nul
if exist "%QT_BIN_DIR%\libwinpthread-1.dll" copy /y "%QT_BIN_DIR%\libwinpthread-1.dll" "%BUILD_EXE_DIR%\" >nul

echo        - Generating Enigma config...
powershell -NoProfile -ExecutionPolicy Bypass -File "%EVB_GENERATOR%" ^
    -BuildExeDir "%BUILD_EXE_DIR%" ^
    -InputExe "%BUILD_EXE_DIR%\%FINAL_EXE_NAME%" ^
    -OutputExe "%PACKED_DIR%\%FINAL_EXE_NAME%" ^
    -EvbFile "%EVB_FILE%"

if errorlevel 1 (
    echo [ERROR] Failed to generate final_pack.evb.
    if not defined NO_PAUSE pause
    exit /b 1
)

echo OK

echo [3/4] Packing with Enigma...
"%ENIGMA_CONSOLE%" "%EVB_FILE%"
if errorlevel 1 (
    echo [ERROR] Enigma packing failed.
    if not defined NO_PAUSE pause
    exit /b 1
)

echo OK

echo [4/4] Verifying result...
echo.

set "OUTPUT_FILE=%PACKED_DIR%\%FINAL_EXE_NAME%"
if exist "%OUTPUT_FILE%" (
    for %%F in ("%OUTPUT_FILE%") do (
        set /a "sizeMB=%%~zF / 1048576"
        echo ------------------------------------------
        echo  SUCCESS! Packaged Successfully.
        echo ------------------------------------------
        echo  File: %%F
        echo  Size: !sizeMB! MB
        echo ------------------------------------------
        if not defined NO_OPEN explorer "%PACKED_DIR%"
    )
) else (
    echo [ERROR] Output file not found.
    echo Expected: %OUTPUT_FILE%
)

echo.
if not defined NO_PAUSE pause
