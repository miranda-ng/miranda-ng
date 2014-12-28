@echo off
cls

echo ========================
echo    Langpack refresher
echo      Author: Robyer
echo ========================
echo.

set err=0
if "%~1" == "" (
	echo ERROR: You must specify language parameter!
	set err=1
) else (
	set "language=%~1"
	
	if not exist "..\..\langpacks\%language%" (
		echo ERROR: This language doesn't exists!
		set err=1
		goto usage
	)
)

if not "%~3" == "" (
	echo ERROR: You must specify 1 or 2 parameters only.
	echo NOTE: If you have path with spaces inside, suround it with "".
	set err=1
	goto usage
)

if not "%~2" == "" (
	set "oldLangpackPath=%~2"
	
	if not exist "..\..\langpacks\%language%\%oldLangpackPath%" (
		echo ERROR: Old langpack file doesn't exists!
		set err=1
		goto usage
	)
)

:usage
if "%err%"=="1" (
	echo.
	echo ========================
	echo Usage: refresher.bat language ["old langpack path"]
	echo Note:  path to old langpack must be relative to "langpacks\<language>\" folder
	echo.
	echo Example 1: refresher.bat czech
	echo Example 2: refresher.bat czech "Deprecated\old langpack\langpack_czech2.txt"
	echo.
	echo If you want own script in langpack dir, look at "langpacks\czech\refresh.bat"
	echo ========================
	echo.
	
	pause
	exit /B 1
)

REM =========================

if not "%oldLangpackPath%" == "" (
	set phase=0
) else (
	set phase=1
)

echo Refreshing started...

:start

cd "..\..\tools\lpgen\"

mkdir "Plugins"
mkdir "Untranslated"
mkdir "Weather"

if "%phase%" == "0" (
	echo Loading strings from old langpack...
	cscript /nologo translate.js /out:".\" /untranslated:"Untranslated" /outfile:"Langpack_%language%.txt" /path:"..\..\langpacks\%language%\" /langpack:"..\..\langpacks\%language%\%oldLangpackPath%"
) else (
	REM load strings from recently created langpack (also to distribute strings between files) and create final langpack
	cscript /nologo translate.js /out:".\" /untranslated:"Untranslated" /release:"Langpack_%language%.txt" /sourcelang:"%language%"
)

REM if previous command failed, exit 
if %ERRORLEVEL% NEQ 0 (
	echo ERROR: Refreshing failed!
	pause
	exit /B 1
)

rmdir /S /Q "..\..\langpacks\%language%\Weather"
rmdir /S /Q "..\..\langpacks\%language%\Plugins"
rmdir /S /Q "..\..\langpacks\%language%\Untranslated"
del /Q "..\..\langpacks\%language%\=CORE=.txt"
del /Q "..\..\langpacks\%language%\Langpack_%language%.txt"

mkdir "..\..\langpacks\%language%\Weather"
mkdir "..\..\langpacks\%language%\Plugins"
mkdir "..\..\langpacks\%language%\Untranslated"
copy "Weather\" "..\..\langpacks\%language%\Weather\"
copy "Plugins\" "..\..\langpacks\%language%\Plugins\"
copy "Untranslated\" "..\..\langpacks\%language%\Untranslated\"
copy "=CORE=.txt" "..\..\langpacks\%language%\=CORE=.txt"
copy "Langpack_%language%.txt" "..\..\langpacks\%language%\Langpack_%language%.txt"

rmdir /S /Q "Weather"
rmdir /S /Q "Plugins"
rmdir /S /Q "Untranslated"
del /Q "=CORE=.txt"
del /Q "Langpack_%language%.txt"

REM cd "..\..\langpacks\tool\"
REM LangpackSuite.exe \q \p%language%

if not "%phase%" == "1" (
	echo Loading strings from new langpack...
	set "phase=1"
	goto start
)

:end
echo Refreshing finished!

REM =========================
