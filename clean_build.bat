@echo off
set "DEVKIT_PATH=C:\raylib\w64devkit\bin"
set "PATH=%DEVKIT_PATH%;%PATH%"

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
