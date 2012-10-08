@echo off
set myopts=

if /i '%2' == 'fpc' (
  ..\FPC\bin\fpc.exe %myopts% %1 %3 %4 %5 %6 %7 %8 %9
) else if /i '%2' == 'fpc64' (
  ..\FPC\bin64\ppcrossx64.exe %myopts% %1 %3 %4 %5 %6 %7 %8 %9
) else if /i '%2' == 'xe2' (
  ..\XE2\bin\dcc32.exe %myopts% %1 %3 %4 %5 %6 %7 %8 %9
) else if /i '%2' == 'xe64' (
  ..\XE2\bin\dcc64.exe %myopts% %1 %3 %4 %5 %6 %7 %8 %9
) else (
  ..\delphi\dcc32 %myopts% %1 %2 %3 %4 %5 %6 %7 %8 %9
)
