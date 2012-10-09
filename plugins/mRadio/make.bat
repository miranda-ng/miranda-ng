rem @echo off
set OUTDIR="..\..\bin10\Release\Plugins" 
if not exist %OUTDIR% mkdir %OUTDIR%
md tmp
set myopts=-O3 -Xs -Sd -dMiranda -FEtmp -Fi..\Utils.pas -Fu..\Utils.pas -Fu..\..\include\delphi -Fu..\ExternalAPI\delphi -Fu..\Libs
set dprname=mradio.dpr

rem brcc32.exe mradio.rc -fomradio.res

if /i '%1' == 'fpc' (
  fpc.exe %myopts% %dprname% %2 %3 %4 %5 %6 %7 %8 %9
) else if /i '%1' == 'fpc64' (
  ppcrossx64.exe %myopts% %dprname% %2 %3 %4 %5 %6 %7 %8 %9
)

move tmp\mradio.dll .
del /Q tmp\*
rd tmp
move /y mradio.dll ..\..\bin10\Release\Plugins\mRadio.dll