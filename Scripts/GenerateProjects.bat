pushd  ..\
@echo off
:loop
set /p a=Choose a version of visual studio to generate project.(1:vs2022, 2:vs2019):
if /i '%a%'=='1' goto vs22
if /i '%a%'=='2' goto vs19
echo Invalidable input, please retry: &&goto loop
:vs22
call  vender\bin\premake\premake5.exe vs2022
popd
PAUSE
@exit
:vs19
call vender\bin\premake\premake5.exe vs2019
popd
PAUSE
@exit