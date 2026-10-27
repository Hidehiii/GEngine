@echo off
setlocal
set "tool="
for %%F in (
    "%~dp0..\dist\cmake-vs2022\windows-x86_64\Release\GEngineTools\GEngineTools.exe"
    "%~dp0..\dist\cmake-vs2022\windows-x86_64\Debug\GEngineTools\GEngineTools.exe"
    "%~dp0..\dist\premake-vs2022\windows-x86_64\Release\GEngineTools\GEngineTools.exe"
    "%~dp0..\dist\premake-vs2022\windows-x86_64\Debug\GEngineTools\GEngineTools.exe"
) do (
    if not defined tool if exist "%%~F" set "tool=%%~F"
)
if not defined tool (
    echo ERROR: GEngineTools.exe was not found. Build it with the CMake or Premake generator first.
    exit /b 1
)
"%tool%" run-qt-editor %*
exit /b %ERRORLEVEL%
