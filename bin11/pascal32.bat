rem @echo off

pushd ..\plugins

pushd Actman
call make.bat fpc 11
popd

pushd ImportTXT
call make.bat fpc 11
popd

pushd mRadio
call make.bat fpc 11
popd

pushd QuickSearch
call make.bat fpc 11
popd

pushd ShlExt
call make.bat fpc 11
popd

pushd Watrack
call make.bat fpc 11
cd icons
call makeicons.bat fpc 11
popd

pushd Dbx_mmap_SA\Cryptors\Athena
call make.bat fpc 11
popd

popd