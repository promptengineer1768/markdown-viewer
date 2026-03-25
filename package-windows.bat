@echo off
REM Package prebuilt Windows release into Inno Setup installer, ZIP, and MSI
REM This is a thin wrapper around scripts\build\package-windows.ps1

powershell -ExecutionPolicy Bypass -File "%~dp0scripts\build\package-windows.ps1" %*
