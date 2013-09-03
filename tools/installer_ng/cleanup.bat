@echo off
rem Cleanup
rd /S /Q tmp
rd /S /Q util
del /F /S /Q vcredist*.exe

pushd InnoNG_32\Files
rd /S /Q Core
rd /S /Q Icons
rd /S /Q Plugins
del /F /Q *.ini
del /F /Q *.txt
del /F /Q *.dll
del /F /Q *.exe
popd

pushd InnoNG_64\Files
rd /S /Q Core
rd /S /Q Icons
rd /S /Q Plugins
del /F /Q *.ini
del /F /Q *.txt
del /F /Q *.dll
del /F /Q *.exe
popd
rem end