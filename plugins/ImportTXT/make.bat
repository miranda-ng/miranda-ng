@echo off
if /i '%1' == 'fpc' (
  set OUTDIR="..\..\bin10\Release\Plugins"
  set FPCBIN=fpc.exe
) else if /i '%1' == 'fpc64' (
  set OUTDIR="..\..\bin10\Release64\Plugins"
  set FPCBIN=ppcrossx64.exe
)
set PROJECT=ImportTXT

if not exist %OUTDIR% mkdir %OUTDIR%
md tmp

rem brcc32 -foImpTxt_Ver.res ImpTxt_Ver.rc
rem brcc32 -foImpTxtDlg.res ImpTxtDlg.rc
rem brcc32 -foImpTxtWiz.res ImpTxtWiz.rc

%FPCBIN% @..\Utils.pas\fpc.cfg %PROJECT%.dpr %2 %3 %4 %5 %6 %7 %8 %9

move .\tmp\%PROJECT%.dll %OUTDIR%
del /Q tmp\*
rd tmp
