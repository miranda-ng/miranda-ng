@echo off
cls

echo ========================
echo    Langpack installer
echo      Author: Robyer
echo ========================
echo.

set err=0
if "%~1" == "" (
	echo ERROR: You must specify language parameter!
	set err=1
)

if "%~2" == "" (
	echo ERROR: You must specify Miranda path!
	set err=1
)

if not "%~3" == "" (
	echo ERROR: You must specify 1 or 2 parameters only.
	echo NOTE: If you have path with spaces inside, suround it with "".
	set err=1
)

if "%err%"=="1" (
	goto usage
)

set "language=%~1"
set "mirandaPath=%~2"

if not exist "..\..\langpacks\%language%" (
	echo ERROR: This language doesn't exists!
	goto exit
)

if not exist "%mirandaPath%\mimcmd.exe" (
	echo ERROR: mimcmd.exe wasn't found in your Miranda path. Make sure you have installed Command line plugin.
	goto exit
)

if not exist "..\..\langpacks\%language%\Langpack_%language%.txt" (
	echo ERROR: Langpack_%language%.txt in your langpack folder wasn't found. Make sure you have freshly refreshed langpack using Refresher.bat script
	goto exit
)

echo Installing langpack...

copy "..\..\langpacks\%language%\Langpack_%language%.txt" "%mirandaPath%\Languages\Langpack_%language%.txt"
cd "%mirandaPath%"
mimcmd.exe callservice LangPack/Reload 0 0

echo Installation successful.
exit /b

:usage
echo.
echo ========================
echo Usage: installer.bat language "Miranda folder path"
echo Note:  for proper installation make sure you have enabled Command line plugin
echo.
echo Example: installer.bat czech "C:\Program files\Miranda NG"
echo.
echo If you want own script in langpack dir, look at "langpacks\czech\refresh.bat"
echo ========================
echo.

:exit
pause
exit /b
