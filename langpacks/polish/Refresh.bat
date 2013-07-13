@echo off

REM ====== VARIABLES ========

set "language=polish"
set "oldLangpackPath=Deprecated\Langpack_polish.txt"

REM =========================

cd "..\..\tools\lpgen\"
refresher.bat %language% "%oldLangpackPath%"

REM copy and reload langpack in your Miranda (uncoment next lines + enable cmdline.dll plugin to use it)
 REM echo Copy and refresh Miranda NG langpack
 REM set "mirandaPath=D:\Programy\Miranda NG"
 REM copy "..\..\langpacks\%language%\Langpack_%language%.txt" "%mirandaPath%\Langpack_%language%.txt"
 REM cd "%mirandaPath%"
 REM mimcmd.exe callservice LangPack/Reload 0 0
