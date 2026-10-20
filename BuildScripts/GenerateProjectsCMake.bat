@echo off
setlocal
set "action=%~1"
if not defined action set "action=vs2022"
if /i "%action%"=="vs2022" (
    set "generator=Visual Studio 17 2022"
) else if /i "%action%"=="vs2019" (
    set "generator=Visual Studio 16 2019"
) else (
    echo Usage: %~nx0 [vs2022^|vs2019]
    exit /b 1
)
if not defined CMAKE_EXE set "CMAKE_EXE=cmake"
"%CMAKE_EXE%" -S "%~dp0CMake" -B "%~dp0..\build\cmake-%action%\windows-x86_64" -G "%generator%" -A x64 -DGE_TOOL_ID=cmake-%action% -DGE_PLATFORM_ID=windows-x86_64
endlocal & exit /b %errorlevel%
