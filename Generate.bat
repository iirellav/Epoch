@echo off
pushd %~dp0
cd ..
call vendor\premake5.exe vs2022
popd
PAUSE