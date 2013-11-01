@echo off

::
:: WARNING: 이 스크립트는 소스코드 파일을 Git 으로 부터 카피해서 덮어쓰게 됩니다. 실행 전에 소스 파일 수정 본을 꼭 백업해주세요.
::

set GIT_BIN=C:\Projects\ProjectAnarchy\Tools\PortableGit-1.8.0-preview20121022\bin2
set PATH=%GIT_BIN%;%PATH%

set GIT_TEMP_FOLDER=%~dp0_git_temp
set PA_GIT_SERVER_HTTPS=https://github.com/shuaiharry/projectanarchy.git


:: Try to execute git to make sure the command exists and exit
:: early if it does not
git >nul
if %errorlevel% neq 1 goto GIT_ERROR

if exist ".git" (
	echo Pulling most recent changes...
	git pull
) else (
	echo Preparing to sync to repository...

	:: Remove the existing target folders if they exist
	echo Removing existing target folders...
	if exist %GIT_TEMP_FOLDER% rmdir /q /s %GIT_TEMP_FOLDER%

	echo Cloning Git repository into temporary folder...
	git clone --no-checkout %PA_GIT_SERVER_HTTPS% %GIT_TEMP_FOLDER%

	if exist %GIT_TEMP_FOLDER% (
		echo Moving Git repository to root of Anarchy SDK folder..
		cd %GIT_TEMP_FOLDER%
		attrib -H .git
		move .git ..
		cd ..

		git reset --hard HEAD
	) else ( goto GIT_ERROR )
	
	echo Cleaning up temporary folder...
	if exist %GIT_TEMP_FOLDER% rmdir /q /s %GIT_TEMP_FOLDER%
)

goto END

:GIT_ERROR

echo Git failure! Please make sure Git is installed and in your PATH.
echo See http://www.projectanarchy.com/ko/git for more details.
goto END

:END

pause