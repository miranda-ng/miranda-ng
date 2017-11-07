rem @echo off
set p1=%1
set p2=%2
if  "%p1%" == "" (echo "please specify target platform by adding 'fpc' or 'fpc64 parameter to command line!'"&&pause&&goto :EOF)
if  "%p2%" == "" (echo "please specify target output directory by adding 15 for bin15 to command line!'"&&pause&&goto :EOF)
if /i '%1' == 'fpc' (
  set OUTDIR="..\..\..\bin%2\Release\Icons"
  set FPCBIN=fpc.exe
) else if /i '%1' == 'fpc64' (
  set OUTDIR="..\..\..\bin%2\Release64\Icons"
  set FPCBIN=ppcrossx64.exe
)
if not exist %OUTDIR% mkdir %OUTDIR%

call make buttons
call make icons

move /Y Tasm\Watrack_*.dll %Outdir%