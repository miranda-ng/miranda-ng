rem @echo off
if /i '%1' == 'fpc' (
  set OUTDIR="..\..\bin10\Release\Plugins"
) else if /i '%1' == 'fpc64' (
  set OUTDIR="..\..\bin10\Release64\Plugins"
)
if not exist %OUTDIR% mkdir %OUTDIR%
md tmp
set myopts=-O3 -Xs -Sd -dMiranda -FE.\tmp -FU.\tmp -FE%OUTDIR% -Fi..\Utils.pas -Fi..\ExternalAPI\delphi -Fu..\Utils.pas -Fu..\..\include\delphi -Fu..\ExternalAPI\delphi -Fu..\Libs
set dprname=mRadio.dpr

rem brcc32.exe mradio.rc -fomradio.res

if /i '%1' == 'fpc' (
  fpc.exe %myopts% %dprname% %2 %3 %4 %5 %6 %7 %8 %9
) else if /i '%1' == 'fpc64' (
  ppcrossx64.exe %myopts% %dprname% %2 %3 %4 %5 %6 %7 %8 %9
)

del /Q tmp\*
rd tmp