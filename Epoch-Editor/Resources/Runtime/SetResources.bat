pushd %~dp0
$ICO_CONVERT_CMD$
call $RCEDIT$ "Runtime.exe" --set-icon "$ICON_PATH$"
call $RCEDIT$ "Runtime.exe" --set-version-string "FileDescription" "$PRUDUCT_NAME$"
call ren "Runtime.exe" "$PRUDUCT_NAME$.exe"
$DELETE_ICO_CMD$
( del /q /f "%~f0" >nul 2>&1 & exit /b 0  )
popd