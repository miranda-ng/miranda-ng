@echo off
set p1=%1
set p2=%2
if  "%p1%" == "" (echo "please specify target platform by adding 'fpc' or 'fpc64' parameter to command line!"&&pause&&goto :EOF)
if  "%p2%" == "" (echo "please specify target output directory by adding '15' for bin15 to command line!"&&pause&&goto :EOF)
if /i '%1' == 'fpc' (
  set OUTDIR="..\..\bin%2\Release\Plugins"
  set FPCBIN=fpc.exe
) else if /i '%1' == 'fpc64' (
  set OUTDIR="..\..\bin%2\Release64\Plugins"
  set FPCBIN=ppcrossx64.exe
)
set PROJECT=Actman

if not exist %OUTDIR% mkdir %OUTDIR%
md tmp

brcc32.exe options.rc
brcc32.exe hooks\hooks.rc
brcc32.exe tasks\tasks.rc
brcc32.exe ua\ua.rc
for /F %%i in ('dir /b *.rc') do brcc32.exe %%i
rc version.rc

%FPCBIN% @..\Utils.pas\fpc.cfg %PROJECT%.dpr %3 %4 %5 %6 %7 %8 %9
if errorlevel 1 exit /b 1

move .\tmp\%PROJECT%.dll %OUTDIR%
move .\tmp\%PROJECT%.map .
del /Q tmp\*
rd tmp
exit /b 0
