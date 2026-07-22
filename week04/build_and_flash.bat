@echo off
setlocal

set PROJECT_NAME=HW04_GPIO
set BUILD_DIR=build

echo ============================
echo   [1/3] Cau hinh CMake...
echo ============================
if not exist %BUILD_DIR% (
    cmake -S . -B %BUILD_DIR% -G "Ninja"
    if errorlevel 1 goto :error
)

echo ============================
echo   [2/3] Build project...
echo ============================
cmake --build %BUILD_DIR%
if errorlevel 1 goto :error

echo ============================
echo   [3/3] Nap code qua ST-Link...
echo ============================
openocd -f interface/stlink.cfg -f target/stm32f1x.cfg ^
    -c "program %BUILD_DIR%/%PROJECT_NAME%.elf verify reset exit"
if errorlevel 1 goto :error

echo.
echo ===== HOAN TAT: Build va nap code thanh cong =====
goto :end

:error
echo.
echo ===== LOI: Qua trinh build hoac nap code that bai =====
exit /b 1

:end
endlocal