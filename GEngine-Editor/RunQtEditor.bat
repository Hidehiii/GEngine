@echo off
setlocal
set "EDITOR_CONFIG=%~1"
if not defined EDITOR_CONFIG set "EDITOR_CONFIG=Release"
if /I not "%EDITOR_CONFIG%"=="Debug" if /I not "%EDITOR_CONFIG%"=="Release" (
    echo Usage: RunQtEditor.bat [Debug^|Release]
    exit /b 1
)
set "EDITOR_HOST=%~dp0..\dist\qt-editor\windows-x86_64\%EDITOR_CONFIG%\GEngine-EditorHost\GEngine-EditorHost.exe"
if not exist "%EDITOR_HOST%" set "EDITOR_HOST=%~dp0..\dist\premake-vs2022\windows-x86_64\%EDITOR_CONFIG%\GEngine-EditorHost\GEngine-EditorHost.exe"
if not exist "%EDITOR_HOST%" (
    echo Build the Qt editor prototype with BuildScripts\BuildQtEditor.ps1 first.
    exit /b 1
)
"%EDITOR_HOST%"
exit /b %ERRORLEVEL%
