@echo off
pushd %~dp0
call %EPOCH_DIR%\vendor\bin\premake5.exe vs2022
popd
