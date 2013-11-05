@echo off
echo Switch to "%VISION_SDK%\%1" as the default binary directory for vForge and vSceneViewer
echo.
setx.exe "VISION_SDK_BIN" "%%VISION_SDK%%\%1"
IF %ERRORLEVEL% NEQ 0 GOTO failed
echo.
pause
exit

:failed
echo.
echo Please try again using "Run as administrator"
echo (use left click on the shortcut)
echo.
pause