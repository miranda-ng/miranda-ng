rem @echo off

pushd ..\plugins

pushd mRadio
call make.bat fpc 15
if errorlevel 1 goto :Error
popd

pushd QuickSearch
call make.bat fpc 15
if errorlevel 1 goto :Error
popd

popd
goto :eof

:Error
echo ============================= FAIL! =============================
pause
exit