@echo off

if "%1" == "64" (set fpcpl=64) else (set fpcpl=)

pushd ..\plugins

pushd Actman
call make.bat fpc%fpcpl% 15
if errorlevel 1 goto :Error
popd

pushd mRadio
call make.bat fpc%fpcpl% 15
if errorlevel 1 goto :Error
popd

pushd Watrack
call make.bat fpc%fpcpl% 15
if errorlevel 1 goto :Error
cd icons
call makeicons.bat fpc%fpcpl% 15
if errorlevel 1 goto :Error
popd

pushd HistoryPlusPlus
call make.bat %1
if errorlevel 1 goto :Error
popd

popd
goto :eof

:Error
echo ============================= FAIL! =============================
pause
exit
