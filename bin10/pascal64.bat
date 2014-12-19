rem @echo off

pushd ..\plugins

pushd Actman
call make.bat fpc64 10
if errorlevel 1 goto :Error
popd

pushd mRadio
call make.bat fpc64 10
if errorlevel 1 goto :Error
popd

pushd QuickSearch
call make.bat fpc64 10
if errorlevel 1 goto :Error
popd

pushd Watrack
call make.bat fpc64 10
if errorlevel 1 goto :Error
cd icons
call makeicons.bat fpc64 10
if errorlevel 1 goto :Error
popd

popd
goto :eof

:Error
echo ============================= FAIL! =============================
pause
exit
