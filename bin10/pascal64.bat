rem @echo off

pushd ..\plugins

pushd Actman
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
cd icons
call makeicons.bat fpc64
popd

pushd Dbx_mmap_SA\Cryptors\Athena
call make.bat fpc64
popd

popd

pushd ..\protocols

pushd mRadio
call make.bat fpc64
popd

popd