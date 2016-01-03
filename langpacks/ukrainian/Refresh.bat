@echo off

REM ====== VARIABLES ========

set "language=serbian"
set "mirandaPath=C:\Program files\Miranda NG"

REM =========================

cd "..\..\tools\lpgen\"
call refresher.bat %language%

REM copy and reload langpack in your Miranda (uncoment next line + enable cmdline.dll plugin to use it)
REM call installer.bat %language% "%mirandaPath%"
