@echo off

REM ====== VARIABLES ========

set "language=polish"
set "oldLangpackPath=Deprecated\Langpack_polish.txt"
set "mirandaPath=C:\Program files\Miranda NG"

REM =========================

cd "..\..\tools\lpgen\"
call refresher.bat %language% "%oldLangpackPath%"

REM copy and reload langpack in your Miranda (uncoment next line + enable cmdline.dll plugin to use it)
REM call installer.bat %language% "%mirandaPath%"
