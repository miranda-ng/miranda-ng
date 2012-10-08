@echo off
set myopts=-dMiranda
set dprname=mradio.dpr

..\delphi\brcc32.exe mradio.rc -fomradio.res

if /i '%1' == 'fpc' (
  ..\FPC\bin\fpc.exe %myopts% %dprname% %2 %3 %4 %5 %6 %7 %8 %9
) else if /i '%1' == 'fpc64' (
  ..\FPC\bin64\ppcrossx64.exe %myopts% %dprname% %2 %3 %4 %5 %6 %7 %8 %9
) else if /i '%1' == 'xe2' (
  ..\XE2\BIN\dcc32.exe -b -dUNICODE_CTRLS -dKOL_MCK %myopts% %dprname% %2 %3 %4 %5 %6 %7 %8 %9
) else if /i '%1' == 'xe64' (
  ..\XE2\BIN\dcc64.exe -b -dUNICODE_CTRLS -dKOL_MCK %myopts% %dprname% %2 %3 %4 %5 %6 %7 %8 %9
) else (
  ..\delphi\dcc32 -b -dUNICODE_CTRLS -dKOL_MCK %myopts% %dprname% %1 %2 %3 %4 %5 %6 %7 %8 %9
)