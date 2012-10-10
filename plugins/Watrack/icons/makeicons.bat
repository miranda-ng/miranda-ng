rem @echo off
if /i '%1' == 'fpc' (
  set OUTDIR="..\..\..\bin10\Release\Icons"
  set FPCBIN=fpc.exe
) else if /i '%1' == 'fpc64' (
  set OUTDIR="..\..\..\bin10\Release64\Icons"
  set FPCBIN=ppcrossx64.exe
)
if not exist %OUTDIR% mkdir %OUTDIR%

call make buttons
call make icons

move /Y Tasm\Watrack_*.dll %Outdir%