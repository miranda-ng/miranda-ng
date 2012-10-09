rem @echo off

pushd ..\plugins

pushd Actman
call make.bat fpc64
popd

pushd mRadio
call make.bat fpc64
popd

pushd QuickSearch
call make.bat fpc64
popd

pushd ShlExt
call make.bat fpc64
popd

pushd Watrack
call make.bat fpc64
popd

popd