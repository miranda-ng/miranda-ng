rem @echo off
set OUTDIR="..\..\bin10\Release\Plugins"
if not exist %OUTDIR% mkdir %OUTDIR%
md tmp
set myopts=-O3 -Xs -Sd -dMiranda -FE.\tmp -Fi..\Utils.pas -Fi..\ExternalAPI\delphi -Fu..\Utils.pas -Fu..\..\include\delphi -Fu..\ExternalAPI\delphi -Fu..\Libs
set dprname=mradio.dpr

rem brcc32.exe mradio.rc -fomradio.res

if /i '%1' == 'fpc' (
  fpc.exe %myopts% %dprname% %2 %3 %4 %5 %6 %7 %8 %9
) else if /i '%1' == 'fpc64' (
  ppcrossx64.exe %myopts% %dprname% %2 %3 %4 %5 %6 %7 %8 %9
)

move /y tmp\mradio.dll %OUTDIR%\mRadio.dll
del /Q tmp\*
rd tmp