@echo off
REM tools/local/setup-act.bat
REM Thin wrapper - installs 'act' for local GitHub Actions simulation.
REM This is a local development tool only - NOT part of the official build system.
REM Requires Docker Desktop to be installed separately.

powershell -ExecutionPolicy Bypass -File "%~dp0setup-act.ps1"
