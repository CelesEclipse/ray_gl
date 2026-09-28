@echo off
:: =========================================================================
:: AUTO-ELEVATION: Force script to run as Administrator to allow icacls
:: =========================================================================
net session >nul 2>&1
if %errorLevel% == 0 (
    goto :START
) else (
    echo Requesting administrative privileges...
    echo Set UAC = CreateObject^("Shell.Application"^) > "%temp%\getadmin.vbs"
    echo UAC.ShellExecute "%~s0", "", "", "runas", 1 >> "%temp%\getadmin.vbs"
    "%temp%\getadmin.vbs"
    del "%temp%\getadmin.vbs"
    exit /B
)

:START
:: Set correct execution context directory to where this file lives
cd /d "%~dp0"

set "DEVKIT_PATH=C:\w64devkit\bin"
set "PATH=%DEVKIT_PATH%;%PATH%"

echo =========================================
echo FIX permissions on current directory ...
echo =========================================
:: Automatically grant Read/Execute/Write to everyone recursively 
:: to bypass Windows 10 re-installation "Access is denied" locks
icacls . /grant "Everyone:(OI)(CI)F" /T /C >nul 2>&1

echo =========================================
echo 1. Remove old build directory ...
echo =========================================
if exist build (
    rmdir /s /q build
)
mkdir build

echo =========================================
echo 2. Run CMake ...
echo =========================================
cd build
"%DEVKIT_PATH%\cmake.exe" .. -G "MinGW Makefiles"
if %errorlevel% neq 0 goto ERROR

echo =========================================
echo 3. Make ...
echo =========================================
"%DEVKIT_PATH%\make.exe"
if %errorlevel% neq 0 goto ERROR

echo =========================================
echo SUCCESSFULLY COMPILED! Run file exe...
echo =========================================
for %%i in (*.exe) do (
    echo Running: %%i
    %%i
    goto END
)

:ERROR
echo -----------------------------------------
echo [ERR] There's errors in build progress!
echo -----------------------------------------

:END
pause
