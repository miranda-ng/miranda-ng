rem @echo off

pushd ..\plugins

pushd Actman
call make.bat fpc
popd

pushd ImportTXT
call make.bat
popd

pushd mRadio
call make.bat fpc
popd

pushd QuickSearch
call make.bat fpc
popd

pushd ShlExt
call make.bat
popd

pushd Watrack
call make.bat fpc
popd

popd