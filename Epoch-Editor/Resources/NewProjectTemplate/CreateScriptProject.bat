@echo off
pushd %~dp0
call vendor\premake5.exe vs2022
popd
