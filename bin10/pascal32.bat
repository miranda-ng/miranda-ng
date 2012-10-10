rem @echo off

pushd ..\plugins

pushd Actman
call make.bat fpc
popd

pushd ImportTXT
call make.bat fpc
popd

pushd QuickSearch
call make.bat fpc
popd

pushd ShlExt
call make.bat fpc
popd

pushd Watrack
call make.bat fpc
popd

pushd Dbx_mmap_SA\Cryptors\Athena
call make.bat fpc
popd

popd

pushd ..\protocols

pushd mRadio
call make.bat fpc
popd

popd