@echo off

REM ====== VARIABLES ========

set "language=czech"
set "oldLangpackPath=Deprecated\old langpack\langpack_czech2.txt"
set "mirandaPath=D:\Programy\Miranda NG"

REM =========================

cd "..\..\tools\lpgen\"
call refresher.bat %language% "%oldLangpackPath%"

REM if previous command failed, exit
if %ERRORLEVEL% NEQ 0 exit /B 1

REM copy and reload langpack in your Miranda (uncoment next line + enable cmdline.dll plugin to use it)
call installer.bat %language% "%mirandaPath%"
