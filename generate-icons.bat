@echo off
REM Generate icons from master PNG using ImageMagick
REM This script must be run before building the installer

setlocal enabledelayedexpansion

set SOURCE_PNG=%~dp0resources\icon.png
set OUTPUT_ICO=%~dp0resources\icon.ico

REM Find ImageMagick
set MAGICK_EXE=
if exist "%ProgramFiles%\ImageMagick-7.1.2-Q16\magick.exe" (
    set MAGICK_EXE=%ProgramFiles%\ImageMagick-7.1.2-Q16\magick.exe
) else if exist "%ProgramFiles%\ImageMagick-7.1.2-Q16-HDRI\magick.exe" (
    set MAGICK_EXE=%ProgramFiles%\ImageMagick-7.1.2-Q16-HDRI\magick.exe
) else if exist "%ProgramFiles%\ImageMagick-7.1.1-Q16\magick.exe" (
    set MAGICK_EXE=%ProgramFiles%\ImageMagick-7.1.1-Q16\magick.exe
) else (
    where magick >nul 2>&1
    if !errorlevel! equ 0 (
        for /f "tokens=*" %%i in ('where magick') do set MAGICK_EXE=%%i
    )
)

if "%MAGICK_EXE%"=="" (
    echo Error: ImageMagick not found
    echo Please install ImageMagick or run bootstrap-windows.ps1
    exit /b 1
)

echo Using ImageMagick at: %MAGICK_EXE%

REM Check if source PNG exists
if not exist "%SOURCE_PNG%" (
    echo Error: Source icon not found at %SOURCE_PNG%
    exit /b 1
)

REM Check if ICO needs to be regenerated
set REGENERATE=0
if not exist "%OUTPUT_ICO%" (
    set REGENERATE=1
) else (
    REM Check if PNG is newer than ICO
    for %%F in ("%SOURCE_PNG%") do set PNG_TIME=%%~tF
    for %%F in ("%OUTPUT_ICO%") do set ICO_TIME=%%~tF
    if "!PNG_TIME!" gtr "!ICO_TIME!" set REGENERATE=1
)

if %REGENERATE% equ 1 (
    echo Generating icon from %SOURCE_PNG%
    "%MAGICK_EXE%" "%SOURCE_PNG%" -background none -define icon:auto-resize=256,128,96,64,48,32,16 "%OUTPUT_ICO%"
    if !errorlevel! neq 0 (
        echo Error: Failed to generate icon
        exit /b 1
    )
    echo Generated: %OUTPUT_ICO%
) else (
    echo Icon already up to date: %OUTPUT_ICO%
)

echo Icon generation complete.
exit /b 0
