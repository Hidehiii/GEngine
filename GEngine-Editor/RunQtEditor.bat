@echo off
setlocal
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0RunQtEditor.ps1" %*
exit /b %ERRORLEVEL%
