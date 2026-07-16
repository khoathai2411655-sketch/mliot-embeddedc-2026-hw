@echo off

echo STEP 1: CLEANING BUILD DIRECTORY
if exist build (
    echo Deleting existing build folder...
    rmdir /s /q build
)

echo.
echo STEP 2: CONFIGURING PROJECT WITH CMAKE
cmake -G Ninja -B build

echo.
echo STEP 3: COMPILING FIRMWARE WITH NINJA
ninja -C build

echo.
echo STEP 4: FLASHING FIRMWARE TO TARGET MCU
STM32_Programmer_CLI -c port=SWD -w build/app_firmware.bin 0x08000000 -rst

echo.
pause