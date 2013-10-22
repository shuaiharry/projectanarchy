::@ECHO OFF
echo off

REM - Set root directory

set CURR_DIR=%~dp0
set ROOT_DIR=%~dp0
echo Files will be copied from:
echo %VISION_SDK%
echo to:
echo %CURR_DIR%

REM - Start copy (Base data)

set SRC_DIR=%VISION_SDK%\Data\Vision\Base
md Data\Vision\Base
xcopy "%SRC_DIR%" /S "%ROOT_DIR%\Data\Vision\Base" /Y

REM - Start copy (Icons for various platforms)

set SRC_DIR=%VISION_SDK%\Data\Common
md Data\Common
IF EXIST "%SRC_DIR%" xcopy "%SRC_DIR%" /S "%ROOT_DIR%\Data\Common" /Y 

xcopy "%VISION_SDK%\Source\Vision\Runtime\Common\VisSampleApp.cpp" /S "%ROOT_DIR%\Source\GameApplication" /Y
xcopy "%VISION_SDK%\Source\Vision\Runtime\Common\VisSampleAppCallbacks.cpp" /S "%ROOT_DIR%\Source\GameApplication" /Y
xcopy "%VISION_SDK%\Source\Vision\Runtime\Common\Tizen\VisTizenGLESApp.cpp" /S "%ROOT_DIR%\Source\GameApplication" /Y