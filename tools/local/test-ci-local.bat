@echo off
REM tools/local/test-ci-local.bat
REM Thin wrapper for test-ci-local.ps1
REM This is a local development tool only - NOT part of the official build system.

setlocal

set "SCRIPT_DIR=%~dp0"
set "PROJECT_ROOT=%SCRIPT_DIR%..\.."

cd /d "%PROJECT_ROOT%"

REM Forward all arguments to PowerShell script
powershell -ExecutionPolicy Bypass -File "%SCRIPT_DIR%test-ci-local.ps1" %*
