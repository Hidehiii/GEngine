@echo off
setlocal
set "EDITOR_CONFIG=%~1"
if not defined EDITOR_CONFIG set "EDITOR_CONFIG=Debug"
if /I not "%EDITOR_CONFIG%"=="Debug" if /I not "%EDITOR_CONFIG%"=="Release" (
    echo Usage: RunEditor.bat [Debug^|Release]
    exit /b 1
)
pushd "%~dp0"
set "EDITOR_EXE=..\dist\cmake-vs2022\windows-x86_64\%EDITOR_CONFIG%\GEngine-Editor\GEngine-Editor.exe"
if not exist "%EDITOR_EXE%" set "EDITOR_EXE=..\dist\premake-vs2022\windows-x86_64\%EDITOR_CONFIG%\GEngine-Editor\GEngine-Editor.exe"
if not exist "%EDITOR_EXE%" (
    echo Build GEngine-Editor in %EDITOR_CONFIG% first.
    popd
    exit /b 1
)
"%EDITOR_EXE%"
set "EDITOR_RESULT=%ERRORLEVEL%"
popd
exit /b %EDITOR_RESULT%
