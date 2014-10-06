@echo off

REM ====== VARIABLES ========

set "language=spanish"
set "oldLangpackPath="
set "mirandaPath="

REM =========================

cd "..\..\tools\lpgen\"
call refresher.bat %language% "%oldLangpackPath%"

REM copy and reload langpack in your Miranda (uncoment next line + enable cmdline.dll plugin to use it)
REM call installer.bat %language% "%mirandaPath%"
