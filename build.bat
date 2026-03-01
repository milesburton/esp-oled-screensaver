@echo off
REM Build script for Windows
REM Usage: build.bat [environment]
REM Example: build.bat esp8266_d1_mini

setlocal enabledelayedexpansion

set DEFAULT_ENV=esp8266_d1_mini
set TARGET_ENV=%1
if "!TARGET_ENV!"=="" set TARGET_ENV=!DEFAULT_ENV!

set BUILD_DIR=.pio\build\!TARGET_ENV!
set FIRMWARE_DIR=firmware
set TIMESTAMP=%date:~10,4%%date:~4,2%%date:~7,2%_%time:~0,2%%time:~3,2%%time:~6,2%
set TIMESTAMP=!TIMESTAMP: =0!

echo.
echo ================================================
echo ESP8266 Weather Clock - Build Script
echo ================================================
echo Environment: !TARGET_ENV!
echo Timestamp: !TIMESTAMP!
echo.

REM Check if PlatformIO is installed
pio --version >nul 2>&1
if errorlevel 1 (
    echo ERROR: PlatformIO not found. Install with: pip install platformio
    exit /b 1
)

REM Create firmware output directory
if not exist "!FIRMWARE_DIR!" mkdir "!FIRMWARE_DIR!"

echo Building firmware...
pio run -e !TARGET_ENV! -d .
if errorlevel 1 (
    echo Build failed!
    exit /b 1
)

echo.
echo Build successful!
echo.

REM Check for compiled firmware
if not exist "!BUILD_DIR!\firmware.bin" (
    echo ERROR: firmware.bin not found at !BUILD_DIR!\firmware.bin
    exit /b 1
)

REM Copy firmware
set FIRMWARE_NAME=esp8266-weather-clock_!TARGET_ENV!_!TIMESTAMP!.bin
set FIRMWARE_PATH=!FIRMWARE_DIR!\!FIRMWARE_NAME!

copy "!BUILD_DIR!\firmware.bin" "!FIRMWARE_PATH!" >nul
echo [SUCCESS] Firmware binary created:
echo   Path: !FIRMWARE_PATH!

REM Get file size
for %%A in ("!FIRMWARE_PATH!") do set SIZE=%%~zA
echo   Size: !SIZE! bytes
echo.

echo ================================================
echo Build Complete!
echo ================================================
echo.
echo Next steps:
echo 1. Upload via OTA: http://^<device-ip^>/update
echo 2. Or upload via serial: pio run -e !TARGET_ENV! -t upload
echo.

endlocal
