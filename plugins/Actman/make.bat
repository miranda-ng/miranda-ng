@echo off
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

rem brcc32.exe %myopts% options.rc     -fooptions.res
rem brcc32.exe %myopts% hooks\hooks.rc -fohooks\hooks.res
rem brcc32.exe %myopts% tasks\tasks.rc -fotasks\tasks.res
rem brcc32.exe %myopts% ua\ua.rc       -foua\ua.res

%FPCBIN% @..\Utils.pas\fpc.cfg %PROJECT%.dpr %3 %4 %5 %6 %7 %8 %9

move .\tmp\%PROJECT%.dll %OUTDIR%
del /Q tmp\*
rd tmp
