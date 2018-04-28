@echo off

REM ====== VARIABLES ========

set "language=bulgarian"
set "oldLangpackPath="
set "mirandaPath=C:\Program files\Miranda NG"

REM =========================

cd "..\..\tools\lpgen\"
call refresher.bat %language% "%oldLangpackPath%"

REM if previous command failed, exit
if %ERRORLEVEL% NEQ 0 exit /B 1

REM copy and reload langpack in your Miranda (uncomment next line + enable cmdline.dll plugin to use it)
REM call installer.bat %language% "%mirandaPath%"
