@echo off
set myopts=-dMiranda
set dprname=actman.dpr

..\delphi\brcc32.exe %myopts% options.rc     -fooptions.res
..\delphi\brcc32.exe %myopts% hooks\hooks.rc -fohooks\hooks.res
..\delphi\brcc32.exe %myopts% tasks\tasks.rc -fotasks\tasks.res
..\delphi\brcc32.exe %myopts% ua\ua.rc       -foua\ua.res

if /i '%1' == 'fpc' (
  ..\FPC\bin\fpc.exe %myopts% %dprname% %2 %3 %4 %5 %6 %7 %8 %9
) else if /i '%1' == 'fpc64' (
  ..\FPC\bin64\ppcrossx64.exe %myopts% %dprname% %2 %3 %4 %5 %6 %7 %8 %9
) else if /i '%1' == 'xe2' (
  ..\XE2\BIN\dcc32.exe %myopts% %dprname% %2 %3 %4 %5 %6 %7 %8 %9
) else if /i '%1' == 'xe64' (
  ..\XE2\BIN\dcc64.exe %myopts% %dprname% %2 %3 %4 %5 %6 %7 %8 %9
) else (
  ..\delphi\dcc32 %myopts% %dprname% %1 %2 %3 %4 %5 %6 %7 %8 %9
)
