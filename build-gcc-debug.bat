@echo off
REM Phase 2: Build Markdown Viewer using GCC in Debug mode (x64).
setlocal
powershell -ExecutionPolicy Bypass -File "%~dp0scripts\build\build-windows.ps1" -Compiler gcc -Config Debug
set "rc=%errorlevel%"
endlocal & exit /b %rc%
