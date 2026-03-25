@echo off
REM Phase 2: Build Markdown Viewer using clang-cl in Debug mode (x64).
setlocal
powershell -ExecutionPolicy Bypass -File "%~dp0scripts\build\build-windows.ps1" -Compiler clang -Config Debug
set "rc=%errorlevel%"
endlocal & exit /b %rc%
