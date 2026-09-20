@echo off
setlocal
set "result=1"
set "action=%~1"
if defined action goto validate

choice /c 123 /n /m "Choose Visual Studio (1: VS2022, 2: VS2019, 3: VS2026): "
if errorlevel 255 exit /b 1
if errorlevel 3 (
    set "action=vs2026"
) else if errorlevel 2 (
    set "action=vs2019"
) else if errorlevel 1 (
    set "action=vs2022"
) else (
    exit /b 1
)

:validate
if /i "%action%"=="vs2022" goto generate
if /i "%action%"=="vs2019" goto generate
if /i "%action%"=="vs2026" goto generate
echo Usage: %~nx0 [vs2022^|vs2019^|vs2026]
exit /b 1

:generate
if not defined PREMAKE_EXE set "PREMAKE_EXE=%~dp0..\vender\bin\premake\premake5.exe"
if not exist "%PREMAKE_EXE%" (
    echo ERROR: Premake executable not found: "%PREMAKE_EXE%"
    goto finish
)
for %%P in ("%PREMAKE_EXE%") do set "PREMAKE_EXE=%%~fP"
pushd "%~dp0.."
if errorlevel 1 goto finish

if /i "%action%"=="vs2026" (
    "%PREMAKE_EXE%" --help | findstr /r /c:"^[ ]*vs2026[ ]" >nul
    if errorlevel 1 (
        echo ERROR: This Premake executable does not support vs2026.
        echo Set PREMAKE_EXE to a Premake executable whose --help lists vs2026.
        echo Clear PREMAKE_EXE to use the bundled Premake 5.0.0-beta8.
        goto cleanup
    )
)

"%PREMAKE_EXE%" %action%
set "result=%errorlevel%"

:cleanup
popd

:finish
if "%~1"=="" pause
endlocal & exit /b %result%
