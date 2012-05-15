@echo off
..\delphi\brcc32.exe athena.rc -foathena.res
if /i '%1' == 'fpc' (
  ..\FPC\bin\fpc.exe athena.dpr %2 %3 %4 %5 %6 %7 %8 %9
) else if /i '%1' == 'fpc64' (
  ..\FPC\bin64\ppcrossx64.exe athena.dpr %2 %3 %4 %5 %6 %7 %8 %9
) else if /i '%1' == 'xe2' (
  ..\XE2\BIN\dcc32.exe athena.dpr %2 %3 %4 %5 %6 %7 %8 %9
) else if /i '%1' == 'xe64' (
  ..\XE2\BIN\dcc64.exe athena.dpr %2 %3 %4 %5 %6 %7 %8 %9
) else (
  ..\delphi\dcc32 athena.dpr %1 %2 %3 %4 %5 %6 %7 %8 %9
)