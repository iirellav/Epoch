pushd %~dp0
call $RCEDIT$ "Runtime.exe" --set-icon "$ICON_PATH$"
call $RCEDIT$ "Runtime.exe" --set-version-string "FileDescription" "$PRUDUCT_NAME$"
call ren "Runtime.exe" "$PRUDUCT_NAME$.exe"
( del /q /f "%~f0" >nul 2>&1 & exit /b 0  )
popd